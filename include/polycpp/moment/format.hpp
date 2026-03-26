/**
 * @file format.hpp
 * @brief Format token system declarations — converts Moment to formatted strings.
 *
 * Provides the core formatting functions used by Moment::format(). Supports all
 * standard Moment.js format tokens (YYYY, MM, DD, HH, mm, ss, etc.), macro tokens
 * (LT, L, LL, LLL, LLLL), escaped text in [brackets], and locale-dependent tokens.
 *
 * @see https://momentjs.com/docs/#/displaying/format/
 * @since 0.2.0
 */
#pragma once

#include <string>

namespace polycpp {
namespace moment {

class Moment; // forward declare

/**
 * @brief Format a Moment using the given format string.
 *
 * The format string supports Moment.js-compatible tokens:
 * - Year: YYYY, YY
 * - Quarter: Q
 * - Month: M, MM, MMM, MMMM
 * - Day of month: D, DD, Do
 * - Day of year: DDD, DDDD
 * - Day of week: d, dd, ddd, dddd, e, E
 * - Week: w, ww, W, WW
 * - Hour: H, HH, h, hh, k, kk
 * - Minute: m, mm
 * - Second: s, ss
 * - Fractional second: S, SS, SSS
 * - AM/PM: a, A
 * - Timezone offset: Z, ZZ
 * - Unix timestamp: X, x
 * - Escaped text: [text]
 * - Macro tokens: LT, LTS, L, LL, LLL, LLLL
 *
 * Unknown tokens are passed through as literal text.
 *
 * @param m          The Moment to format.
 * @param format_str The format string.
 * @return The formatted date/time string.
 * @since 0.2.0
 */
std::string formatMoment(const Moment& m, const std::string& format_str);

/**
 * @brief Format a Moment using the default ISO 8601 format.
 *
 * Produces output like "2024-03-15T14:30:45.123+00:00" for UTC moments.
 *
 * @param m The Moment to format.
 * @return The default-formatted date/time string.
 * @since 0.2.0
 */
std::string defaultFormat(const Moment& m);

} // namespace moment
} // namespace polycpp
