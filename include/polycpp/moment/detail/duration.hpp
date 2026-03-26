/**
 * @file detail/duration.hpp
 * @brief Inline implementation of the Duration class.
 *
 * Follows the Moment.js internal structure: three raw buckets
 * (_milliseconds, _days, _months) that are "bubbled" into individual
 * component fields (years, months, days, hours, minutes, seconds, ms).
 *
 * Key reference files from moment.js:
 *   - src/lib/duration/constructor.js (three-bucket storage)
 *   - src/lib/duration/bubble.js      (bubbling algorithm)
 *   - src/lib/duration/as.js          (total conversion)
 *   - src/lib/duration/humanize.js    (relative-time thresholds)
 *   - src/lib/duration/iso-string.js  (ISO 8601 serialization)
 *
 * @since 0.1.0
 */
#pragma once

#include <polycpp/moment/duration.hpp>
#include <polycpp/moment/units.hpp>
#include <polycpp/moment/locale.hpp>
#include <polycpp/moment/detail/units.hpp>
#include <polycpp/moment/detail/locale.hpp>

#include <cmath>
#include <cstdlib>
#include <regex>
#include <sstream>
#include <string>
#include <variant>

namespace polycpp {
namespace moment {
namespace detail {

// -- Conversion constants matching moment.js bubble.js --

/// @brief Convert days to months (400 years = 146097 days = 4800 months).
inline double daysToMonths(double days) {
    return days * 4800.0 / 146097.0;
}

/// @brief Convert months to days (inverse of daysToMonths).
inline double monthsToDays(double months) {
    return months * 146097.0 / 4800.0;
}

/// @brief Floor toward zero (like Math.trunc but for negative numbers, floors toward zero).
inline int64_t absFloor(double x) {
    return static_cast<int64_t>(x >= 0 ? std::floor(x) : std::ceil(x));
}

/// @brief Ceil away from zero.
inline int64_t absCeil(double x) {
    return static_cast<int64_t>(x >= 0 ? std::ceil(x) : std::floor(x));
}

/// @brief Sign function matching moment.js: returns -1, 0, or 1.
inline int sign(double x) {
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
}

/// @brief Format a RelativeTimeValue with a number, producing a human string.
inline std::string formatRelativeTime(const RelativeTimeValue& val, int number,
                                       bool withoutSuffix, const std::string& key,
                                       bool isFuture) {
    if (std::holds_alternative<std::string>(val)) {
        std::string tmpl = std::get<std::string>(val);
        // Replace %d with the number
        auto pos = tmpl.find("%d");
        if (pos != std::string::npos) {
            tmpl.replace(pos, 2, std::to_string(number));
        }
        return tmpl;
    } else {
        const auto& fn = std::get<RelativeTimeFn>(val);
        return fn(number, withoutSuffix, key, isFuture);
    }
}

/// @brief Look up a relativeTime entry by key string.
inline const RelativeTimeValue& getRelativeTimeEntry(
    const RelativeTimeFormats& rt, const std::string& key) {
    if (key == "s")  return rt.s;
    if (key == "ss") return rt.ss;
    if (key == "m")  return rt.m;
    if (key == "mm") return rt.mm;
    if (key == "h")  return rt.h;
    if (key == "hh") return rt.hh;
    if (key == "d")  return rt.d;
    if (key == "dd") return rt.dd;
    if (key == "w")  return rt.w;
    if (key == "ww") return rt.ww;
    if (key == "M")  return rt.M;
    if (key == "MM") return rt.MM;
    if (key == "y")  return rt.y;
    if (key == "yy") return rt.yy;
    // Fallback (should not happen)
    return rt.s;
}

} // namespace detail

// =========================================================================
// Duration implementation
// =========================================================================

inline void Duration::bubble() {
    int64_t milliseconds = raw_milliseconds_;
    int days = raw_days_;
    int months = raw_months_;

    // If we have a mix of positive and negative values, bubble down first.
    // See: https://github.com/moment/moment/issues/2166
    if (!((milliseconds >= 0 && days >= 0 && months >= 0) ||
          (milliseconds <= 0 && days <= 0 && months <= 0))) {
        milliseconds += detail::absCeil(detail::monthsToDays(months) + days) * 86400000LL;
        days = 0;
        months = 0;
    }

    // Bubble up milliseconds -> seconds -> minutes -> hours
    milliseconds_ = static_cast<int>(milliseconds % 1000);

    int64_t secs = detail::absFloor(static_cast<double>(milliseconds) / 1000.0);
    seconds_ = static_cast<int>(secs % 60);

    int64_t mins = detail::absFloor(static_cast<double>(secs) / 60.0);
    minutes_ = static_cast<int>(mins % 60);

    int64_t hrs = detail::absFloor(static_cast<double>(mins) / 60.0);
    hours_ = static_cast<int>(hrs % 24);

    days += static_cast<int>(detail::absFloor(static_cast<double>(hrs) / 24.0));

    // Convert days to months
    int monthsFromDays = static_cast<int>(detail::absFloor(detail::daysToMonths(days)));
    months += monthsFromDays;
    days -= static_cast<int>(detail::absCeil(detail::monthsToDays(monthsFromDays)));

    // 12 months -> 1 year
    int yrs = static_cast<int>(detail::absFloor(static_cast<double>(months) / 12.0));
    months %= 12;

    days_ = days;
    months_ = months;
    years_ = yrs;

    // IMPORTANT: Do NOT update raw_milliseconds_, raw_days_, raw_months_.
    // The raw buckets are the canonical storage (like moment.js _milliseconds,
    // _days, _months). The bubbled fields (years_, months_, days_, etc.) are
    // the display components (like moment.js _data). The as() function uses
    // the raw buckets directly.
}

inline Duration::Duration()
    : raw_milliseconds_(0), raw_days_(0), raw_months_(0),
      years_(0), months_(0), days_(0), hours_(0), minutes_(0),
      seconds_(0), milliseconds_(0), is_valid_(true), locale_key_("en") {}

inline Duration::Duration(int64_t milliseconds)
    : raw_milliseconds_(milliseconds), raw_days_(0), raw_months_(0),
      is_valid_(true), locale_key_("en") {
    bubble();
}

inline Duration::Duration(int amount, const std::string& unit)
    : raw_milliseconds_(0), raw_days_(0), raw_months_(0),
      is_valid_(true), locale_key_("en") {
    Unit u = normalizeUnit(unit);
    switch (u) {
        case Unit::Year:
            raw_months_ = amount * 12;
            break;
        case Unit::Quarter:
            raw_months_ = amount * 3;
            break;
        case Unit::Month:
            raw_months_ = amount;
            break;
        case Unit::Week:
            raw_days_ = amount * 7;
            break;
        case Unit::Day:
        case Unit::Date:
            raw_days_ = amount;
            break;
        case Unit::Hour:
            raw_milliseconds_ = static_cast<int64_t>(amount) * 3600000LL;
            break;
        case Unit::Minute:
            raw_milliseconds_ = static_cast<int64_t>(amount) * 60000LL;
            break;
        case Unit::Second:
            raw_milliseconds_ = static_cast<int64_t>(amount) * 1000LL;
            break;
        case Unit::Millisecond:
            raw_milliseconds_ = amount;
            break;
        default:
            is_valid_ = false;
            break;
    }
    bubble();
}

inline Duration::Duration(const std::string& iso_string)
    : raw_milliseconds_(0), raw_days_(0), raw_months_(0),
      is_valid_(true), locale_key_("en") {
    // ISO 8601 duration regex, matching moment.js isoRegex
    // Allows optional leading sign, decimal values with . or ,
    static const std::regex isoRegex(
        R"(^(-|\+)?P(?:([-+]?[0-9]*\.?[0-9]*)Y)?(?:([-+]?[0-9]*\.?[0-9]*)M)?(?:([-+]?[0-9]*\.?[0-9]*)W)?(?:([-+]?[0-9]*\.?[0-9]*)D)?(?:T(?:([-+]?[0-9]*\.?[0-9]*)H)?(?:([-+]?[0-9]*\.?[0-9]*)M)?(?:([-+]?[0-9]*\.?[0-9]*)S)?)?$)");

    std::smatch match;
    if (!std::regex_match(iso_string, match, isoRegex)) {
        is_valid_ = false;
        return;
    }

    double signMul = (match[1].matched && match[1].str() == "-") ? -1.0 : 1.0;

    auto parseField = [&](int group) -> double {
        if (!match[group].matched || match[group].str().empty()) return 0.0;
        std::string s = match[group].str();
        // Replace comma with dot for decimal support
        for (auto& c : s) { if (c == ',') c = '.'; }
        double val = std::stod(s);
        return val * signMul;
    };

    double y = parseField(2);
    double M = parseField(3);
    double w = parseField(4);
    double d = parseField(5);
    double h = parseField(6);
    double m = parseField(7);
    double s_val = parseField(8);

    // Split fractional seconds into seconds + milliseconds
    double int_s = 0;
    double frac_s = std::modf(s_val, &int_s);
    int ms_from_frac = static_cast<int>(std::round(frac_s * 1000.0));

    raw_months_ = static_cast<int>(y) * 12 + static_cast<int>(M);
    raw_days_ = static_cast<int>(d) + static_cast<int>(w) * 7;
    raw_milliseconds_ = static_cast<int64_t>(int_s) * 1000LL
                       + static_cast<int64_t>(std::round(h * 3600000.0))
                       + static_cast<int64_t>(std::round(m * 60000.0))
                       + ms_from_frac;

    bubble();
}

inline Duration::Duration(const DurationInput& input)
    : raw_milliseconds_(0), raw_days_(0), raw_months_(0),
      is_valid_(true), locale_key_("en") {
    raw_months_ = input.years * 12 + input.months;
    raw_days_ = input.days + input.weeks * 7;
    raw_milliseconds_ = static_cast<int64_t>(input.milliseconds)
                       + static_cast<int64_t>(input.seconds) * 1000LL
                       + static_cast<int64_t>(input.minutes) * 60000LL
                       + static_cast<int64_t>(input.hours) * 3600000LL;
    bubble();
}

inline Duration::Duration(const polycpp::JsonObject& obj)
    : raw_milliseconds_(0), raw_days_(0), raw_months_(0),
      is_valid_(true), locale_key_("en") {

    auto getInt = [&](const std::string& key, int def) -> int {
        auto it = obj.find(key);
        if (it != obj.end() && it->second.isNumber()) return it->second.asInt();
        return def;
    };

    int years   = getInt("years", 0);
    int months  = getInt("months", 0);
    int weeks   = getInt("weeks", 0);
    int days    = getInt("days", 0);
    int hours   = getInt("hours", 0);
    int minutes = getInt("minutes", 0);
    int seconds = getInt("seconds", 0);
    int ms      = getInt("milliseconds", 0);

    raw_months_ = years * 12 + months;
    raw_days_   = days + weeks * 7;
    raw_milliseconds_ = static_cast<int64_t>(ms)
                       + static_cast<int64_t>(seconds) * 1000LL
                       + static_cast<int64_t>(minutes) * 60000LL
                       + static_cast<int64_t>(hours) * 3600000LL;
    bubble();
}

inline Duration Duration::clone() const {
    return *this;
}

inline Duration& Duration::abs() {
    raw_milliseconds_ = std::abs(raw_milliseconds_);
    raw_days_ = std::abs(raw_days_);
    raw_months_ = std::abs(raw_months_);
    bubble();
    return *this;
}

// -- Arithmetic --

inline Duration& Duration::add(int amount, const std::string& unit) {
    Unit u = normalizeUnit(unit);
    switch (u) {
        case Unit::Year:
            raw_months_ += amount * 12;
            break;
        case Unit::Quarter:
            raw_months_ += amount * 3;
            break;
        case Unit::Month:
            raw_months_ += amount;
            break;
        case Unit::Week:
            raw_days_ += amount * 7;
            break;
        case Unit::Day:
        case Unit::Date:
            raw_days_ += amount;
            break;
        case Unit::Hour:
            raw_milliseconds_ += static_cast<int64_t>(amount) * 3600000LL;
            break;
        case Unit::Minute:
            raw_milliseconds_ += static_cast<int64_t>(amount) * 60000LL;
            break;
        case Unit::Second:
            raw_milliseconds_ += static_cast<int64_t>(amount) * 1000LL;
            break;
        case Unit::Millisecond:
            raw_milliseconds_ += amount;
            break;
        default:
            break;
    }
    bubble();
    return *this;
}

inline Duration& Duration::add(const Duration& other) {
    raw_milliseconds_ += other.raw_milliseconds_;
    raw_days_ += other.raw_days_;
    raw_months_ += other.raw_months_;
    bubble();
    return *this;
}

inline Duration& Duration::subtract(int amount, const std::string& unit) {
    return add(-amount, unit);
}

inline Duration& Duration::subtract(const Duration& other) {
    raw_milliseconds_ -= other.raw_milliseconds_;
    raw_days_ -= other.raw_days_;
    raw_months_ -= other.raw_months_;
    bubble();
    return *this;
}

// -- Humanize --

inline std::string Duration::humanize(bool withSuffix) const {
    if (!is_valid_) {
        return localeData(locale_key_).invalidDate;
    }

    const auto& loc = localeData(locale_key_);
    const auto& rt = loc.relativeTime;

    // Get thresholds
    double th_ss = relativeTimeThreshold("ss");
    double th_s  = relativeTimeThreshold("s");
    double th_m  = relativeTimeThreshold("m");
    double th_h  = relativeTimeThreshold("h");
    double th_d  = relativeTimeThreshold("d");
    double th_w  = relativeTimeThreshold("w");
    double th_M  = relativeTimeThreshold("M");

    // Compute totals using as() for each unit
    double totalMs = asMilliseconds();
    int absSec    = static_cast<int>(std::round(std::abs(as("s"))));
    int absMin    = static_cast<int>(std::round(std::abs(as("m"))));
    int absHour   = static_cast<int>(std::round(std::abs(as("h"))));
    int absDay    = static_cast<int>(std::round(std::abs(as("d"))));
    int absMonth  = static_cast<int>(std::round(std::abs(as("M"))));
    int absWeek   = static_cast<int>(std::round(std::abs(as("w"))));
    int absYear   = static_cast<int>(std::round(std::abs(as("y"))));

    bool isFuture = totalMs > 0;

    // Determine the appropriate unit and key string.
    // This mirrors moment.js humanize.js relativeTime() logic exactly.
    std::string key;
    int number = 0;

    if (absSec <= static_cast<int>(th_ss)) {
        key = "s"; number = absSec;
    } else if (absSec < static_cast<int>(th_s)) {
        key = "ss"; number = absSec;
    } else if (absMin <= 1) {
        key = "m"; number = 1;
    } else if (absMin < static_cast<int>(th_m)) {
        key = "mm"; number = absMin;
    } else if (absHour <= 1) {
        key = "h"; number = 1;
    } else if (absHour < static_cast<int>(th_h)) {
        key = "hh"; number = absHour;
    } else if (absDay <= 1) {
        key = "d"; number = 1;
    } else if (absDay < static_cast<int>(th_d)) {
        key = "dd"; number = absDay;
    } else if (th_w > 0) {
        // Weeks are enabled
        if (absWeek <= 1) {
            key = "w"; number = 1;
        } else if (absWeek < static_cast<int>(th_w)) {
            key = "ww"; number = absWeek;
        } else if (absMonth <= 1) {
            key = "M"; number = 1;
        } else if (absMonth < static_cast<int>(th_M)) {
            key = "MM"; number = absMonth;
        } else if (absYear <= 1) {
            key = "y"; number = 1;
        } else {
            key = "yy"; number = absYear;
        }
    } else {
        if (absMonth <= 1) {
            key = "M"; number = 1;
        } else if (absMonth < static_cast<int>(th_M)) {
            key = "MM"; number = absMonth;
        } else if (absYear <= 1) {
            key = "y"; number = 1;
        } else {
            key = "yy"; number = absYear;
        }
    }

    // Look up the relativeTime entry for this key
    const auto& entry = detail::getRelativeTimeEntry(rt, key);
    std::string output = detail::formatRelativeTime(entry, number, !withSuffix, key, isFuture);

    if (withSuffix) {
        if (isFuture) {
            // Replace %s in "in %s"
            std::string wrapper = rt.future;
            auto pos = wrapper.find("%s");
            if (pos != std::string::npos) {
                wrapper.replace(pos, 2, output);
            }
            output = wrapper;
        } else {
            // Replace %s in "%s ago"
            std::string wrapper = rt.past;
            auto pos = wrapper.find("%s");
            if (pos != std::string::npos) {
                wrapper.replace(pos, 2, output);
            }
            output = wrapper;
        }
    }

    return output;
}

// -- Component getters --

inline int Duration::years() const { return is_valid_ ? years_ : 0; }
inline int Duration::months() const { return is_valid_ ? months_ : 0; }
inline int Duration::days() const { return is_valid_ ? days_ : 0; }
inline int Duration::weeks() const {
    return is_valid_ ? detail::absFloor(static_cast<double>(days_) / 7.0) : 0;
}
inline int Duration::hours() const { return is_valid_ ? hours_ : 0; }
inline int Duration::minutes() const { return is_valid_ ? minutes_ : 0; }
inline int Duration::seconds() const { return is_valid_ ? seconds_ : 0; }
inline int Duration::milliseconds() const { return is_valid_ ? milliseconds_ : 0; }

inline int Duration::get(const std::string& unit) const {
    Unit u = normalizeUnit(unit);
    switch (u) {
        case Unit::Year:        return years();
        case Unit::Month:       return months();
        case Unit::Week:        return weeks();
        case Unit::Day:
        case Unit::Date:        return days();
        case Unit::Hour:        return hours();
        case Unit::Minute:      return minutes();
        case Unit::Second:      return seconds();
        case Unit::Millisecond: return milliseconds();
        default:                return 0;
    }
}

// -- Total conversion (as) --
// These follow moment.js as.js exactly, using the three-bucket storage.

inline double Duration::as(const std::string& unit) const {
    if (!is_valid_) return 0.0;

    Unit u = normalizeUnit(unit);

    if (u == Unit::Month || u == Unit::Quarter || u == Unit::Year) {
        // For month/quarter/year, convert days+ms into months
        double days = raw_days_ + static_cast<double>(raw_milliseconds_) / 86400000.0;
        double months = raw_months_ + detail::daysToMonths(days);
        switch (u) {
            case Unit::Month:   return months;
            case Unit::Quarter: return months / 3.0;
            case Unit::Year:    return months / 12.0;
            default: break;
        }
    }

    // For all other units, convert months to days then compute
    double days = raw_days_ + std::round(detail::monthsToDays(raw_months_));
    double ms = raw_milliseconds_;
    switch (u) {
        case Unit::Week:
            return days / 7.0 + ms / 604800000.0;
        case Unit::Day:
        case Unit::Date:
            return days + ms / 86400000.0;
        case Unit::Hour:
            return days * 24.0 + ms / 3600000.0;
        case Unit::Minute:
            return days * 1440.0 + ms / 60000.0;
        case Unit::Second:
            return days * 86400.0 + ms / 1000.0;
        case Unit::Millisecond:
            return std::floor(days * 86400000.0) + ms;
        default:
            return 0.0;
    }
}

inline double Duration::asMilliseconds() const { return as("ms"); }
inline double Duration::asSeconds() const { return as("s"); }
inline double Duration::asMinutes() const { return as("m"); }
inline double Duration::asHours() const { return as("h"); }
inline double Duration::asDays() const { return as("d"); }
inline double Duration::asWeeks() const { return as("w"); }
inline double Duration::asMonths() const { return as("M"); }
inline double Duration::asYears() const { return as("y"); }

// -- Serialization --

inline std::string Duration::toISOString() const {
    if (!is_valid_) {
        return localeData(locale_key_).invalidDate;
    }

    // For ISO strings, moment.js does NOT use the normal bubbling rules.
    // Instead:
    //   - milliseconds bubble up to hours (but not to days)
    //   - months bubble up to years (12 months -> 1 year)
    //   - days do not bubble at all
    double absMs = std::abs(static_cast<double>(raw_milliseconds_));
    int absDays = std::abs(raw_days_);
    int absMonths = std::abs(raw_months_);

    // Compute total as seconds (for sign detection and zero check)
    double totalSec = as("s");
    if (totalSec == 0.0) {
        return "P0D";
    }

    // Bubble ms -> seconds -> minutes -> hours
    double seconds = absMs / 1000.0;
    int64_t mins = detail::absFloor(seconds / 60.0);
    int64_t hours = detail::absFloor(static_cast<double>(mins) / 60.0);
    seconds = std::fmod(seconds, 60.0);
    mins = mins % 60;

    // Bubble months -> years
    int years = detail::absFloor(static_cast<double>(absMonths) / 12.0);
    int months = absMonths % 12;

    // Compute signs for each component group
    // totalSign: overall sign
    int totalSign = detail::sign(totalSec);
    // ymSign: sign mismatch between months and total
    std::string ymSignStr = (detail::sign(raw_months_) != totalSign) ? "-" : "";
    // daysSign: sign mismatch between days and total
    std::string daysSignStr = (detail::sign(raw_days_) != totalSign) ? "-" : "";
    // hmsSign: sign mismatch between milliseconds and total
    std::string hmsSignStr = (detail::sign(raw_milliseconds_) != totalSign) ? "-" : "";

    // Format seconds: remove trailing zeros after decimal point
    std::string secStr;
    if (seconds != 0.0) {
        std::ostringstream oss;
        oss << std::fixed;
        // Use 3 decimal places then strip trailing zeros
        oss.precision(3);
        oss << seconds;
        secStr = oss.str();
        // Remove trailing zeros after decimal point
        if (secStr.find('.') != std::string::npos) {
            auto last = secStr.find_last_not_of('0');
            if (last != std::string::npos && secStr[last] == '.') {
                secStr.erase(last);
            } else {
                secStr.erase(last + 1);
            }
        }
    }

    std::string result;
    if (totalSign < 0) result += "-";
    result += "P";
    if (years)   result += ymSignStr + std::to_string(years) + "Y";
    if (months)  result += ymSignStr + std::to_string(months) + "M";
    if (absDays) result += daysSignStr + std::to_string(absDays) + "D";
    if (hours || mins || seconds != 0.0) {
        result += "T";
        if (hours) result += hmsSignStr + std::to_string(hours) + "H";
        if (mins)  result += hmsSignStr + std::to_string(mins) + "M";
        if (seconds != 0.0) result += hmsSignStr + secStr + "S";
    }

    return result;
}

inline std::string Duration::toJSON() const {
    return toISOString();
}

inline polycpp::JsonObject Duration::toObject() const {
    return polycpp::JsonObject{
        {"years", years_},
        {"months", months_},
        {"days", days_},
        {"hours", hours_},
        {"minutes", minutes_},
        {"seconds", seconds_},
        {"milliseconds", milliseconds_}
    };
}

inline int64_t Duration::valueOf() const {
    return static_cast<int64_t>(asMilliseconds());
}

// -- Validation --

inline bool Duration::isValid() const {
    return is_valid_;
}

// -- Locale --

inline std::string Duration::locale() const {
    return locale_key_;
}

inline Duration& Duration::locale(const std::string& key) {
    locale_key_ = key;
    return *this;
}

// -- Factory functions --

inline Duration duration(int64_t milliseconds) {
    return Duration(milliseconds);
}

inline Duration duration(int amount, const std::string& unit) {
    return Duration(amount, unit);
}

inline Duration duration(const std::string& iso_string) {
    return Duration(iso_string);
}

inline Duration duration(const DurationInput& input) {
    return Duration(input);
}

inline Duration duration(const polycpp::JsonObject& obj) {
    return Duration(obj);
}

} // namespace moment
} // namespace polycpp
