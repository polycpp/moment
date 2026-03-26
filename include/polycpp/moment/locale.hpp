/**
 * @file locale.hpp
 * @brief Locale data structures and registry — C++ port of Moment.js locale system.
 *
 * Defines LocaleData and supporting types, plus free functions for managing
 * the global locale registry. Thread-safe for concurrent reads, exclusive
 * writes.
 *
 * @see https://momentjs.com/docs/#/i18n/
 * @since 0.1.0
 */
#pragma once

#include <array>
#include <functional>
#include <string>
#include <variant>
#include <vector>

namespace polycpp {
namespace moment {

// ── Callback type aliases ──────────────────────────────────────────────

/**
 * @brief Ordinal formatting function.
 * @param number  The number to ordinalize (e.g. 1, 2, 3).
 * @param token   The format token that triggered the call (e.g. "D", "M").
 * @return The ordinalized string (e.g. "1st", "2nd").
 * @since 0.1.0
 */
using OrdinalFn = std::function<std::string(int number, const std::string& token)>;

/**
 * @brief Meridiem formatting function.
 * @param hours   Hour of day (0-23).
 * @param minutes Minute of hour (0-59).
 * @param isLower If true, return lowercase ("am"/"pm"); otherwise uppercase.
 * @return The meridiem string.
 * @since 0.1.0
 */
using MeridiemFn = std::function<std::string(int hours, int minutes, bool isLower)>;

/**
 * @brief Check whether a meridiem string indicates PM.
 * @param input The meridiem string to check.
 * @return true if the string represents PM.
 * @since 0.1.0
 */
using IsPMFn = std::function<bool(const std::string& input)>;

/**
 * @brief Relative-time formatting function (for locale-specific forms).
 * @param number        The numeric value.
 * @param withoutSuffix If true, omit future/past wrapper.
 * @param key           The key string (e.g. "s", "mm", "h").
 * @param isFuture      If true, this is a future-relative time.
 * @return The formatted relative-time string.
 * @since 0.1.0
 */
using RelativeTimeFn = std::function<std::string(int number, bool withoutSuffix,
                                                  const std::string& key, bool isFuture)>;

/**
 * @brief A relative-time value: either a plain string or a function.
 * @since 0.1.0
 */
using RelativeTimeValue = std::variant<std::string, RelativeTimeFn>;

/**
 * @brief Calendar format value: a string template or a function returning one.
 *
 * The function form takes no arguments; the Moment reference can be bound
 * via lambda capture at call sites.
 *
 * @since 0.1.0
 */
using CalendarFn = std::function<std::string()>;

/**
 * @brief A calendar format entry: either a plain string or a function.
 * @since 0.1.0
 */
using CalendarValue = std::variant<std::string, CalendarFn>;

/**
 * @brief Pre/post format transform function.
 * @param str The string to transform.
 * @return The transformed string.
 * @since 0.1.0
 */
using PrePostFormatFn = std::function<std::string(const std::string& str)>;

// ── Data structures ────────────────────────────────────────────────────

/**
 * @brief Long date format strings keyed by token (LT, LTS, L, LL, LLL, LLLL).
 * @since 0.1.0
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
 * @since 0.1.0
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
 * @since 0.1.0
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
 * @since 0.1.0
 */
struct WeekConfig {
    int dow = 0; ///< Day-of-week that starts the week (0=Sunday, 1=Monday, ...)
    int doy = 6; ///< The day-of-year threshold for the first week (6 = US, 4 = ISO)
};

/**
 * @brief Complete locale data for one locale.
 *
 * Mirrors Moment.js Locale prototype fields. All arrays use 0-based indexing
 * (months[0] = January, weekdays[0] = Sunday).
 *
 * @since 0.1.0
 */
struct LocaleData {
    std::string name; ///< Locale key (e.g. "en", "fr")

    // ── Month names ──
    std::array<std::string, 12> months;       ///< Full month names
    std::array<std::string, 12> monthsShort;  ///< Abbreviated month names

    // ── Weekday names ──
    std::array<std::string, 7> weekdays;      ///< Full weekday names (0=Sunday)
    std::array<std::string, 7> weekdaysShort; ///< Abbreviated weekday names
    std::array<std::string, 7> weekdaysMin;   ///< Minimal weekday names

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

    // ── Optional transforms ──
    PrePostFormatFn preparse;    ///< Pre-parse transform (identity by default)
    PrePostFormatFn postformat;  ///< Post-format transform (identity by default)

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
 * @since 0.1.0
 */
void defineLocale(const std::string& name, const LocaleData& data);

/**
 * @brief Update an existing locale, merging with defaults if it doesn't exist.
 *
 * @param name The locale key.
 * @param data The locale data to merge/replace.
 * @since 0.1.0
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
 * @since 0.1.0
 */
const LocaleData& localeData(const std::string& key = "");

/**
 * @brief List all registered locale keys.
 *
 * @return A sorted vector of locale key strings.
 * @since 0.1.0
 */
std::vector<std::string> locales();

/**
 * @brief Get the current global locale key.
 *
 * @return The global locale key (default: "en").
 * @since 0.1.0
 */
std::string globalLocale();

/**
 * @brief Set the global locale key.
 *
 * The locale must already be registered via defineLocale().
 *
 * @param key The locale key to set as global.
 * @return The new global locale key, or the previous one if key was not found.
 * @since 0.1.0
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
 * @since 0.1.0
 */
void relativeTimeThreshold(const std::string& unit, double limit);

/**
 * @brief Get a relative-time threshold value.
 *
 * @param unit The threshold key ("ss", "s", "m", "h", "d", "w", "M").
 * @return The current threshold value, or -1 if the key is unknown.
 * @since 0.1.0
 */
double relativeTimeThreshold(const std::string& unit);

} // namespace moment
} // namespace polycpp
