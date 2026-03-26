/**
 * @file detail/moment.hpp
 * @brief Moment class inline implementations.
 *
 * Contains all get/set, manipulation, and UTC/local mode logic.
 * Uses <ctime> functions for timestamp <-> component decomposition.
 *
 * @since 0.1.0
 */
#pragma once

#include <polycpp/moment/moment.hpp>
#include <polycpp/moment/units.hpp>
#include <polycpp/moment/locale.hpp>
#include <chrono>
#include <ctime>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <string>

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

/// @brief Portable timegm: convert struct tm (UTC) to time_t.
inline time_t portableTimegm(struct tm* t) {
#if defined(_WIN32)
    return _mkgmtime(t);
#else
    return timegm(t);
#endif
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
    int week = static_cast<int>(std::floor(static_cast<double>(momDayOfYear - weekOffset - 1) / 7.0)) + 1;
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
/// Returns offset in minutes (e.g., EST = -300).
inline int localUtcOffsetMinutes(int64_t timestamp_ms) {
    time_t t = static_cast<time_t>(timestamp_ms / 1000);
    struct tm local_tm;
    struct tm utc_tm;
    memset(&local_tm, 0, sizeof(local_tm));
    memset(&utc_tm, 0, sizeof(utc_tm));
    localtime_r(&t, &local_tm);
    gmtime_r(&t, &utc_tm);

    // Compute difference in seconds
    int64_t local_sec = static_cast<int64_t>(local_tm.tm_hour) * 3600
                      + static_cast<int64_t>(local_tm.tm_min) * 60
                      + local_tm.tm_sec;
    int64_t utc_sec = static_cast<int64_t>(utc_tm.tm_hour) * 3600
                    + static_cast<int64_t>(utc_tm.tm_min) * 60
                    + utc_tm.tm_sec;

    // Handle day boundary crossing
    int day_diff = local_tm.tm_yday - utc_tm.tm_yday;
    if (day_diff > 1) day_diff = -1;   // year boundary: Dec 31 vs Jan 1
    else if (day_diff < -1) day_diff = 1;

    int64_t diff_sec = local_sec - utc_sec + day_diff * 86400LL;
    return static_cast<int>(diff_sec / 60);
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
        hours = std::stoi(clean);
    } else if (clean.size() == 3) {
        hours = std::stoi(clean.substr(0, 1));
        minutes = std::stoi(clean.substr(1, 2));
    } else if (clean.size() >= 4) {
        hours = std::stoi(clean.substr(0, 2));
        minutes = std::stoi(clean.substr(2, 2));
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
    time_t t = static_cast<time_t>(timestamp_ms_ / 1000);
    int ms_part = static_cast<int>(((timestamp_ms_ % 1000) + 1000) % 1000);

    struct tm result;
    std::memset(&result, 0, sizeof(result));

    if (is_utc_ || has_fixed_offset_) {
        gmtime_r(&t, &result);
        if (has_fixed_offset_ && !is_utc_) {
            // Apply the fixed offset to get the "local" view
            // But actually, has_fixed_offset_ means we interpret as UTC + offset
            // The timestamp is always UTC. We adjust the components.
            int offset_sec = utc_offset_minutes_ * 60;
            time_t adjusted = t + offset_sec;
            gmtime_r(&adjusted, &result);
            // Adjust ms_part if needed — ms is not affected by offset
        }
    } else {
        localtime_r(&t, &result);
    }

    DateComponents c;
    c.year = result.tm_year + 1900;
    c.month = result.tm_mon;   // 0-11, same as moment.js
    c.day = result.tm_mday;    // 1-31
    c.hour = result.tm_hour;   // 0-23
    c.minute = result.tm_min;  // 0-59
    c.second = result.tm_sec;  // 0-59
    c.ms = ms_part;
    return c;
}

inline void Moment::fromComponents(const DateComponents& c) {
    struct tm t;
    std::memset(&t, 0, sizeof(t));
    t.tm_year = c.year - 1900;
    t.tm_mon = c.month;
    t.tm_mday = c.day;
    t.tm_hour = c.hour;
    t.tm_min = c.minute;
    t.tm_sec = c.second;
    t.tm_isdst = -1; // let mktime figure it out

    time_t epoch;
    if (is_utc_ || has_fixed_offset_) {
        epoch = detail::portableTimegm(&t);
        if (has_fixed_offset_ && !is_utc_) {
            // Components were in offset time, so subtract offset to get UTC
            epoch -= utc_offset_minutes_ * 60;
        }
    } else {
        epoch = mktime(&t);
    }

    timestamp_ms_ = static_cast<int64_t>(epoch) * 1000 + c.ms;
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
    c.day = value; // struct tm / timegm handles overflow
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

} // namespace moment
} // namespace polycpp
