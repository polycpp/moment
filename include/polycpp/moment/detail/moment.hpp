/**
 * @file detail/moment.hpp
 * @brief Moment class inline implementations.
 *
 * Contains all get/set, manipulation, and UTC/local mode logic.
 * Uses polycpp::Date for timestamp <-> component decomposition.
 *
 * @since 0.1.0
 */
#pragma once

#include <polycpp/moment/moment.hpp>
#include <polycpp/moment/units.hpp>
#include <polycpp/moment/locale.hpp>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <limits>
#include <string>
#include <variant>
#include <polycpp/core/date.hpp>
#include <polycpp/core/math.hpp>
#include <polycpp/core/number.hpp>

namespace polycpp {
namespace moment {

// ── Helper: days in a given month (0-based month) ─────────────────────

namespace detail {

inline bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

inline int daysInMonth(int year, int month) {
    // month is 0-based (0=Jan, 11=Dec)
    // Normalize month into valid range
    int m = ((month % 12) + 12) % 12;
    year += (month - m) / 12;
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 1 && isLeapYear(year)) return 29;
    return days[m];
}

inline int daysInYear(int year) {
    return isLeapYear(year) ? 366 : 365;
}

/// @brief Compute day of year (1-based) from year/month(0-based)/day(1-based).
inline int dayOfYear(int year, int month, int day) {
    static const int cumDays[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    int doy = cumDays[month] + day;
    if (month > 1 && isLeapYear(year)) {
        doy += 1;
    }
    return doy;
}

/// @brief Get the day of week (0=Sunday) for a given UTC date.
/// Uses Tomohiko Sakamoto's algorithm.
inline int dayOfWeekFromDate(int y, int m, int d) {
    // m is 0-based (0=Jan)
    // Convert to 1-based month for the algorithm
    int month = m + 1;
    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (month < 3) y -= 1;
    int dow = (y + y / 4 - y / 100 + y / 400 + t[month - 1] + d) % 7;
    return dow; // 0=Sunday
}

// ── Week calculation helpers (ported from moment.js week-calendar-utils.js) ──

/// @brief Compute the offset from Jan 1 to the start of the first week.
/// @param year Calendar year.
/// @param dow  First day of week (0=Sunday, 1=Monday, ...).
/// @param doy  Day-of-year threshold for the first week (6=US, 4=ISO).
/// @return Offset in days from Jan 1 to start of first week (can be negative).
inline int firstWeekOffset(int year, int dow, int doy) {
    // fwd: first-week day -- which January date is always in the first week
    int fwd = 7 + dow - doy;
    // fwdlw: first-week day local weekday -- which local weekday is fwd
    int fwdlw = (7 + dayOfWeekFromDate(year, 0, fwd) - dow) % 7;
    return -fwdlw + fwd - 1;
}

/// @brief Compute the number of weeks in a year given week config.
inline int weeksInYear(int year, int dow, int doy) {
    int weekOffset = firstWeekOffset(year, dow, doy);
    int weekOffsetNext = firstWeekOffset(year + 1, dow, doy);
    return (daysInYear(year) - weekOffset + weekOffsetNext) / 7;
}

struct WeekResult {
    int week;
    int year;
};

/// @brief Compute the week number and week-year for a given moment.
/// @param momYear  The calendar year of the moment.
/// @param momDayOfYear The day-of-year (1-based) of the moment.
/// @param dow First day of week.
/// @param doy First week threshold.
inline WeekResult weekOfYear(int momYear, int momDayOfYear, int dow, int doy) {
    int weekOffset = firstWeekOffset(momYear, dow, doy);
    int week = static_cast<int>(polycpp::Math::floor(static_cast<double>(momDayOfYear - weekOffset - 1) / 7.0)) + 1;
    int resWeek, resYear;

    if (week < 1) {
        resYear = momYear - 1;
        resWeek = week + weeksInYear(resYear, dow, doy);
    } else if (week > weeksInYear(momYear, dow, doy)) {
        resWeek = week - weeksInYear(momYear, dow, doy);
        resYear = momYear + 1;
    } else {
        resYear = momYear;
        resWeek = week;
    }

    return {resWeek, resYear};
}

/// @brief Compute day-of-year from week-year, week number, weekday, and week config.
struct DayOfYearResult {
    int year;
    int dayOfYear;
};

inline DayOfYearResult dayOfYearFromWeeks(int year, int week, int weekday, int dow, int doy) {
    int localWeekday = (7 + weekday - dow) % 7;
    int weekOffset = firstWeekOffset(year, dow, doy);
    int dayOfYear = 1 + 7 * (week - 1) + localWeekday + weekOffset;
    int resYear, resDayOfYear;

    if (dayOfYear <= 0) {
        resYear = year - 1;
        resDayOfYear = daysInYear(resYear) + dayOfYear;
    } else if (dayOfYear > daysInYear(year)) {
        resYear = year + 1;
        resDayOfYear = dayOfYear - daysInYear(year);
    } else {
        resYear = year;
        resDayOfYear = dayOfYear;
    }

    return {resYear, resDayOfYear};
}

/// @brief Convert day-of-year (1-based) to month (0-based) and day (1-based).
inline void dayOfYearToMonthDay(int year, int doy, int& outMonth, int& outDay) {
    static const int cumDays[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
    static const int cumDaysLeap[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};
    const int* cum = isLeapYear(year) ? cumDaysLeap : cumDays;
    for (int m = 11; m >= 0; --m) {
        if (doy > cum[m]) {
            outMonth = m;
            outDay = doy - cum[m];
            return;
        }
    }
    outMonth = 0;
    outDay = doy;
}

/// @brief Compute the UTC offset of the local timezone for a given UTC timestamp (ms).
/// Returns offset in minutes (e.g., EST = -300, IST = +330).
/// Uses polycpp::Date::getTimezoneOffset() which returns UTC-local in minutes.
inline int localUtcOffsetMinutes(int64_t timestamp_ms) {
    polycpp::Date d(static_cast<double>(timestamp_ms));
    // getTimezoneOffset() returns minutes: UTC - local (positive = west of UTC)
    // Our convention: offset = local - UTC (positive = east of UTC)
    return -static_cast<int>(d.getTimezoneOffset());
}

/// @brief Parse a UTC offset string like "+05:30", "-0530", "+05", "Z" to minutes.
inline int parseOffsetString(const std::string& s) {
    if (s.empty() || s == "Z" || s == "z") return 0;

    int sign = 1;
    size_t pos = 0;
    if (s[0] == '+') { sign = 1; pos = 1; }
    else if (s[0] == '-') { sign = -1; pos = 1; }

    std::string rest = s.substr(pos);
    int hours = 0, minutes = 0;

    // Remove colon if present
    std::string clean;
    for (char c : rest) {
        if (c != ':') clean += c;
    }

    if (clean.size() <= 2) {
        hours = polycpp::Number::parseInt(clean);
    } else if (clean.size() == 3) {
        hours = polycpp::Number::parseInt(clean.substr(0, 1));
        minutes = polycpp::Number::parseInt(clean.substr(1, 2));
    } else if (clean.size() >= 4) {
        hours = polycpp::Number::parseInt(clean.substr(0, 2));
        minutes = polycpp::Number::parseInt(clean.substr(2, 2));
    }

    return sign * (hours * 60 + minutes);
}

} // namespace detail

// ── Constructors ─────────────────────────────────────────────────────

inline Moment::Moment() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    timestamp_ms_ = ms;
}

inline Moment::Moment(int64_t timestamp_ms) : timestamp_ms_(timestamp_ms) {}

// ── Component decomposition ──────────────────────────────────────────

inline Moment::DateComponents Moment::toComponents() const {
    if (is_utc_ || has_fixed_offset_) {
        // For UTC or fixed offset: adjust timestamp by offset, decompose as UTC
        double adjusted = static_cast<double>(timestamp_ms_);
        if (has_fixed_offset_ && !is_utc_) {
            adjusted += static_cast<double>(utc_offset_minutes_) * 60000.0;
        }
        polycpp::Date d(adjusted);
        return {
            static_cast<int>(d.getUTCFullYear()),
            static_cast<int>(d.getUTCMonth()),
            static_cast<int>(d.getUTCDate()),
            static_cast<int>(d.getUTCHours()),
            static_cast<int>(d.getUTCMinutes()),
            static_cast<int>(d.getUTCSeconds()),
            static_cast<int>(d.getUTCMilliseconds())
        };
    } else {
        // For local time: decompose using local getters
        polycpp::Date d(static_cast<double>(timestamp_ms_));
        return {
            static_cast<int>(d.getFullYear()),
            static_cast<int>(d.getMonth()),
            static_cast<int>(d.getDate()),
            static_cast<int>(d.getHours()),
            static_cast<int>(d.getMinutes()),
            static_cast<int>(d.getSeconds()),
            static_cast<int>(d.getMilliseconds())
        };
    }
}

inline void Moment::fromComponents(const DateComponents& c) {
    if (is_utc_ || has_fixed_offset_) {
        // Build UTC timestamp from components using Date::UTC()
        double utc_ms = polycpp::Date::UTC(c.year, c.month, c.day,
                                            c.hour, c.minute, c.second, c.ms);
        // Date::UTC maps years 0-99 to 1900-1999 per ES spec.
        // moment.js treats these as literal years, so correct if needed.
        if (c.year >= 0 && c.year <= 99) {
            polycpp::Date temp(utc_ms);
            temp.setUTCFullYear(c.year);
            utc_ms = temp.getTime();
        }
        timestamp_ms_ = static_cast<int64_t>(utc_ms);
        if (has_fixed_offset_ && !is_utc_) {
            // Components were in offset time, so subtract offset to get UTC
            timestamp_ms_ -= static_cast<int64_t>(utc_offset_minutes_) * 60000LL;
        }
    } else {
        // Build local timestamp using Date constructor (local time)
        polycpp::Date d(c.year, c.month, c.day, c.hour, c.minute, c.second, c.ms);
        // Same year 0-99 correction for local time
        if (c.year >= 0 && c.year <= 99) {
            d.setFullYear(c.year);
        }
        timestamp_ms_ = static_cast<int64_t>(d.getTime());
    }
}

// ── Get/Set: Year ────────────────────────────────────────────────────

inline int Moment::year() const {
    return toComponents().year;
}

inline Moment& Moment::year(int value) {
    auto c = toComponents();
    c.year = value;
    // Clamp day if needed (e.g., Feb 29 in non-leap year)
    int maxDay = detail::daysInMonth(c.year, c.month);
    if (c.day > maxDay) c.day = maxDay;
    fromComponents(c);
    return *this;
}

// ── Get/Set: Month ───────────────────────────────────────────────────

inline int Moment::month() const {
    return toComponents().month;
}

inline Moment& Moment::month(int value) {
    auto c = toComponents();
    // Handle month overflow/underflow by adjusting year
    if (value >= 0) {
        c.year += value / 12;
        c.month = value % 12;
    } else {
        // For negative months
        int adj = (-value - 1) / 12 + 1;
        c.year -= adj;
        c.month = value + adj * 12;
    }
    // Clamp day to valid range for the new month
    int maxDay = detail::daysInMonth(c.year, c.month);
    if (c.day > maxDay) c.day = maxDay;
    fromComponents(c);
    return *this;
}

// ── Get/Set: Date (day of month) ─────────────────────────────────────

inline int Moment::date() const {
    return toComponents().day;
}

inline Moment& Moment::date(int value) {
    auto c = toComponents();
    c.day = value;
    fromComponents(c);
    return *this;
}

// ── Get/Set: Day (day of week) ───────────────────────────────────────

inline int Moment::day() const {
    auto c = toComponents();
    return detail::dayOfWeekFromDate(c.year, c.month, c.day);
}

inline Moment& Moment::day(int value) {
    int current = day();
    int diff = value - current;
    timestamp_ms_ += static_cast<int64_t>(diff) * 86400000LL;
    return *this;
}

// ── Get/Set: Weekday (locale-aware) ──────────────────────────────────

inline int Moment::weekday() const {
    const auto& loc = localeData(locale_key_);
    int dow = loc.week.dow;
    return (day() + 7 - dow) % 7;
}

inline Moment& Moment::weekday(int value) {
    const auto& loc = localeData(locale_key_);
    int dow = loc.week.dow;
    // Convert locale weekday back to absolute day-of-week
    int targetDow = (value + dow) % 7;
    return day(targetDow);
}

// ── Get/Set: ISO Weekday ─────────────────────────────────────────────

inline int Moment::isoWeekday() const {
    int d = day();
    return d == 0 ? 7 : d; // Sunday=7, Monday=1
}

inline Moment& Moment::isoWeekday(int value) {
    // ISO weekday: 1=Monday, 7=Sunday
    int targetDow = value == 7 ? 0 : value; // Convert to 0-6 (Sunday=0)
    return day(targetDow);
}

// ── Get/Set: Day of Year ─────────────────────────────────────────────

inline int Moment::dayOfYear() const {
    auto c = toComponents();
    return detail::dayOfYear(c.year, c.month, c.day);
}

inline Moment& Moment::dayOfYear(int value) {
    auto c = toComponents();
    // Set to Jan 1 of same year, then add (value-1) days
    c.month = 0;
    c.day = value; // polycpp::Date handles overflow normalization
    fromComponents(c);
    return *this;
}

// ── Get/Set: Week (locale-aware) ─────────────────────────────────────

inline int Moment::week() const {
    const auto& loc = localeData(locale_key_);
    int doy = dayOfYear();
    int yr = year();
    auto result = detail::weekOfYear(yr, doy, loc.week.dow, loc.week.doy);
    return result.week;
}

inline Moment& Moment::week(int value) {
    int currentWeek = week();
    int diff = value - currentWeek;
    timestamp_ms_ += static_cast<int64_t>(diff) * 7 * 86400000LL;
    return *this;
}

// ── Get/Set: ISO Week ────────────────────────────────────────────────

inline int Moment::isoWeek() const {
    int doy = dayOfYear();
    int yr = year();
    auto result = detail::weekOfYear(yr, doy, 1, 4); // ISO: dow=1(Mon), doy=4
    return result.week;
}

inline Moment& Moment::isoWeek(int value) {
    int currentWeek = isoWeek();
    int diff = value - currentWeek;
    timestamp_ms_ += static_cast<int64_t>(diff) * 7 * 86400000LL;
    return *this;
}

// ── Get/Set: Week Year (locale-aware) ────────────────────────────────

inline int Moment::weekYear() const {
    const auto& loc = localeData(locale_key_);
    int doy = dayOfYear();
    int yr = year();
    auto result = detail::weekOfYear(yr, doy, loc.week.dow, loc.week.doy);
    return result.year;
}

inline Moment& Moment::weekYear(int value) {
    const auto& loc = localeData(locale_key_);
    int currentWeekYear = weekYear();
    int w = week();
    int wd = weekday() + loc.week.dow; // absolute weekday
    int dow = loc.week.dow;
    int doy = loc.week.doy;

    // Clamp week to max weeks in target year
    int maxWeeks = detail::weeksInYear(value, dow, doy);
    if (w > maxWeeks) w = maxWeeks;

    auto result = detail::dayOfYearFromWeeks(value, w, wd, dow, doy);

    // Convert dayOfYear result to a date
    int outMonth, outDay;
    detail::dayOfYearToMonthDay(result.year, result.dayOfYear, outMonth, outDay);

    auto c = toComponents();
    c.year = result.year;
    c.month = outMonth;
    c.day = outDay;
    fromComponents(c);
    return *this;
}

// ── Get/Set: ISO Week Year ───────────────────────────────────────────

inline int Moment::isoWeekYear() const {
    int doy = dayOfYear();
    int yr = year();
    auto result = detail::weekOfYear(yr, doy, 1, 4);
    return result.year;
}

inline Moment& Moment::isoWeekYear(int value) {
    int w = isoWeek();
    int wd = isoWeekday();

    // Clamp week to max weeks in target year
    int maxWeeks = detail::weeksInYear(value, 1, 4);
    if (w > maxWeeks) w = maxWeeks;

    auto result = detail::dayOfYearFromWeeks(value, w, wd, 1, 4);

    int outMonth, outDay;
    detail::dayOfYearToMonthDay(result.year, result.dayOfYear, outMonth, outDay);

    auto c = toComponents();
    c.year = result.year;
    c.month = outMonth;
    c.day = outDay;
    fromComponents(c);
    return *this;
}

// ── Get/Set: Quarter ─────────────────────────────────────────────────

inline int Moment::quarter() const {
    return (month() / 3) + 1;
}

inline Moment& Moment::quarter(int value) {
    int currentQuarter = quarter();
    int diff = value - currentQuarter;
    return add(diff * 3, "month");
}

// ── Get/Set: Hour ────────────────────────────────────────────────────

inline int Moment::hour() const {
    return toComponents().hour;
}

inline Moment& Moment::hour(int value) {
    auto c = toComponents();
    c.hour = value;
    fromComponents(c);
    return *this;
}

// ── Get/Set: Minute ──────────────────────────────────────────────────

inline int Moment::minute() const {
    return toComponents().minute;
}

inline Moment& Moment::minute(int value) {
    auto c = toComponents();
    c.minute = value;
    fromComponents(c);
    return *this;
}

// ── Get/Set: Second ──────────────────────────────────────────────────

inline int Moment::second() const {
    return toComponents().second;
}

inline Moment& Moment::second(int value) {
    auto c = toComponents();
    c.second = value;
    fromComponents(c);
    return *this;
}

// ── Get/Set: Millisecond ─────────────────────────────────────────────

inline int Moment::millisecond() const {
    return toComponents().ms;
}

inline Moment& Moment::millisecond(int value) {
    auto c = toComponents();
    c.ms = value;
    fromComponents(c);
    return *this;
}

// ── Generic Get/Set ──────────────────────────────────────────────────

inline int Moment::get(const std::string& unit) const {
    Unit u = normalizeUnit(unit);
    switch (u) {
        case Unit::Year:        return year();
        case Unit::Month:       return month();
        case Unit::Date:        return date();
        case Unit::Day:         return day();
        case Unit::Weekday:     return weekday();
        case Unit::IsoWeekday:  return isoWeekday();
        case Unit::DayOfYear:   return dayOfYear();
        case Unit::Week:        return week();
        case Unit::IsoWeek:     return isoWeek();
        case Unit::WeekYear:    return weekYear();
        case Unit::IsoWeekYear: return isoWeekYear();
        case Unit::Quarter:     return quarter();
        case Unit::Hour:        return hour();
        case Unit::Minute:      return minute();
        case Unit::Second:      return second();
        case Unit::Millisecond: return millisecond();
        default:                return 0;
    }
}

inline Moment& Moment::set(const std::string& unit, int value) {
    Unit u = normalizeUnit(unit);
    switch (u) {
        case Unit::Year:        return year(value);
        case Unit::Month:       return month(value);
        case Unit::Date:        return date(value);
        case Unit::Day:         return day(value);
        case Unit::Weekday:     return weekday(value);
        case Unit::IsoWeekday:  return isoWeekday(value);
        case Unit::DayOfYear:   return dayOfYear(value);
        case Unit::Week:        return week(value);
        case Unit::IsoWeek:     return isoWeek(value);
        case Unit::WeekYear:    return weekYear(value);
        case Unit::IsoWeekYear: return isoWeekYear(value);
        case Unit::Quarter:     return quarter(value);
        case Unit::Hour:        return hour(value);
        case Unit::Minute:      return minute(value);
        case Unit::Second:      return second(value);
        case Unit::Millisecond: return millisecond(value);
        default:                return *this;
    }
}

// ── UTC Offset ───────────────────────────────────────────────────────

inline int Moment::utcOffset() const {
    if (has_fixed_offset_) {
        return utc_offset_minutes_;
    }
    if (is_utc_) {
        return 0;
    }
    // Compute local offset
    return detail::localUtcOffsetMinutes(timestamp_ms_);
}

inline Moment& Moment::utcOffset(int offset_minutes, bool keepLocalTime) {
    if (keepLocalTime) {
        // Keep the same wall-clock time but change the offset
        int oldOffset = utcOffset();
        has_fixed_offset_ = true;
        utc_offset_minutes_ = offset_minutes;
        is_utc_ = (offset_minutes == 0);
        // Adjust timestamp to preserve wall-clock time
        timestamp_ms_ += static_cast<int64_t>(oldOffset - offset_minutes) * 60000LL;
    } else {
        has_fixed_offset_ = true;
        utc_offset_minutes_ = offset_minutes;
        is_utc_ = (offset_minutes == 0);
    }
    return *this;
}

inline Moment& Moment::utcOffset(const std::string& offset, bool keepLocalTime) {
    int minutes = detail::parseOffsetString(offset);
    return utcOffset(minutes, keepLocalTime);
}

// ── Days In Month / Weeks In Year ────────────────────────────────────

inline int Moment::daysInMonth() const {
    auto c = toComponents();
    return detail::daysInMonth(c.year, c.month);
}

inline int Moment::weeksInYear() const {
    const auto& loc = localeData(locale_key_);
    return detail::weeksInYear(year(), loc.week.dow, loc.week.doy);
}

inline int Moment::isoWeeksInYear() const {
    return detail::weeksInYear(year(), 1, 4);
}

// ── Manipulation: add ────────────────────────────────────────────────

inline Moment& Moment::add(int amount, const std::string& unit) {
    Unit u = normalizeUnit(unit);
    switch (u) {
        case Unit::Millisecond:
            timestamp_ms_ += amount;
            break;
        case Unit::Second:
            timestamp_ms_ += static_cast<int64_t>(amount) * 1000LL;
            break;
        case Unit::Minute:
            timestamp_ms_ += static_cast<int64_t>(amount) * 60000LL;
            break;
        case Unit::Hour:
            timestamp_ms_ += static_cast<int64_t>(amount) * 3600000LL;
            break;
        case Unit::Day:
        case Unit::Date:
            timestamp_ms_ += static_cast<int64_t>(amount) * 86400000LL;
            break;
        case Unit::Week:
        case Unit::IsoWeek:
            timestamp_ms_ += static_cast<int64_t>(amount) * 604800000LL;
            break;
        case Unit::Month: {
            auto c = toComponents();
            int newMonth = c.month + amount;
            if (newMonth >= 0) {
                c.year += newMonth / 12;
                c.month = newMonth % 12;
            } else {
                int adj = (-newMonth - 1) / 12 + 1;
                c.year -= adj;
                c.month = newMonth + adj * 12;
            }
            // Clamp day to valid range for new month
            int maxDay = detail::daysInMonth(c.year, c.month);
            if (c.day > maxDay) c.day = maxDay;
            fromComponents(c);
            break;
        }
        case Unit::Year: {
            auto c = toComponents();
            c.year += amount;
            // Clamp day if needed (Feb 29 -> Feb 28)
            int maxDay = detail::daysInMonth(c.year, c.month);
            if (c.day > maxDay) c.day = maxDay;
            fromComponents(c);
            break;
        }
        case Unit::Quarter:
            return add(amount * 3, "month");
        default:
            break;
    }
    return *this;
}

// ── Manipulation: subtract ───────────────────────────────────────────

inline Moment& Moment::subtract(int amount, const std::string& unit) {
    return add(-amount, unit);
}

// ── Manipulation: startOf ────────────────────────────────────────────

inline Moment& Moment::startOf(const std::string& unit) {
    Unit u = normalizeUnit(unit);
    if (u == Unit::Invalid || u == Unit::Millisecond) {
        return *this;
    }

    auto c = toComponents();

    switch (u) {
        case Unit::Year:
            c.month = 0;
            c.day = 1;
            c.hour = 0;
            c.minute = 0;
            c.second = 0;
            c.ms = 0;
            fromComponents(c);
            break;
        case Unit::Quarter: {
            int qMonth = c.month - (c.month % 3);
            c.month = qMonth;
            c.day = 1;
            c.hour = 0;
            c.minute = 0;
            c.second = 0;
            c.ms = 0;
            fromComponents(c);
            break;
        }
        case Unit::Month:
            c.day = 1;
            c.hour = 0;
            c.minute = 0;
            c.second = 0;
            c.ms = 0;
            fromComponents(c);
            break;
        case Unit::Week: {
            // Go back to the locale's first day of week
            int wd = weekday(); // locale-relative weekday
            c.day -= wd;
            c.hour = 0;
            c.minute = 0;
            c.second = 0;
            c.ms = 0;
            fromComponents(c);
            break;
        }
        case Unit::IsoWeek: {
            // Go back to Monday
            int iwd = isoWeekday(); // 1=Mon, 7=Sun
            c.day -= (iwd - 1);
            c.hour = 0;
            c.minute = 0;
            c.second = 0;
            c.ms = 0;
            fromComponents(c);
            break;
        }
        case Unit::Day:
        case Unit::Date:
            c.hour = 0;
            c.minute = 0;
            c.second = 0;
            c.ms = 0;
            fromComponents(c);
            break;
        case Unit::Hour:
            c.minute = 0;
            c.second = 0;
            c.ms = 0;
            fromComponents(c);
            break;
        case Unit::Minute:
            c.second = 0;
            c.ms = 0;
            fromComponents(c);
            break;
        case Unit::Second:
            c.ms = 0;
            fromComponents(c);
            break;
        default:
            break;
    }
    return *this;
}

// ── Manipulation: endOf ──────────────────────────────────────────────

inline Moment& Moment::endOf(const std::string& unit) {
    Unit u = normalizeUnit(unit);
    if (u == Unit::Invalid || u == Unit::Millisecond) {
        return *this;
    }

    auto c = toComponents();

    switch (u) {
        case Unit::Year:
            c.month = 11;
            c.day = 31;
            c.hour = 23;
            c.minute = 59;
            c.second = 59;
            c.ms = 999;
            fromComponents(c);
            break;
        case Unit::Quarter: {
            int qEndMonth = c.month - (c.month % 3) + 2;
            c.month = qEndMonth;
            c.day = detail::daysInMonth(c.year, c.month);
            c.hour = 23;
            c.minute = 59;
            c.second = 59;
            c.ms = 999;
            fromComponents(c);
            break;
        }
        case Unit::Month:
            c.day = detail::daysInMonth(c.year, c.month);
            c.hour = 23;
            c.minute = 59;
            c.second = 59;
            c.ms = 999;
            fromComponents(c);
            break;
        case Unit::Week: {
            // End of locale week = start of week + 6 days, 23:59:59.999
            int wd = weekday();
            c.day += (6 - wd);
            c.hour = 23;
            c.minute = 59;
            c.second = 59;
            c.ms = 999;
            fromComponents(c);
            break;
        }
        case Unit::IsoWeek: {
            // End of ISO week = Sunday 23:59:59.999
            int iwd = isoWeekday(); // 1=Mon, 7=Sun
            c.day += (7 - iwd);
            c.hour = 23;
            c.minute = 59;
            c.second = 59;
            c.ms = 999;
            fromComponents(c);
            break;
        }
        case Unit::Day:
        case Unit::Date:
            c.hour = 23;
            c.minute = 59;
            c.second = 59;
            c.ms = 999;
            fromComponents(c);
            break;
        case Unit::Hour:
            c.minute = 59;
            c.second = 59;
            c.ms = 999;
            fromComponents(c);
            break;
        case Unit::Minute:
            c.second = 59;
            c.ms = 999;
            fromComponents(c);
            break;
        case Unit::Second:
            c.ms = 999;
            fromComponents(c);
            break;
        default:
            break;
    }
    return *this;
}

// ── UTC / Local mode ─────────────────────────────────────────────────

inline Moment& Moment::utc(bool keepLocalTime) {
    if (keepLocalTime) {
        // Keep wall-clock time the same, adjust timestamp
        int oldOffset = utcOffset();
        is_utc_ = true;
        has_fixed_offset_ = false;
        utc_offset_minutes_ = 0;
        // Subtract old offset to keep the same wall-clock time
        timestamp_ms_ -= static_cast<int64_t>(oldOffset) * 60000LL;
    } else {
        is_utc_ = true;
        has_fixed_offset_ = false;
        utc_offset_minutes_ = 0;
    }
    return *this;
}

inline Moment& Moment::local(bool keepLocalTime) {
    if (keepLocalTime) {
        // Keep wall-clock time the same when switching from UTC/offset to local
        int oldOffset = utcOffset();
        is_utc_ = false;
        has_fixed_offset_ = false;
        utc_offset_minutes_ = 0;
        // Compute the new local offset for the current timestamp
        int newOffset = detail::localUtcOffsetMinutes(timestamp_ms_);
        // Adjust timestamp to keep wall-clock time
        timestamp_ms_ += static_cast<int64_t>(newOffset - oldOffset) * 60000LL;
    } else {
        is_utc_ = false;
        has_fixed_offset_ = false;
        utc_offset_minutes_ = 0;
    }
    return *this;
}

// ── Display / Query ──────────────────────────────────────────────────

inline int64_t Moment::valueOf() const { return timestamp_ms_; }

inline bool Moment::isValid() const { return is_valid_; }

inline bool Moment::isLeapYear() const {
    return detail::isLeapYear(year());
}

inline bool Moment::isUtc() const {
    return is_utc_ && utc_offset_minutes_ == 0;
}

inline bool Moment::isLocal() const {
    return !is_utc_ && !has_fixed_offset_;
}

inline bool Moment::isUtcOffset() const {
    return is_utc_ || has_fixed_offset_;
}

inline Moment Moment::clone() const { return *this; }

// ── Display: diff ────────────────────────────────────────────────────

namespace detail {

/// @brief Month-based diff matching moment.js monthDiff algorithm.
inline double monthDiff(Moment a, Moment b) {
    // If a.date() < b.date(), negate the result of swapped args
    if (a.date() < b.date()) {
        return -monthDiff(b, a);
    }

    int wholeMonthDiff = (b.year() - a.year()) * 12 + (b.month() - a.month());
    Moment anchor = a.clone().add(wholeMonthDiff, "months");

    double adjust;
    if (b.valueOf() - anchor.valueOf() < 0) {
        Moment anchor2 = a.clone().add(wholeMonthDiff - 1, "months");
        double denom = static_cast<double>(anchor.valueOf() - anchor2.valueOf());
        adjust = (denom != 0.0) ? static_cast<double>(b.valueOf() - anchor.valueOf()) / denom : 0.0;
    } else {
        Moment anchor2 = a.clone().add(wholeMonthDiff + 1, "months");
        double denom = static_cast<double>(anchor2.valueOf() - anchor.valueOf());
        adjust = (denom != 0.0) ? static_cast<double>(b.valueOf() - anchor.valueOf()) / denom : 0.0;
    }

    double result = -(wholeMonthDiff + adjust);
    return (result == 0.0) ? 0.0 : result; // avoid negative zero
}

} // namespace detail

inline double Moment::diff(const Moment& other, const std::string& unit, bool precise) const {
    if (!isValid() || !other.isValid()) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    Unit u = normalizeUnit(unit);
    double output;

    // Zone delta for day/week calculations (negate DST effects)
    int64_t zoneDelta = static_cast<int64_t>(other.utcOffset() - utcOffset()) * 60000LL;

    switch (u) {
        case Unit::Year:
            output = detail::monthDiff(*this, other) / 12.0;
            break;
        case Unit::Month:
            output = detail::monthDiff(*this, other);
            break;
        case Unit::Quarter:
            output = detail::monthDiff(*this, other) / 3.0;
            break;
        case Unit::Second:
            output = static_cast<double>(timestamp_ms_ - other.timestamp_ms_) / 1000.0;
            break;
        case Unit::Minute:
            output = static_cast<double>(timestamp_ms_ - other.timestamp_ms_) / 60000.0;
            break;
        case Unit::Hour:
            output = static_cast<double>(timestamp_ms_ - other.timestamp_ms_) / 3600000.0;
            break;
        case Unit::Day:
        case Unit::Date:
            output = static_cast<double>(timestamp_ms_ - other.timestamp_ms_ - zoneDelta) / 86400000.0;
            break;
        case Unit::Week:
        case Unit::IsoWeek:
            output = static_cast<double>(timestamp_ms_ - other.timestamp_ms_ - zoneDelta) / 604800000.0;
            break;
        default:
            // Millisecond and anything else
            output = static_cast<double>(timestamp_ms_ - other.timestamp_ms_);
            break;
    }

    if (!precise) {
        // Truncate toward zero (not floor)
        output = polycpp::Math::trunc(output);
    }

    return output;
}

// ── Display: relative time helpers ───────────────────────────────────

namespace detail {

/// @brief Replace the first occurrence of a substring in a string.
inline std::string replaceFirst(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = result.find(from);
    if (pos != std::string::npos) {
        result.replace(pos, from.size(), to);
    }
    return result;
}

/// @brief Get a relative time string from a RelativeTimeValue for the given parameters.
inline std::string resolveRelativeTime(const RelativeTimeValue& val, int number,
                                        bool withoutSuffix, const std::string& key,
                                        bool isFuture) {
    if (std::holds_alternative<std::string>(val)) {
        std::string tmpl = std::get<std::string>(val);
        return replaceFirst(tmpl, "%d", std::to_string(number));
    } else {
        return std::get<RelativeTimeFn>(val)(number, withoutSuffix, key, isFuture);
    }
}

/// @brief Lookup a RelativeTimeValue by key from RelativeTimeFormats.
inline const RelativeTimeValue& lookupRelativeTimeKey(const RelativeTimeFormats& rt, const std::string& key) {
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
    // Fallback
    return rt.s;
}

/// @brief Compute relative time string from a millisecond duration.
/// @param ms_duration Positive or negative duration in milliseconds.
/// @param withoutSuffix If true, omit future/past wrapper.
/// @param locale_key Locale to use for formatting.
/// @return The formatted relative time string.
inline std::string computeRelativeTime(int64_t ms_duration, bool withoutSuffix,
                                        const std::string& locale_key) {
    const auto& loc = localeData(locale_key);
    const auto& rt = loc.relativeTime;

    bool isFuture = ms_duration > 0;
    int64_t abs_ms = std::abs(ms_duration);

    // Compute rounded values in each unit
    int seconds = static_cast<int>(polycpp::Math::round(static_cast<double>(abs_ms) / 1000.0));
    int minutes = static_cast<int>(polycpp::Math::round(static_cast<double>(abs_ms) / 60000.0));
    int hours   = static_cast<int>(polycpp::Math::round(static_cast<double>(abs_ms) / 3600000.0));
    int days    = static_cast<int>(polycpp::Math::round(static_cast<double>(abs_ms) / 86400000.0));
    int months  = static_cast<int>(polycpp::Math::round(static_cast<double>(days) / 30.4375));
    int years   = static_cast<int>(polycpp::Math::round(static_cast<double>(months) / 12.0));

    // Look up thresholds
    double th_ss = relativeTimeThreshold("ss");
    double th_s  = relativeTimeThreshold("s");
    double th_m  = relativeTimeThreshold("m");
    double th_h  = relativeTimeThreshold("h");
    double th_d  = relativeTimeThreshold("d");
    double th_M  = relativeTimeThreshold("M");

    std::string key;
    int value = 0;

    if (seconds <= static_cast<int>(th_ss)) {
        key = "s"; value = seconds;
    } else if (seconds < static_cast<int>(th_s)) {
        key = "ss"; value = seconds;
    } else if (minutes <= 1) {
        key = "m"; value = 1;
    } else if (minutes < static_cast<int>(th_m)) {
        key = "mm"; value = minutes;
    } else if (hours <= 1) {
        key = "h"; value = 1;
    } else if (hours < static_cast<int>(th_h)) {
        key = "hh"; value = hours;
    } else if (days <= 1) {
        key = "d"; value = 1;
    } else if (days < static_cast<int>(th_d)) {
        key = "dd"; value = days;
    } else if (months <= 1) {
        key = "M"; value = 1;
    } else if (months < static_cast<int>(th_M)) {
        key = "MM"; value = months;
    } else if (years <= 1) {
        key = "y"; value = 1;
    } else {
        key = "yy"; value = years;
    }

    const auto& rtv = lookupRelativeTimeKey(rt, key);
    std::string output = resolveRelativeTime(rtv, value, withoutSuffix, key, isFuture);

    if (!withoutSuffix) {
        // Wrap with future/past
        if (isFuture) {
            output = replaceFirst(rt.future, "%s", output);
        } else {
            output = replaceFirst(rt.past, "%s", output);
        }
    }

    return output;
}

} // namespace detail

// ── Display: from / fromNow / to / toNow ────────────────────────────

inline std::string Moment::from(const Moment& other, bool withoutSuffix) const {
    if (!isValid() || !other.isValid()) {
        return localeData(locale_key_).invalidDate;
    }
    // ms_duration: positive if this is in the future relative to other
    int64_t ms_duration = timestamp_ms_ - other.timestamp_ms_;
    return detail::computeRelativeTime(ms_duration, withoutSuffix, locale_key_);
}

inline std::string Moment::fromNow(bool withoutSuffix) const {
    return from(Moment(), withoutSuffix);
}

inline std::string Moment::to(const Moment& other, bool withoutSuffix) const {
    if (!isValid() || !other.isValid()) {
        return localeData(locale_key_).invalidDate;
    }
    // Reversed direction from "from"
    int64_t ms_duration = other.timestamp_ms_ - timestamp_ms_;
    return detail::computeRelativeTime(ms_duration, withoutSuffix, locale_key_);
}

inline std::string Moment::toNow(bool withoutSuffix) const {
    return to(Moment(), withoutSuffix);
}

// ── Display: calendar ────────────────────────────────────────────────

inline std::string Moment::calendar() const {
    return calendar(Moment());
}

inline std::string Moment::calendar(const Moment& reference) const {
    if (!isValid()) {
        return localeData(locale_key_).invalidDate;
    }

    // Compute diff in days from start-of-day of reference
    Moment sod = reference.clone();
    // Clone to local/utc matching this moment's mode for consistent comparison
    if (is_utc_) {
        sod.utc();
    }
    sod.startOf("day");
    double dayDiff = diff(sod, "day", true);

    std::string calKey;
    if (dayDiff < -6) {
        calKey = "sameElse";
    } else if (dayDiff < -1) {
        calKey = "lastWeek";
    } else if (dayDiff < 0) {
        calKey = "lastDay";
    } else if (dayDiff < 1) {
        calKey = "sameDay";
    } else if (dayDiff < 2) {
        calKey = "nextDay";
    } else if (dayDiff < 7) {
        calKey = "nextWeek";
    } else {
        calKey = "sameElse";
    }

    const auto& loc = localeData(locale_key_);
    const auto& cal = loc.calendar;

    // Get the CalendarValue for this key
    const CalendarValue* cv = nullptr;
    if (calKey == "sameDay")  cv = &cal.sameDay;
    else if (calKey == "nextDay")  cv = &cal.nextDay;
    else if (calKey == "nextWeek") cv = &cal.nextWeek;
    else if (calKey == "lastDay")  cv = &cal.lastDay;
    else if (calKey == "lastWeek") cv = &cal.lastWeek;
    else                           cv = &cal.sameElse;

    std::string fmtStr;
    if (std::holds_alternative<std::string>(*cv)) {
        fmtStr = std::get<std::string>(*cv);
    } else {
        fmtStr = std::get<CalendarFn>(*cv)();
    }

    return format(fmtStr);
}

// ── Display: toJSON, toString, toArray ───────────────────────────────

inline std::string Moment::toJSON() const {
    if (!isValid()) {
        return "null";
    }
    return toISOString();
}

inline std::string Moment::toString() const {
    if (!isValid()) {
        return "Invalid date";
    }

    // Fixed English names for toString (not locale-dependent)
    static const char* dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char* monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    auto c = toComponents();
    int dow = detail::dayOfWeekFromDate(c.year, c.month, c.day);

    // Format UTC offset as +HHMM or -HHMM
    int off = utcOffset();
    char signChar = off >= 0 ? '+' : '-';
    int absOff = std::abs(off);
    int offHours = absOff / 60;
    int offMins = absOff % 60;

    // "Fri Mar 15 2024 14:30:45 GMT+0000"
    char buf[128];
    std::snprintf(buf, sizeof(buf), "%s %s %02d %04d %02d:%02d:%02d GMT%c%02d%02d",
                  dayNames[dow], monthNames[c.month],
                  c.day, c.year,
                  c.hour, c.minute, c.second,
                  signChar, offHours, offMins);
    return std::string(buf);
}

inline polycpp::JsonArray Moment::toArray() const {
    auto c = toComponents();
    return polycpp::JsonArray{c.year, c.month, c.day, c.hour, c.minute, c.second, c.ms};
}

inline polycpp::JsonObject Moment::toObject() const {
    auto c = toComponents();
    return polycpp::JsonObject{
        {"years", c.year},
        {"months", c.month},
        {"date", c.day},
        {"hours", c.hour},
        {"minutes", c.minute},
        {"seconds", c.second},
        {"milliseconds", c.ms}
    };
}

// ── Query: isBefore / isAfter / isSame / isBetween ──────────────────

inline bool Moment::isBefore(const Moment& other) const {
    if (!isValid() || !other.isValid()) return false;
    return timestamp_ms_ < other.timestamp_ms_;
}

inline bool Moment::isBefore(const Moment& other, const std::string& unit) const {
    if (!isValid() || !other.isValid()) return false;
    Unit u = normalizeUnit(unit);
    if (u == Unit::Millisecond || u == Unit::Invalid) {
        return timestamp_ms_ < other.timestamp_ms_;
    }
    // this.endOf(unit) < other
    return clone().endOf(unit).valueOf() < other.valueOf();
}

inline bool Moment::isAfter(const Moment& other) const {
    if (!isValid() || !other.isValid()) return false;
    return timestamp_ms_ > other.timestamp_ms_;
}

inline bool Moment::isAfter(const Moment& other, const std::string& unit) const {
    if (!isValid() || !other.isValid()) return false;
    Unit u = normalizeUnit(unit);
    if (u == Unit::Millisecond || u == Unit::Invalid) {
        return timestamp_ms_ > other.timestamp_ms_;
    }
    // other < this.startOf(unit)
    return other.valueOf() < clone().startOf(unit).valueOf();
}

inline bool Moment::isSame(const Moment& other) const {
    if (!isValid() || !other.isValid()) return false;
    return timestamp_ms_ == other.timestamp_ms_;
}

inline bool Moment::isSame(const Moment& other, const std::string& unit) const {
    if (!isValid() || !other.isValid()) return false;
    Unit u = normalizeUnit(unit);
    if (u == Unit::Millisecond || u == Unit::Invalid) {
        return timestamp_ms_ == other.timestamp_ms_;
    }
    int64_t inputMs = other.valueOf();
    return clone().startOf(unit).valueOf() <= inputMs &&
           inputMs <= clone().endOf(unit).valueOf();
}

inline bool Moment::isSameOrBefore(const Moment& other) const {
    return isSame(other) || isBefore(other);
}

inline bool Moment::isSameOrBefore(const Moment& other, const std::string& unit) const {
    return isSame(other, unit) || isBefore(other, unit);
}

inline bool Moment::isSameOrAfter(const Moment& other) const {
    return isSame(other) || isAfter(other);
}

inline bool Moment::isSameOrAfter(const Moment& other, const std::string& unit) const {
    return isSame(other, unit) || isAfter(other, unit);
}

inline bool Moment::isBetween(const Moment& from, const Moment& to) const {
    return isBetween(from, to, "", "()");
}

inline bool Moment::isBetween(const Moment& from, const Moment& to, const std::string& unit) const {
    return isBetween(from, to, unit, "()");
}

inline bool Moment::isBetween(const Moment& from, const Moment& to,
                                const std::string& unit, const std::string& inclusivity) const {
    if (!isValid() || !from.isValid() || !to.isValid()) return false;

    std::string incl = inclusivity.empty() ? "()" : inclusivity;

    bool startOk, endOk;
    if (incl[0] == '(') {
        startOk = unit.empty() ? isAfter(from) : isAfter(from, unit);
    } else {
        // '['
        startOk = unit.empty() ? !isBefore(from) : !isBefore(from, unit);
    }
    if (incl.size() > 1 && incl[1] == ')') {
        endOk = unit.empty() ? isBefore(to) : isBefore(to, unit);
    } else {
        // ']'
        endOk = unit.empty() ? !isAfter(to) : !isAfter(to, unit);
    }

    return startOk && endOk;
}

// ── Query: isDST ─────────────────────────────────────────────────────

inline bool Moment::isDST() const {
    // Compare UTC offset now vs January 1 of the same year
    // If the current offset is greater than Jan 1 offset, DST is active
    Moment jan1 = clone();
    jan1.month(0).date(1).startOf("day");
    return utcOffset() > jan1.utcOffset();
}

// ── Operators ────────────────────────────────────────────────────────

inline bool Moment::operator==(const Moment& other) const { return timestamp_ms_ == other.timestamp_ms_; }
inline bool Moment::operator!=(const Moment& other) const { return timestamp_ms_ != other.timestamp_ms_; }
inline bool Moment::operator<(const Moment& other) const { return timestamp_ms_ < other.timestamp_ms_; }
inline bool Moment::operator<=(const Moment& other) const { return timestamp_ms_ <= other.timestamp_ms_; }
inline bool Moment::operator>(const Moment& other) const { return timestamp_ms_ > other.timestamp_ms_; }
inline bool Moment::operator>=(const Moment& other) const { return timestamp_ms_ >= other.timestamp_ms_; }

// ── Factory functions ────────────────────────────────────────────────

inline Moment now() {
    return Moment();
}

inline int64_t nowMs() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
}

// ── JsonObject construction ─────────────────────────────────────────

namespace detail {

/// @brief Extract an int from a JsonObject, trying two key names with a default.
inline int jsonGetInt(const polycpp::JsonObject& obj,
                      const std::string& key1, const std::string& key2, int def) {
    auto it = obj.find(key1);
    if (it != obj.end() && it->second.isNumber()) return it->second.asInt();
    it = obj.find(key2);
    if (it != obj.end() && it->second.isNumber()) return it->second.asInt();
    return def;
}

} // namespace detail

inline Moment fromObject(const polycpp::JsonObject& obj) {
    int year  = detail::jsonGetInt(obj, "year",        "years",        2000);
    int month = detail::jsonGetInt(obj, "month",       "months",       0);
    int day   = detail::jsonGetInt(obj, "date",        "day",          1);
    int hour  = detail::jsonGetInt(obj, "hour",        "hours",        0);
    int min   = detail::jsonGetInt(obj, "minute",      "minutes",      0);
    int sec   = detail::jsonGetInt(obj, "second",      "seconds",      0);
    int ms    = detail::jsonGetInt(obj, "millisecond", "milliseconds", 0);

    return fromDate(year, month, day, hour, min, sec, ms);
}

inline Moment utcFromObject(const polycpp::JsonObject& obj) {
    int year  = detail::jsonGetInt(obj, "year",        "years",        2000);
    int month = detail::jsonGetInt(obj, "month",       "months",       0);
    int day   = detail::jsonGetInt(obj, "date",        "day",          1);
    int hour  = detail::jsonGetInt(obj, "hour",        "hours",        0);
    int min   = detail::jsonGetInt(obj, "minute",      "minutes",      0);
    int sec   = detail::jsonGetInt(obj, "second",      "seconds",      0);
    int ms    = detail::jsonGetInt(obj, "millisecond", "milliseconds", 0);

    return utcFromDate(year, month, day, hour, min, sec, ms);
}

} // namespace moment
} // namespace polycpp
