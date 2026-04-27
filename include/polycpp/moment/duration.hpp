/**
 * @file duration.hpp
 * @brief Duration class declaration -- C++ port of Moment.js Duration.
 *
 * Represents a length of time with individual year, month, day, hour,
 * minute, second, and millisecond components. Mutable: arithmetic methods
 * modify in-place and return *this for chaining.
 *
 * @see https://momentjs.com/docs/#/durations/
 * @since 1.0.0
 */
#pragma once

#include <cstdint>
#include <string>
#include <polycpp/core/json.hpp>

namespace polycpp {
namespace moment {

struct LocaleData;
struct RelativeTimeThresholds;

/**
 * @brief Input struct for creating Duration from named components.
 *
 * All fields default to 0. Weeks are converted to days (* 7) during
 * construction.
 *
 * @since 1.0.0
 */
struct DurationInput {
    int years = 0;
    int months = 0;
    int weeks = 0;
    int days = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    int milliseconds = 0;
};

/**
 * @brief A mutable time duration, inspired by Moment.js Duration.
 *
 * Internally stores three aggregate values (_months, _days, _milliseconds)
 * following the Moment.js approach, then "bubbles" them into individual
 * component fields (years_, months_, days_, hours_, minutes_, seconds_,
 * milliseconds_) for the component getters.
 *
 * @since 1.0.0
 */
class Duration {
public:
    /// @brief Default constructor: zero duration.
    Duration();

    /// @brief Construct from total milliseconds.
    explicit Duration(int64_t milliseconds);

    /// @brief Construct from an amount and a unit string.
    Duration(int amount, const std::string& unit);

    /// @brief Construct from an ISO 8601 duration string (e.g. "P1Y2M3DT4H5M6S").
    explicit Duration(const std::string& iso_string);

    /// @brief Construct from a DurationInput struct.
    explicit Duration(const DurationInput& input);

    /**
     * @brief Construct from a JsonObject with unit keys.
     *
     * Accepts keys: years, months, weeks, days, hours, minutes, seconds,
     * milliseconds. Missing keys default to 0.
     *
     * @param obj JsonObject with duration component keys.
     * @par Example
     * @code
     * Duration d(JsonObject{{"hours", 2}, {"minutes", 30}});
     * @endcode
     * @see https://momentjs.com/docs/#/durations/creating/
     * @since 1.0.0
     */
    explicit Duration(const polycpp::JsonObject& obj);

    /// @brief Create a deep copy.
    Duration clone() const;

    /// @brief Make all components positive (absolute value).
    Duration& abs();

    // -- Arithmetic (mutate in-place, chainable) --

    /// @brief Add an amount of a unit to this duration.
    Duration& add(int amount, const std::string& unit);

    /// @brief Add another duration to this duration.
    Duration& add(const Duration& other);

    /// @brief Subtract an amount of a unit from this duration.
    Duration& subtract(int amount, const std::string& unit);

    /// @brief Subtract another duration from this duration.
    Duration& subtract(const Duration& other);

    // -- Humanize --

    /**
     * @brief Human-readable description of this duration.
     * @param withSuffix If true, wrap with "in ..." / "... ago".
     * @return A human-readable string like "a few seconds" or "2 hours".
     */
    std::string humanize(bool withSuffix = false) const;

    /**
     * @brief Human-readable description with per-call threshold overrides.
     * @param thresholds Relative-time threshold overrides for this call.
     * @return A human-readable string like "a few seconds" or "2 hours".
     */
    std::string humanize(const RelativeTimeThresholds& thresholds) const;

    /**
     * @brief Human-readable description with suffix and per-call thresholds.
     * @param withSuffix If true, wrap with "in ..." / "... ago".
     * @param thresholds Relative-time threshold overrides for this call.
     * @return A human-readable string like "in a minute".
     */
    std::string humanize(bool withSuffix, const RelativeTimeThresholds& thresholds) const;

    // -- Component getters (the bubbled component, not the total) --

    int years() const;
    int months() const;
    int days() const;
    int weeks() const;       ///< floor(days / 7)
    int hours() const;
    int minutes() const;
    int seconds() const;
    int milliseconds() const;

    /// @brief Generic component getter by unit string.
    int get(const std::string& unit) const;

    // -- Total conversion (entire duration expressed in one unit) --

    double asMilliseconds() const;
    double asSeconds() const;
    double asMinutes() const;
    double asHours() const;
    double asDays() const;
    double asWeeks() const;
    double asQuarters() const;
    double asMonths() const;
    double asYears() const;

    /// @brief Generic total-conversion by unit string.
    double as(const std::string& unit) const;

    // -- Serialization --

    /// @brief ISO 8601 duration string, e.g. "P1Y2M3DT4H5M6S".
    std::string toISOString() const;

    /// @brief Alias for toISOString() (JSON serialization).
    std::string toJSON() const;

    /// @brief Alias for toISOString(), matching Moment.js duration display.
    std::string toString() const;

    /**
     * @brief Get the duration components as a JsonObject.
     * @return JsonObject with keys: years, months, days, hours, minutes, seconds, milliseconds.
     * @note This is a typed C++ convenience; upstream Moment.js durations do not expose toObject().
     * @see https://momentjs.com/docs/#/durations/
     * @since 1.0.0
     */
    polycpp::JsonObject toObject() const;

    /// @brief Total milliseconds as integer (same as asMilliseconds cast).
    int64_t valueOf() const;

    // -- Validation --

    /// @brief Returns true if this duration was constructed from valid input.
    bool isValid() const;

    // -- Locale --

    /// @brief Get the locale key for this duration.
    std::string locale() const;

    /// @brief Set the locale key for this duration.
    Duration& locale(const std::string& key);

    /// @brief Get this duration's locale data.
    const LocaleData& localeData() const;

private:
    // Three-bucket storage (matching moment.js internals)
    int64_t raw_milliseconds_ = 0; // ms + s*1000 + m*60000 + h*3600000
    int raw_days_ = 0;             // days + weeks*7
    int raw_months_ = 0;           // months + years*12

    // Bubbled component fields (populated by bubble())
    int years_ = 0;
    int months_ = 0;
    int days_ = 0;
    int hours_ = 0;
    int minutes_ = 0;
    int seconds_ = 0;
    int milliseconds_ = 0;

    bool is_valid_ = true;
    std::string locale_key_ = "en";

    /// @brief Bubble the three raw buckets into individual components.
    void bubble();
};

// -- Factory functions --

/// @brief Create a Duration from total milliseconds.
Duration duration(int64_t milliseconds);

/// @brief Create a Duration from an amount and a unit string.
Duration duration(int amount, const std::string& unit);

/// @brief Create a Duration from an ISO 8601 string.
Duration duration(const std::string& iso_string);

/// @brief Create a Duration from a DurationInput struct.
Duration duration(const DurationInput& input);

/**
 * @brief Create a Duration from a JsonObject with unit keys.
 * @param obj JsonObject with optional keys: years, months, weeks, days,
 *            hours, minutes, seconds, milliseconds.
 * @par Example
 * @code
 * auto d = moment::duration(JsonObject{{"years", 1}, {"months", 6}});
 * @endcode
 * @see https://momentjs.com/docs/#/durations/creating/
 * @since 1.0.0
 */
Duration duration(const polycpp::JsonObject& obj);

} // namespace moment
} // namespace polycpp

#include <polycpp/moment/detail/duration.hpp>
