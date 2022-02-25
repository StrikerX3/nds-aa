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
///
/// TODO: describe antialiasing
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
    /// The number of bits reserved for fractional antialiasing coverage calculations.
    /// </summary>
    static constexpr u32 kAAFracBits = 5;

    /// <summary>
    /// The antialiasing coverage value range with fractional bits.
    /// </summary>
    static constexpr u32 kAAFracRange = kAARange << kAAFracBits;

    /// <summary>
    /// Configures the slope to interpolate the line (X0,X1)-(Y0,Y1) using screen coordinates.
    /// </summary>
    /// <param name="x0">First X coordinate</param>
    /// <param name="y0">First Y coordinate</param>
    /// <param name="x1">Second X coordinate</param>
    /// <param name="y1">Second Y coordinate</param>
    /// <param name="left">true for left edge, false for right edge</param>
    constexpr void Setup(i32 x0, i32 y0, i32 x1, i32 y1, bool left) {
        // Always interpolate top to bottom
        if (y1 < y0) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        // Store reference coordinates and edge side
        m_x0 = x0;
        m_x0Frac = x0 << kFracBits;
        m_y0 = y0;
        m_leftEdge = left;

        // Determine if this is a negative slope and adjust accordingly
        m_negative = (x1 < x0);
        if (m_negative) {
            m_x0Frac--;
            std::swap(x0, x1);
        }

        // Store size
        m_width = x1 - x0;
        m_height = y1 - y0;

        // Compute coordinate deltas and determine if the slope is X-major
        m_xMajor = (m_width > m_height);
        const bool diagonal = (m_width == m_height);

        // Precompute bias for X-major or diagonal slopes
        if (m_xMajor || diagonal) {
            if (m_negative) {
                m_x0Frac -= kBias;
            } else {
                m_x0Frac += kBias;
            }
        }

        // Compute X displacement per scanline
        m_dx = m_width;
        if (m_height != 0) {
            m_dx *= kOne / m_height; // This ensures the division is performed before the multiplication
        } else {
            m_dx *= kOne;
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
            return m_x0Frac - displacement;
        } else {
            return m_x0Frac + displacement;
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

    constexpr i32 X0() const {
        return m_x0;
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
    /// Determines if the slope is Y-major.
    /// </summary>
    /// <returns>true if the slope is Y-major, but not a perfect diagonal.</returns>
    constexpr bool IsYMajor() const {
        return !m_xMajor && !IsDiagonal();
    }

    /// <summary>
    /// Determines if the slope is a perfect diagonal.
    /// </summary>
    /// <returns>true if the slope's width is equal to its height.</returns>
    constexpr bool IsDiagonal() const {
        return m_width == m_height;
    }

    /// <summary>
    /// Determines if the slope is negative (i.e. X decreases as Y increases).
    /// </summary>
    /// <returns>true if the slope is negative.</returns>
    constexpr bool IsNegative() const {
        return m_negative;
    }

    /// <summary>
    /// Determines if the slope is positive (i.e. X increases as Y increases).
    /// </summary>
    /// <returns>true if the slope is positive.</returns>
    constexpr bool IsPositive() const {
        return !m_negative;
    }

    /// <summary>
    /// Determines if the slope is a left edge.
    /// </summary>
    /// <returns>true if the slope is a left edge.</returns>
    constexpr bool IsLeftEdge() const {
        return m_leftEdge;
    }

    /// <summary>
    /// Determines if the slope is a right edge.
    /// </summary>
    /// <returns>true if the slope is a right edge.</returns>
    constexpr bool IsRightEdge() const {
        return !m_leftEdge;
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
    /// Computes the antialiasing coverage at the specified coordinates.
    /// </summary>
    /// <param name="x">The X coordinate</param>
    /// <param name="y">The Y coordinate</param>
    /// <returns>The antialiasing coverage value at (X,Y)</returns>
    constexpr i32 AACoverage(i32 x, i32 y) const {
        return FracAACoverage(x, y) >> kAAFracBits;
    }

    /// <summary>
    /// Computes the antialiasing coverage at the specified coordinates, including the fractional part.
    /// </summary>
    /// <param name="x">The X coordinate</param>
    /// <param name="y">The Y coordinate</param>
    /// <returns>The antialiasing coverage value at (X,Y)</returns>
    constexpr i32 FracAACoverage(i32 x, i32 y) const {
        // Antialiasing notes:
        // - AA coverage calculation uses different variables depending on whether the slope is X-major or not
        //   - The formula is consistent across all slope types
        // - Perfectly horizontal or vertical edges (DX or DY == 0) are drawn in full alpha
        // - Perfectly diagonal edges (DX == DY) are drawn in half alpha (15, or 16 if gradient is inverted)
        // - Gradients may be positive or negative
        //   - Positive gradient: AA coverage increases as X or Y increases
        //   - Negative gradient: AA coverage decreases as X or Y increases
        // - The following edges produce a positive gradient:
        //   - Left X-major (both positive and negative)
        //   - Left negative Y-major
        //   - Right positive Y-major
        // - Negative gradients are calculated by inverting the output of the corresponding positive gradient
        // - The last pixel of every vertical subspan of Y-major edges has fixed coverage depending on the gradient:
        //   - Positive gradient: full coverage
        //   - Negative gradient: zero coverage

        const auto invertGradient = [&](i32 coverage) -> i32 {
            // LR = m_leftEdge (false = R, true = L)
            // PN = m_negative (false = P, true = N)
            // XY = m_xMajor   (false = Y, true = X)
            //
            // normal:  RPY LPX LNY LNX
            // inverse: RPX RNY RNX LPY
            //
            // LR PN XY code rev PN|XY LR^(PN|XY) LR!=(PN|XY)
            // -  -  -  RPY   -    -       -           -
            // -  -  +  RPX   +    +       +           +
            // -  +  -  RNY   +    +       +           +
            // -  +  +  RNX   +    +       +           +
            // +  -  -  LPY   +    -       +           +
            // +  -  +  LPX   -    +       -           -
            // +  +  -  LNY   -    +       -           -
            // +  +  +  LNX   -    +       -           -
            //
            // Any of these matches our requirements:
            //   LR ^ (PN | XY)
            //   LR != (PN || XY)
            if (m_leftEdge != (m_negative || m_xMajor)) {
                coverage ^= kAAFracRange - 1;
            }
            return coverage;
        };

        if (m_width == 0 && m_height == 0) {
            // Zero by zero produces no output
            return 0;
        }
        if (m_width == 0 || m_height == 0) {
            // Perfect horizontals and verticals have full alpha
            return invertGradient(0);
        }
        if (m_width == m_height) {
            // Perfect diagonals always have half alpha
            return invertGradient((kAAFracRange >> 1) - 1);
        }

        if (m_xMajor) {
            // TODO: fix off-by-one errors
            // - Coverage bias is slightly off in ~2% of the cases
            // - Handle the artifacts in 186x185 and many others
            //   - The line has two perfect diagonal segments: 0x0 to 106x106, and 107x106 to 185x184
            //   - The calculation produces a gradient that's too short for the first segment, which causes values to
            //     wrap around to 31 before the segment ends (starting at 93x93)
            //   - The gradient goes ..., 30, 30, 30, 29, >2, 31<, 29, 29, 28, ... at the transition between segments
            //     (excerpt from 102x102 to 110x109)
            //   - The values 2 and 31 are in the scanline that contains the transition (106)
            //   - These two values come from the corresponding negative slope at the same exact position within the
            //     gradient, which goes ..., 3, 3, 3, 2, >2, 31<, 2, 1, 1, 1, ...
            //   - It could also be seen as a miscalculation of the coverage bias

            const i32 startX = m_negative ? XEnd(y) : XStart(y);
            const i32 xOffsetOrigin = m_negative ? startX - (m_x0 - m_width) : startX - m_x0;
            const i32 xOffsetSegment = x - startX;
            const i32 coverageStep = m_height * kAAFracRange / m_width;
            const i32 coverageBias = ((2 * xOffsetOrigin + 1) * m_height * kAAFracRange) / (2 * m_width);
            const i32 fracCoverage = xOffsetSegment * coverageStep;
            const i32 finalCoverage = (fracCoverage + coverageBias) % kAAFracRange;
            return invertGradient(finalCoverage);
        } else {
            if (XStart(y) != XStart(y + 1)) {
                // Last pixel of a vertical slice seems to be forced to maximum coverage
                return invertGradient(kAAFracRange - 1);
            }
            const i32 fxs = (m_negative ? kOne - FracXEnd(y) : FracXStart(y)) % kOne;
            const i32 baseCoverage = (fxs & kMask) >> 8;
            const i32 coverageStep = m_width * kAAFracRange / m_height;
            const i32 coverageBias = coverageStep / 2;
            return invertGradient(baseCoverage + coverageBias);
        }
    }

private:
    i32 m_x0;        // X0 coordinate
    i32 m_x0Frac;    // Fractional X0 coordinate (minus 1 if this is a negative slope)
    i32 m_y0;        // Y0 coordinate
    i32 m_width;     // Slope width (abs(X1 - X0))
    i32 m_height;    // Slope height (abs(Y1 - Y0))
    i32 m_dx;        // X displacement per scanline
    bool m_negative; // True if the slope is negative (X1 < X0)
    bool m_xMajor;   // True if the slope is X-major (X1-X0 > Y1-Y0)
    bool m_leftEdge; // True if this is a left edge, false for a right edge
};
