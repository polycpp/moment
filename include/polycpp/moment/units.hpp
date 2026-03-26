/**
 * @file units.hpp
 * @brief Unit enum and normalization — maps string aliases to canonical units.
 *
 * Mirrors the Moment.js unit alias system (src/lib/units/aliases.js).
 * All public get/set and manipulation methods accept unit strings that are
 * normalized through this module.
 *
 * @see https://momentjs.com/docs/#/manipulating/
 * @since 0.1.0
 */
#pragma once

#include <string>

namespace polycpp {
namespace moment {

/**
 * @brief Canonical time-unit enumeration.
 *
 * Each value corresponds to a Moment.js unit. Day and Date are distinct:
 * Day refers to the day-of-week (0-6), Date refers to the day-of-month (1-31).
 *
 * @since 0.1.0
 */
enum class Unit {
    Year,          ///< Calendar year
    Quarter,       ///< Quarter of year (1-4)
    Month,         ///< Month (0-11 internally, 1-12 for display)
    Week,          ///< Locale-aware week of year
    IsoWeek,       ///< ISO 8601 week of year
    Day,           ///< Day of week (0=Sunday, 6=Saturday)
    Date,          ///< Day of month (1-31)
    Hour,          ///< Hour (0-23)
    Minute,        ///< Minute (0-59)
    Second,        ///< Second (0-59)
    Millisecond,   ///< Millisecond (0-999)
    WeekYear,      ///< Locale-aware week-numbering year
    IsoWeekYear,   ///< ISO 8601 week-numbering year
    DayOfYear,     ///< Day of year (1-366)
    Weekday,       ///< Locale-aware weekday number
    IsoWeekday,    ///< ISO weekday (1=Monday, 7=Sunday)
    Invalid        ///< Unrecognized unit string
};

/**
 * @brief Normalize a unit string alias to its canonical Unit enum value.
 *
 * Accepts singular, plural, and shorthand forms (e.g. "year", "years", "y"
 * all map to Unit::Year). Case-sensitive for shorthand forms ("M" is Month,
 * "m" is Minute). Falls back to a lowered lookup for long forms.
 *
 * @param unit The unit string to normalize.
 * @return The canonical Unit, or Unit::Invalid if unrecognized.
 * @since 0.1.0
 */
Unit normalizeUnit(const std::string& unit);

/**
 * @brief Convert a Unit enum value back to its canonical string name.
 *
 * Returns the singular long form (e.g. Unit::Year -> "year").
 *
 * @param unit The Unit enum value.
 * @return The canonical string, or "invalid" for Unit::Invalid.
 * @since 0.1.0
 */
std::string unitToString(Unit unit);

} // namespace moment
} // namespace polycpp
