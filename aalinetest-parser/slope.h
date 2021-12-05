#pragma once

#include <cstdint>
#include <utility>

/// <summary>
/// Computes 3D rasterization slopes based on Nintendo DS's hardware interpolation.
/// </summary>
/// <remarks>
/// The algorithm implemented by this class produces pixel-perfect slopes matching the Nintendo DS's 3D interpolator.
///
/// The hardware uses 32-bit integers with 18-bit fractional parts throughout the interpolation process, with one
/// notable exception in X-major slopes.
///
/// To calculate the X increment per scanline (DX), the hardware first computes the reciprocal of Y1-Y0 then multiplies
/// the result by X1-X0. This order of operations avoids a multiplication overflow at the cost of precision on the
/// division.
///
/// For X-major lines, the interpolator produces line spans for each scanline. The start of the span is calculated by
/// first offseting the Y coordinate to the Y0-Y1 range (subtracting Y0 from Y), then multiplying the offset Y by DX,
/// adding the X0 offset and finally a +0.5 bias. The end on the span is computed based on its starting coordinate,
/// discarding (masking out) the 9 least significant bits (which could be seen as rounding down, or the floor function),
/// then adding DX and subtracting 1.0. The exact algorithm is unknown, but it is possible that the starting coordinate
/// is shifted right by 9 for an intermediate calculation then shifted back left by 9 to restore the fractional part.
///
/// The formulae for determining the starting and ending X coordinates of a span of an X-major slope are:
///
///    DX = 1 / (Y1 - Y0) * (X1 - X0)
///    Xstart = (Y - Y0) * DX + X0 + 0.5
///    Xend = Xstart[discarding 9 LSBs] + DX - 1.0
///
/// Due to the 9 LSBs being discarded, certain X-major slopes (such as 69x49, 70x66, 71x49 and more) display a one-pixel
/// gap on hardware. This is calculated accurately with the formulae above.
///
/// Y-major slopes contain only one pixel per scanline. The formula for interpolating the X coordinate based on the Y
/// coordinate is very similar to that of the X-major interpolation, with the only difference being that the +0.5 bias
/// is not applied.
///
///    X = (Y - Y0) * DX + X0
///
/// Note that there is no need to compute a span as there's always going to be only one pixel per scanline. Also, there
/// are no one-pixel gaps on Y-major lines since the Nintendo DS's rasterizer is scanline-based and the interpolation is
/// computed on every scanline.
///
/// Negative slopes work in a similar fashion. In fact, negative slopes perfectly match their positive counterparts down
/// to the one-pixel gaps which happen in exactly the same spots. The gaps in negative slopes are to the left of a span,
/// while in positive slopes the gaps are to the right of a span, as shown below (rows 35 to 39 of 69x49 slopes):
///
///    Positive slope        Negative slope
///      ##  +---- mind the gap ----+  ##
///        # |                      | #
///         #V                      V#
///           #                    #
///            #                  #
///
/// The behavior for negative slopes is implemented in this class in the following manner, compared to positive slopes:
/// - The raw value of X0 coordinate used to compute the starting X coordinate of a span is decremented by 1, that is,
///   the value is subtracted an amount equal to 1.0 / 2^fractionalBits
/// - X0 and X1 are swapped; as a consequence, DX remains positive
/// - The starting X coordinate is the span's rightmost pixel; conversely, the ending X coordinate is its leftmost pixel
/// - The starting X coordinate is decremented by the computed Y*DX displacement instead of incremented
/// - The nine least significant bits of the ending X coordinate are rounded up the to the largest number less than 1.0
///   (511 as a raw integer with 9 fractional bits)
///
/// All other operations are otherwise identical.
/// </remarks>
class Slope {
    using u32 = uint32_t;
    using i32 = int32_t;

public:
    /// <summary>
    /// The number of fractional bits (aka resolution) of the interpolator.
    /// </summary>
    /// <remarks>
    /// The Nintendo DS uses 18 fractional bits for interpolation.
    /// </remarks>
    static constexpr u32 kFracBits = 18;

    /// <summary>
    /// The value 1.0 with fractional bits.
    /// </summary>
    static constexpr u32 kOne = (1 << kFracBits);

    /// <summary>
    /// The bias applied to the interpolation of X-major spans.
    /// </summary>
    static constexpr u32 kBias = (kOne >> 1);

    /// <summary>
    /// The mask applied during interpolation of X-major spans, removing half of the least significant fractional bits
    /// (rounded down).
    /// </summary>
    static constexpr u32 kMask = (~0u << (kFracBits / 2));

    /// <summary>
    /// The antialiasing coverage value range.
    /// </summary>
    static constexpr u32 kAARange = 32;

    /// <summary>
    /// The number of bits reserved for fractional antialiasing coverage calculations for X-major slopes.
    /// </summary>
    static constexpr u32 kAAFracBitsX = 5;

    /// <summary>
    /// The number of bits reserved for fractional antialiasing coverage calculations for Y-major slopes.
    /// </summary>
    static constexpr u32 kAAFracBitsY = 8;

    /// <summary>
    /// The base antialiasing coverage value for X-major slopes.
    /// </summary>
    static constexpr u32 kAABaseX = kAARange << kAAFracBitsX;

    /// <summary>
    /// The base antialiasing coverage value for Y-major slopes.
    /// </summary>
    static constexpr u32 kAABaseY = kAARange << kAAFracBitsY;

    /// <summary>
    /// Configures the slope to interpolate the line (X0,X1)-(Y0,Y1) using screen coordinates.
    /// </summary>
    /// <param name="x0">First X coordinate</param>
    /// <param name="y0">First Y coordinate</param>
    /// <param name="x1">Second X coordinate</param>
    /// <param name="y1">Second Y coordinate</param>
    constexpr void Setup(i32 x0, i32 y0, i32 x1, i32 y1) {
        // Always interpolate top to bottom
        if (y1 < y0) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        // Store reference coordinates and size
        m_x0 = x0 << kFracBits;
        m_y0 = y0;
        m_width = x1 - x0;
        m_height = y1 - y0;

        // Determine if this is a negative slope and adjust accordingly
        m_negative = (x1 < x0);
        if (m_negative) {
            m_x0--;
            m_width = -m_width;
            std::swap(x0, x1);
        }

        // Compute coordinate deltas and determine if the slope is X-major
        const i32 dx = (x1 - x0);
        const i32 dy = (y1 - y0);
        m_xMajor = (dx > dy);
        m_diagonal = (dx == dy);

        // Precompute bias for X-major or diagonal slopes
        if (m_xMajor || m_diagonal) {
            if (m_negative) {
                m_x0 -= kBias;
            } else {
                m_x0 += kBias;
            }
        }

        // Compute X displacement per scanline
        m_dx = dx;
        if (dy != 0) {
            m_dx *= kOne / dy; // This ensures the division is performed before the multiplication
        } else {
            m_dx *= kOne;
        }

        // Compute antialiasing bias and step per pixel
        // TODO: not quite correct yet
        // There might be vertical AA coverage step in addition to horizontal or the step changes depending on whether
        // it's an x-major or y-major slope
        if (dx == 0 || dy == 0) {
            // Perfectly horizontal or vertical slopes have full coverage
            m_aaStep = 0;
            m_aaBias = (m_xMajor) ? (kAABaseX - 1) : 0 /*(kAABaseY - 1)*/;
        } else if (dx == dy) {
            // Perfect diagonals have half coverage
            m_aaStep = 0;
            m_aaBias = kAABaseX / 2;
        } else if (m_xMajor) {
            m_aaStep = kAABaseX * dy / dx;
            m_aaBias = m_aaStep / 2;
        } else {
            m_aaStep = kAABaseY * dx / dy;
            m_aaBias = m_aaStep / 2;
        }
    }

    /// <summary>
    /// Computes the starting position of the span at the specified Y coordinate, including the fractional part.
    /// </summary>
    /// <param name="y">The Y coordinate, which must be between Y0 and Y1 specified in Setup.</param>
    /// <returns>The starting X coordinate of the specified scanline's span</returns>
    constexpr i32 FracXStart(i32 y) const {
        i32 displacement = (y - m_y0) * m_dx;
        if (m_negative) {
            return m_x0 - displacement;
        } else {
            return m_x0 + displacement;
        }
    }

    /// <summary>
    /// Computes the ending position of the span at the specified Y coordinate, including the fractional part.
    /// </summary>
    /// <param name="y">The Y coordinate, which must be between Y0 and Y1 specified in Setup.</param>
    /// <returns>The ending X coordinate of the specified scanline's span</returns>
    constexpr i32 FracXEnd(i32 y) const {
        i32 result = FracXStart(y);
        if (m_xMajor) {
            if (m_negative) {
                // The bit manipulation sequence (~mask - (x & ~mask)) acts like a ceiling function.
                // Since we're working in the opposite direction here, the "floor" is actually the ceiling.
                result = result + (~kMask - (result & ~kMask)) - m_dx + kOne;
            } else {
                result = (result & kMask) + m_dx - kOne;
            }
        }
        return result;
    }

    /// <summary>
    /// Computes the starting position of the span at the specified Y coordinate as a screen coordinate (dropping the
    /// fractional part).
    /// </summary>
    /// <param name="y">The Y coordinate, which must be between Y0 and Y1 specified in Setup.</param>
    /// <returns>The starting X screen coordinate of the scanline's span</returns>
    constexpr i32 XStart(i32 y) const {
        return FracXStart(y) >> kFracBits;
    }

    /// <summary>
    /// Computes the ending position of the span at the specified Y coordinate as a screen coordinate (dropping the
    /// fractional part).
    /// </summary>
    /// <param name="y">The Y coordinate, which must be between Y0 and Y1 specified in Setup.</param>
    /// <returns>The ending X screen coordinate of the scanline's span</returns>
    constexpr i32 XEnd(i32 y) const {
        return FracXEnd(y) >> kFracBits;
    }

    /// <summary>
    /// Retrieves the X coordinate increment per scanline.
    /// </summary>
    /// <returns>The X displacement per scanline (DX)</returns>
    constexpr i32 DX() const {
        return m_dx;
    }

    /// <summary>
    /// Determines if the slope is X-major.
    /// </summary>
    /// <returns>true if the slope is X-major.</returns>
    constexpr bool IsXMajor() const {
        return m_xMajor;
    }

    /// <summary>
    /// Determines if the slope is diagonal.
    /// </summary>
    /// <returns>true if the slope is diagonal.</returns>
    constexpr bool IsDiagonal() const {
        return m_diagonal;
    }

    /// <summary>
    /// Determines if the slope is negative (i.e. X decreases as Y increases).
    /// </summary>
    /// <returns>true if the slope is negative.</returns>
    constexpr bool IsNegative() const {
        return m_negative;
    }

    /// <summary>
    /// Returns the width of the slope (X1 - X0).
    /// </summary>
    /// <returns>The width of the slope</returns>
    constexpr i32 Width() const {
        return m_width;
    }

    /// <summary>
    /// Returns the height of the slope (Y1 - Y0).
    /// </summary>
    /// <returns>The height of the slope</returns>
    constexpr i32 Height() const {
        return m_height;
    }

    /// <summary>
    /// Retrieves the antialiasing coverage initial bias.
    /// </summary>
    /// <returns>The initial value of the antialiasing coverage</returns>
    constexpr u32 AABias() const {
        return m_aaBias;
    }

    /// <summary>
    /// Retrieves the antialiasign coverage step.
    /// </summary>
    /// <returns>The amount to increment the antialiasing coverage value for each pixel</returns>
    constexpr u32 AAStep() const {
        return m_aaStep;
    }

    /// <summary>
    /// Computes the antialiasing coverage at the specified coordinates.
    /// </summary>
    /// <param name="x">The X coordinate</param>
    /// <param name="y">The Y coordinate</param>
    /// <returns>The antialiasing coverage value at (X,Y)</returns>
    constexpr i32 AACoverage(i32 x, i32 y) const {
        if (m_diagonal) {
            return kAARange / 2;
        } else if (m_xMajor) {
            return FracAACoverage(x, y) >> kAAFracBitsX;
        } else {
            return FracAACoverage(x, y) >> kAAFracBitsY;
        }
    }

    /// <summary>
    /// Computes the antialiasing coverage at the specified coordinates, including the fractional part.
    /// </summary>
    /// <param name="x">The X coordinate</param>
    /// <param name="y">The Y coordinate</param>
    /// <returns>The antialiasing coverage value at (X,Y)</returns>
    constexpr i32 FracAACoverage(i32 x, i32 y) const {
        // TODO: right/bottom edges are still off by a lot
        // TODO: left/top edges are off by a small margin
        // TODO: Y-major slopes

        // Antialiasing notes:
        // - Perfectly horizontal or vertical edges (DX or DY == 0) are drawn in full alpha
        // - Perfectly diagonal edges (DX == DY) are drawn in half alpha
        // - AA coverage calculation changes depending on the slope's parameters:
        //   - Left or right edge
        //   - Positive or negative slope
        //   - X-major or Y-major slope
        // - Some edges are drawn with lots of zero or full alpha pixels (more noticeable the closer DX and DY are):
        //   - Left positive Y-major   (zero alpha)
        //   - Right negative Y-major  (zero alpha)
        //   - Left negative Y-major   (full alpha)
        //   - Right positive Y-major  (full alpha)
        //   - The pixels seem to be the last pixel of any given vertical span which is the majority of spans in the
        //     case of near-diagonal edges, but definitely visible on larger spans
        // - The following edges produce a positive gradient (AA coverage increases as X or Y increases):
        //   - Left X-major (both positive and negative)
        //   - Left negative Y-major
        //   - Right positive Y-major

        // const i32 xOffset =
        //    (m_negative ? FracXStart(y) - (x << kFracBits) - kBias : (x << kFracBits) + kBias - FracXStart(y)) >>
        //    kFracBits;
        const i32 xOffset = m_negative ? (m_x0 >> kFracBits) - x : x - (m_x0 >> kFracBits);
        const i32 yOffset = m_negative ? m_y0 - y : y - m_y0;
        if (m_diagonal) {
            // Diagonals are considered not X-major, so use the Y fractional bits
            return (kAARange / 2) << kAAFracBitsY;
        } else if (m_xMajor) {
            /*const u32 fracCoverage = xOffset * m_aaStep;
            const u32 finalCoverage =
                m_negative ? (fracCoverage - m_aaBias - 1) % kAABaseX : (fracCoverage + m_aaBias) % kAABaseX;
            return finalCoverage;*/

            /*constexpr u32 kDropBits = 9;
            const u32 start = (m_negative ? FracXEnd(y) : FracXStart(y)) >> kDropBits;
            const u32 offset = m_negative ? start - (x << (kFracBits - kDropBits))
                                          : (x << (kFracBits - kDropBits)) + (kBias >> kDropBits) - start;
            const u32 fracCoverage = (offset * m_aaStep) >> (kFracBits - kDropBits);
            const u32 finalCoverage =
                m_negative ? (fracCoverage - m_aaBias - 1) % kAABaseX : (fracCoverage + m_aaBias) % kAABaseX;
            return finalCoverage;*/

            // TODO: fix the calculation
            // Theory 0: the formula does not use the existing X coordinate; instead, it calculates its own offsets
            // Theory 1: the X-major formula depends on the Y coordinate
            // - It might be recomputing offsets on every scanline
            // Theory 2: the formula depends on how long the span is in a given scanline
            // - VERY obvious on long near-diagonals like 186x185
            // - For 186x185, doing a perfect floating-point interpolation produces perfect values for every scanline
            //   that has a span of a single pixel, but breaks precisely on the only scanline that has two pixels
            //   (106..107x106)
            // - For 236x185, the same perfect floating-point interpolation also works for every single-pixel scanline,
            //   but sometimes breaks on scanlines that have two pixels
            // - The formula seems to be wildly inaccurate/incorrect as it occasionally produces weird discontinuities
            //   on scanlines with two or more pixels
            // - Still not sure how to produce the artifact in 186x185
            //   - The line has two perfect diagonal segments: 0x0 to 106x106, and 107x106 to 185x184
            //   - The calculation produces a gradient that's too short for the first segment, which causes values to
            //     wrap around to 31 before the segment ends (starting at 93x93)
            //   - The gradient goes ..., 30, 30, 30, 29, 2, 31, 29, 29, 28, ... at the transition between segments
            //     (excerpt from 102x102 to 110x109)
            //   - The values 2 and 31 are in the same scanline (106) which contains the transition
            //   - My theory is that this line happens to be calculated more "accurately" (loosely speaking). The 2 sits
            //     at the end of the first segment, and the 31 is at the start of the next segment
            //   - It might be using the FracXEnd(y) as reference instead of the start for long spans
            //   - Or maybe it always uses FracXEnd(y)?

            // Previous best implementation
            /*const i32 start = m_negative ? FracXEnd(y) : FracXStart(y);
            const i32 offset = m_negative ? start - (x << kFracBits) : (x << kFracBits) + kBias - start;
            const i32 fracCoverage = (offset * (i32)m_aaStep) >> kFracBits;
            const i32 finalCoverage =
                m_negative ? (fracCoverage - m_aaBias - 1) % kAABaseX : (fracCoverage + m_aaBias) % kAABaseX;
            return finalCoverage;*/

            // TODO: brute-force formula
            // - build full data set from existing data
            //   - data sets:
            //     - left positive X-major
            //     - left positive Y-major
            //     - left negative X-major
            //     - left negative Y-major
            //     - right positive X-major
            //     - right positive Y-major
            //     - right negative X-major
            //     - right negative Y-major
            //   - contents:
            //     - input: width,height; x,y coordinates (using slope generator)
            //     - output: expected coverage value at x,y (including zeros)
            // - make a function generator
            //   - supported operations: + - * / % >> << & | ^ ~ -(unary)  (all on i32)
            //   - variables: all input values above, plus some constants:
            //     - 1 (useful in many cases)
            //     - 32 (antialiasing coverage range)
            //     - 18 (fractional x coordinate bits)
            //     - 9 (half of fractional x coordinate bits)
            //     - 5 (fractional antialiasing coverage bits)
            //   - stack-based
            //     - variables are pushed onto the stack
            //     - operators pop one or two values from the stack and push the result
            //   - stop condition: when a function perfectly matches the input/output set

            // TODO: negative slopes
            const i32 startX = XStart(y);
            const i32 endX = XEnd(y);
            const i32 deltaX = endX - startX + 1;
            const i32 baseCoverage = ((startX * m_height * kAARange) << kAAFracBitsX) / m_width;
            const i32 fullCoverage = ((deltaX * m_height * kAARange) << kAAFracBitsX) / m_width;
            const i32 coverageStep = fullCoverage / deltaX;
            const i32 coverageBias = coverageStep / 2;
            const i32 offset = x - startX;
            // const i32 fracCoverage = baseCoverage + fullCoverage - (deltaX - offset) * coverageStep;
            const i32 fracCoverage = baseCoverage + offset * coverageStep;
            const i32 finalCoverage = (fracCoverage + coverageBias) % kAABaseX;
            return finalCoverage;
        } else {
            const i32 fracCoverage = yOffset * (i32)m_aaStep - xOffset * (i32)kAABaseY;
            const i32 finalCoverage = m_negative ? (fracCoverage - (i32)m_aaBias - 1) : (fracCoverage + (i32)m_aaBias);
            const i32 output = m_negative ? std::min(finalCoverage, (i32)kAABaseY - 1)
                                          : (i32)kAABaseY - 1 - std::min(finalCoverage, (i32)kAABaseY - 1);
            if (output >= (i32)m_aaBias) {
                return output;
            } else {
                return 0;
            }
        }
    }

private:
    i32 m_x0;        // X0 coordinate (minus 1 if this is a negative slope)
    i32 m_y0;        // Y0 coordinate
    i32 m_width;     // Slope width (abs(X1 - X0))
    i32 m_height;    // Slope height (abs(Y1 - Y0))
    i32 m_dx;        // X displacement per scanline
    bool m_negative; // True if the slope is negative (X1 < X0)
    bool m_xMajor;   // True if the slope is X-major (X1-X0 > Y1-Y0)
    bool m_diagonal; // True if the slope is diagonal (X1-X0 == Y1-Y0)
    u32 m_aaBias;    // Antialiasing coverage initial bias
    u32 m_aaStep;    // Antialiasing coverage step per pixel
};
