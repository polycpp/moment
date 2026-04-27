/**
 * @file locale.hpp
 * @brief Locale data structures and registry — C++ port of Moment.js locale system.
 *
 * Defines LocaleData and supporting types, plus free functions for managing
 * the global locale registry. Thread-safe for concurrent reads, exclusive
 * writes.
 *
 * @see https://momentjs.com/docs/#/i18n/
 * @since 1.0.0
 */
#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace polycpp {
namespace moment {

class Moment;

namespace detail {
#ifndef POLYCPP_MOMENT_DISABLE_GENERATED_LOCALES
void ensureGeneratedMomentLocalesRegistered();
#endif
} // namespace detail

// ── Callback type aliases ──────────────────────────────────────────────

/**
 * @brief Ordinal formatting function.
 * @param number  The number to ordinalize (e.g. 1, 2, 3).
 * @param token   The format token that triggered the call (e.g. "D", "M").
 * @return The ordinalized string (e.g. "1st", "2nd").
 * @since 1.0.0
 */
using OrdinalFn = std::function<std::string(int number, const std::string& token)>;

/**
 * @brief Meridiem formatting function.
 * @param hours   Hour of day (0-23).
 * @param minutes Minute of hour (0-59).
 * @param isLower If true, return lowercase ("am"/"pm"); otherwise uppercase.
 * @return The meridiem string.
 * @since 1.0.0
 */
using MeridiemFn = std::function<std::string(int hours, int minutes, bool isLower)>;

/**
 * @brief Check whether a meridiem string indicates PM.
 * @param input The meridiem string to check.
 * @return true if the string represents PM.
 * @since 1.0.0
 */
using IsPMFn = std::function<bool(const std::string& input)>;

/**
 * @brief Convert a 12-hour clock value plus locale meridiem text to a 24-hour value.
 * @param hour     Parsed 12-hour clock value.
 * @param meridiem Parsed meridiem text.
 * @return Hour of day (0-23).
 * @since 1.0.0
 */
using MeridiemHourFn = std::function<int(int hour, const std::string& meridiem)>;

/**
 * @brief Relative-time formatting function (for locale-specific forms).
 * @param number        The numeric value.
 * @param withoutSuffix If true, omit future/past wrapper.
 * @param key           The key string (e.g. "s", "mm", "h").
 * @param isFuture      If true, this is a future-relative time.
 * @return The formatted relative-time string.
 * @since 1.0.0
 */
using RelativeTimeFn = std::function<std::string(int number, bool withoutSuffix,
                                                  const std::string& key, bool isFuture)>;

/**
 * @brief A relative-time value: either a plain string or a function.
 * @since 1.0.0
 */
using RelativeTimeValue = std::variant<std::string, RelativeTimeFn>;

/**
 * @brief Calendar format value: a string template or a function returning one.
 *
 * @since 1.0.0
 */
using CalendarFn = std::function<std::string(const Moment& moment, const Moment& reference)>;

/**
 * @brief A calendar format entry: either a plain string or a function.
 * @since 1.0.0
 */
using CalendarValue = std::variant<std::string, CalendarFn>;

/**
 * @brief Pre/post format transform function.
 * @param str The string to transform.
 * @return The transformed string.
 * @since 1.0.0
 */
using PrePostFormatFn = std::function<std::string(const std::string& str)>;

/**
 * @brief Era-year ordinal parser.
 *
 * The function receives the remaining input at the era-year ordinal token and
 * writes the number of bytes consumed to @p parsedLength.
 *
 * @since 1.0.0
 */
using EraYearOrdinalParseFn = std::function<int(const std::string& input, size_t& parsedLength)>;

/**
 * @brief Rounding hook used by relative-time humanization.
 *
 * Matches `moment.relativeTimeRounding(fn)` at a typed C++ level.
 *
 * @since 1.0.0
 */
using RelativeTimeRoundingFn = std::function<double(double value)>;

/**
 * @brief Per-call relative-time threshold overrides for Duration::humanize().
 *
 * Empty fields fall back to the process-global thresholds. If `s` is set and
 * `ss` is not set, `ss` follows Moment.js behavior and becomes `s - 1`.
 *
 * @since 1.0.0
 */
struct RelativeTimeThresholds {
    std::optional<double> ss; ///< Upper bound for "a few seconds"
    std::optional<double> s;  ///< Upper bound before "a minute"
    std::optional<double> m;  ///< Upper bound before "an hour"
    std::optional<double> h;  ///< Upper bound before "a day"
    std::optional<double> d;  ///< Upper bound before "a month" or weeks
    std::optional<double> w;  ///< Upper bound for weeks; negative disables weeks
    std::optional<double> M;  ///< Upper bound before "a year"
};

// ── Data structures ────────────────────────────────────────────────────

/**
 * @brief Long date format strings keyed by token (LT, LTS, L, LL, LLL, LLLL).
 * @since 1.0.0
 */
struct LongDateFormats {
    std::string LT;   ///< Time only, e.g. "h:mm A"
    std::string LTS;  ///< Time with seconds, e.g. "h:mm:ss A"
    std::string L;    ///< Date (numeric), e.g. "MM/DD/YYYY"
    std::string LL;   ///< Date (full month), e.g. "MMMM D, YYYY"
    std::string LLL;  ///< Date + time, e.g. "MMMM D, YYYY h:mm A"
    std::string LLLL; ///< Full date + time, e.g. "dddd, MMMM D, YYYY h:mm A"
};

/**
 * @brief Calendar format strings for relative-day references.
 * @since 1.0.0
 */
struct CalendarFormats {
    CalendarValue sameDay;  ///< Format for today
    CalendarValue nextDay;  ///< Format for tomorrow
    CalendarValue nextWeek; ///< Format for next week
    CalendarValue lastDay;  ///< Format for yesterday
    CalendarValue lastWeek; ///< Format for last week
    CalendarValue sameElse; ///< Fallback format
};

/**
 * @brief Relative-time format strings/functions keyed by duration unit.
 * @since 1.0.0
 */
struct RelativeTimeFormats {
    std::string future; ///< Wrapper for future, e.g. "in %s"
    std::string past;   ///< Wrapper for past, e.g. "%s ago"
    RelativeTimeValue s;   ///< Seconds (singular threshold)
    RelativeTimeValue ss;  ///< Seconds (plural)
    RelativeTimeValue m;   ///< One minute
    RelativeTimeValue mm;  ///< Multiple minutes
    RelativeTimeValue h;   ///< One hour
    RelativeTimeValue hh;  ///< Multiple hours
    RelativeTimeValue d;   ///< One day
    RelativeTimeValue dd;  ///< Multiple days
    RelativeTimeValue w;   ///< One week
    RelativeTimeValue ww;  ///< Multiple weeks
    RelativeTimeValue M;   ///< One month
    RelativeTimeValue MM;  ///< Multiple months
    RelativeTimeValue y;   ///< One year
    RelativeTimeValue yy;  ///< Multiple years
};

/**
 * @brief Week configuration: first day of week and first-week-of-year rule.
 * @since 1.0.0
 */
struct WeekConfig {
    int dow = 0; ///< Day-of-week that starts the week (0=Sunday, 1=Monday, ...)
    int doy = 6; ///< The day-of-year threshold for the first week (6 = US, 4 = ISO)
};

/**
 * @brief Era range and labels, matching Moment.js locale era entries.
 *
 * Timestamps are milliseconds since the Unix epoch. `since` and `until` may be
 * ordered either ascending or descending; descending ranges model eras such as
 * BC where time moves backward from the `since` boundary.
 *
 * @since 1.0.0
 */
struct EraSpec {
    int64_t since = 0;       ///< Inclusive start boundary in milliseconds
    int64_t until = 0;       ///< Inclusive end boundary in milliseconds
    int offset = 1;          ///< Era-year offset at the since boundary
    std::string name;        ///< Full era name, e.g. "Anno Domini"
    std::string narrow;      ///< Narrow era label, e.g. "AD" or "㋿"
    std::string abbr;        ///< Abbreviated era label, e.g. "AD" or "R"
};

/**
 * @brief Locale-specific parse table entry.
 *
 * Generated locale data uses these entries for month names, weekday names,
 * meridiem labels, and ordinal forms that Moment.js locales accept through
 * their regex parser hooks. `strict` marks entries accepted by upstream strict
 * parsing; non-strict parsing may use both strict and lenient-only entries.
 *
 * @since 1.0.0
 */
struct LocaleParseEntry {
    std::string text; ///< Text accepted at the current parse position
    int value = 0;    ///< Parsed value (month index, weekday index, ordinal, or PM flag)
    bool strict = true; ///< Whether the text is accepted in strict parsing
};

/**
 * @brief Complete locale data for one locale.
 *
 * Mirrors Moment.js Locale prototype fields. All arrays use 0-based indexing
 * (months[0] = January, weekdays[0] = Sunday).
 *
 * @since 1.0.0
 */
struct LocaleData {
    std::string name; ///< Locale key (e.g. "en", "fr")

    // ── Month names ──
    std::array<std::string, 12> months;       ///< Full month names
    std::array<std::string, 12> monthsShort;  ///< Abbreviated month names
    std::array<std::string, 12> monthsStandalone; ///< Standalone full month names for global listers
    std::array<std::string, 12> monthsShortStandalone; ///< Standalone abbreviated month names

    // ── Weekday names ──
    std::array<std::string, 7> weekdays;      ///< Full weekday names (0=Sunday)
    std::array<std::string, 7> weekdaysShort; ///< Abbreviated weekday names
    std::array<std::string, 7> weekdaysMin;   ///< Minimal weekday names
    std::array<std::string, 7> weekdaysStandalone; ///< Standalone full weekday names for global listers
    std::array<std::string, 7> weekdaysFormat; ///< Format-context full weekday names
    std::string weekdaysFormatRegexSource; ///< Upstream weekday format-context regex source, if any

    // ── Locale parser data ──
    std::vector<LocaleParseEntry> monthsParse;      ///< Full-month parser entries
    std::vector<LocaleParseEntry> monthsShortParse; ///< Short-month parser entries
    std::vector<LocaleParseEntry> weekdaysParse;    ///< Full-weekday parser entries
    std::vector<LocaleParseEntry> weekdaysShortParse; ///< Short-weekday parser entries
    std::vector<LocaleParseEntry> weekdaysMinParse; ///< Minimal-weekday parser entries
    std::vector<LocaleParseEntry> meridiemParse;    ///< Meridiem parser entries; value 1 means PM
    std::vector<LocaleParseEntry> dayOfMonthOrdinalParse; ///< `Do` parser entries
    std::vector<LocaleParseEntry> monthOrdinalParse; ///< `Mo` parser entries

    // ── Formats ──
    LongDateFormats longDateFormat; ///< Long date format strings
    CalendarFormats calendar;       ///< Calendar-relative format strings
    RelativeTimeFormats relativeTime; ///< Relative time format strings

    // ── Week config ──
    WeekConfig week; ///< First day of week and first-week-of-year rule

    // ── Callbacks ──
    OrdinalFn ordinal;     ///< Ordinal formatter (e.g. 1 -> "1st")
    MeridiemFn meridiem;   ///< Meridiem formatter (e.g. 13,0,false -> "PM")
    IsPMFn isPM;           ///< PM detector
    MeridiemHourFn meridiemHour; ///< Optional locale-specific 12h-to-24h converter

    // ── Optional transforms ──
    PrePostFormatFn preparse;    ///< Pre-parse transform (identity by default)
    PrePostFormatFn postformat;  ///< Post-format transform (identity by default)

    // ── Era config ──
    std::vector<EraSpec> eras; ///< Locale-specific eras; empty means use English eras
    EraYearOrdinalParseFn eraYearOrdinalParse; ///< Optional parser for `yo`

    // ── Invalid date string ──
    std::string invalidDate = "Invalid date"; ///< String for invalid moments
};

// ── Locale registry free functions ─────────────────────────────────────

/**
 * @brief Register a new locale in the global registry.
 *
 * If a locale with the given name already exists, it is overwritten.
 *
 * @param name The locale key (e.g. "en", "fr").
 * @param data The complete locale data.
 * @since 1.0.0
 */
void defineLocale(const std::string& name, const LocaleData& data);

/**
 * @brief Update an existing locale, merging with defaults if it doesn't exist.
 *
 * @param name The locale key.
 * @param data The locale data to merge/replace.
 * @since 1.0.0
 */
void updateLocale(const std::string& name, const LocaleData& data);

/**
 * @brief Retrieve locale data by key.
 *
 * If key is empty, returns the current global locale data.
 * If the key is not found, returns the English ("en") locale.
 *
 * @param key The locale key to look up (default: current global locale).
 * @return A const reference to the locale data.
 * @since 1.0.0
 */
const LocaleData& localeData(const std::string& key = "");

/**
 * @brief List all registered locale keys.
 *
 * @return A sorted vector of locale key strings.
 * @since 1.0.0
 */
std::vector<std::string> locales();

/**
 * @brief List full month names for the current global locale.
 *
 * @return Month names indexed January=0 through December=11.
 * @since 1.0.0
 */
std::vector<std::string> months();

/**
 * @brief Get one full month name for the current global locale.
 *
 * @param index Month index. Values are normalized modulo 12.
 * @return The month name.
 * @since 1.0.0
 */
std::string months(int index);

/**
 * @brief List full month names using Moment-style format context.
 *
 * Some locales distinguish standalone month names from names used in dates
 * such as "D MMMM". The format string selects the matching context.
 *
 * @param format Moment-style format context.
 * @return Month names indexed January=0 through December=11.
 * @since 1.0.0
 */
std::vector<std::string> months(const std::string& format);
/// @brief List full month names using a string-literal format context.
std::vector<std::string> months(const char* format);

/**
 * @brief Get one full month name using Moment-style format context.
 *
 * @param format Moment-style format context.
 * @param index Month index. Values are normalized modulo 12.
 * @return The month name.
 * @since 1.0.0
 */
std::string months(const std::string& format, int index);
/// @brief Get one full month name using a string-literal format context.
std::string months(const char* format, int index);

/**
 * @brief List abbreviated month names for the current global locale.
 *
 * @return Abbreviated month names indexed January=0 through December=11.
 * @since 1.0.0
 */
std::vector<std::string> monthsShort();

/**
 * @brief Get one abbreviated month name for the current global locale.
 *
 * @param index Month index. Values are normalized modulo 12.
 * @return The abbreviated month name.
 * @since 1.0.0
 */
std::string monthsShort(int index);

/**
 * @brief List abbreviated month names using Moment-style format context.
 *
 * @param format Moment-style format context.
 * @return Abbreviated month names indexed January=0 through December=11.
 * @since 1.0.0
 */
std::vector<std::string> monthsShort(const std::string& format);
/// @brief List abbreviated month names using a string-literal format context.
std::vector<std::string> monthsShort(const char* format);

/**
 * @brief Get one abbreviated month name using Moment-style format context.
 *
 * @param format Moment-style format context.
 * @param index Month index. Values are normalized modulo 12.
 * @return The abbreviated month name.
 * @since 1.0.0
 */
std::string monthsShort(const std::string& format, int index);
/// @brief Get one abbreviated month name using a string-literal format context.
std::string monthsShort(const char* format, int index);

/**
 * @brief List full weekday names for the current global locale.
 *
 * @return Weekday names indexed Sunday=0 through Saturday=6.
 * @since 1.0.0
 */
std::vector<std::string> weekdays();

/**
 * @brief Get one full weekday name for the current global locale.
 *
 * @param index Weekday index. Values are normalized modulo 7.
 * @return The weekday name.
 * @since 1.0.0
 */
std::string weekdays(int index);

/**
 * @brief List full weekday names using Moment-style format context.
 *
 * @param format Moment-style format context.
 * @return Weekday names indexed Sunday=0 through Saturday=6.
 * @since 1.0.0
 */
std::vector<std::string> weekdays(const std::string& format);
/// @brief List full weekday names using a string-literal format context.
std::vector<std::string> weekdays(const char* format);

/**
 * @brief Get one full weekday name using Moment-style format context.
 *
 * @param format Moment-style format context.
 * @param index Weekday index. Values are normalized modulo 7.
 * @return The weekday name.
 * @since 1.0.0
 */
std::string weekdays(const std::string& format, int index);
/// @brief Get one full weekday name using a string-literal format context.
std::string weekdays(const char* format, int index);

/**
 * @brief List full weekday names, optionally shifted to locale week order.
 *
 * @param localeSorted If true, start at the locale's first day of week.
 * @return Weekday names.
 * @since 1.0.0
 */
std::vector<std::string> weekdays(bool localeSorted);

/**
 * @brief Get one full weekday name, optionally from locale week order.
 *
 * @param localeSorted If true, index 0 is the locale's first day of week.
 * @param index Weekday index. Values are normalized modulo 7.
 * @return The weekday name.
 * @since 1.0.0
 */
std::string weekdays(bool localeSorted, int index);

/**
 * @brief List full weekday names with format context and locale ordering.
 *
 * @param localeSorted If true, start at the locale's first day of week.
 * @param format Moment-style format context.
 * @return Weekday names.
 * @since 1.0.0
 */
std::vector<std::string> weekdays(bool localeSorted, const std::string& format);
/// @brief List full weekday names with string-literal format context and locale ordering.
std::vector<std::string> weekdays(bool localeSorted, const char* format);

/**
 * @brief Get one full weekday name with format context and locale ordering.
 *
 * @param localeSorted If true, index 0 is the locale's first day of week.
 * @param format Moment-style format context.
 * @param index Weekday index. Values are normalized modulo 7.
 * @return The weekday name.
 * @since 1.0.0
 */
std::string weekdays(bool localeSorted, const std::string& format, int index);
/// @brief Get one full weekday name with string-literal format context and locale ordering.
std::string weekdays(bool localeSorted, const char* format, int index);

/// @brief List abbreviated weekday names for the current global locale.
std::vector<std::string> weekdaysShort();
/// @brief Get one abbreviated weekday name for the current global locale.
std::string weekdaysShort(int index);
/// @brief List abbreviated weekday names using Moment-style format context.
std::vector<std::string> weekdaysShort(const std::string& format);
/// @brief List abbreviated weekday names using a string-literal format context.
std::vector<std::string> weekdaysShort(const char* format);
/// @brief Get one abbreviated weekday name using Moment-style format context.
std::string weekdaysShort(const std::string& format, int index);
/// @brief Get one abbreviated weekday name using a string-literal format context.
std::string weekdaysShort(const char* format, int index);
/// @brief List abbreviated weekday names, optionally shifted to locale week order.
std::vector<std::string> weekdaysShort(bool localeSorted);
/// @brief Get one abbreviated weekday name, optionally from locale week order.
std::string weekdaysShort(bool localeSorted, int index);
/// @brief List abbreviated weekday names with format context and locale ordering.
std::vector<std::string> weekdaysShort(bool localeSorted, const std::string& format);
/// @brief List abbreviated weekday names with string-literal format context and locale ordering.
std::vector<std::string> weekdaysShort(bool localeSorted, const char* format);
/// @brief Get one abbreviated weekday name with format context and locale ordering.
std::string weekdaysShort(bool localeSorted, const std::string& format, int index);
/// @brief Get one abbreviated weekday name with string-literal format context and locale ordering.
std::string weekdaysShort(bool localeSorted, const char* format, int index);

/// @brief List minimal weekday names for the current global locale.
std::vector<std::string> weekdaysMin();
/// @brief Get one minimal weekday name for the current global locale.
std::string weekdaysMin(int index);
/// @brief List minimal weekday names using Moment-style format context.
std::vector<std::string> weekdaysMin(const std::string& format);
/// @brief List minimal weekday names using a string-literal format context.
std::vector<std::string> weekdaysMin(const char* format);
/// @brief Get one minimal weekday name using Moment-style format context.
std::string weekdaysMin(const std::string& format, int index);
/// @brief Get one minimal weekday name using a string-literal format context.
std::string weekdaysMin(const char* format, int index);
/// @brief List minimal weekday names, optionally shifted to locale week order.
std::vector<std::string> weekdaysMin(bool localeSorted);
/// @brief Get one minimal weekday name, optionally from locale week order.
std::string weekdaysMin(bool localeSorted, int index);
/// @brief List minimal weekday names with format context and locale ordering.
std::vector<std::string> weekdaysMin(bool localeSorted, const std::string& format);
/// @brief List minimal weekday names with string-literal format context and locale ordering.
std::vector<std::string> weekdaysMin(bool localeSorted, const char* format);
/// @brief Get one minimal weekday name with format context and locale ordering.
std::string weekdaysMin(bool localeSorted, const std::string& format, int index);
/// @brief Get one minimal weekday name with string-literal format context and locale ordering.
std::string weekdaysMin(bool localeSorted, const char* format, int index);

/**
 * @brief Get the current global locale key.
 *
 * @return The global locale key (default: "en").
 * @since 1.0.0
 */
std::string globalLocale();

/**
 * @brief Set the global locale key.
 *
 * The locale must already be registered via defineLocale().
 *
 * @param key The locale key to set as global.
 * @return The new global locale key, or the previous one if key was not found.
 * @since 1.0.0
 */
std::string globalLocale(const std::string& key);

/**
 * @brief Set a relative-time threshold value.
 *
 * Thresholds control when relative time switches units (e.g. from "seconds"
 * to "a minute"). Default thresholds: ss=44, s=45, m=45, h=22, d=26, M=11.
 *
 * @param unit  The threshold key ("ss", "s", "m", "h", "d", "w", "M").
 * @param limit The new threshold value.
 * @since 1.0.0
 */
void relativeTimeThreshold(const std::string& unit, double limit);

/**
 * @brief Get a relative-time threshold value.
 *
 * @param unit The threshold key ("ss", "s", "m", "h", "d", "w", "M").
 * @return The current threshold value, or -1 if the key is unknown.
 * @since 1.0.0
 */
double relativeTimeThreshold(const std::string& unit);

/**
 * @brief Set the relative-time rounding function.
 *
 * @param roundingFunction Function applied to relative-time unit values.
 * @return true when the function was installed.
 * @since 1.0.0
 */
bool relativeTimeRounding(RelativeTimeRoundingFn roundingFunction);

/**
 * @brief Get the current relative-time rounding function.
 *
 * @return The active rounding function.
 * @since 1.0.0
 */
RelativeTimeRoundingFn relativeTimeRounding();

} // namespace moment
} // namespace polycpp

#include <polycpp/moment/detail/locale.hpp>
#include <polycpp/moment/detail/locale_en.hpp>
