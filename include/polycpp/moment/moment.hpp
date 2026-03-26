/**
 * @file moment.hpp
 * @brief Core Moment class — C++ port of Moment.js.
 * @see https://momentjs.com/docs/
 */
#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <chrono>

namespace polycpp {
namespace moment {

// Forward declaration — Duration will be implemented in a later plan.
class Duration;

/**
 * @brief A mutable date/time wrapper inspired by Moment.js.
 *
 * Wraps a millisecond-precision timestamp with UTC/local mode awareness.
 * Manipulation methods mutate in-place and return *this for chaining.
 * Use clone() when you need an independent copy.
 *
 * @see https://momentjs.com/docs/
 * @since 0.1.0
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

    // ── Display / Query ──────────────────────────────────────────────

    /// @brief Get the millisecond timestamp (ms since Unix epoch).
    int64_t valueOf() const;

    /// @brief Check if this Moment represents a valid date/time.
    bool isValid() const;

    /// @brief Check if this is a leap year.
    bool isLeapYear() const;

    /// @brief Check if in UTC mode.
    bool isUtc() const;

    /// @brief Check if in local mode.
    bool isLocal() const;

    /// @brief Check if UTC offset was explicitly set.
    bool isUtcOffset() const;

    /// @brief Deep copy this Moment.
    Moment clone() const;

    // ── Operators ────────────────────────────────────────────────────

    bool operator==(const Moment& other) const;
    bool operator!=(const Moment& other) const;
    bool operator<(const Moment& other) const;
    bool operator<=(const Moment& other) const;
    bool operator>(const Moment& other) const;
    bool operator>=(const Moment& other) const;

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
};

// ── Factory functions ────────────────────────────────────────────────

/**
 * @brief Create a Moment representing the current time.
 * @return A valid Moment set to now.
 * @since 0.1.0
 */
Moment now();

/**
 * @brief Get the current timestamp in milliseconds since epoch.
 * @return Milliseconds since Unix epoch.
 * @since 0.1.0
 */
int64_t nowMs();

} // namespace moment
} // namespace polycpp
