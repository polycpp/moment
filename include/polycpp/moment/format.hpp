/**
 * @file format.hpp
 * @brief Format token system declarations — converts Moment to formatted strings.
 *
 * Provides the core formatting functions used by Moment::format(). Supports all
 * standard Moment.js format tokens (YYYY, MM, DD, HH, mm, ss, etc.), macro tokens
 * (LT, L, LL, LLL, LLLL), escaped text in [brackets], and locale-dependent tokens.
 *
 * @see https://momentjs.com/docs/#/displaying/format/
 * @since 1.0.0
 */
#pragma once

#include <string>

namespace polycpp {
namespace moment {

class Moment; // forward declare

/**
 * @brief Moment.js-compatible HTML5 input format constants.
 *
 * These constants mirror upstream `moment.HTML5_FMT` and can be passed to
 * `Moment::format()` or the custom-format parsing overloads.
 *
 * @see https://momentjs.com/docs/#/parsing/string-format/
 * @since 1.0.0
 */
namespace HTML5_FMT {

inline constexpr char DATETIME_LOCAL[] = "YYYY-MM-DDTHH:mm";
inline constexpr char DATETIME_LOCAL_SECONDS[] = "YYYY-MM-DDTHH:mm:ss";
inline constexpr char DATETIME_LOCAL_MS[] = "YYYY-MM-DDTHH:mm:ss.SSS";
inline constexpr char DATE[] = "YYYY-MM-DD";
inline constexpr char TIME[] = "HH:mm";
inline constexpr char TIME_SECONDS[] = "HH:mm:ss";
inline constexpr char TIME_MS[] = "HH:mm:ss.SSS";
inline constexpr char WEEK[] = "GGGG-[W]WW";
inline constexpr char MONTH[] = "YYYY-MM";

} // namespace HTML5_FMT

namespace html5_fmt = HTML5_FMT;

/**
 * @brief Format a Moment using the given format string.
 *
 * The format string supports Moment.js-compatible tokens:
 * - Year: Y, YYYYY, YYYYYY, YYYY, YY
 * - Era: N, NN, NNN, NNNN, NNNNN, y, yy, yyy, yyyy, yo
 * - Quarter: Q
 * - Month: M, MM, MMM, MMMM
 * - Day of month: D, DD, Do
 * - Day of year: DDD, DDDD, DDDo
 * - Day of week: d, dd, ddd, dddd, do, e, E
 * - Week: w, ww, wo, W, WW, Wo, g, gg, gggg, ggggg, G, GG, GGGG, GGGGG
 * - Hour: H, HH, h, hh, k, kk, Hmm, Hmmss, hmm, hmmss
 * - Minute: m, mm
 * - Second: s, ss
 * - Fractional second: S through SSSSSSSSS
 * - AM/PM: a, A
 * - Timezone offset/name: Z, ZZ, z, zz
 * - Unix timestamp: X, x
 * - Escaped text: [text] and backslash-escaped next character
 * - Macro tokens: LT, LTS, L, LL, LLL, LLLL
 *
 * Unknown tokens are passed through as literal text.
 *
 * @param m          The Moment to format.
 * @param format_str The format string.
 * @return The formatted date/time string.
 * @since 1.0.0
 */
std::string formatMoment(const Moment& m, const std::string& format_str);

/**
 * @brief Format a Moment using the default ISO 8601 format.
 *
 * Produces output like "2024-03-15T14:30:45.123+00:00" for UTC moments.
 *
 * @param m The Moment to format.
 * @return The default-formatted date/time string.
 * @since 1.0.0
 */
std::string defaultFormat(const Moment& m);

} // namespace moment
} // namespace polycpp
