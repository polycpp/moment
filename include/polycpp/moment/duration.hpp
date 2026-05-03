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
    int years = 0;        ///< Year component; converted to 12 months each.
    int months = 0;       ///< Month component.
    int weeks = 0;        ///< Week component; converted to 7 days each.
    int days = 0;         ///< Day component.
    int hours = 0;        ///< Hour component.
    int minutes = 0;      ///< Minute component.
    int seconds = 0;      ///< Second component.
    int milliseconds = 0; ///< Millisecond component.
};

/**
 * @brief A mutable time duration, inspired by Moment.js Duration.
 *
 * Internally stores three aggregate values (`_months`, `_days`, `_milliseconds`)
 * following the Moment.js approach, then "bubbles" them into individual
 * component fields (`years_`, `months_`, `days_`, `hours_`, `minutes_`,
 * `seconds_`, `milliseconds_`) for the component getters.
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

    /**
     * @brief Construct from a JsonValue containing a JsonObject with unit keys.
     *
     * Accepts the same keys as Duration(const JsonObject&). Non-object values
     * create an invalid Duration.
     *
     * @param value JsonValue containing duration component keys.
     * @see https://momentjs.com/docs/#/durations/creating/
     * @since 1.0.0
     */
    explicit Duration(const polycpp::JsonValue& value);

    /// @brief Create a copy of this duration.
    /// @return A duration with the same components, validity state, and locale.
    Duration clone() const;

    /// @brief Make all raw duration buckets positive.
    /// @return `*this` for chaining.
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
     *
     * Uses this duration's locale and the process-global relative-time
     * thresholds and rounding function.
     *
     * @param withSuffix If true, wrap with "in ..." / "... ago".
     * @return A human-readable string like "a few seconds" or "2 hours".
     * @see relativeTimeThreshold()
     * @see relativeTimeRounding()
     */
    std::string humanize(bool withSuffix = false) const;

    /**
     * @brief Human-readable description with per-call threshold overrides.
     *
     * Empty threshold fields fall back to the process-global threshold values.
     *
     * @param thresholds Relative-time threshold overrides for this call.
     * @return A human-readable string like "a few seconds" or "2 hours".
     * @see RelativeTimeThresholds
     */
    std::string humanize(const RelativeTimeThresholds& thresholds) const;

    /**
     * @brief Human-readable description with suffix and per-call thresholds.
     * @param withSuffix If true, wrap with "in ..." / "... ago".
     * @param thresholds Relative-time threshold overrides for this call.
     * @return A human-readable string like "in a minute".
     * @see RelativeTimeThresholds
     */
    std::string humanize(bool withSuffix, const RelativeTimeThresholds& thresholds) const;

    // -- Component getters (the bubbled component, not the total) --

    /// @brief Get the bubbled year component.
    /// @return Year component, or 0 when invalid.
    int years() const;
    /// @brief Get the bubbled month component.
    /// @return Month component, or 0 when invalid.
    int months() const;
    /// @brief Get the bubbled day component.
    /// @return Day component, or 0 when invalid.
    int days() const;
    /// @brief Get whole weeks from the bubbled day component.
    /// @return `floor(days() / 7)`, or 0 when invalid.
    int weeks() const;
    /// @brief Get the bubbled hour component.
    /// @return Hour component, or 0 when invalid.
    int hours() const;
    /// @brief Get the bubbled minute component.
    /// @return Minute component, or 0 when invalid.
    int minutes() const;
    /// @brief Get the bubbled second component.
    /// @return Second component, or 0 when invalid.
    int seconds() const;
    /// @brief Get the bubbled millisecond component.
    /// @return Millisecond component, or 0 when invalid.
    int milliseconds() const;

    /// @brief Generic component getter by unit string.
    /// @param unit Moment-style unit name or alias.
    /// @return The matching bubbled component, or 0 for invalid input.
    int get(const std::string& unit) const;

    // -- Total conversion (entire duration expressed in one unit) --

    /// @brief Convert the entire duration to milliseconds.
    /// @return Total milliseconds, or 0 when invalid.
    double asMilliseconds() const;
    /// @brief Convert the entire duration to seconds.
    /// @return Total seconds, or 0 when invalid.
    double asSeconds() const;
    /// @brief Convert the entire duration to minutes.
    /// @return Total minutes, or 0 when invalid.
    double asMinutes() const;
    /// @brief Convert the entire duration to hours.
    /// @return Total hours, or 0 when invalid.
    double asHours() const;
    /// @brief Convert the entire duration to days.
    /// @return Total days, or 0 when invalid.
    double asDays() const;
    /// @brief Convert the entire duration to weeks.
    /// @return Total weeks, or 0 when invalid.
    double asWeeks() const;
    /// @brief Convert the entire duration to quarters.
    /// @return Total quarters, or 0 when invalid.
    double asQuarters() const;
    /// @brief Convert the entire duration to months.
    /// @return Total months, or 0 when invalid.
    double asMonths() const;
    /// @brief Convert the entire duration to years.
    /// @return Total years, or 0 when invalid.
    double asYears() const;

    /// @brief Generic total-conversion by unit string.
    /// @param unit Moment-style unit name or alias.
    /// @return Entire duration expressed in the requested unit, or 0 for invalid input.
    double as(const std::string& unit) const;

    // -- Serialization --

    /// @brief Serialize as an ISO 8601 duration string.
    /// @return ISO 8601 duration string, e.g. "P1Y2M3DT4H5M6S".
    std::string toISOString() const;

    /// @brief Serialize for JSON.
    /// @return The same string as toISOString().
    std::string toJSON() const;

    /// @brief Serialize for string display.
    /// @return The same string as toISOString(), matching Moment.js duration display.
    std::string toString() const;

    /**
     * @brief Get the duration components as a JsonObject.
     * @return JsonObject with keys: years, months, days, hours, minutes, seconds, milliseconds.
     * @note This is a typed C++ convenience; upstream Moment.js durations do not expose toObject().
     * @see https://momentjs.com/docs/#/durations/
     * @since 1.0.0
     */
    polycpp::JsonObject toObject() const;

    /// @brief Total milliseconds as integer.
    /// @return The same value as asMilliseconds() cast to int64_t.
    int64_t valueOf() const;

    // -- Validation --

    /// @brief Check whether this duration was constructed from valid input.
    /// @return true when the duration is valid.
    bool isValid() const;

    // -- Locale --

    /// @brief Get the locale key for this duration.
    /// @return Locale key used by humanize().
    std::string locale() const;

    /// @brief Set the locale key for this duration.
    /// @param key Locale key to use for humanize() and invalid serialization text.
    /// @return `*this` for chaining.
    Duration& locale(const std::string& key);

    /// @brief Get this duration's locale data.
    /// @return Locale data for locale(), falling back through localeData().
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

/**
 * @brief Create a Duration from a JsonValue containing a JsonObject with unit keys.
 * @param value JsonValue containing optional keys: years, months, weeks, days,
 *              hours, minutes, seconds, milliseconds.
 * @par Example
 * @code
 * auto value = JsonValue(JsonObject{{"hours", 2}, {"minutes", 30}});
 * auto d = moment::duration(value);
 * @endcode
 * @see https://momentjs.com/docs/#/durations/creating/
 * @since 1.0.0
 */
Duration duration(const polycpp::JsonValue& value);

} // namespace moment
} // namespace polycpp

#include <polycpp/moment/detail/duration.hpp>
