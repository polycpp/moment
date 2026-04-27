/**
 * @file detail/parse.hpp
 * @brief Parse implementation — creates Moment objects from string input.
 *
 * ## Parsing pipeline
 *
 * ### Auto-detection (no format specified):
 * 1. Try ISO 8601 parsing via regex.
 * 2. Try RFC 2822 parsing via regex.
 * 3. Try deterministic `polycpp::Date::parse` fallback formats.
 * 4. If none matches, return an invalid Moment.
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
 * Following Moment.js: 69-99 maps to 1969-1999, 00-68 maps to 2000-2068.
 *
 * @since 1.0.0
 */
#pragma once

#include <polycpp/moment/moment.hpp>
#include <polycpp/moment/locale.hpp>
#include <polycpp/moment/detail/moment.hpp>
#include <polycpp/moment/detail/format.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <regex>
#include <string>
#include <utility>
#include <vector>
#include <polycpp/core/date.hpp>
#include <polycpp/core/number.hpp>

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
    int day_of_year = 0;     // Day-of-year parsed from DDD/DDDD/DDDo
    int offset_minutes = 0;  // UTC offset in minutes
    bool has_year = false;    // Whether a year token was parsed
    bool has_month = false;   // Whether a month token was parsed
    bool has_day = false;     // Whether a day/date token was parsed
    bool has_hour = false;    // Whether an hour token was parsed
    bool has_minute = false;  // Whether a minute token was parsed
    bool has_second = false;  // Whether a second token was parsed
    bool has_millisecond = false; // Whether a millisecond token was parsed
    bool has_day_of_year = false; // Whether a day-of-year token was parsed
    bool has_offset = false;  // Whether an offset was parsed
    bool is_utc = false;      // Whether 'Z' was found
    bool valid = false;       // Whether parsing succeeded
    bool is_pm = false;       // Whether PM was detected (for 12h parsing)
    bool has_meridiem = false; // Whether a/A was parsed
    std::string meridiem;      // Parsed meridiem text for locale meridiemHour hooks
    bool has_era = false;      // Whether an era token was parsed
    bool has_era_year = false; // Whether a lower-case era-year token was parsed
    EraSpec era;               // Parsed era metadata
    int iso_week_year = -1;    // ISO week-numbering year from G/GG/GGGG tokens
    int iso_week = -1;         // ISO week number from W/WW tokens
    int iso_weekday = 1;       // ISO weekday, defaulting to Monday
    bool has_iso_week_year = false;
    bool has_iso_week = false;
    bool has_iso_weekday = false;
    int week_year = -1;        // Locale week-numbering year from g/gg/gggg tokens
    int week = -1;             // Locale week number from w/ww tokens
    int weekday = -1;          // Absolute weekday from d/dd/ddd/dddd tokens (0=Sunday)
    int local_weekday = -1;    // Locale weekday from e token (0=first day of week)
    bool has_week_year = false;
    bool has_week = false;
    bool has_weekday = false;
    bool has_local_weekday = false;
    MomentParsingFlags flags;
};

inline std::vector<int> parsedDateParts(const ParsedComponents& pc) {
    std::vector<int> parts;
    if (pc.has_year) parts.push_back(pc.year);
    if (pc.has_month) parts.push_back(pc.month);
    if (pc.has_day) parts.push_back(pc.day);
    if (pc.has_hour) parts.push_back(pc.hour);
    if (pc.has_minute) parts.push_back(pc.minute);
    if (pc.has_second) parts.push_back(pc.second);
    if (pc.has_millisecond) parts.push_back(pc.millisecond);
    return parts;
}

inline int componentOverflow(const ParsedComponents& pc) {
    if (pc.month < 0 || pc.month > 11) return 1;
    if (pc.day < 1 || pc.day > daysInMonth(pc.year, pc.month)) return 2;
    if (pc.hour < 0 || pc.hour > 24 ||
        (pc.hour == 24 && (pc.minute != 0 || pc.second != 0 || pc.millisecond != 0))) {
        return 3;
    }
    if (pc.minute < 0 || pc.minute > 59) return 4;
    if (pc.second < 0 || pc.second > 59) return 5;
    if (pc.millisecond < 0 || pc.millisecond > 999) return 6;
    return -1;
}

inline MomentParsingFlags finalizedFlags(const ParsedComponents& pc) {
    MomentParsingFlags flags = pc.flags;
    flags.parsedDateParts = parsedDateParts(pc);
    if (!pc.meridiem.empty()) {
        flags.meridiem = pc.meridiem;
    }
    if (pc.has_era) {
        flags.era = !pc.era.abbr.empty() ? pc.era.abbr : pc.era.name;
    }
    if (flags.overflow == -2 && pc.valid) {
        flags.overflow = componentOverflow(pc);
    }
    return flags;
}

// ── Two-digit year interpretation ────────────────────────────────────

/// @brief Convert a 2-digit year to a 4-digit year.
/// 69-99 -> 1969-1999, 00-68 -> 2000-2068 (matching moment.js).
inline int expandTwoDigitYear(int yy) {
    return yy > 68 ? 1900 + yy : 2000 + yy;
}

// ── Fractional seconds normalization ─────────────────────────────────

/// @brief Convert a fractional seconds string (1-9 digits) to milliseconds.
/// "1" -> 100, "12" -> 120, "123" -> 123, "1234" -> 123, "123456789" -> 123.
inline int fracToMs(const std::string& frac) {
    if (frac.empty()) return 0;
    // Pad or truncate to exactly 3 digits for milliseconds
    std::string s = frac;
    while (s.size() < 3) s += '0';
    return polycpp::Number::parseInt(s.substr(0, 3));
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
    pc.flags.iso = true;
    pc.year = polycpp::Number::parseInt(match[1].str());
    pc.has_year = true;

    if (match[2].matched) {
        pc.month = polycpp::Number::parseInt(match[2].str()) - 1; // Convert to 0-based
        pc.has_month = true;
    } else {
        pc.month = 0; // Default January
    }

    if (match[3].matched) {
        pc.day = polycpp::Number::parseInt(match[3].str());
        pc.has_day = true;
    }

    if (match[4].matched) {
        pc.hour = polycpp::Number::parseInt(match[4].str());
        pc.has_hour = true;
    }
    if (match[5].matched) {
        pc.minute = polycpp::Number::parseInt(match[5].str());
        pc.has_minute = true;
    }
    if (match[6].matched) {
        pc.second = polycpp::Number::parseInt(match[6].str());
        pc.has_second = true;
    }
    if (match[7].matched) {
        pc.millisecond = fracToMs(match[7].str());
        pc.has_millisecond = true;
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
        int hrs = polycpp::Number::parseInt(match[10].str());
        int mins = match[11].matched ? polycpp::Number::parseInt(match[11].str()) : 0;
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
    pc.flags.rfc2822 = true;
    pc.day = polycpp::Number::parseInt(match[1].str());
    pc.has_day = true;
    pc.month = monthIdx;
    pc.has_month = true;
    pc.year = polycpp::Number::parseInt(match[3].str());
    pc.has_year = true;
    pc.hour = polycpp::Number::parseInt(match[4].str());
    pc.has_hour = true;
    pc.minute = polycpp::Number::parseInt(match[5].str());
    pc.has_minute = true;
    if (match[6].matched) {
        pc.second = polycpp::Number::parseInt(match[6].str());
        pc.has_second = true;
    }

    if (match[7].matched) {
        pc.has_offset = true;
        int sign = match[7].str() == "+" ? 1 : -1;
        pc.offset_minutes = sign * (polycpp::Number::parseInt(match[8].str()) * 60 + polycpp::Number::parseInt(match[9].str()));
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
    value = polycpp::Number::parseInt(input.substr(start, count));
    pos = end;
    return true;
}

inline bool matchSignedNumber(const std::string& input, size_t& pos,
                              int minDigits, int maxDigits, int& value) {
    size_t start = pos;
    int sign = 1;
    if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) {
        sign = input[pos] == '-' ? -1 : 1;
        ++pos;
    }

    size_t digitsStart = pos;
    while (pos < input.size() && pos - digitsStart < static_cast<size_t>(maxDigits) &&
           std::isdigit(static_cast<unsigned char>(input[pos]))) {
        ++pos;
    }

    int count = static_cast<int>(pos - digitsStart);
    if (count < minDigits) {
        pos = start;
        return false;
    }

    value = sign * polycpp::Number::parseInt(input.substr(digitsStart, count));
    return true;
}

inline bool matchDigitString(const std::string& input, size_t& pos,
                             int minDigits, int maxDigits, std::string& value) {
    size_t start = pos;
    while (pos < input.size() && pos - start < static_cast<size_t>(maxDigits) &&
           std::isdigit(static_cast<unsigned char>(input[pos]))) {
        ++pos;
    }

    int count = static_cast<int>(pos - start);
    if (count < minDigits) {
        pos = start;
        return false;
    }

    value = input.substr(start, count);
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

inline bool startsWithLiteral(const std::string& input, size_t pos, const std::string& literal) {
    if (pos + literal.size() > input.size()) return false;
    for (size_t i = 0; i < literal.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(input[pos + i])) !=
            std::tolower(static_cast<unsigned char>(literal[i]))) {
            return false;
        }
    }
    return true;
}

inline bool matchLocaleParseEntry(const std::string& input, size_t& pos,
                                  const std::vector<LocaleParseEntry>& entries,
                                  bool strict, int& value) {
    const LocaleParseEntry* best = nullptr;
    size_t bestLen = 0;

    for (const auto& entry : entries) {
        if (entry.text.empty() || (strict && !entry.strict)) {
            continue;
        }
        if (entry.text.size() > bestLen && startsWithLiteral(input, pos, entry.text)) {
            best = &entry;
            bestLen = entry.text.size();
        }
    }

    if (!best) {
        return false;
    }

    value = best->value;
    pos += bestLen;
    return true;
}

inline bool matchUnsignedNumber(const std::string& input, size_t& pos, int& value) {
    size_t start = pos;
    while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
        ++pos;
    }
    if (pos == start) return false;
    value = polycpp::Number::parseInt(input.substr(start, pos - start));
    return true;
}

inline const std::vector<EraSpec>& parseErasForLocale(const LocaleData& locale) {
    if (!locale.eras.empty()) {
        return locale.eras;
    }

    const auto& english = localeData("en");
    if (!english.eras.empty()) {
        return english.eras;
    }

    static const std::vector<EraSpec> empty;
    return empty;
}

inline bool matchEra(const std::string& input, size_t& pos, const LocaleData& locale,
                     const std::string& token, bool strict, ParsedComponents& pc) {
    const auto& eras = parseErasForLocale(locale);
    const EraSpec* bestEra = nullptr;
    size_t bestLen = 0;

    auto consider = [&](const EraSpec& era, const std::string& value) {
        if (!value.empty() && value.size() > bestLen && startsWithLiteral(input, pos, value)) {
            bestEra = &era;
            bestLen = value.size();
        }
    };

    for (const auto& era : eras) {
        if (strict) {
            if (token == "N" || token == "NN" || token == "NNN") {
                consider(era, era.abbr);
            } else if (token == "NNNN") {
                consider(era, era.name);
            } else if (token == "NNNNN") {
                consider(era, era.narrow);
            }
        } else {
            consider(era, era.name);
            consider(era, era.abbr);
            consider(era, era.narrow);
        }
    }

    if (!bestEra) {
        return false;
    }

    pc.era = *bestEra;
    pc.has_era = true;
    pos += bestLen;
    return true;
}

inline bool matchEraYearOrdinal(const std::string& input, size_t& pos,
                                const LocaleData& locale, int& value) {
    if (locale.eraYearOrdinalParse) {
        size_t parsedLength = 0;
        int parsed = locale.eraYearOrdinalParse(input.substr(pos), parsedLength);
        if (parsedLength > 0) {
            value = parsed;
            pos += parsedLength;
            return true;
        }
    }

    return matchUnsignedNumber(input, pos, value);
}

inline bool applyDayOfYear(ParsedComponents& pc, int year, int dayOfYear) {
    const int totalDays = isLeapYear(year) ? 366 : 365;
    if (dayOfYear < 1 || dayOfYear > totalDays) {
        return false;
    }

    int month = 0;
    int remaining = dayOfYear;
    while (month < 12) {
        int dim = daysInMonth(year, month);
        if (remaining <= dim) {
            pc.year = year;
            pc.month = month;
            pc.day = remaining;
            pc.has_year = true;
            pc.has_month = true;
            pc.has_day = true;
            return true;
        }
        remaining -= dim;
        ++month;
    }

    return false;
}

/// @brief Try to match a month name (long or short) from the locale.
/// Returns the 0-based month index, or -1 if no match.
inline int matchMonthName(const std::string& input, size_t& pos,
                           const LocaleData& locale, bool isShort, bool strict) {
    int parsed = -1;
    const auto& parseEntries = isShort ? locale.monthsShortParse : locale.monthsParse;
    if (matchLocaleParseEntry(input, pos, parseEntries, strict, parsed)) {
        return parsed;
    }

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

/// @brief Try to match a weekday name from the locale and return 0=Sunday.
inline bool matchWeekdayName(const std::string& input, size_t& pos,
                              const LocaleData& locale, int nameType, bool strict,
                              int& weekday) {
    // nameType: 0=min, 1=short, 2=full
    const auto* parseEntries = (nameType == 0) ? &locale.weekdaysMinParse :
                               (nameType == 1) ? &locale.weekdaysShortParse :
                                                 &locale.weekdaysParse;
    int parsed = -1;
    if (matchLocaleParseEntry(input, pos, *parseEntries, strict, parsed)) {
        weekday = parsed;
        return true;
    }

    const auto* names = (nameType == 0) ? &locale.weekdaysMin :
                        (nameType == 1) ? &locale.weekdaysShort :
                                          &locale.weekdays;
    int bestIndex = -1;
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
                bestIndex = i;
                bestLen = name.size();
            }
        }
    }
    if (bestLen > 0) {
        pos += bestLen;
        weekday = bestIndex;
        return true;
    }
    return false;
}

/// @brief Try to match a meridiem string (AM/PM/am/pm).
/// Sets pc.is_pm and pc.has_meridiem.
inline bool matchMeridiem(const std::string& input, size_t& pos,
                           const LocaleData& locale, ParsedComponents& pc) {
    int parsed = 0;
    size_t start = pos;
    if (matchLocaleParseEntry(input, pos, locale.meridiemParse, false, parsed)) {
        pc.is_pm = parsed != 0;
        pc.has_meridiem = true;
        pc.meridiem = input.substr(start, pos - start);
        return true;
    }

    static const std::pair<const char*, bool> defaultTokens[] = {
        {"a.m.", false},
        {"p.m.", true},
        {"am", false},
        {"pm", true},
        {"a", false},
        {"p", true},
    };
    for (const auto& [token, isPm] : defaultTokens) {
        if (startsWithLiteral(input, pos, token)) {
            pc.is_pm = isPm;
            pc.has_meridiem = true;
            pc.meridiem = token;
            pos += std::string(token).size();
            return true;
        }
    }

    return false;
}

inline bool matchOrdinalValue(const std::string& input, size_t& pos,
                              const std::vector<LocaleParseEntry>& entries,
                              bool strict, int minValue, int maxValue, int& value) {
    if (matchLocaleParseEntry(input, pos, entries, strict, value)) {
        return value >= minValue && value <= maxValue;
    }

    const int maxDigits = maxValue > 99 ? 3 : 2;
    if (!matchNumber(input, pos, 1, maxDigits, value)) {
        return false;
    }

    if (value < minValue || value > maxValue) {
        return false;
    }

    while (pos < input.size() &&
           (std::isalpha(static_cast<unsigned char>(input[pos])) || input[pos] == '.')) {
        ++pos;
    }

    return true;
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
inline ParsedComponents parseWithFormat(const std::string& rawInput,
                                         const std::string& format,
                                         bool strict,
                                         const LocaleData& locale) {
    ParsedComponents pc;
    std::string input = locale.preparse ? locale.preparse(rawInput) : rawInput;

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
            pc.has_year = true;
        }
        else if (tok == "Y") {
            if (!matchSignedNumber(input, pos, 1, 9, val)) { if (strict) return pc; continue; }
            pc.year = val;
            pc.has_year = true;
        }
        else if (tok == "YYYYY" || tok == "YYYYYY") {
            if (!matchSignedNumber(input, pos, 1, 6, val)) { if (strict) return pc; continue; }
            pc.year = val;
            pc.has_year = true;
        }
        else if (tok == "YY") {
            if (!matchNumber(input, pos, 2, 2, val)) { if (strict) return pc; continue; }
            pc.year = expandTwoDigitYear(val);
            pc.has_year = true;
        }
        else if (tok == "y" || tok == "yy" || tok == "yyy" || tok == "yyyy") {
            if (!matchUnsignedNumber(input, pos, val)) { if (strict) return pc; continue; }
            pc.year = val;
            pc.has_year = true;
            pc.has_era_year = true;
        }
        else if (tok == "yo") {
            if (!matchEraYearOrdinal(input, pos, locale, val)) { if (strict) return pc; continue; }
            pc.year = val;
            pc.has_year = true;
            pc.has_era_year = true;
        }
        // ── Era tokens ──
        else if (tok == "N" || tok == "NN" || tok == "NNN" || tok == "NNNN" || tok == "NNNNN") {
            if (!matchEra(input, pos, locale, tok, strict, pc)) {
                pc.flags.invalidEra = pos < input.size() ? input.substr(pos) : tok;
                if (strict) return pc;
            }
        }
        // ── Quarter ──
        else if (tok == "Q") {
            if (!matchNumber(input, pos, 1, 1, val)) { if (strict) return pc; continue; }
            // Quarter parsing: set month to start of quarter
            if (pc.month < 0) {
                pc.month = (val - 1) * 3;
                pc.has_month = true;
            }
        }
        // ── Month tokens ──
        else if (tok == "MM") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.month = val - 1; // Convert to 0-based
            pc.has_month = true;
        }
        else if (tok == "M") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.month = val - 1;
            pc.has_month = true;
        }
        else if (tok == "MMMM") {
            int idx = matchMonthName(input, pos, locale, false, strict);
            if (idx < 0) {
                pc.flags.invalidMonth = pos < input.size() ? input.substr(pos) : tok;
                if (strict) return pc;
                continue;
            }
            pc.month = idx;
            pc.has_month = true;
        }
        else if (tok == "MMM") {
            int idx = matchMonthName(input, pos, locale, true, strict);
            if (idx < 0) {
                pc.flags.invalidMonth = pos < input.size() ? input.substr(pos) : tok;
                if (strict) return pc;
                continue;
            }
            pc.month = idx;
            pc.has_month = true;
        }
        else if (tok == "Mo") {
            if (!matchOrdinalValue(input, pos, locale.monthOrdinalParse, strict, 1, 12, val)) {
                if (strict) return pc;
                continue;
            }
            pc.month = val - 1;
            pc.has_month = true;
        }
        // ── Day of month tokens ──
        else if (tok == "DD") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.day = val;
            pc.has_day = true;
        }
        else if (tok == "D") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.day = val;
            pc.has_day = true;
        }
        else if (tok == "Do") {
            if (!matchOrdinalValue(input, pos, locale.dayOfMonthOrdinalParse, strict, 1, 31, val)) {
                if (strict) return pc;
                continue;
            }
            pc.day = val;
            pc.has_day = true;
        }
        // ── Day of year tokens ──
        else if (tok == "DDDD") {
            if (!matchNumber(input, pos, 3, 3, val)) { if (strict) return pc; continue; }
            pc.day_of_year = val;
            pc.has_day_of_year = true;
        }
        else if (tok == "DDD") {
            if (!matchNumber(input, pos, 1, 3, val)) { if (strict) return pc; continue; }
            pc.day_of_year = val;
            pc.has_day_of_year = true;
        }
        // ── Day of week tokens (ignored for date calculation) ──
        else if (tok == "dddd") {
            if (!matchWeekdayName(input, pos, locale, 2, strict, val)) {
                pc.flags.invalidWeekday = pos < input.size() ? input.substr(pos) : tok;
                if (strict) return pc;
            } else {
                pc.weekday = val;
                pc.has_weekday = true;
            }
        }
        else if (tok == "ddd") {
            if (!matchWeekdayName(input, pos, locale, 1, strict, val)) {
                pc.flags.invalidWeekday = pos < input.size() ? input.substr(pos) : tok;
                if (strict) return pc;
            } else {
                pc.weekday = val;
                pc.has_weekday = true;
            }
        }
        else if (tok == "dd") {
            if (!matchWeekdayName(input, pos, locale, 0, strict, val)) {
                pc.flags.invalidWeekday = pos < input.size() ? input.substr(pos) : tok;
                if (strict) return pc;
            } else {
                pc.weekday = val;
                pc.has_weekday = true;
            }
        }
        else if (tok == "E") {
            if (!matchNumber(input, pos, 1, 1, val)) { if (strict) return pc; continue; }
            pc.iso_weekday = val;
            pc.has_iso_weekday = true;
        }
        else if (tok == "d") {
            if (!matchNumber(input, pos, 1, 1, val)) { if (strict) return pc; continue; }
            pc.weekday = val;
            pc.has_weekday = true;
        }
        else if (tok == "e") {
            if (!matchNumber(input, pos, 1, 1, val)) { if (strict) return pc; continue; }
            pc.local_weekday = val;
            pc.has_local_weekday = true;
        }
        // ── Hour tokens ──
        else if (tok == "HH") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.hour = val;
            pc.has_hour = true;
        }
        else if (tok == "H") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.hour = val;
            pc.has_hour = true;
        }
        else if (tok == "hh" || tok == "h") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.hour = val; // Will be adjusted for AM/PM later
            pc.has_hour = true;
        }
        else if (tok == "Hmm" || tok == "hmm") {
            std::string digits;
            if (!matchDigitString(input, pos, 3, 4, digits)) { if (strict) return pc; continue; }
            pc.hour = polycpp::Number::parseInt(digits.substr(0, digits.size() - 2));
            pc.minute = polycpp::Number::parseInt(digits.substr(digits.size() - 2));
            pc.has_hour = true;
            pc.has_minute = true;
        }
        else if (tok == "Hmmss" || tok == "hmmss") {
            std::string digits;
            if (!matchDigitString(input, pos, 5, 6, digits)) { if (strict) return pc; continue; }
            pc.hour = polycpp::Number::parseInt(digits.substr(0, digits.size() - 4));
            pc.minute = polycpp::Number::parseInt(digits.substr(digits.size() - 4, 2));
            pc.second = polycpp::Number::parseInt(digits.substr(digits.size() - 2));
            pc.has_hour = true;
            pc.has_minute = true;
            pc.has_second = true;
        }
        else if (tok == "kk" || tok == "k") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.hour = val == 24 ? 0 : val;
            pc.has_hour = true;
        }
        // ── Minute tokens ──
        else if (tok == "mm") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.minute = val;
            pc.has_minute = true;
        }
        else if (tok == "m") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.minute = val;
            pc.has_minute = true;
        }
        // ── Second tokens ──
        else if (tok == "ss") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.second = val;
            pc.has_second = true;
        }
        else if (tok == "s") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.second = val;
            pc.has_second = true;
        }
        // ── Fractional second tokens ──
        else if (!tok.empty() && tok[0] == 'S' && tok.find_first_not_of('S') == std::string::npos) {
            std::string digits;
            if (!matchDigitString(input, pos, 1, static_cast<int>(tok.size()), digits)) {
                if (strict) return pc;
                continue;
            }
            pc.millisecond = fracToMs(digits);
            pc.has_millisecond = true;
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
                double secs = polycpp::Number::parseFloat(input.substr(start, pos - start));
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
                pc.has_year = true;
                pc.month = temp.month();
                pc.has_month = true;
                pc.day = temp.date();
                pc.has_day = true;
                pc.hour = temp.hour();
                pc.has_hour = true;
                pc.minute = temp.minute();
                pc.has_minute = true;
                pc.second = temp.second();
                pc.has_second = true;
                pc.millisecond = temp.millisecond();
                pc.has_millisecond = true;
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
                pc.has_year = true;
                pc.month = temp.month();
                pc.has_month = true;
                pc.day = temp.date();
                pc.has_day = true;
                pc.hour = temp.hour();
                pc.has_hour = true;
                pc.minute = temp.minute();
                pc.has_minute = true;
                pc.second = temp.second();
                pc.has_second = true;
                pc.millisecond = temp.millisecond();
                pc.has_millisecond = true;
                pc.valid = true;
                pc.has_offset = true;
                pc.is_utc = true;
            } else {
                if (strict) return pc;
            }
        }
        // ── Week tokens ──
        else if (tok == "WW" || tok == "W") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.iso_week = val;
            pc.has_iso_week = true;
        }
        else if (tok == "ww" || tok == "w") {
            if (!matchNumber(input, pos, 1, 2, val)) { if (strict) return pc; continue; }
            pc.week = val;
            pc.has_week = true;
        }
        else if (tok == "G") {
            if (!matchSignedNumber(input, pos, 1, 9, val)) { if (strict) return pc; continue; }
            pc.iso_week_year = val;
            pc.has_iso_week_year = true;
        }
        else if (tok == "GGGG") {
            if (!matchNumber(input, pos, 4, 4, val)) { if (strict) return pc; continue; }
            pc.iso_week_year = val;
            pc.has_iso_week_year = true;
        }
        else if (tok == "GGGGG") {
            if (!matchSignedNumber(input, pos, 1, 6, val)) { if (strict) return pc; continue; }
            pc.iso_week_year = val;
            pc.has_iso_week_year = true;
        }
        else if (tok == "GG") {
            if (!matchNumber(input, pos, 2, 2, val)) { if (strict) return pc; continue; }
            pc.iso_week_year = expandTwoDigitYear(val);
            pc.has_iso_week_year = true;
        }
        else if (tok == "g") {
            if (!matchSignedNumber(input, pos, 1, 9, val)) { if (strict) return pc; continue; }
            pc.week_year = val;
            pc.has_week_year = true;
        }
        else if (tok == "gggg" || tok == "ggggg") {
            if (!matchSignedNumber(input, pos, 1, 6, val)) { if (strict) return pc; continue; }
            pc.week_year = val;
            pc.has_week_year = true;
        }
        else if (tok == "gg") {
            if (!matchNumber(input, pos, 2, 2, val)) { if (strict) return pc; continue; }
            pc.week_year = expandTwoDigitYear(val);
            pc.has_week_year = true;
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

    // Apply era conversion after all tokens have populated the year field.
    if (pc.has_era) {
        pc.year = convertEraYearToGregorian(pc.era, pc.year, pc.has_era_year || pc.has_year);
        pc.has_year = true;
    }

    if ((pc.has_iso_week_year || pc.has_iso_week || pc.has_iso_weekday) &&
        pc.month < 0 && !pc.has_day && !pc.has_day_of_year) {
        const int weekYear = pc.has_iso_week_year ? pc.iso_week_year :
                             (pc.has_year ? pc.year : 1970);
        const int week = pc.has_iso_week ? pc.iso_week : 1;
        const int weekday = pc.has_iso_weekday ? pc.iso_weekday : 1;
        const int maxWeeks = weeksInYear(weekYear, 1, 4);
        if (week < 1 || week > maxWeeks || weekday < 1 || weekday > 7) {
            return pc;
        }

        const auto result = dayOfYearFromWeeks(weekYear, week, weekday, 1, 4);
        if (!applyDayOfYear(pc, result.year, result.dayOfYear)) {
            return pc;
        }
    }

    if ((pc.has_week_year || pc.has_week || pc.has_weekday || pc.has_local_weekday) &&
        pc.month < 0 && !pc.has_day && !pc.has_day_of_year &&
        !(pc.has_iso_week_year || pc.has_iso_week || pc.has_iso_weekday)) {
        const int dow = locale.week.dow;
        const int doy = locale.week.doy;
        Moment current;
        current.locale(locale.name.empty() ? globalLocale() : locale.name);
        const auto currentWeek = weekOfYear(current.year(), current.dayOfYear(), dow, doy);
        const int weekYear = pc.has_week_year ? pc.week_year :
                             (pc.has_year ? pc.year : currentWeek.year);
        const int week = pc.has_week ? pc.week : currentWeek.week;
        int weekday = dow;
        if (pc.has_weekday) {
            if (pc.weekday < 0 || pc.weekday > 6) {
                return pc;
            }
            weekday = pc.weekday;
        } else if (pc.has_local_weekday) {
            if (pc.local_weekday < 0 || pc.local_weekday > 6) {
                return pc;
            }
            weekday = pc.local_weekday + dow;
        }

        const int maxWeeks = weeksInYear(weekYear, dow, doy);
        if (week < 1 || week > maxWeeks) {
            return pc;
        }

        const auto result = dayOfYearFromWeeks(weekYear, week, weekday, dow, doy);
        if (!applyDayOfYear(pc, result.year, result.dayOfYear)) {
            return pc;
        }
    }

    // Apply AM/PM adjustment
    if (pc.has_meridiem) {
        if (locale.meridiemHour && !pc.meridiem.empty()) {
            pc.hour = locale.meridiemHour(pc.hour, pc.meridiem);
        } else if (pc.is_pm && pc.hour < 12) {
            pc.hour += 12;
        }
        if (!locale.meridiemHour && !pc.is_pm && pc.hour == 12) {
            pc.hour = 0;
        }
    }

    // Validate: consider the parse successful if at least one meaningful
    // component was extracted (year, month, day, hour, etc.).
    // For time-only formats (no year/month/day), default to epoch date.
    bool hasAnyComponent = pc.has_year || (pc.month >= 0) ||
                           (pc.hour > 0) || (pc.minute > 0) || (pc.second > 0) ||
                           pc.has_day || pc.has_day_of_year ||
                           pc.has_hour || pc.has_minute || pc.has_second ||
                           pc.has_millisecond ||
                           pc.has_meridiem || pc.has_offset || pc.has_era ||
                           pc.has_week_year || pc.has_week ||
                           pc.has_weekday || pc.has_local_weekday ||
                           pc.has_iso_week_year || pc.has_iso_week || pc.has_iso_weekday;
    if (hasAnyComponent) {
        pc.valid = true;
    }
    // Default missing components
    if (!pc.has_year) {
        pc.year = 1970; // Default to epoch year for time-only formats
    }
    if (pc.has_day_of_year) {
        if (!applyDayOfYear(pc, pc.year, pc.day_of_year)) {
            pc.flags.overflow = 2;
            return pc;
        }
    }
    if (pc.month < 0) {
        pc.month = 0;
    }

    if (pc.valid && (pc.has_weekday || pc.has_local_weekday || pc.has_iso_weekday)) {
        const int actualWeekday = dayOfWeekFromDate(pc.year, pc.month, pc.day);
        if (pc.has_weekday && pc.weekday != actualWeekday) {
            pc.flags.weekdayMismatch = true;
            pc.valid = false;
        }
        if (pc.has_local_weekday) {
            const int dow = locale.week.dow;
            const int actualLocalWeekday = (7 + actualWeekday - dow) % 7;
            if (pc.local_weekday != actualLocalWeekday) {
                pc.flags.weekdayMismatch = true;
                pc.valid = false;
            }
        }
        if (pc.has_iso_weekday) {
            const int actualIsoWeekday = actualWeekday == 0 ? 7 : actualWeekday;
            if (pc.iso_weekday != actualIsoWeekday) {
                pc.flags.weekdayMismatch = true;
                pc.valid = false;
            }
        }
    }

    // Check in strict mode that the entire input was consumed
    if (strict && pos != input.size()) {
        pc.valid = false;
        pc.flags.charsLeftOver = static_cast<int>(input.size() - pos);
        if (pos < input.size()) {
            pc.flags.unusedInput.push_back(input.substr(pos));
        }
    }

    return pc;
}

// ── Moment construction from parsed components ───────────────────────

/// @brief Build a Moment from ParsedComponents.
/// If has_offset is true, the Moment is created with a fixed UTC offset.
/// Otherwise, it's created in local time.
inline Moment buildMoment(const ParsedComponents& pc, bool forceUtc = false) {
    using Access = Moment::InternalAccess;
    MomentParsingFlags flags = finalizedFlags(pc);

    if (!pc.valid) {
        return Access::makeInvalid(flags);
    }

    if (flags.overflow >= 0 || flags.weekdayMismatch ||
        !flags.invalidEra.empty() || !flags.invalidMonth.empty() ||
        !flags.invalidWeekday.empty()) {
        return Access::makeInvalid(flags);
    }

    Moment m(0);

    if (forceUtc && pc.has_offset && !pc.is_utc) {
        m = Access::fromUtcComponents(pc.year, pc.month, pc.day,
                                       pc.hour, pc.minute, pc.second, pc.millisecond);
        int64_t ts = Access::getTimestamp(m);
        ts -= static_cast<int64_t>(pc.offset_minutes) * 60000LL;
        Access::setTimestamp(m, ts);
        Access::setUtc(m, true);
    } else if (forceUtc || pc.is_utc) {
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

    Access::setParsingFlags(m, flags);
    return m;
}

/// @brief Build a Moment from polycpp::Date::parse fallback formats.
inline Moment buildMomentFromDateParseFallback(const std::string& input) {
    double parsed = polycpp::Date::parse(input);
    if (std::isnan(parsed)) {
        return Moment::InternalAccess::makeInvalid();
    }

    return Moment(static_cast<int64_t>(parsed));
}

/// @brief Extract a GMT offset from polycpp::Date::toString()-style input.
inline bool extractGmtOffset(const std::string& input, int& offsetMinutes) {
    static const std::regex gmtOffsetRe(
        R"(.*\sGMT([+-])(\d{2})(\d{2})(?:\s*\([^)]*\))?$)",
        std::regex::optimize
    );

    std::smatch match;
    if (!std::regex_match(input, match, gmtOffsetRe)) {
        return false;
    }

    int sign = match[1].str() == "+" ? 1 : -1;
    int hours = polycpp::Number::parseInt(match[2].str());
    int minutes = polycpp::Number::parseInt(match[3].str());
    offsetMinutes = sign * (hours * 60 + minutes);
    return true;
}

inline bool extractInputOffset(const std::string& input, int& offsetMinutes) {
    if (extractGmtOffset(input, offsetMinutes)) {
        return true;
    }

    static const std::regex trailingOffsetRe(
        R"(.*(?:([Zz])|([+-])(\d{2})(?::?(\d{2}))?)\s*$)",
        std::regex::optimize
    );

    std::smatch match;
    if (!std::regex_match(input, match, trailingOffsetRe)) {
        return false;
    }

    if (match[1].matched) {
        offsetMinutes = 0;
        return true;
    }

    int sign = match[2].str() == "+" ? 1 : -1;
    int hours = polycpp::Number::parseInt(match[3].str());
    int minutes = match[4].matched ? polycpp::Number::parseInt(match[4].str()) : 0;
    offsetMinutes = sign * (hours * 60 + minutes);
    return true;
}

inline MomentCreationData makeCreationData(const std::string& input,
                                           const std::string& format,
                                           bool strict,
                                           const std::string& localeKey,
                                           bool isUtc) {
    MomentCreationData data;
    data.input = input;
    data.format = format;
    data.locale = localeKey;
    data.isUTC = isUtc;
    data.strict = strict;
    return data;
}

inline Moment withCreationData(Moment moment,
                               const std::string& input,
                               const std::string& format,
                               bool strict,
                               const std::string& localeKey,
                               bool isUtc) {
    Moment::InternalAccess::setCreationData(
        moment,
        makeCreationData(input, format, strict, localeKey, isUtc)
    );
    return moment;
}

inline bool isIsoSentinelFormat(const std::string& format) {
    return format == ISO_8601;
}

inline bool isRfc2822SentinelFormat(const std::string& format) {
    return format == RFC_2822;
}

inline ParsedComponents parseExplicitSentinelFormat(const std::string& input,
                                                    const std::string& format) {
    if (isIsoSentinelFormat(format)) {
        return parseISO8601(input);
    }
    if (isRfc2822SentinelFormat(format)) {
        return parseRFC2822(input);
    }
    return ParsedComponents{};
}

inline bool isExplicitSentinelFormat(const std::string& format) {
    return isIsoSentinelFormat(format) || isRfc2822SentinelFormat(format);
}

inline void applyParseZoneMode(Moment& moment, const ParsedComponents& pc) {
    if (pc.has_offset && !pc.is_utc) {
        Moment::InternalAccess::setOffset(moment, pc.offset_minutes);
    } else if (pc.is_utc) {
        Moment::InternalAccess::setUtc(moment, true);
    } else {
        moment.utcOffset(0, true);
    }
}

inline Moment invalidFromInput(const std::string& input,
                               const std::string& format,
                               bool strict,
                               const std::string& localeKey,
                               const MomentParsingFlags& flags = MomentParsingFlags{}) {
    Moment m = Moment::InternalAccess::makeInvalid(flags);
    return withCreationData(m, input, format, strict, localeKey, false);
}

} // namespace detail

// ── Factory function implementations ─────────────────────────────────

inline int parseTwoDigitYear(const std::string& input) {
    return detail::expandTwoDigitYear(polycpp::Number::parseInt(input));
}

inline Moment& Moment::parseZone() {
    if (!isValid()) {
        return *this;
    }

    int offsetMinutes = 0;
    if (detail::extractInputOffset(creation_data_.input, offsetMinutes)) {
        return utcOffset(offsetMinutes);
    }
    return utcOffset(0, true);
}

// ── parse(input) — auto-detect format ────────────────────────────────

inline Moment parse(const std::string& input) {
    const auto& loc = localeData();
    const std::string localeKey = loc.name.empty() ? globalLocale() : loc.name;

    if (input.empty()) {
        MomentParsingFlags flags;
        flags.nullInput = true;
        return detail::invalidFromInput(input, "", false, localeKey, flags);
    }

    const std::string parseInput = loc.preparse ? loc.preparse(input) : input;

    // Try ISO 8601 first
    auto pc = detail::parseISO8601(parseInput);
    if (pc.valid) {
        Moment m = detail::buildMoment(pc);
        return detail::withCreationData(m, input, "", false, localeKey, m.isUtcOffset());
    }

    // Try RFC 2822
    pc = detail::parseRFC2822(parseInput);
    if (pc.valid) {
        Moment m = detail::buildMoment(pc);
        return detail::withCreationData(m, input, "", false, localeKey, m.isUtcOffset());
    }

    // Fall back to the deterministic JavaScript Date parser provided by polycpp.
    // This covers Date::toString()/toUTCString() round trips without inheriting
    // host-specific native Date.parse quirks.
    Moment m = detail::buildMomentFromDateParseFallback(parseInput);
    return detail::withCreationData(m, input, "", false, localeKey, m.isUtcOffset());
}

// ── parse(input, format) ─────────────────────────────────────────────

inline Moment parse(const std::string& input, const std::string& format) {
    return parse(input, format, false);
}

// ── parse(input, format, strict) ─────────────────────────────────────

inline Moment parse(const std::string& input, const std::string& format, bool strict) {
    const auto& loc = localeData();
    const std::string localeKey = loc.name.empty() ? globalLocale() : loc.name;

    if (input.empty()) {
        MomentParsingFlags flags;
        flags.nullInput = true;
        return detail::invalidFromInput(input, format, strict, localeKey, flags);
    }

    if (detail::isExplicitSentinelFormat(format)) {
        const std::string parseInput = loc.preparse ? loc.preparse(input) : input;
        auto pc = detail::parseExplicitSentinelFormat(parseInput, format);
        Moment m = detail::buildMoment(pc);
        return detail::withCreationData(m, input, format, strict, localeKey, m.isUtcOffset());
    }

    auto pc = detail::parseWithFormat(input, format, strict, loc);
    if (pc.valid) {
        Moment m = detail::buildMoment(pc);
        return detail::withCreationData(m, input, format, strict, localeKey, m.isUtcOffset());
    }

    Moment m = detail::buildMoment(pc);
    return detail::withCreationData(m, input, format, strict, localeKey, false);
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
    const auto& loc = localeData();
    const std::string localeKey = loc.name.empty() ? globalLocale() : loc.name;
    MomentParsingFlags flags;
    flags.invalidFormat = formats.empty();
    return detail::invalidFromInput(input, "", false, localeKey, flags);
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
    auto data = Moment::InternalAccess::getCreationData(m);
    data.isUTC = true;
    Moment::InternalAccess::setCreationData(m, data);
    return m;
}

// ── utcFromFormat(input, format) ─────────────────────────────────────

inline Moment utcFromFormat(const std::string& input, const std::string& format) {
    const auto& loc = localeData();
    const std::string localeKey = loc.name.empty() ? globalLocale() : loc.name;

    if (input.empty()) {
        MomentParsingFlags flags;
        flags.nullInput = true;
        Moment m = detail::invalidFromInput(input, format, false, localeKey, flags);
        auto data = Moment::InternalAccess::getCreationData(m);
        data.isUTC = true;
        Moment::InternalAccess::setCreationData(m, data);
        return m;
    }

    if (detail::isExplicitSentinelFormat(format)) {
        const std::string parseInput = loc.preparse ? loc.preparse(input) : input;
        auto pc = detail::parseExplicitSentinelFormat(parseInput, format);
        Moment m = detail::buildMoment(pc, true);
        return detail::withCreationData(m, input, format, false, localeKey, true);
    }

    auto pc = detail::parseWithFormat(input, format, false, loc);
    if (pc.valid) {
        Moment m = detail::buildMoment(pc, true); // force UTC
        return detail::withCreationData(m, input, format, false, localeKey, true);
    }
    Moment m = detail::buildMoment(pc, true);
    return detail::withCreationData(m, input, format, false, localeKey, true);
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
    const auto& loc = localeData();
    const std::string localeKey = loc.name.empty() ? globalLocale() : loc.name;

    if (input.empty()) {
        MomentParsingFlags flags;
        flags.nullInput = true;
        return detail::invalidFromInput(input, "", false, localeKey, flags);
    }

    const std::string parseInput = loc.preparse ? loc.preparse(input) : input;

    // Try ISO 8601
    auto pc = detail::parseISO8601(parseInput);
    if (!pc.valid) {
        // Try RFC 2822
        pc = detail::parseRFC2822(parseInput);
    }

    if (pc.valid) {
        Moment m = detail::buildMoment(pc);
        detail::applyParseZoneMode(m, pc);
        return detail::withCreationData(m, input, "", false, localeKey, m.isUtcOffset());
    }

    Moment m = detail::buildMomentFromDateParseFallback(parseInput);
    if (m.isValid()) {
        int offsetMinutes = 0;
        if (detail::extractGmtOffset(parseInput, offsetMinutes)) {
            Moment::InternalAccess::setOffset(m, offsetMinutes);
        } else {
            m.utc();
        }
        return detail::withCreationData(m, input, "", false, localeKey, m.isUtcOffset());
    }

    return detail::withCreationData(m, input, "", false, localeKey, false);
}

// ── parseZone(input, format) ─────────────────────────────────────────

inline Moment parseZone(const std::string& input, const std::string& format) {
    const auto& loc = localeData();
    const std::string localeKey = loc.name.empty() ? globalLocale() : loc.name;

    if (input.empty()) {
        MomentParsingFlags flags;
        flags.nullInput = true;
        return detail::invalidFromInput(input, format, false, localeKey, flags);
    }

    if (detail::isExplicitSentinelFormat(format)) {
        const std::string parseInput = loc.preparse ? loc.preparse(input) : input;
        auto pc = detail::parseExplicitSentinelFormat(parseInput, format);
        Moment m = detail::buildMoment(pc);
        if (pc.valid) {
            detail::applyParseZoneMode(m, pc);
        }
        return detail::withCreationData(m, input, format, false, localeKey, m.isUtcOffset());
    }

    auto pc = detail::parseWithFormat(input, format, false, loc);

    if (pc.valid) {
        Moment m = detail::buildMoment(pc);
        detail::applyParseZoneMode(m, pc);
        return detail::withCreationData(m, input, format, false, localeKey, m.isUtcOffset());
    }

    Moment m = detail::buildMoment(pc);
    return detail::withCreationData(m, input, format, false, localeKey, false);
}

// ── invalid() ────────────────────────────────────────────────────────

inline Moment invalid() {
    MomentParsingFlags flags;
    flags.userInvalidated = true;
    Moment m = Moment::InternalAccess::makeInvalid(flags);
    const std::string localeKey = globalLocale();
    return detail::withCreationData(m, "", "", false, localeKey, true);
}

inline Moment invalid(const MomentParsingFlags& flags) {
    Moment m = Moment::InternalAccess::makeInvalid(flags);
    const std::string localeKey = globalLocale();
    return detail::withCreationData(m, "", "", false, localeKey, true);
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
