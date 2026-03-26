/**
 * @file detail/parse.hpp
 * @brief Parse implementation — creates Moment objects from string input.
 *
 * ## Parsing pipeline
 *
 * ### Auto-detection (no format specified):
 * 1. Try ISO 8601 parsing via regex.
 * 2. Try RFC 2822 parsing via regex.
 * 3. If neither matches, return an invalid Moment.
 *
 * ### ISO 8601 regex
 * Matches patterns like:
 * - `YYYY`
 * - `YYYY-MM`
 * - `YYYY-MM-DD`
 * - `YYYY-MM-DDTHH:mm`
 * - `YYYY-MM-DDTHH:mm:ss`
 * - `YYYY-MM-DDTHH:mm:ss.SSS`
 * - With optional timezone: `Z`, `+HH:MM`, `+HHMM`, `+HH`
 * - `T` separator can also be a space.
 *
 * ### RFC 2822 regex
 * Matches: `ddd, DD MMM YYYY HH:mm:ss +HHMM`
 *
 * ### Custom format parsing
 * 1. Tokenize the format string (same tokenizer as format.hpp).
 * 2. Walk the input string, matching each token's expected pattern.
 * 3. Extract date components from the matched groups.
 * 4. Build a Moment from the extracted components.
 *
 * ### Two-digit year interpretation
 * Following Moment.js: 68-99 maps to 1968-1999, 00-67 maps to 2000-2067.
 *
 * @since 0.2.0
 */
#pragma once

#include <polycpp/moment/moment.hpp>
#include <polycpp/moment/locale.hpp>
#include <polycpp/moment/detail/format.hpp>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <initializer_list>
#include <regex>
#include <string>
#include <vector>

namespace polycpp {
namespace moment {
namespace detail {

// ── Helper type: parsed date/time components ─────────────────────────

/// @brief Intermediate structure holding parsed date/time components.
/// Fields default to -1 meaning "not parsed / not specified".
struct ParsedComponents {
    int year = -1;
    int month = -1;    // 0-based (0=Jan)
    int day = 1;       // default to 1
    int hour = 0;
    int minute = 0;
    int second = 0;
    int millisecond = 0;
    int offset_minutes = 0;  // UTC offset in minutes
    bool has_offset = false;  // Whether an offset was parsed
    bool is_utc = false;      // Whether 'Z' was found
    bool valid = false;       // Whether parsing succeeded
    bool is_pm = false;       // Whether PM was detected (for 12h parsing)
    bool has_meridiem = false; // Whether a/A was parsed
};

// ── Two-digit year interpretation ────────────────────────────────────

/// @brief Convert a 2-digit year to a 4-digit year.
/// 68-99 -> 1968-1999, 00-67 -> 2000-2067 (matching moment.js).
inline int expandTwoDigitYear(int yy) {
    return yy >= 68 ? 1900 + yy : 2000 + yy;
}

// ── Fractional seconds normalization ─────────────────────────────────

/// @brief Convert a fractional seconds string (1-9 digits) to milliseconds.
/// "1" -> 100, "12" -> 120, "123" -> 123, "1234" -> 123, "123456789" -> 123.
inline int fracToMs(const std::string& frac) {
    if (frac.empty()) return 0;
    // Pad or truncate to exactly 3 digits for milliseconds
    std::string s = frac;
    while (s.size() < 3) s += '0';
    return std::stoi(s.substr(0, 3));
}

// ── ISO 8601 parsing ─────────────────────────────────────────────────

/// @brief Parse an ISO 8601 date/time string.
///
/// Supported patterns:
/// - YYYY
/// - YYYY-MM
/// - YYYY-MM-DD
/// - YYYY-MM-DDThh:mm
/// - YYYY-MM-DDThh:mm:ss
/// - YYYY-MM-DDThh:mm:ss.fractional
/// - Any of the above with Z, +HH:MM, +HHMM, -HH:MM, -HHMM
/// - 'T' can be replaced with a space
inline ParsedComponents parseISO8601(const std::string& input) {
    ParsedComponents pc;

    // Comprehensive ISO 8601 regex.
    // Groups:
    //   1: year (4+ digits, optional sign)
    //   2: month (2 digits, optional)
    //   3: day (2 digits, optional)
    //   4: hour (2 digits, optional)
    //   5: minute (2 digits, optional)
    //   6: second (2 digits, optional)
    //   7: fractional seconds (1-9 digits, optional)
    //   8: 'Z' (optional)
    //   9: offset sign (optional)
    //  10: offset hours (optional)
    //  11: offset minutes (optional)
    static const std::regex iso_re(
        "^([+-]?\\d{4,})"                        // 1: year
        "(?:-(\\d{2})"                            // 2: month
          "(?:-(\\d{2})"                          // 3: day
            "(?:[T ](\\d{2})"                     // 4: hour
              "(?::(\\d{2})"                      // 5: minute
                "(?::(\\d{2})"                    // 6: second
                  "(?:\\.(\\d{1,9}))?"             // 7: fractional seconds
                ")?"
              ")?"
            ")?"
          ")?"
        ")?"
        "(?:(Z)|([+-])(\\d{2}):?(\\d{2})?)?$",   // 8: Z, 9: sign, 10: offset hrs, 11: offset min
        std::regex::optimize
    );

    std::smatch match;
    if (!std::regex_match(input, match, iso_re)) {
        return pc; // valid=false
    }

    pc.valid = true;
    pc.year = std::stoi(match[1].str());

    if (match[2].matched) {
        pc.month = std::stoi(match[2].str()) - 1; // Convert to 0-based
    } else {
        pc.month = 0; // Default January
    }

    if (match[3].matched) {
        pc.day = std::stoi(match[3].str());
    }

    if (match[4].matched) {
        pc.hour = std::stoi(match[4].str());
    }
    if (match[5].matched) {
        pc.minute = std::stoi(match[5].str());
    }
    if (match[6].matched) {
        pc.second = std::stoi(match[6].str());
    }
    if (match[7].matched) {
        pc.millisecond = fracToMs(match[7].str());
    }

    // Timezone
    if (match[8].matched) {
        // 'Z' found: UTC
        pc.is_utc = true;
        pc.has_offset = true;
        pc.offset_minutes = 0;
    } else if (match[9].matched) {
        // Offset found
        pc.has_offset = true;
        int sign = match[9].str() == "+" ? 1 : -1;
        int hrs = std::stoi(match[10].str());
        int mins = match[11].matched ? std::stoi(match[11].str()) : 0;
        pc.offset_minutes = sign * (hrs * 60 + mins);
    }
    // If no timezone info, it's treated as local time (per ISO 8601 spec).

    return pc;
}

// ── RFC 2822 parsing ─────────────────────────────────────────────────

/// @brief Convert a 3-letter English month abbreviation to 0-based month index.
/// Returns -1 if not recognized.
inline int monthAbbrevToIndex(const std::string& abbrev) {
    static const char* months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    for (int i = 0; i < 12; ++i) {
        if (abbrev.size() >= 3 &&
            std::tolower(static_cast<unsigned char>(abbrev[0])) == std::tolower(static_cast<unsigned char>(months[i][0])) &&
            std::tolower(static_cast<unsigned char>(abbrev[1])) == std::tolower(static_cast<unsigned char>(months[i][1])) &&
            std::tolower(static_cast<unsigned char>(abbrev[2])) == std::tolower(static_cast<unsigned char>(months[i][2]))) {
            return i;
        }
    }
    return -1;
}

/// @brief Parse an RFC 2822 date/time string.
/// Example: "Fri, 15 Mar 2024 14:30:45 +0000"
/// Also handles optional day-of-week prefix and various spacing.
inline ParsedComponents parseRFC2822(const std::string& input) {
    ParsedComponents pc;

    // RFC 2822 regex:
    // Optional: "Day, " prefix
    // DD Mon YYYY HH:mm:ss +HHMM
    static const std::regex rfc_re(
        "^(?:[A-Za-z]{3},?\\s+)?"           // optional day name + comma
        "(\\d{1,2})\\s+"                     // 1: day
        "([A-Za-z]{3})\\s+"                  // 2: month abbrev
        "(\\d{4})\\s+"                       // 3: year
        "(\\d{2}):(\\d{2})"                  // 4: hour, 5: minute
        "(?::(\\d{2}))?"                     // 6: second (optional)
        "\\s*(?:([+-])(\\d{2})(\\d{2})|"     // 7: sign, 8: offset hrs, 9: offset mins
        "(GMT|UTC|Z))?\\s*$",                // 10: named timezone
        std::regex::optimize
    );

    std::smatch match;
    if (!std::regex_match(input, match, rfc_re)) {
        return pc; // valid=false
    }

    int monthIdx = monthAbbrevToIndex(match[2].str());
    if (monthIdx < 0) return pc; // invalid month

    pc.valid = true;
    pc.day = std::stoi(match[1].str());
    pc.month = monthIdx;
    pc.year = std::stoi(match[3].str());
    pc.hour = std::stoi(match[4].str());
    pc.minute = std::stoi(match[5].str());
    if (match[6].matched) {
        pc.second = std::stoi(match[6].str());
    }

    if (match[7].matched) {
        pc.has_offset = true;
        int sign = match[7].str() == "+" ? 1 : -1;
        pc.offset_minutes = sign * (std::stoi(match[8].str()) * 60 + std::stoi(match[9].str()));
    } else if (match[10].matched) {
        pc.has_offset = true;
        pc.offset_minutes = 0; // GMT/UTC/Z
        pc.is_utc = true;
    }

    return pc;
}

// ── Custom format parsing ────────────────────────────────────────────

/// @brief Try to match a number of the given max digit count at the current
///        position in the input string.
/// @param input The input string.
/// @param pos   Current position (updated on success).
/// @param minDigits Minimum digits to match.
/// @param maxDigits Maximum digits to match.
/// @param[out] value The parsed integer value.
/// @return true if at least minDigits were matched.
inline bool matchNumber(const std::string& input, size_t& pos,
                         int minDigits, int maxDigits, int& value) {
    size_t start = pos;
    size_t end = pos;
    while (end < input.size() && end - start < static_cast<size_t>(maxDigits) &&
           std::isdigit(static_cast<unsigned char>(input[end]))) {
        ++end;
    }
    int count = static_cast<int>(end - start);
    if (count < minDigits) return false;
    value = std::stoi(input.substr(start, count));
    pos = end;
    return true;
}

/// @brief Try to match a literal string at the current position (case-insensitive).
inline bool matchLiteral(const std::string& input, size_t& pos, const std::string& literal) {
    if (pos + literal.size() > input.size()) return false;
    for (size_t i = 0; i < literal.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(input[pos + i])) !=
            std::tolower(static_cast<unsigned char>(literal[i]))) {
            return false;
        }
    }
    pos += literal.size();
    return true;
}

/// @brief Try to match a month name (long or short) from the locale.
/// Returns the 0-based month index, or -1 if no match.
inline int matchMonthName(const std::string& input, size_t& pos,
                           const LocaleData& locale, bool isShort) {
    const auto& names = isShort ? locale.monthsShort : locale.months;
    // Try longest match first (sort by length descending)
    int bestIndex = -1;
    size_t bestLen = 0;
    for (int i = 0; i < 12; ++i) {
        const auto& name = names[i];
        if (name.size() > bestLen && pos + name.size() <= input.size()) {
            bool match = true;
            for (size_t j = 0; j < name.size(); ++j) {
                if (std::tolower(static_cast<unsigned char>(input[pos + j])) !=
                    std::tolower(static_cast<unsigned char>(name[j]))) {
                    match = false;
                    break;
                }
            }
            if (match) {
                bestIndex = i;
                bestLen = name.size();
            }
        }
    }
    if (bestIndex >= 0) {
        pos += bestLen;
    }
    return bestIndex;
}

/// @brief Try to match a weekday name from the locale (for parsing, we skip it
///        since it doesn't contribute to the date calculation).
inline bool matchWeekdayName(const std::string& input, size_t& pos,
                              const LocaleData& locale, int nameType) {
    // nameType: 0=min, 1=short, 2=full
    const auto* names = (nameType == 0) ? &locale.weekdaysMin :
                        (nameType == 1) ? &locale.weekdaysShort :
                                          &locale.weekdays;
    size_t bestLen = 0;
    for (int i = 0; i < 7; ++i) {
        const auto& name = (*names)[i];
        if (name.size() > bestLen && pos + name.size() <= input.size()) {
            bool match = true;
            for (size_t j = 0; j < name.size(); ++j) {
                if (std::tolower(static_cast<unsigned char>(input[pos + j])) !=
                    std::tolower(static_cast<unsigned char>(name[j]))) {
                    match = false;
                    break;
                }
            }
            if (match) {
                bestLen = name.size();
            }
        }
    }
    if (bestLen > 0) {
        pos += bestLen;
        return true;
    }
    return false;
}

/// @brief Try to match a meridiem string (AM/PM/am/pm).
/// Sets pc.is_pm and pc.has_meridiem.
inline bool matchMeridiem(const std::string& input, size_t& pos,
                           const LocaleData& locale, ParsedComponents& pc) {
    // Try locale-specific meridiem detection
    // Check for common patterns: AM, PM, am, pm, a.m., p.m.
    if (pos + 2 <= input.size()) {
        std::string two = input.substr(pos, 2);
        std::string twoLower;
        for (char c : two) twoLower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        if (twoLower == "pm") {
            pc.is_pm = true;
            pc.has_meridiem = true;
            pos += 2;
            return true;
        }
        if (twoLower == "am") {
            pc.is_pm = false;
            pc.has_meridiem = true;
            pos += 2;
            return true;
        }
    }
    return false;
}

/// @brief Try to match a timezone offset at the current position.
/// Handles: Z, +HH:MM, +HHMM, +HH, -HH:MM, -HHMM, -HH
inline bool matchOffset(const std::string& input, size_t& pos, ParsedComponents& pc) {
    if (pos >= input.size()) return false;

    if (input[pos] == 'Z' || input[pos] == 'z') {
        pc.has_offset = true;
        pc.is_utc = true;
        pc.offset_minutes = 0;
        ++pos;
        return true;
    }

    if (input[pos] == '+' || input[pos] == '-') {
        int sign = input[pos] == '+' ? 1 : -1;
        ++pos;
        int hrs = 0, mins = 0;
        if (!matchNumber(input, pos, 2, 2, hrs)) return false;

        // Optional colon
        if (pos < input.size() && input[pos] == ':') ++pos;

        // Optional minutes
        if (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
            matchNumber(input, pos, 2, 2, mins);
        }

        pc.has_offset = true;
        pc.offset_minutes = sign * (hrs * 60 + mins);
        return true;
    }

    return false;
}

/// @brief Parse a date/time string using a custom format string.
///
/// Walks the format string token by token, attempting to match each token's
/// expected pattern at the current position in the input string.
///
/// @param input  The date/time string to parse.
/// @param format The format string with Moment.js tokens.
/// @param strict If true, the entire input must be consumed.
/// @param locale The locale data for month/weekday names.
/// @return ParsedComponents with valid=true on success.
inline ParsedComponents parseWithFormat(const std::string& input,
                                         const std::string& format,
                                         bool strict,
                                         const LocaleData& locale) {
    ParsedComponents pc;

    // Expand macro tokens first
    std::string expandedFmt = expandMacroTokens(format, locale.longDateFormat);

    // Tokenize the format string
    auto segments = tokenize(expandedFmt);

    size_t pos = 0; // Current position in input

    for (const auto& seg : segments) {
        if (seg.type == SegmentType::Literal) {
            // Match literal text character by character (skip whitespace loosely)
            for (char c : seg.value) {
                if (pos >= input.size()) {
                    if (strict) return pc; // strict: must match all
                    break;
                }
                if (std::isspace(static_cast<unsigned char>(c))) {
                    // Skip whitespace in both
                    while (pos < input.size() && std::isspace(static_cast<unsigned char>(input[pos]))) {
                        ++pos;
                    }
                } else {
                    if (input[pos] == c) {
                        ++pos;
                    } else if (!strict) {
                        // Lenient: skip mismatched literal
                    } else {
                        return pc; // strict: mismatch
                    }
                }
            }
            continue;
        }

        // Format token: extract the corresponding value from the input
        const std::string& tok = seg.value;
        int val = 0;

        // ── Year tokens ──
        if (tok == "YYYY") {
            if (!matchNumber(input, pos, 4, 4, val)) { if (strict) return pc; continue; }
            pc.year = val;
        }
        else if (tok == "YY") {
            if (!matchNumber(input, pos, 2, 2, val)) { if (strict) return pc; continue; }
            pc.year = expandTwoDigitYear(val);
        }
        // ── Quarter ──
        else if (tok == "Q") {
            if (!matchNumber(input, pos, 1, 1, val)) { if (strict) return pc; continue; }
            // Quarter parsing: set month to start of quarter
            if (pc.month < 0) pc.month = (val - 1) * 3;
        }
        // ── Month tokens ──
        else if (tok == "MM") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.month = val - 1; // Convert to 0-based
        }
        else if (tok == "M") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.month = val - 1;
        }
        else if (tok == "MMMM") {
            int idx = matchMonthName(input, pos, locale, false);
            if (idx < 0) { if (strict) return pc; continue; }
            pc.month = idx;
        }
        else if (tok == "MMM") {
            int idx = matchMonthName(input, pos, locale, true);
            if (idx < 0) { if (strict) return pc; continue; }
            pc.month = idx;
        }
        else if (tok == "Mo") {
            // Ordinal month: match digits then skip ordinal suffix
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            // Skip ordinal suffix (st, nd, rd, th)
            if (pos + 2 <= input.size() && std::isalpha(static_cast<unsigned char>(input[pos]))) {
                pos += 2; // skip "st", "nd", "rd", "th"
            }
            pc.month = val - 1;
        }
        // ── Day of month tokens ──
        else if (tok == "DD") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.day = val;
        }
        else if (tok == "D") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.day = val;
        }
        else if (tok == "Do") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            // Skip ordinal suffix
            if (pos + 2 <= input.size() && std::isalpha(static_cast<unsigned char>(input[pos]))) {
                pos += 2;
            }
            pc.day = val;
        }
        // ── Day of year tokens ──
        else if (tok == "DDDD") {
            if (!matchNumber(input, pos, 3, 3, val)) { if (strict) return pc; continue; }
            // Convert day-of-year to month/day (deferred to component building)
            pc.day = val; // Store as special: will be reinterpreted
        }
        else if (tok == "DDD") {
            if (!matchNumber(input, pos, 1, 3, val)) { if (strict) return pc; continue; }
            pc.day = val;
        }
        // ── Day of week tokens (ignored for date calculation) ──
        else if (tok == "dddd") {
            matchWeekdayName(input, pos, locale, 2);
        }
        else if (tok == "ddd") {
            matchWeekdayName(input, pos, locale, 1);
        }
        else if (tok == "dd") {
            matchWeekdayName(input, pos, locale, 0);
        }
        else if (tok == "d" || tok == "e" || tok == "E") {
            matchNumber(input, pos, 1, 1, val); // Consume but ignore
        }
        // ── Hour tokens ──
        else if (tok == "HH") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.hour = val;
        }
        else if (tok == "H") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.hour = val;
        }
        else if (tok == "hh" || tok == "h") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.hour = val; // Will be adjusted for AM/PM later
        }
        else if (tok == "kk" || tok == "k") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.hour = val == 24 ? 0 : val;
        }
        // ── Minute tokens ──
        else if (tok == "mm") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.minute = val;
        }
        else if (tok == "m") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.minute = val;
        }
        // ── Second tokens ──
        else if (tok == "ss") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.second = val;
        }
        else if (tok == "s") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.second = val;
        }
        // ── Fractional second tokens ──
        else if (tok == "SSS") {
            if (!matchNumber(input, pos, 1, 3, val)) { if (strict) return pc; continue; }
            // If fewer than 3 digits were matched, adjust
            std::string orig = std::to_string(val);
            while (orig.size() < 3) orig += '0'; // right-pad
            pc.millisecond = std::stoi(orig.substr(0, 3));
        }
        else if (tok == "SS") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.millisecond = val * 10;
        }
        else if (tok == "S") {
            if (!matchNumber(input, pos, 1, 1, val)) { if (strict) return pc; continue; }
            pc.millisecond = val * 100;
        }
        // ── Meridiem tokens ──
        else if (tok == "A" || tok == "a") {
            if (!matchMeridiem(input, pos, locale, pc)) {
                if (strict) return pc;
            }
        }
        // ── Timezone tokens ──
        else if (tok == "Z" || tok == "ZZ") {
            if (!matchOffset(input, pos, pc)) {
                if (strict) return pc;
            }
        }
        // ── Unix timestamp tokens ──
        else if (tok == "X") {
            // Match a decimal number for unix seconds
            size_t start = pos;
            if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) ++pos;
            while (pos < input.size() && (std::isdigit(static_cast<unsigned char>(input[pos])) || input[pos] == '.')) ++pos;
            if (pos > start) {
                double secs = std::stod(input.substr(start, pos - start));
                int64_t ms = static_cast<int64_t>(secs * 1000.0);
                // Override all components: we'll create from timestamp directly
                pc.year = 1970;
                pc.month = 0;
                pc.day = 1;
                pc.hour = 0;
                pc.minute = 0;
                pc.second = 0;
                pc.millisecond = static_cast<int>(ms % 1000);
                // This is a special case: we should create from raw timestamp
                // Set a sentinel
                pc.valid = true;
                pc.has_offset = true;
                pc.is_utc = true;
                pc.offset_minutes = 0;
                // Directly compute from timestamp
                Moment temp(ms);
                temp.utc();
                pc.year = temp.year();
                pc.month = temp.month();
                pc.day = temp.date();
                pc.hour = temp.hour();
                pc.minute = temp.minute();
                pc.second = temp.second();
                pc.millisecond = temp.millisecond();
            } else {
                if (strict) return pc;
            }
        }
        else if (tok == "x") {
            // Match an integer for unix milliseconds
            int64_t msVal = 0;
            size_t start = pos;
            if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) ++pos;
            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) ++pos;
            if (pos > start) {
                msVal = std::stoll(input.substr(start, pos - start));
                Moment temp(msVal);
                temp.utc();
                pc.year = temp.year();
                pc.month = temp.month();
                pc.day = temp.date();
                pc.hour = temp.hour();
                pc.minute = temp.minute();
                pc.second = temp.second();
                pc.millisecond = temp.millisecond();
                pc.valid = true;
                pc.has_offset = true;
                pc.is_utc = true;
            } else {
                if (strict) return pc;
            }
        }
        // ── Week tokens (ignored for basic parsing) ──
        else if (tok == "ww" || tok == "w" || tok == "WW" || tok == "W") {
            matchNumber(input, pos, 1, 2, val); // consume but ignore
        }
        else if (tok == "GGGG" || tok == "gggg") {
            matchNumber(input, pos, 4, 4, val); // consume but ignore
        }
        else if (tok == "GG" || tok == "gg") {
            matchNumber(input, pos, 2, 2, val); // consume but ignore
        }
        // ── Unknown tokens: treat as literal ──
        else {
            // Try to match the token text literally
            for (char c : tok) {
                if (pos < input.size() && input[pos] == c) {
                    ++pos;
                }
            }
        }
    }

    // Apply AM/PM adjustment
    if (pc.has_meridiem) {
        if (pc.is_pm && pc.hour < 12) {
            pc.hour += 12;
        }
        if (!pc.is_pm && pc.hour == 12) {
            pc.hour = 0;
        }
    }

    // Validate: consider the parse successful if at least one meaningful
    // component was extracted (year, month, day, hour, etc.).
    // For time-only formats (no year/month/day), default to epoch date.
    bool hasAnyComponent = (pc.year >= 0) || (pc.month >= 0) ||
                           (pc.hour > 0) || (pc.minute > 0) || (pc.second > 0) ||
                           pc.has_meridiem || pc.has_offset;
    if (hasAnyComponent) {
        pc.valid = true;
    }
    // Default missing components
    if (pc.year < 0) {
        pc.year = 1970; // Default to epoch year for time-only formats
    }
    if (pc.month < 0) {
        pc.month = 0;
    }

    // Check in strict mode that the entire input was consumed
    if (strict && pos != input.size()) {
        pc.valid = false;
    }

    return pc;
}

// ── Moment construction from parsed components ───────────────────────

/// @brief Build a Moment from ParsedComponents.
/// If has_offset is true, the Moment is created with a fixed UTC offset.
/// Otherwise, it's created in local time.
inline Moment buildMoment(const ParsedComponents& pc, bool forceUtc = false) {
    using Access = Moment::InternalAccess;

    if (!pc.valid) {
        return Access::makeInvalid();
    }

    Moment m(0);

    if (forceUtc || pc.is_utc) {
        // Create in UTC
        m = Access::fromUtcComponents(pc.year, pc.month, pc.day,
                                       pc.hour, pc.minute, pc.second, pc.millisecond);
        Access::setUtc(m, true);
    } else if (pc.has_offset) {
        // Create in UTC, then set fixed offset
        // The components represent wall-clock time at the given offset.
        // Convert to UTC: subtract the offset.
        m = Access::fromUtcComponents(pc.year, pc.month, pc.day,
                                       pc.hour, pc.minute, pc.second, pc.millisecond);
        // Adjust timestamp: components are at offset time, so subtract offset to get UTC
        int64_t ts = Access::getTimestamp(m);
        ts -= static_cast<int64_t>(pc.offset_minutes) * 60000LL;
        Access::setTimestamp(m, ts);
        Access::setOffset(m, pc.offset_minutes);
    } else {
        // Create in local time
        m = Access::fromLocalComponents(pc.year, pc.month, pc.day,
                                         pc.hour, pc.minute, pc.second, pc.millisecond);
    }

    return m;
}

} // namespace detail

// ── Factory function implementations ─────────────────────────────────

// ── parse(input) — auto-detect format ────────────────────────────────

inline Moment parse(const std::string& input) {
    if (input.empty()) {
        return Moment::InternalAccess::makeInvalid();
    }

    // Try ISO 8601 first
    auto pc = detail::parseISO8601(input);
    if (pc.valid) {
        return detail::buildMoment(pc);
    }

    // Try RFC 2822
    pc = detail::parseRFC2822(input);
    if (pc.valid) {
        return detail::buildMoment(pc);
    }

    // Nothing matched
    return Moment::InternalAccess::makeInvalid();
}

// ── parse(input, format) ─────────────────────────────────────────────

inline Moment parse(const std::string& input, const std::string& format) {
    return parse(input, format, false);
}

// ── parse(input, format, strict) ─────────────────────────────────────

inline Moment parse(const std::string& input, const std::string& format, bool strict) {
    if (input.empty()) {
        return Moment::InternalAccess::makeInvalid();
    }

    const auto& loc = localeData();
    auto pc = detail::parseWithFormat(input, format, strict, loc);
    if (pc.valid) {
        return detail::buildMoment(pc);
    }

    return Moment::InternalAccess::makeInvalid();
}

// ── parse(input, formats) — try multiple formats ─────────────────────

inline Moment parse(const std::string& input, const std::vector<std::string>& formats) {
    // Try each format in strict mode first (moment.js behavior: pick the best match).
    // Strict mode ensures the input is fully consumed by the format.
    for (const auto& fmt : formats) {
        Moment m = parse(input, fmt, true);
        if (m.isValid()) return m;
    }
    // Fall back to lenient mode
    for (const auto& fmt : formats) {
        Moment m = parse(input, fmt, false);
        if (m.isValid()) return m;
    }
    return Moment::InternalAccess::makeInvalid();
}

// ── fromUnixTimestamp(seconds) ───────────────────────────────────────

inline Moment fromUnixTimestamp(int64_t seconds) {
    return Moment(seconds * 1000LL);
}

// ── fromMilliseconds(ms) ─────────────────────────────────────────────

inline Moment fromMilliseconds(int64_t ms) {
    return Moment(ms);
}

// ── fromDate(components, local time) ─────────────────────────────────

inline Moment fromDate(int year, int month, int day, int hour,
                        int minute, int second, int ms) {
    return Moment::InternalAccess::fromLocalComponents(year, month, day,
                                                        hour, minute, second, ms);
}

// ── utcNow() ─────────────────────────────────────────────────────────

inline Moment utcNow() {
    Moment m;
    m.utc();
    return m;
}

// ── utcFromString(input) ─────────────────────────────────────────────

inline Moment utcFromString(const std::string& input) {
    Moment m = parse(input);
    if (m.isValid()) {
        // If no offset was parsed, treat as UTC
        if (!Moment::InternalAccess::getHasFixedOffset(m) &&
            !Moment::InternalAccess::getIsUtc(m)) {
            m.utc();
        }
    }
    return m;
}

// ── utcFromFormat(input, format) ─────────────────────────────────────

inline Moment utcFromFormat(const std::string& input, const std::string& format) {
    if (input.empty()) {
        return Moment::InternalAccess::makeInvalid();
    }
    const auto& loc = localeData();
    auto pc = detail::parseWithFormat(input, format, false, loc);
    if (pc.valid) {
        return detail::buildMoment(pc, true); // force UTC
    }
    return Moment::InternalAccess::makeInvalid();
}

// ── utcFromMs(ms) ────────────────────────────────────────────────────

inline Moment utcFromMs(int64_t ms) {
    Moment m(ms);
    m.utc();
    return m;
}

// ── utcFromDate(components, UTC) ─────────────────────────────────────

inline Moment utcFromDate(int year, int month, int day, int hour,
                           int minute, int second, int ms) {
    Moment m = Moment::InternalAccess::fromUtcComponents(year, month, day,
                                                          hour, minute, second, ms);
    Moment::InternalAccess::setUtc(m, true);
    return m;
}

// ── parseZone(input) — preserve the parsed offset ────────────────────

inline Moment parseZone(const std::string& input) {
    if (input.empty()) {
        return Moment::InternalAccess::makeInvalid();
    }

    // Try ISO 8601
    auto pc = detail::parseISO8601(input);
    if (!pc.valid) {
        // Try RFC 2822
        pc = detail::parseRFC2822(input);
    }

    if (pc.valid) {
        Moment m = detail::buildMoment(pc);
        if (pc.has_offset && !pc.is_utc) {
            Moment::InternalAccess::setOffset(m, pc.offset_minutes);
        } else if (pc.is_utc) {
            Moment::InternalAccess::setUtc(m, true);
        }
        return m;
    }

    return Moment::InternalAccess::makeInvalid();
}

// ── parseZone(input, format) ─────────────────────────────────────────

inline Moment parseZone(const std::string& input, const std::string& format) {
    if (input.empty()) {
        return Moment::InternalAccess::makeInvalid();
    }

    const auto& loc = localeData();
    auto pc = detail::parseWithFormat(input, format, false, loc);

    if (pc.valid) {
        Moment m = detail::buildMoment(pc);
        if (pc.has_offset && !pc.is_utc) {
            Moment::InternalAccess::setOffset(m, pc.offset_minutes);
        } else if (pc.is_utc) {
            Moment::InternalAccess::setUtc(m, true);
        }
        return m;
    }

    return Moment::InternalAccess::makeInvalid();
}

// ── invalid() ────────────────────────────────────────────────────────

inline Moment invalid() {
    return Moment::InternalAccess::makeInvalid();
}

// ── min/max ──────────────────────────────────────────────────────────

inline Moment min(std::initializer_list<Moment> moments) {
    if (moments.size() == 0) return Moment::InternalAccess::makeInvalid();

    const Moment* best = nullptr;
    for (const auto& m : moments) {
        if (!m.isValid()) continue;
        if (!best || m.valueOf() < best->valueOf()) {
            best = &m;
        }
    }
    return best ? *best : Moment::InternalAccess::makeInvalid();
}

inline Moment max(std::initializer_list<Moment> moments) {
    if (moments.size() == 0) return Moment::InternalAccess::makeInvalid();

    const Moment* best = nullptr;
    for (const auto& m : moments) {
        if (!m.isValid()) continue;
        if (!best || m.valueOf() > best->valueOf()) {
            best = &m;
        }
    }
    return best ? *best : Moment::InternalAccess::makeInvalid();
}

inline Moment min(const std::vector<Moment>& moments) {
    if (moments.empty()) return Moment::InternalAccess::makeInvalid();

    const Moment* best = nullptr;
    for (const auto& m : moments) {
        if (!m.isValid()) continue;
        if (!best || m.valueOf() < best->valueOf()) {
            best = &m;
        }
    }
    return best ? *best : Moment::InternalAccess::makeInvalid();
}

inline Moment max(const std::vector<Moment>& moments) {
    if (moments.empty()) return Moment::InternalAccess::makeInvalid();

    const Moment* best = nullptr;
    for (const auto& m : moments) {
        if (!m.isValid()) continue;
        if (!best || m.valueOf() > best->valueOf()) {
            best = &m;
        }
    }
    return best ? *best : Moment::InternalAccess::makeInvalid();
}

} // namespace moment
} // namespace polycpp
