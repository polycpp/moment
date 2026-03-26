/**
 * @file detail/format.hpp
 * @brief Format token system implementation — converts Moment to formatted strings.
 *
 * ## Tokenization algorithm
 *
 * The format string is broken into segments of three types:
 * 1. **Escaped text**: `[anything here]` -- the contents are emitted verbatim.
 * 2. **Format tokens**: matched by longest-prefix from an ordered list of known
 *    tokens (e.g., MMMM before MMM before MM before M).
 * 3. **Literal characters**: any character that does not start a recognized token
 *    or escape sequence is emitted verbatim.
 *
 * ## Macro token expansion
 *
 * Tokens like LT, LTS, L, LL, LLL, LLLL (and their lowercase variants) are
 * "macro tokens" that expand to locale-specific format strings. For example,
 * LT expands to "h:mm A" in the English locale. Expansion happens once before
 * tokenization (no recursive expansion).
 *
 * ## Token formatter mapping
 *
 * Each recognized token maps to a formatting function that reads data from the
 * Moment (year, month, day, etc.) and returns a string. Zero-padding, 12-hour
 * conversion, ordinal suffixes, and locale lookups are handled per-token.
 *
 * @since 0.2.0
 */
#pragma once

#include <polycpp/moment/moment.hpp>
#include <polycpp/moment/format.hpp>
#include <polycpp/moment/locale.hpp>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include <polycpp/core/error.hpp>

namespace polycpp {
namespace moment {
namespace detail {

// ── Zero-padding helpers ─────────────────────────────────────────────

/// @brief Pad an integer to at least @p width digits with leading zeros.
inline std::string zeroPad(int value, int width) {
    std::string s = std::to_string(value < 0 ? -value : value);
    while (static_cast<int>(s.size()) < width) {
        s = "0" + s;
    }
    if (value < 0) s = "-" + s;
    return s;
}

/// @brief Pad an int64_t to at least @p width digits with leading zeros.
inline std::string zeroPad64(int64_t value, int width) {
    std::string s = std::to_string(value < 0 ? -value : value);
    while (static_cast<int>(s.size()) < width) {
        s = "0" + s;
    }
    if (value < 0) s = "-" + s;
    return s;
}

// ── Macro token expansion ────────────────────────────────────────────

/// @brief Ordered list of macro tokens, longest first, for greedy matching.
/// Each pair is {token, field_name_in_LongDateFormats}.
struct MacroToken {
    const char* token;
    int id; // 0=LT, 1=LTS, 2=L, 3=LL, 4=LLL, 5=LLLL, 6=l, 7=ll, 8=lll, 9=llll
};

/// @brief Get the expansion for a macro token from the locale's longDateFormat.
/// Returns an empty string if the token is not a macro token.
inline std::string expandMacroToken(const std::string& token, const LongDateFormats& ldf) {
    if (token == "LLLL") return ldf.LLLL;
    if (token == "LLL")  return ldf.LLL;
    if (token == "LTS")  return ldf.LTS;
    if (token == "LT")   return ldf.LT;
    if (token == "LL")   return ldf.LL;
    if (token == "L")    return ldf.L;
    // Lowercase variants: derive from uppercase by removing day name, lowercasing month
    // For simplicity, use the same as uppercase (moment.js auto-derives these).
    if (token == "llll") return ldf.LLLL.empty() ? "" : ldf.LLLL;
    if (token == "lll")  return ldf.LLL.empty() ? "" : ldf.LLL;
    if (token == "ll")   return ldf.LL.empty() ? "" : ldf.LL;
    if (token == "l")    return ldf.L.empty() ? "" : ldf.L;
    return "";
}

/// @brief Expand all macro tokens in a format string (single pass, no recursion).
/// Replaces LT, LTS, L, LL, LLL, LLLL (and lowercase) with their locale expansions.
inline std::string expandMacroTokens(const std::string& fmt, const LongDateFormats& ldf) {
    std::string result;
    result.reserve(fmt.size() * 2);

    size_t i = 0;
    while (i < fmt.size()) {
        // Skip escaped text [...]
        if (fmt[i] == '[') {
            size_t end = fmt.find(']', i + 1);
            if (end == std::string::npos) end = fmt.size();
            result.append(fmt, i, end - i + 1);
            i = end + 1;
            continue;
        }

        // Try matching macro tokens (longest first)
        bool matched = false;
        // Order: LLLL, LLL, LTS, LT, LL, L, llll, lll, ll, l
        static const char* macros[] = {
            "LLLL", "LLL", "LTS", "LT", "LL", "L",
            "llll", "lll", "ll", "l"
        };
        for (const char* macro : macros) {
            size_t len = std::char_traits<char>::length(macro);
            if (i + len <= fmt.size() && fmt.compare(i, len, macro) == 0) {
                std::string expansion = expandMacroToken(macro, ldf);
                if (!expansion.empty()) {
                    result.append(expansion);
                    i += len;
                    matched = true;
                    break;
                }
            }
        }

        if (!matched) {
            result += fmt[i];
            ++i;
        }
    }

    return result;
}

// ── Tokenizer ────────────────────────────────────────────────────────

/// @brief Segment type in a tokenized format string.
enum class SegmentType {
    Literal, ///< Literal text to emit verbatim
    Token    ///< A recognized format token
};

/// @brief A single segment from tokenizing a format string.
struct FormatSegment {
    SegmentType type;
    std::string value; ///< The token name or literal text
};

/// @brief Master list of format tokens, ordered longest-first for greedy matching.
/// This ensures "MMMM" is tried before "MMM" before "MM" before "M", etc.
inline const std::vector<std::string>& formatTokens() {
    static const std::vector<std::string> tokens = {
        // 9-char fractional seconds
        "SSSSSSSSS",
        // 8-char
        "SSSSSSSS",
        // 7-char
        "SSSSSSS",
        // 6-char
        "YYYYYY", "SSSSSS",
        // 5-char
        "YYYYY", "NNNNN", "SSSSS", "Hmmss", "hmmss",
        // 4-char
        "YYYY", "MMMM", "DDDD", "dddd", "GGGG", "gggg", "NNNN", "SSSS", "Hmm",
        // 3-char
        "MMM", "DDD", "ddd", "SSS", "NNN",
        // 2-char
        "YY", "MM", "Mo", "DD", "Do", "dd", "do", "HH", "hh", "kk", "mm",
        "ss", "SS", "ZZ", "WW", "Wo", "ww", "wo", "GG", "gg", "NN", "Qo",
        // 1-char
        "Q", "M", "D", "d", "H", "h", "k", "m", "s", "S",
        "A", "a", "Z", "X", "x", "W", "w", "E", "e", "N",
    };
    return tokens;
}

/// @brief Tokenize a format string into segments of literal text and format tokens.
///
/// Algorithm:
/// 1. Walk the string character by character.
/// 2. If `[` is found, everything until `]` is a literal.
/// 3. Otherwise, try matching the longest known token at the current position.
/// 4. If no token matches, the current character is a literal.
/// Adjacent literals are merged for efficiency.
inline std::vector<FormatSegment> tokenize(const std::string& fmt) {
    std::vector<FormatSegment> segments;
    const auto& tokens = formatTokens();
    size_t i = 0;

    while (i < fmt.size()) {
        // Handle escaped literal text in [brackets]
        if (fmt[i] == '[') {
            size_t end = fmt.find(']', i + 1);
            if (end == std::string::npos) {
                // Unterminated bracket: treat rest as literal
                segments.push_back({SegmentType::Literal, fmt.substr(i + 1)});
                break;
            }
            std::string lit = fmt.substr(i + 1, end - i - 1);
            segments.push_back({SegmentType::Literal, lit});
            i = end + 1;
            continue;
        }

        // Try matching a format token (longest first)
        bool matched = false;
        for (const auto& tok : tokens) {
            if (i + tok.size() <= fmt.size() &&
                fmt.compare(i, tok.size(), tok) == 0) {
                segments.push_back({SegmentType::Token, tok});
                i += tok.size();
                matched = true;
                break;
            }
        }

        if (!matched) {
            // Single literal character
            if (!segments.empty() && segments.back().type == SegmentType::Literal) {
                segments.back().value += fmt[i];
            } else {
                segments.push_back({SegmentType::Literal, std::string(1, fmt[i])});
            }
            ++i;
        }
    }

    return segments;
}

// ── Token formatters ─────────────────────────────────────────────────

/// @brief Format a single token for the given Moment.
/// Returns the string representation of the token's value.
inline std::string formatToken(const std::string& token, const Moment& m,
                                const LocaleData& locale) {
    // Helper to convert 24h hour to 12h
    auto hour12 = [](int h) -> int {
        int h12 = h % 12;
        return h12 == 0 ? 12 : h12;
    };

    // ── Year tokens ──
    if (token == "YYYY") {
        return zeroPad(m.year(), 4);
    }
    if (token == "YY") {
        return zeroPad(m.year() % 100, 2);
    }
    if (token == "YYYYYY") {
        // Signed 6-digit year
        int y = m.year();
        std::string s = zeroPad(y < 0 ? -y : y, 6);
        return (y < 0 ? "-" : "+") + s;
    }
    if (token == "YYYYY") {
        return zeroPad(m.year(), 5);
    }

    // ── Quarter token ──
    if (token == "Q") {
        return std::to_string(m.quarter());
    }
    if (token == "Qo") {
        if (locale.ordinal) return locale.ordinal(m.quarter(), "Q");
        return std::to_string(m.quarter());
    }

    // ── Month tokens ──
    if (token == "MMMM") {
        int mo = m.month();
        if (mo >= 0 && mo < 12) return locale.months[mo];
        return "";
    }
    if (token == "MMM") {
        int mo = m.month();
        if (mo >= 0 && mo < 12) return locale.monthsShort[mo];
        return "";
    }
    if (token == "MM") {
        return zeroPad(m.month() + 1, 2);
    }
    if (token == "Mo") {
        if (locale.ordinal) return locale.ordinal(m.month() + 1, "M");
        return std::to_string(m.month() + 1);
    }
    if (token == "M") {
        return std::to_string(m.month() + 1);
    }

    // ── Day of month tokens ──
    if (token == "DD") {
        return zeroPad(m.date(), 2);
    }
    if (token == "Do") {
        if (locale.ordinal) return locale.ordinal(m.date(), "D");
        return std::to_string(m.date());
    }
    if (token == "D") {
        return std::to_string(m.date());
    }

    // ── Day of year tokens ──
    if (token == "DDDD") {
        return zeroPad(m.dayOfYear(), 3);
    }
    if (token == "DDD") {
        return std::to_string(m.dayOfYear());
    }

    // ── Day of week tokens ──
    if (token == "dddd") {
        int d = m.day();
        if (d >= 0 && d < 7) return locale.weekdays[d];
        return "";
    }
    if (token == "ddd") {
        int d = m.day();
        if (d >= 0 && d < 7) return locale.weekdaysShort[d];
        return "";
    }
    if (token == "dd") {
        int d = m.day();
        if (d >= 0 && d < 7) return locale.weekdaysMin[d];
        return "";
    }
    if (token == "do") {
        if (locale.ordinal) return locale.ordinal(m.day(), "d");
        return std::to_string(m.day());
    }
    if (token == "d") {
        return std::to_string(m.day());
    }

    // ── Locale / ISO day of week tokens ──
    if (token == "e") {
        return std::to_string(m.weekday());
    }
    if (token == "E") {
        return std::to_string(m.isoWeekday());
    }

    // ── Week tokens ──
    if (token == "ww") {
        return zeroPad(m.week(), 2);
    }
    if (token == "wo") {
        if (locale.ordinal) return locale.ordinal(m.week(), "w");
        return std::to_string(m.week());
    }
    if (token == "w") {
        return std::to_string(m.week());
    }
    if (token == "WW") {
        return zeroPad(m.isoWeek(), 2);
    }
    if (token == "Wo") {
        if (locale.ordinal) return locale.ordinal(m.isoWeek(), "W");
        return std::to_string(m.isoWeek());
    }
    if (token == "W") {
        return std::to_string(m.isoWeek());
    }

    // ── Week year tokens ──
    if (token == "GGGG") {
        return zeroPad(m.isoWeekYear(), 4);
    }
    if (token == "GG") {
        return zeroPad(m.isoWeekYear() % 100, 2);
    }
    if (token == "gggg") {
        return zeroPad(m.weekYear(), 4);
    }
    if (token == "gg") {
        return zeroPad(m.weekYear() % 100, 2);
    }

    // ── Hour tokens (24h) ──
    if (token == "HH") {
        return zeroPad(m.hour(), 2);
    }
    if (token == "H") {
        return std::to_string(m.hour());
    }

    // ── Hour tokens (12h) ──
    if (token == "hh") {
        return zeroPad(hour12(m.hour()), 2);
    }
    if (token == "h") {
        return std::to_string(hour12(m.hour()));
    }

    // ── Hour tokens (1-24) ──
    if (token == "kk") {
        int k = m.hour() == 0 ? 24 : m.hour();
        return zeroPad(k, 2);
    }
    if (token == "k") {
        int k = m.hour() == 0 ? 24 : m.hour();
        return std::to_string(k);
    }

    // ── Combined hour:minute tokens ──
    if (token == "Hmm") {
        return std::to_string(m.hour()) + zeroPad(m.minute(), 2);
    }
    if (token == "Hmmss") {
        return std::to_string(m.hour()) + zeroPad(m.minute(), 2) + zeroPad(m.second(), 2);
    }
    if (token == "hmm") {
        return std::to_string(hour12(m.hour())) + zeroPad(m.minute(), 2);
    }
    if (token == "hmmss") {
        return std::to_string(hour12(m.hour())) + zeroPad(m.minute(), 2) + zeroPad(m.second(), 2);
    }

    // ── Minute tokens ──
    if (token == "mm") {
        return zeroPad(m.minute(), 2);
    }
    if (token == "m") {
        return std::to_string(m.minute());
    }

    // ── Second tokens ──
    if (token == "ss") {
        return zeroPad(m.second(), 2);
    }
    if (token == "s") {
        return std::to_string(m.second());
    }

    // ── Fractional second tokens ──
    // S = tenths, SS = hundredths, SSS = milliseconds, SSSS+ = fractional with more digits
    if (token.size() >= 1 && token[0] == 'S' && token.find_first_not_of('S') == std::string::npos) {
        int ms = m.millisecond();
        int digits = static_cast<int>(token.size());
        if (digits == 1) {
            return std::to_string(ms / 100);
        } else if (digits == 2) {
            return zeroPad(ms / 10, 2);
        } else if (digits == 3) {
            return zeroPad(ms, 3);
        } else {
            // For more than 3 digits, append trailing zeros
            std::string base = zeroPad(ms, 3);
            while (static_cast<int>(base.size()) < digits) {
                base += '0';
            }
            return base;
        }
    }

    // ── Meridiem tokens ──
    if (token == "A") {
        if (locale.meridiem) return locale.meridiem(m.hour(), m.minute(), false);
        return m.hour() >= 12 ? "PM" : "AM";
    }
    if (token == "a") {
        if (locale.meridiem) return locale.meridiem(m.hour(), m.minute(), true);
        return m.hour() >= 12 ? "pm" : "am";
    }

    // ── Timezone offset tokens ──
    if (token == "Z") {
        int offset = m.utcOffset();
        if (offset == 0) return "+00:00";
        char sign = offset >= 0 ? '+' : '-';
        int abs_offset = offset < 0 ? -offset : offset;
        int hrs = abs_offset / 60;
        int mins = abs_offset % 60;
        return std::string(1, sign) + zeroPad(hrs, 2) + ":" + zeroPad(mins, 2);
    }
    if (token == "ZZ") {
        int offset = m.utcOffset();
        if (offset == 0) return "+0000";
        char sign = offset >= 0 ? '+' : '-';
        int abs_offset = offset < 0 ? -offset : offset;
        int hrs = abs_offset / 60;
        int mins = abs_offset % 60;
        return std::string(1, sign) + zeroPad(hrs, 2) + zeroPad(mins, 2);
    }

    // ── Unix timestamp tokens ──
    if (token == "X") {
        int64_t ts = m.valueOf();
        int64_t secs = ts / 1000;
        int64_t frac = ts % 1000;
        if (frac == 0) {
            return std::to_string(secs);
        }
        // Produce decimal representation like "1710505845.123"
        std::string fracStr = zeroPad64(frac < 0 ? -frac : frac, 3);
        // Trim trailing zeros
        while (fracStr.size() > 1 && fracStr.back() == '0') {
            fracStr.pop_back();
        }
        return std::to_string(secs) + "." + fracStr;
    }
    if (token == "x") {
        return std::to_string(m.valueOf());
    }

    // ── Era tokens (N, NN, NNN, NNNN, NNNNN) — simplified ──
    if (token.size() >= 1 && token[0] == 'N' && token.find_first_not_of('N') == std::string::npos) {
        // Simple era: AD/BC
        bool isAD = m.year() > 0;
        if (token.size() <= 2) return isAD ? "AD" : "BC";
        if (token.size() == 3) return isAD ? "AD" : "BC";
        if (token.size() == 4) return isAD ? "Anno Domini" : "Before Christ";
        return isAD ? "AD" : "BC";
    }

    // Unknown token: return as literal
    return token;
}

} // namespace detail

// ── Public API implementations ───────────────────────────────────────

/**
 * @brief Default format string for ISO 8601 output with timezone offset.
 *
 * Produces: YYYY-MM-DDTHH:mm:ss.SSSZ (e.g. "2024-03-15T14:30:45.123+00:00")
 */
inline std::string defaultFormat(const Moment& m) {
    return formatMoment(m, "YYYY-MM-DD[T]HH:mm:ss.SSSZ");
}

/**
 * @brief Format a Moment using the given format string.
 *
 * Implementation:
 * 1. If the Moment is invalid, return the locale's invalidDate string.
 * 2. If the format string is empty, use the default ISO 8601 format.
 * 3. Expand macro tokens (LT, L, LL, etc.) using the locale's longDateFormat.
 * 4. Tokenize the expanded format string into segments.
 * 5. For each token segment, look up the formatter and append the result.
 * 6. For each literal segment, append the text verbatim.
 */
inline std::string formatMoment(const Moment& m, const std::string& format_str) {
    if (!m.isValid()) {
        const auto& loc = localeData(Moment::InternalAccess::getLocaleKey(m));
        return loc.invalidDate;
    }

    // Use default format if empty
    std::string fmt = format_str.empty()
        ? "YYYY-MM-DD[T]HH:mm:ss.SSSZ"
        : format_str;

    // Step 1: Expand macro tokens using locale's longDateFormat
    const auto& loc = localeData(Moment::InternalAccess::getLocaleKey(m));
    fmt = detail::expandMacroTokens(fmt, loc.longDateFormat);

    // Step 2: Tokenize
    auto segments = detail::tokenize(fmt);

    // Step 3: Format each segment
    std::string result;
    result.reserve(fmt.size() * 2);
    for (const auto& seg : segments) {
        if (seg.type == detail::SegmentType::Literal) {
            result.append(seg.value);
        } else {
            result.append(detail::formatToken(seg.value, m, loc));
        }
    }

    // Step 4: Apply postformat transform if available
    if (loc.postformat) {
        result = loc.postformat(result);
    }

    return result;
}

// ── Moment::format() method implementation ───────────────────────────

inline std::string Moment::format(const std::string& fmt) const {
    return formatMoment(*this, fmt);
}

// ── Moment::toISOString() method implementation ─────────────────────

inline std::string Moment::toISOString(bool keepOffset) const {
    if (!isValid()) {
        throw polycpp::RangeError("Invalid time value");
    }
    if (keepOffset) {
        return format("YYYY-MM-DD[T]HH:mm:ss.SSSZ");
    }
    // Always output in UTC with Z suffix
    Moment utcCopy = clone();
    utcCopy.utc();
    return utcCopy.format("YYYY-MM-DD[T]HH:mm:ss.SSS[Z]");
}

// ── Moment::unix() method implementation ─────────────────────────────

inline int64_t Moment::unix() const {
    return timestamp_ms_ / 1000;
}

} // namespace moment
} // namespace polycpp
