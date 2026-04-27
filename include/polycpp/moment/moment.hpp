/**
 * @file moment.hpp
 * @brief Core Moment class — C++ port of Moment.js.
 * @see https://momentjs.com/docs/
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <polycpp/core/date.hpp>
#include <polycpp/core/json.hpp>

namespace polycpp {
namespace moment {

// Forward declaration — Duration will be implemented in a later plan.
class Duration;
struct CalendarFormats;
struct LocaleData;

/**
 * @brief Diagnostic flags captured during Moment parsing.
 *
 * Mirrors Moment.js parsing flags with typed C++ fields. Empty strings mean
 * the corresponding optional upstream string value is not present.
 *
 * @since 1.0.0
 */
struct MomentParsingFlags {
    bool empty = false;
    std::vector<std::string> unusedTokens;
    std::vector<std::string> unusedInput;
    int overflow = -2;
    int charsLeftOver = 0;
    bool nullInput = false;
    std::string invalidEra;
    std::string invalidMonth;
    std::string invalidWeekday;
    bool invalidFormat = false;
    bool userInvalidated = false;
    bool iso = false;
    bool rfc2822 = false;
    std::vector<int> parsedDateParts;
    std::string era;
    std::string meridiem;
    bool weekdayMismatch = false;
    bool bigHour = false;

    /**
     * @brief Convert parsing diagnostics to a JsonObject.
     *
     * Optional string diagnostics are encoded as JSON null when absent.
     *
     * @return JsonObject with Moment.js parsing flag keys.
     * @since 1.0.0
     */
    polycpp::JsonObject toObject() const;

    /**
     * @brief Convert parsing diagnostics for polycpp JSON serialization.
     * @return JsonValue containing the same object as toObject().
     * @since 1.0.0
     */
    polycpp::JsonValue toJSON() const;
};

/**
 * @brief Inputs used to create a Moment.
 *
 * This is the typed C++ equivalent of Moment.js `creationData()`.
 *
 * @since 1.0.0
 */
struct MomentCreationData {
    std::string input;
    std::string format;
    std::string locale;
    bool isUTC = false;
    bool strict = false;

    /**
     * @brief Convert creation metadata to a JsonObject.
     * @return JsonObject with input, format, locale, isUTC, and strict.
     * @since 1.0.0
     */
    polycpp::JsonObject toObject() const;

    /**
     * @brief Convert creation metadata for polycpp JSON serialization.
     * @return JsonValue containing the same object as toObject().
     * @since 1.0.0
     */
    polycpp::JsonValue toJSON() const;
};

/**
 * @brief A mutable date/time wrapper inspired by Moment.js.
 *
 * Wraps a millisecond-precision timestamp with UTC/local mode awareness.
 * Manipulation methods mutate in-place and return *this for chaining.
 * Use clone() when you need an independent copy.
 *
 * @see https://momentjs.com/docs/
 * @since 1.0.0
 */
class Moment {
public:
    /// @brief Construct a Moment representing the current time.
    Moment();

    /// @brief Construct a Moment from a millisecond timestamp (epoch).
    explicit Moment(int64_t timestamp_ms);

    // ── Get/Set methods ──────────────────────────────────────────────

    /// @brief Get the year.
    int year() const;
    /// @brief Set the year.
    Moment& year(int value);

    /// @brief Get the month (0-11).
    int month() const;
    /// @brief Set the month (0-11). Day is clamped to valid range.
    Moment& month(int value);

    /// @brief Get the day of month (1-31).
    int date() const;
    /// @brief Set the day of month (1-31).
    Moment& date(int value);

    /// @brief Get the day of week (0-6, Sunday=0).
    int day() const;
    /// @brief Set the day of week (0-6, Sunday=0).
    Moment& day(int value);

    /// @brief Get the locale-aware weekday.
    int weekday() const;
    /// @brief Set the locale-aware weekday.
    Moment& weekday(int value);

    /// @brief Get the ISO day of week (1-7, Monday=1).
    int isoWeekday() const;
    /// @brief Set the ISO day of week (1-7, Monday=1).
    Moment& isoWeekday(int value);

    /// @brief Get the day of year (1-366).
    int dayOfYear() const;
    /// @brief Set the day of year (1-366).
    Moment& dayOfYear(int value);

    /// @brief Get the locale-aware week of year.
    int week() const;
    /// @brief Set the locale-aware week of year.
    Moment& week(int value);

    /// @brief Get the ISO week of year.
    int isoWeek() const;
    /// @brief Set the ISO week of year.
    Moment& isoWeek(int value);

    /// @brief Get the locale-aware week-numbering year.
    int weekYear() const;
    /// @brief Set the locale-aware week-numbering year.
    Moment& weekYear(int value);

    /// @brief Get the ISO week-numbering year.
    int isoWeekYear() const;
    /// @brief Set the ISO week-numbering year.
    Moment& isoWeekYear(int value);

    /// @brief Get the quarter (1-4).
    int quarter() const;
    /// @brief Set the quarter (1-4).
    Moment& quarter(int value);

    /// @brief Get the hour (0-23).
    int hour() const;
    /// @brief Set the hour (0-23).
    Moment& hour(int value);

    /// @brief Get the minute (0-59).
    int minute() const;
    /// @brief Set the minute (0-59).
    Moment& minute(int value);

    /// @brief Get the second (0-59).
    int second() const;
    /// @brief Set the second (0-59).
    Moment& second(int value);

    /// @brief Get the millisecond (0-999).
    int millisecond() const;
    /// @brief Set the millisecond (0-999).
    Moment& millisecond(int value);

    /// @brief Generic get by unit string.
    int get(const std::string& unit) const;
    /// @brief Generic set by unit string and value.
    Moment& set(const std::string& unit, int value);

    /// @brief Get the UTC offset in minutes.
    int utcOffset() const;
    /// @brief Set the UTC offset in minutes.
    Moment& utcOffset(int offset_minutes, bool keepLocalTime = false);
    /// @brief Set the UTC offset from a string like "+05:30" or "-0530".
    Moment& utcOffset(const std::string& offset, bool keepLocalTime = false);

    /// @brief Get the number of days in the current month.
    int daysInMonth() const;
    /// @brief Get the number of weeks in the year (locale-aware).
    int weeksInYear() const;
    /// @brief Get the number of ISO weeks in the year.
    int isoWeeksInYear() const;
    /// @brief Get the number of weeks in the week-numbering year.
    int weeksInWeekYear() const;
    /// @brief Get the number of ISO weeks in the ISO week-numbering year.
    int isoWeeksInISOWeekYear() const;

    // ── Manipulation methods ─────────────────────────────────────────

    /// @brief Add an amount of a given unit. Mutates in-place.
    Moment& add(int amount, const std::string& unit);
    /// @brief Subtract an amount of a given unit. Mutates in-place.
    Moment& subtract(int amount, const std::string& unit);

    /// @brief Set to the start of a unit of time. Mutates in-place.
    Moment& startOf(const std::string& unit);
    /// @brief Set to the end of a unit of time. Mutates in-place.
    Moment& endOf(const std::string& unit);

    /// @brief Switch to UTC mode. Mutates in-place.
    Moment& utc(bool keepLocalTime = false);
    /// @brief Switch to local mode. Mutates in-place.
    Moment& local(bool keepLocalTime = false);

    // ── Locale ────────────────────────────────────────────────────────

    /// @brief Get this moment's locale key.
    std::string locale() const;
    /// @brief Set this moment's locale key when the locale is registered.
    Moment& locale(const std::string& key);
    /// @brief Get this moment's locale data.
    const LocaleData& localeData() const;

    // ── Display ───────────────────────────────────────────────────────

    /**
     * @brief Format this Moment as a string using format tokens.
     *
     * Supports all standard Moment.js format tokens (YYYY, MM, DD, HH, mm, ss,
     * SSS, etc.) plus macro tokens (LT, L, LL, LLL, LLLL) and escaped text
     * in `[brackets]`.
     *
     * If @p fmt is empty, the default ISO 8601 format with offset is used.
     *
     * @param fmt The format string (default: "").
     * @return The formatted date/time string.
     * @since 1.0.0
     */
    std::string format(const std::string& fmt = "") const;

    /**
     * @brief Get the ISO 8601 string representation.
     * @param keepOffset If true, use the moment's UTC offset instead of Z.
     * @return ISO 8601 formatted string.
     * @since 1.0.0
     */
    std::string toISOString(bool keepOffset = false) const;

    /**
     * @brief Get the Unix timestamp in seconds.
     * @return Seconds since Unix epoch.
     * @since 1.0.0
     */
    int64_t unix() const;

    /// @brief Get the full era name for this moment's locale.
    std::string eraName() const;

    /// @brief Get the narrow era label for this moment's locale.
    std::string eraNarrow() const;

    /// @brief Get the abbreviated era label for this moment's locale.
    std::string eraAbbr() const;

    /// @brief Get the year number within this moment's era.
    int eraYear() const;

    /**
     * @brief Human-readable relative time from now ("3 hours ago").
     * @param withoutSuffix If true, omit the "ago"/"in" wrapper.
     * @return Relative time string.
     * @since 1.0.0
     */
    std::string fromNow(bool withoutSuffix = false) const;

    /**
     * @brief Human-readable relative time from another moment.
     * @param other The reference moment.
     * @param withoutSuffix If true, omit the "ago"/"in" wrapper.
     * @return Relative time string.
     * @since 1.0.0
     */
    std::string from(const Moment& other, bool withoutSuffix = false) const;

    /**
     * @brief Human-readable relative time to now.
     * @param withoutSuffix If true, omit the "ago"/"in" wrapper.
     * @return Relative time string.
     * @since 1.0.0
     */
    std::string toNow(bool withoutSuffix = false) const;

    /**
     * @brief Human-readable relative time to another moment.
     * @param other The reference moment.
     * @param withoutSuffix If true, omit the "ago"/"in" wrapper.
     * @return Relative time string.
     * @since 1.0.0
     */
    std::string to(const Moment& other, bool withoutSuffix = false) const;

    /**
     * @brief Calendar time relative to now ("Today at 2:30 PM").
     * @return Calendar-formatted string.
     * @since 1.0.0
     */
    std::string calendar() const;

    /**
     * @brief Calendar time relative to a reference moment.
     * @param reference The reference moment.
     * @return Calendar-formatted string.
     * @since 1.0.0
     */
    std::string calendar(const Moment& reference) const;

    /**
     * @brief Calendar time relative to now using explicit calendar formats.
     * @param formats Calendar formats to use instead of locale defaults.
     * @return Calendar-formatted string.
     * @since 1.0.0
     */
    std::string calendar(const CalendarFormats& formats) const;

    /**
     * @brief Calendar time relative to a reference moment using explicit calendar formats.
     * @param reference The reference moment.
     * @param formats Calendar formats to use instead of locale defaults.
     * @return Calendar-formatted string.
     * @since 1.0.0
     */
    std::string calendar(const Moment& reference, const CalendarFormats& formats) const;

    /**
     * @brief Compute the difference between this moment and another.
     *
     * Returns `this - other` in the specified unit. If precise is false,
     * the result is truncated toward zero (not floored).
     *
     * @param other The moment to compare against.
     * @param unit The unit of measurement (default: "ms").
     * @param precise If true, return fractional result; otherwise truncate.
     * @return The difference as a double.
     * @since 1.0.0
     */
    double diff(const Moment& other, const std::string& unit = "ms", bool precise = false) const;

    /**
     * @brief Get the JSON representation (same as toISOString).
     * @return ISO 8601 formatted string.
     * @since 1.0.0
     */
    std::string toJSON() const;

    /**
     * @brief Get a string representation like "Fri Mar 15 2024 14:30:45 GMT+0000".
     *
     * Always uses English names, independent of locale.
     *
     * @return Formatted string.
     * @since 1.0.0
     */
    std::string toString() const;

    /**
     * @brief Get a JavaScript-style Date copy of this Moment.
     * @return A polycpp::Date with the same timestamp, or invalid Date for invalid moments.
     * @since 1.0.0
     */
    polycpp::Date toDate() const;

    /**
     * @brief Get an eval-like diagnostic representation.
     * @return A string such as `moment("2024-03-15T14:30:45.123")`.
     * @since 1.0.0
     */
    std::string inspect() const;

    /**
     * @brief Get the date components as a JsonArray.
     * @return JsonArray: [year, month, day, hour, minute, second, millisecond].
     * @see https://momentjs.com/docs/#/displaying/as-array/
     * @since 1.0.0
     */
    polycpp::JsonArray toArray() const;

    /**
     * @brief Get the date components as a JsonObject.
     * @return JsonObject with keys: years, months, date, hours, minutes, seconds, milliseconds.
     * @par Example
     * @code
     * auto obj = moment::utcFromDate(2024, 2, 15, 14, 30, 45, 123).toObject();
     * // obj["years"].asInt() == 2024, obj["months"].asInt() == 2, obj["date"].asInt() == 15
     * @endcode
     * @see https://momentjs.com/docs/#/displaying/as-object/
     * @since 1.0.0
     */
    polycpp::JsonObject toObject() const;

    // ── Query ────────────────────────────────────────────────────────

    /// @brief Get the millisecond timestamp (ms since Unix epoch).
    int64_t valueOf() const;

    /// @brief Check if this Moment represents a valid date/time.
    bool isValid() const;

    /// @brief Get parse diagnostic flags.
    MomentParsingFlags parsingFlags() const;

    /// @brief Get the invalid component index, or -1 when there is no overflow.
    int invalidAt() const;

    /// @brief Get the inputs used to create this moment.
    MomentCreationData creationData() const;

    /// @brief Check if this is a leap year.
    bool isLeapYear() const;

    /// @brief Check if in UTC mode.
    bool isUtc() const;
    /// @brief Check if in UTC mode.
    bool isUTC() const;

    /// @brief Check if in local mode.
    bool isLocal() const;

    /// @brief Check if UTC offset was explicitly set.
    bool isUtcOffset() const;

    /// @brief Convert this moment to the fixed offset parsed from its creation input.
    Moment& parseZone();

    /// @brief Check if this moment's offset aligns with UTC on an hour boundary.
    bool hasAlignedHourOffset() const;

    /// @brief Check if this moment's offset aligns with another moment on an hour boundary.
    bool hasAlignedHourOffset(const Moment& other) const;

    /// @brief Get this moment's timezone abbreviation.
    std::string zoneAbbr() const;

    /// @brief Get this moment's timezone name.
    std::string zoneName() const;

    /// @brief Check if this moment is before another.
    bool isBefore(const Moment& other) const;
    /// @brief Check if this moment is before another at a given unit granularity.
    bool isBefore(const Moment& other, const std::string& unit) const;

    /// @brief Check if this moment is after another.
    bool isAfter(const Moment& other) const;
    /// @brief Check if this moment is after another at a given unit granularity.
    bool isAfter(const Moment& other, const std::string& unit) const;

    /// @brief Check if this moment is the same as another.
    bool isSame(const Moment& other) const;
    /// @brief Check if this moment is the same as another at a given unit granularity.
    bool isSame(const Moment& other, const std::string& unit) const;

    /// @brief Check if this moment is the same as or before another.
    bool isSameOrBefore(const Moment& other) const;
    /// @brief Check if this moment is the same as or before another at a given unit granularity.
    bool isSameOrBefore(const Moment& other, const std::string& unit) const;

    /// @brief Check if this moment is the same as or after another.
    bool isSameOrAfter(const Moment& other) const;
    /// @brief Check if this moment is the same as or after another at a given unit granularity.
    bool isSameOrAfter(const Moment& other, const std::string& unit) const;

    /// @brief Check if this moment is between two others (exclusive by default).
    bool isBetween(const Moment& from, const Moment& to) const;
    /// @brief Check if this moment is between two others at a given unit granularity.
    bool isBetween(const Moment& from, const Moment& to, const std::string& unit) const;
    /**
     * @brief Check if this moment is between two others with inclusivity control.
     * @param from Start boundary.
     * @param to End boundary.
     * @param unit Unit granularity (empty or "millisecond" for raw comparison).
     * @param inclusivity Two-char string: "()" exclusive, "[]" inclusive, "[)" or "(]" half-open.
     * @since 1.0.0
     */
    bool isBetween(const Moment& from, const Moment& to, const std::string& unit, const std::string& inclusivity) const;

    /// @brief Check if daylight saving time is active.
    bool isDST() const;

    /// @brief Deep copy this Moment.
    Moment clone() const;

    // ── Operators ────────────────────────────────────────────────────

    bool operator==(const Moment& other) const;
    bool operator!=(const Moment& other) const;
    bool operator<(const Moment& other) const;
    bool operator<=(const Moment& other) const;
    bool operator>(const Moment& other) const;
    bool operator>=(const Moment& other) const;

    // ── Friends — parse functions need access to internal state ────────

    /// @brief Access helper for parse/format implementation details.
    struct InternalAccess;
    friend struct InternalAccess;

private:
    // ── Internal helpers ─────────────────────────────────────────────

    /// @brief Decomposed date/time components.
    struct DateComponents {
        int year;
        int month;  // 0-11
        int day;    // 1-31
        int hour;   // 0-23
        int minute; // 0-59
        int second; // 0-59
        int ms;     // 0-999
    };

    /// @brief Decompose timestamp to date/time components.
    DateComponents toComponents() const;

    /// @brief Recompose date/time components back to timestamp.
    void fromComponents(const DateComponents& c);

    // ── Private data ─────────────────────────────────────────────────

    int64_t timestamp_ms_ = 0;
    bool is_utc_ = false;
    int utc_offset_minutes_ = 0;
    bool has_fixed_offset_ = false;
    bool is_valid_ = true;
    std::string locale_key_ = "en";
    MomentParsingFlags parsing_flags_;
    MomentCreationData creation_data_;
};

/**
 * @brief Provides controlled access to Moment's private members for
 *        factory functions and formatting internals.
 *
 * This avoids the need for many individual friend declarations.
 * @since 1.0.0
 */
struct Moment::InternalAccess {
    static void setTimestamp(Moment& m, int64_t ts) { m.timestamp_ms_ = ts; }
    static void setValid(Moment& m, bool v) { m.is_valid_ = v; }
    static void setUtc(Moment& m, bool v) { m.is_utc_ = v; }
    static void setOffset(Moment& m, int minutes) {
        m.utc_offset_minutes_ = minutes;
        m.has_fixed_offset_ = true;
        m.is_utc_ = (minutes == 0);
    }
    static void setLocale(Moment& m, const std::string& loc) { m.locale_key_ = loc; }
    static void setParsingFlags(Moment& m, const MomentParsingFlags& flags) {
        m.parsing_flags_ = flags;
    }
    static void setCreationData(Moment& m, const MomentCreationData& data) {
        m.creation_data_ = data;
    }
    static int64_t getTimestamp(const Moment& m) { return m.timestamp_ms_; }
    static bool getIsUtc(const Moment& m) { return m.is_utc_; }
    static bool getHasFixedOffset(const Moment& m) { return m.has_fixed_offset_; }
    static int getOffsetMinutes(const Moment& m) { return m.utc_offset_minutes_; }
    static std::string getLocaleKey(const Moment& m) { return m.locale_key_; }
    static const MomentParsingFlags& getParsingFlags(const Moment& m) { return m.parsing_flags_; }
    static const MomentCreationData& getCreationData(const Moment& m) { return m.creation_data_; }

    /// @brief Create an invalid Moment.
    static Moment makeInvalid() {
        Moment m(0);
        m.is_valid_ = false;
        return m;
    }

    /// @brief Create an invalid Moment with parsing flags.
    static Moment makeInvalid(const MomentParsingFlags& flags) {
        Moment m = makeInvalid();
        m.parsing_flags_ = flags;
        return m;
    }

    /// @brief Create a Moment from components in UTC.
    static Moment fromUtcComponents(int year, int month, int day,
                                     int hour, int minute, int second, int ms) {
        Moment m(0);
        m.is_utc_ = true;
        Moment::DateComponents c{year, month, day, hour, minute, second, ms};
        m.fromComponents(c);
        return m;
    }

    /// @brief Create a Moment from components in local time.
    static Moment fromLocalComponents(int year, int month, int day,
                                       int hour, int minute, int second, int ms) {
        Moment m(0);
        m.is_utc_ = false;
        Moment::DateComponents c{year, month, day, hour, minute, second, ms};
        m.fromComponents(c);
        return m;
    }
};

// ── Factory functions ────────────────────────────────────────────────

/**
 * @brief Create a Moment representing the current time.
 * @return A valid Moment set to now.
 * @since 1.0.0
 */
Moment now();

/**
 * @brief Get the current timestamp in milliseconds since epoch.
 * @return Milliseconds since Unix epoch.
 * @since 1.0.0
 */
int64_t nowMs();

/**
 * @brief Explicit ISO 8601 parsing sentinel for parse/UTC/parseZone format overloads.
 * @since 1.0.0
 */
inline constexpr const char* ISO_8601 = "__polycpp_moment_ISO_8601";

/**
 * @brief Explicit RFC 2822 parsing sentinel for parse/UTC/parseZone format overloads.
 * @since 1.0.0
 */
inline constexpr const char* RFC_2822 = "__polycpp_moment_RFC_2822";

/**
 * @brief Parse a date/time string, auto-detecting the format.
 *
 * Attempts ISO 8601 first, then RFC 2822. Returns an invalid Moment
 * if neither format matches.
 *
 * @param input The date/time string to parse.
 * @return A Moment representing the parsed date/time, or an invalid Moment.
 * @since 1.0.0
 */
Moment parse(const std::string& input);

/**
 * @brief Parse a date/time string using a specific format string.
 *
 * The format string uses Moment.js-compatible tokens (YYYY, MM, DD, etc.).
 *
 * @param input  The date/time string to parse.
 * @param format The format string specifying the expected token layout.
 * @return A Moment representing the parsed date/time, or an invalid Moment.
 * @since 1.0.0
 */
Moment parse(const std::string& input, const std::string& format);

/**
 * @brief Parse a date/time string using a format string with optional strict mode.
 *
 * In strict mode, the input must match the format exactly with no extra characters.
 *
 * @param input  The date/time string to parse.
 * @param format The format string specifying the expected token layout.
 * @param strict If true, enforce strict matching.
 * @return A Moment representing the parsed date/time, or an invalid Moment.
 * @since 1.0.0
 */
Moment parse(const std::string& input, const std::string& format, bool strict);

/**
 * @brief Parse a date/time string trying multiple format strings.
 *
 * Each format is tried in order; the first successful parse is returned.
 *
 * @param input   The date/time string to parse.
 * @param formats A vector of format strings to try.
 * @return A Moment from the first matching format, or an invalid Moment.
 * @since 1.0.0
 */
Moment parse(const std::string& input, const std::vector<std::string>& formats);

/**
 * @brief Create a Moment from a Unix timestamp in seconds.
 * @param seconds Seconds since Unix epoch.
 * @return A Moment representing the specified time.
 * @since 1.0.0
 */
Moment fromUnixTimestamp(int64_t seconds);

/**
 * @brief Create a Moment from a millisecond timestamp.
 * @param ms Milliseconds since Unix epoch.
 * @return A Moment representing the specified time.
 * @since 1.0.0
 */
Moment fromMilliseconds(int64_t ms);

/**
 * @brief Create a local Moment from a polycpp::Date.
 *
 * The timestamp is copied from the Date. Invalid Date values produce an
 * invalid Moment.
 *
 * @param date The polycpp::Date to copy.
 * @return A local Moment with the same millisecond timestamp.
 * @since 1.0.0
 */
Moment fromDate(const polycpp::Date& date);

/**
 * @brief Create a Moment from date/time components in local time.
 *
 * Month is 0-based (0=January, 11=December), matching Moment.js convention.
 *
 * @param year   The year.
 * @param month  The month (0-11).
 * @param day    The day of month (default: 1).
 * @param hour   The hour (default: 0).
 * @param minute The minute (default: 0).
 * @param second The second (default: 0).
 * @param ms     The millisecond (default: 0).
 * @return A Moment representing the specified date/time in local time.
 * @since 1.0.0
 */
Moment fromDate(int year, int month, int day = 1, int hour = 0,
                int minute = 0, int second = 0, int ms = 0);

/**
 * @brief Create a Moment representing the current time in UTC mode.
 * @return A valid UTC Moment set to now.
 * @since 1.0.0
 */
Moment utcNow();

/**
 * @brief Parse a date/time string in UTC mode.
 * @param input The date/time string to parse.
 * @return A UTC Moment from the parsed date/time, or an invalid Moment.
 * @since 1.0.0
 */
Moment utcFromString(const std::string& input);

/**
 * @brief Parse a date/time string using a format string in UTC mode.
 * @param input  The date/time string.
 * @param format The format string.
 * @return A UTC Moment from the parsed date/time, or an invalid Moment.
 * @since 1.0.0
 */
Moment utcFromFormat(const std::string& input, const std::string& format);

/**
 * @brief Create a UTC Moment from a millisecond timestamp.
 * @param ms Milliseconds since Unix epoch.
 * @return A UTC Moment.
 * @since 1.0.0
 */
Moment utcFromMs(int64_t ms);

/**
 * @brief Create a UTC Moment from a polycpp::Date.
 *
 * The timestamp is copied from the Date. Invalid Date values produce an
 * invalid Moment.
 *
 * @param date The polycpp::Date to copy.
 * @return A UTC Moment with the same millisecond timestamp.
 * @since 1.0.0
 */
Moment utcFromDate(const polycpp::Date& date);

/**
 * @brief Create a Moment from date/time components in UTC.
 *
 * Month is 0-based (0=January, 11=December).
 *
 * @param year   The year.
 * @param month  The month (0-11).
 * @param day    The day of month (default: 1).
 * @param hour   The hour (default: 0).
 * @param minute The minute (default: 0).
 * @param second The second (default: 0).
 * @param ms     The millisecond (default: 0).
 * @return A UTC Moment.
 * @since 1.0.0
 */
Moment utcFromDate(int year, int month, int day = 1, int hour = 0,
                   int minute = 0, int second = 0, int ms = 0);

/**
 * @brief Parse a date/time string preserving the parsed timezone offset.
 *
 * The resulting Moment will have a fixed UTC offset matching the one
 * found in the input string. If no offset is present, UTC is assumed.
 *
 * @param input The date/time string (should include timezone offset).
 * @return A Moment with a fixed offset, or an invalid Moment.
 * @since 1.0.0
 */
Moment parseZone(const std::string& input);

/**
 * @brief Parse a date/time string with a format, preserving the timezone offset.
 * @param input  The date/time string.
 * @param format The format string.
 * @return A Moment with a fixed offset, or an invalid Moment.
 * @since 1.0.0
 */
Moment parseZone(const std::string& input, const std::string& format);

/**
 * @brief Parse a two-digit year using Moment.js expansion rules.
 *
 * Values 00-68 map to 2000-2068; values 69-99 map to 1969-1999.
 *
 * @since 1.0.0
 */
int parseTwoDigitYear(const std::string& input);

/**
 * @brief Create an invalid Moment.
 *
 * The returned Moment has isValid() == false and will format as "Invalid date".
 *
 * @return An invalid Moment.
 * @since 1.0.0
 */
Moment invalid();

/**
 * @brief Create an invalid Moment with explicit parsing flags.
 * @param flags Diagnostic parsing flags to attach.
 * @return An invalid Moment.
 * @since 1.0.0
 */
Moment invalid(const MomentParsingFlags& flags);

/**
 * @brief Return the earliest (minimum) of the given moments.
 * @param moments An initializer list of Moment objects.
 * @return The Moment with the smallest timestamp.
 * @since 1.0.0
 */
Moment min(std::initializer_list<Moment> moments);

/**
 * @brief Return the latest (maximum) of the given moments.
 * @param moments An initializer list of Moment objects.
 * @return The Moment with the largest timestamp.
 * @since 1.0.0
 */
Moment max(std::initializer_list<Moment> moments);

/**
 * @brief Return the earliest (minimum) of the given moments.
 * @param moments A vector of Moment objects.
 * @return The Moment with the smallest timestamp.
 * @since 1.0.0
 */
Moment min(const std::vector<Moment>& moments);

/**
 * @brief Return the latest (maximum) of the given moments.
 * @param moments A vector of Moment objects.
 * @return The Moment with the largest timestamp.
 * @since 1.0.0
 */
Moment max(const std::vector<Moment>& moments);

/**
 * @brief Create a local Moment from a JsonObject with date component keys.
 *
 * Accepts both singular and plural key forms (e.g. "year" or "years").
 * Missing keys default to: year=2000, month=0, date=1, others=0.
 *
 * @param obj JsonObject with optional keys: year/years, month/months,
 *            day/date, hour/hours, minute/minutes, second/seconds,
 *            millisecond/milliseconds
 * @return Moment constructed from the specified components in local time.
 * @par Example
 * @code
 * auto m = moment::fromObject(JsonObject{{"year", 2024}, {"month", 2}, {"date", 15}});
 * @endcode
 * @see https://momentjs.com/docs/#/parsing/object/
 * @since 1.0.0
 */
Moment fromObject(const polycpp::JsonObject& obj);

/**
 * @brief Create a local Moment from a JsonValue containing a JsonObject.
 *
 * Accepts the same date component keys as fromObject(const JsonObject&).
 * Non-object values return an invalid Moment.
 *
 * @param value JsonValue containing an object with optional date component keys.
 * @return Moment constructed from the specified components in local time.
 * @see https://momentjs.com/docs/#/parsing/object/
 * @since 1.0.0
 */
Moment fromObject(const polycpp::JsonValue& value);

/**
 * @brief Create a UTC Moment from a JsonObject with date component keys.
 *
 * Same key semantics as fromObject(), but the resulting Moment is in UTC mode.
 *
 * @param obj JsonObject with optional date component keys.
 * @return Moment constructed from the specified components in UTC.
 * @see https://momentjs.com/docs/#/parsing/object/
 * @since 1.0.0
 */
Moment utcFromObject(const polycpp::JsonObject& obj);

/**
 * @brief Create a UTC Moment from a JsonValue containing a JsonObject.
 *
 * Accepts the same date component keys as utcFromObject(const JsonObject&).
 * Non-object values return an invalid Moment.
 *
 * @param value JsonValue containing an object with optional date component keys.
 * @return Moment constructed from the specified components in UTC.
 * @see https://momentjs.com/docs/#/parsing/object/
 * @since 1.0.0
 */
Moment utcFromObject(const polycpp::JsonValue& value);

/**
 * @brief Select a Moment.js calendar key for a moment relative to a reference.
 *
 * Returns one of `sameElse`, `lastWeek`, `lastDay`, `sameDay`, `nextDay`, or
 * `nextWeek`.
 *
 * @since 1.0.0
 */
std::string calendarFormat(const Moment& moment, const Moment& reference);

} // namespace moment
} // namespace polycpp

#include <polycpp/moment/detail/aggregator.hpp>
