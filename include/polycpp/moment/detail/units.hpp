/**
 * @file detail/units.hpp
 * @brief Inline implementation of unit normalization and conversion.
 *
 * Contains the static lookup table mapping Moment.js unit aliases to the
 * canonical Unit enum. The alias set mirrors src/lib/units/aliases.js in
 * the original Moment.js source.
 *
 * @since 0.1.0
 */
#pragma once

#include <polycpp/moment/units.hpp>
#include <algorithm>
#include <string>
#include <unordered_map>

namespace polycpp {
namespace moment {

/**
 * @brief Look up a unit string in the alias table.
 *
 * The table is searched in two passes:
 *  1. Exact match (handles case-sensitive shorthand like "M" vs "m").
 *  2. Lowercased match (handles case-insensitive long forms like "Month").
 *
 * This mirrors the Moment.js behaviour:
 *   aliases[units] || aliases[units.toLowerCase()]
 */
inline Unit normalizeUnit(const std::string& unit) {
    // Static alias table — built once, never modified.
    static const std::unordered_map<std::string, Unit> aliases = {
        // Year
        {"year",   Unit::Year},
        {"years",  Unit::Year},
        {"y",      Unit::Year},

        // Quarter
        {"quarter",  Unit::Quarter},
        {"quarters", Unit::Quarter},
        {"Q",        Unit::Quarter},

        // Month — note: "M" (uppercase) is Month, "m" (lowercase) is Minute
        {"month",  Unit::Month},
        {"months", Unit::Month},
        {"M",      Unit::Month},

        // Week
        {"week",  Unit::Week},
        {"weeks", Unit::Week},
        {"w",     Unit::Week},

        // IsoWeek
        {"isoWeek",  Unit::IsoWeek},
        {"isoWeeks", Unit::IsoWeek},
        {"isoweek",  Unit::IsoWeek},
        {"isoweeks", Unit::IsoWeek},
        {"W",        Unit::IsoWeek},

        // Day (day-of-week)
        {"day",  Unit::Day},
        {"days", Unit::Day},
        {"d",    Unit::Day},

        // Date (day-of-month)
        {"date",  Unit::Date},
        {"dates", Unit::Date},
        {"D",     Unit::Date},

        // Hour
        {"hour",  Unit::Hour},
        {"hours", Unit::Hour},
        {"h",     Unit::Hour},

        // Minute — note: "m" (lowercase) is Minute, "M" (uppercase) is Month
        {"minute",  Unit::Minute},
        {"minutes", Unit::Minute},
        {"m",       Unit::Minute},

        // Second
        {"second",  Unit::Second},
        {"seconds", Unit::Second},
        {"s",       Unit::Second},

        // Millisecond
        {"millisecond",  Unit::Millisecond},
        {"milliseconds", Unit::Millisecond},
        {"ms",           Unit::Millisecond},

        // WeekYear
        {"weekYear",  Unit::WeekYear},
        {"weekYears", Unit::WeekYear},
        {"weekyear",  Unit::WeekYear},
        {"weekyears", Unit::WeekYear},
        {"gg",        Unit::WeekYear},

        // IsoWeekYear
        {"isoWeekYear",  Unit::IsoWeekYear},
        {"isoWeekYears", Unit::IsoWeekYear},
        {"isoweekyear",  Unit::IsoWeekYear},
        {"isoweekyears", Unit::IsoWeekYear},
        {"GG",           Unit::IsoWeekYear},

        // DayOfYear
        {"dayOfYear",  Unit::DayOfYear},
        {"dayOfYears", Unit::DayOfYear},
        {"dayofyear",  Unit::DayOfYear},
        {"dayofyears", Unit::DayOfYear},
        {"DDD",        Unit::DayOfYear},

        // Weekday
        {"weekday",  Unit::Weekday},
        {"weekdays", Unit::Weekday},
        {"e",        Unit::Weekday},

        // IsoWeekday
        {"isoWeekday",  Unit::IsoWeekday},
        {"isoWeekdays", Unit::IsoWeekday},
        {"isoweekday",  Unit::IsoWeekday},
        {"isoweekdays", Unit::IsoWeekday},
        {"E",           Unit::IsoWeekday},
    };

    // Pass 1: exact match (preserves case-sensitive shorthand)
    auto it = aliases.find(unit);
    if (it != aliases.end()) {
        return it->second;
    }

    // Pass 2: lowercased lookup (handles "Year", "MONTH", etc.)
    std::string lower = unit;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    it = aliases.find(lower);
    if (it != aliases.end()) {
        return it->second;
    }

    return Unit::Invalid;
}

/**
 * @brief Convert a Unit enum back to its canonical singular string form.
 */
inline std::string unitToString(Unit unit) {
    switch (unit) {
        case Unit::Year:        return "year";
        case Unit::Quarter:     return "quarter";
        case Unit::Month:       return "month";
        case Unit::Week:        return "week";
        case Unit::IsoWeek:     return "isoWeek";
        case Unit::Day:         return "day";
        case Unit::Date:        return "date";
        case Unit::Hour:        return "hour";
        case Unit::Minute:      return "minute";
        case Unit::Second:      return "second";
        case Unit::Millisecond: return "millisecond";
        case Unit::WeekYear:    return "weekYear";
        case Unit::IsoWeekYear: return "isoWeekYear";
        case Unit::DayOfYear:   return "dayOfYear";
        case Unit::Weekday:     return "weekday";
        case Unit::IsoWeekday:  return "isoWeekday";
        case Unit::Invalid:     return "invalid";
    }
    return "invalid"; // unreachable, silences compiler warning
}

} // namespace moment
} // namespace polycpp
