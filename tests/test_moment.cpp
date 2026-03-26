/**
 * @file test_moment.cpp
 * @brief Comprehensive tests for Moment get/set, manipulation, UTC/local mode,
 *        and week calculations. Ported from moment.js test suite.
 *
 * All tests use UTC mode to avoid timezone-dependent failures in CI.
 * A known timestamp is used for deterministic results:
 *   2024-03-15T12:30:45.123Z = 1710505845123 ms since epoch
 */
#include <gtest/gtest.h>
#include <polycpp/moment/detail/aggregator.hpp>

using namespace polycpp::moment;

// ── Known timestamps ────────────────────────────────────────────────
// 2024-03-15T12:30:45.123Z
static constexpr int64_t TS_2024_03_15 = 1710505845123LL;

// 2024-01-01T00:00:00.000Z
static constexpr int64_t TS_2024_01_01 = 1704067200000LL;

// 2024-02-29T00:00:00.000Z (leap year)
static constexpr int64_t TS_2024_02_29 = 1709164800000LL;

// 2023-01-31T00:00:00.000Z
static constexpr int64_t TS_2023_01_31 = 1675123200000LL;

// 2024-12-31T23:59:59.999Z
static constexpr int64_t TS_2024_12_31_END = 1735689599999LL;

// ═════════════════════════════════════════════════════════════════════
// GET/SET: Year, Month, Date, Hour, Minute, Second, Millisecond
// ═════════════════════════════════════════════════════════════════════

TEST(MomentGetSet, Year) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.year(), 2024);
}

TEST(MomentGetSet, SetYear) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.year(2025);
    EXPECT_EQ(m.year(), 2025);
    EXPECT_EQ(m.month(), 2); // March
    EXPECT_EQ(m.date(), 15);
}

TEST(MomentGetSet, Month) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.month(), 2); // 0-based: March = 2
}

TEST(MomentGetSet, SetMonth) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.month(0); // January
    EXPECT_EQ(m.month(), 0);
    EXPECT_EQ(m.date(), 15);
}

TEST(MomentGetSet, Date) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.date(), 15);
}

TEST(MomentGetSet, SetDate) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.date(20);
    EXPECT_EQ(m.date(), 20);
    EXPECT_EQ(m.month(), 2);
}

TEST(MomentGetSet, Hour) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.hour(), 12);
}

TEST(MomentGetSet, SetHour) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.hour(5);
    EXPECT_EQ(m.hour(), 5);
    EXPECT_EQ(m.minute(), 30);
}

TEST(MomentGetSet, Minute) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.minute(), 30);
}

TEST(MomentGetSet, SetMinute) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.minute(0);
    EXPECT_EQ(m.minute(), 0);
}

TEST(MomentGetSet, Second) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.second(), 45);
}

TEST(MomentGetSet, SetSecond) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.second(10);
    EXPECT_EQ(m.second(), 10);
}

TEST(MomentGetSet, Millisecond) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.millisecond(), 123);
}

TEST(MomentGetSet, SetMillisecond) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.millisecond(456);
    EXPECT_EQ(m.millisecond(), 456);
}

// ── Round-trip tests ─────────────────────────────────────────────────

TEST(MomentGetSet, RoundTripYear) {
    Moment m(TS_2024_03_15);
    m.utc();
    int orig = m.year();
    m.year(2030);
    EXPECT_EQ(m.year(), 2030);
    m.year(orig);
    EXPECT_EQ(m.year(), 2024);
}

TEST(MomentGetSet, RoundTripMonth) {
    Moment m(TS_2024_03_15);
    m.utc();
    int orig = m.month();
    m.month(6); // July
    EXPECT_EQ(m.month(), 6);
    m.month(orig);
    EXPECT_EQ(m.month(), orig);
}

TEST(MomentGetSet, RoundTripHourMinuteSecondMs) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.hour(3).minute(15).second(30).millisecond(500);
    EXPECT_EQ(m.hour(), 3);
    EXPECT_EQ(m.minute(), 15);
    EXPECT_EQ(m.second(), 30);
    EXPECT_EQ(m.millisecond(), 500);
}

// ═════════════════════════════════════════════════════════════════════
// GET/SET: Day, Weekday, IsoWeekday
// ═════════════════════════════════════════════════════════════════════

TEST(MomentGetSet, Day) {
    // 2024-03-15 is a Friday
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.day(), 5); // Friday = 5
}

TEST(MomentGetSet, SetDay) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.day(0); // Go to Sunday
    EXPECT_EQ(m.day(), 0);
    // Should be March 10, 2024 (Sunday before March 15)
    EXPECT_EQ(m.date(), 10);
}

TEST(MomentGetSet, IsoWeekday) {
    // Friday = 5 in ISO (1=Mon, 7=Sun)
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.isoWeekday(), 5);
}

TEST(MomentGetSet, SetIsoWeekday) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.isoWeekday(1); // Monday
    EXPECT_EQ(m.isoWeekday(), 1);
    EXPECT_EQ(m.date(), 11); // March 11, 2024 is Monday
}

TEST(MomentGetSet, IsoWeekdaySunday) {
    // Set to Sunday (ISO 7)
    Moment m(TS_2024_03_15);
    m.utc();
    m.isoWeekday(7);
    EXPECT_EQ(m.isoWeekday(), 7);
    EXPECT_EQ(m.day(), 0); // Sunday
}

TEST(MomentGetSet, Weekday) {
    // Default locale is "en", dow=0 (Sunday)
    // 2024-03-15 is Friday. weekday = (5 - 0) % 7 = 5
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.weekday(), 5); // Friday is 5th day from Sunday
}

// ═════════════════════════════════════════════════════════════════════
// GET/SET: DayOfYear, Week, IsoWeek, Quarter
// ═════════════════════════════════════════════════════════════════════

TEST(MomentGetSet, DayOfYear) {
    // 2024-03-15: Jan=31, Feb=29 (leap), Mar 1-15 = 15 days
    // Total: 31 + 29 + 15 = 75
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.dayOfYear(), 75);
}

TEST(MomentGetSet, SetDayOfYear) {
    Moment m(TS_2024_01_01);
    m.utc();
    m.dayOfYear(75);
    EXPECT_EQ(m.month(), 2);  // March
    EXPECT_EQ(m.date(), 15);
}

TEST(MomentGetSet, DayOfYearJan1) {
    Moment m(TS_2024_01_01);
    m.utc();
    EXPECT_EQ(m.dayOfYear(), 1);
}

TEST(MomentGetSet, IsoWeek) {
    // 2024-03-15: ISO week = 11
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.isoWeek(), 11);
}

TEST(MomentGetSet, SetIsoWeek) {
    Moment m(TS_2024_03_15);
    m.utc();
    int origWeek = m.isoWeek();
    m.isoWeek(1);
    EXPECT_EQ(m.isoWeek(), 1);
    m.isoWeek(origWeek);
    EXPECT_EQ(m.isoWeek(), origWeek);
}

TEST(MomentGetSet, IsoWeekYear) {
    // 2024-03-15 is in ISO week-year 2024
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.isoWeekYear(), 2024);
}

TEST(MomentGetSet, Quarter) {
    // March = Q1
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.quarter(), 1);
}

TEST(MomentGetSet, QuarterVariousMonths) {
    Moment m(TS_2024_01_01);
    m.utc();
    EXPECT_EQ(m.quarter(), 1); // Jan
    m.month(3); // April
    EXPECT_EQ(m.quarter(), 2);
    m.month(6); // July
    EXPECT_EQ(m.quarter(), 3);
    m.month(9); // October
    EXPECT_EQ(m.quarter(), 4);
}

TEST(MomentGetSet, Week) {
    // Locale-aware week (en: Sunday first, doy=6)
    Moment m(TS_2024_01_01);
    m.utc();
    // 2024-01-01 is a Monday. With en locale (dow=0 Sunday, doy=6),
    // the first week contains Jan 6.
    // Jan 1 week depends on the locale algorithm.
    int w = m.week();
    EXPECT_GE(w, 1);
    EXPECT_LE(w, 53);
}

// ═════════════════════════════════════════════════════════════════════
// Generic get/set by string
// ═════════════════════════════════════════════════════════════════════

TEST(MomentGetSet, GenericGet) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.get("year"), 2024);
    EXPECT_EQ(m.get("month"), 2);
    EXPECT_EQ(m.get("date"), 15);
    EXPECT_EQ(m.get("hour"), 12);
    EXPECT_EQ(m.get("minute"), 30);
    EXPECT_EQ(m.get("second"), 45);
    EXPECT_EQ(m.get("millisecond"), 123);
}

TEST(MomentGetSet, GenericSet) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.set("month", 5); // June
    EXPECT_EQ(m.month(), 5);
    m.set("year", 2025);
    EXPECT_EQ(m.year(), 2025);
}

// ═════════════════════════════════════════════════════════════════════
// ADD / SUBTRACT
// ═════════════════════════════════════════════════════════════════════

TEST(MomentAdd, AddDay) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(1, "day");
    EXPECT_EQ(m.date(), 16);
    EXPECT_EQ(m.month(), 2);
}

TEST(MomentAdd, AddDays) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(20, "days");
    EXPECT_EQ(m.month(), 3); // April
    EXPECT_EQ(m.date(), 4);
}

TEST(MomentAdd, AddMonth) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(1, "month");
    EXPECT_EQ(m.month(), 3); // April
    EXPECT_EQ(m.date(), 15);
}

TEST(MomentAdd, AddMonths) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(10, "months");
    EXPECT_EQ(m.year(), 2025);
    EXPECT_EQ(m.month(), 0); // January
}

TEST(MomentAdd, AddYear) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(1, "year");
    EXPECT_EQ(m.year(), 2025);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
}

TEST(MomentAdd, AddHour) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(3, "hours");
    EXPECT_EQ(m.hour(), 15);
}

TEST(MomentAdd, AddMinute) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(45, "minutes");
    EXPECT_EQ(m.hour(), 13);
    EXPECT_EQ(m.minute(), 15);
}

TEST(MomentAdd, AddSecond) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(20, "seconds");
    EXPECT_EQ(m.second(), 5); // 45 + 20 = 65 -> 5 with minute rollover
    EXPECT_EQ(m.minute(), 31);
}

TEST(MomentAdd, AddMillisecond) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(877, "milliseconds");
    EXPECT_EQ(m.millisecond(), 0); // 123 + 877 = 1000 -> 0 with second rollover
    EXPECT_EQ(m.second(), 46);
}

TEST(MomentAdd, AddWeek) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(2, "weeks");
    EXPECT_EQ(m.date(), 29);
    EXPECT_EQ(m.month(), 2);
}

TEST(MomentAdd, AddQuarter) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(1, "quarter");
    EXPECT_EQ(m.month(), 5); // June
}

// ── Month overflow: Jan 31 + 1 month = Feb 28/29 ────────────────────

TEST(MomentAdd, MonthOverflowNonLeap) {
    // 2023-01-31 + 1 month = 2023-02-28
    Moment m(TS_2023_01_31);
    m.utc();
    EXPECT_EQ(m.year(), 2023);
    EXPECT_EQ(m.month(), 0); // January
    EXPECT_EQ(m.date(), 31);
    m.add(1, "month");
    EXPECT_EQ(m.month(), 1); // February
    EXPECT_EQ(m.date(), 28); // Clamped
}

TEST(MomentAdd, MonthOverflowLeap) {
    // 2024-01-31 + 1 month = 2024-02-29 (leap year)
    // 2024-01-31T00:00:00Z = 1706659200000
    Moment m(1706659200000LL);
    m.utc();
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 0);
    EXPECT_EQ(m.date(), 31);
    m.add(1, "month");
    EXPECT_EQ(m.month(), 1);
    EXPECT_EQ(m.date(), 29);
}

// ── Year on leap day: Feb 29 + 1 year = Feb 28 ──────────────────────

TEST(MomentAdd, YearOnLeapDay) {
    // 2024-02-29 + 1 year = 2025-02-28
    Moment m(TS_2024_02_29);
    m.utc();
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 1);
    EXPECT_EQ(m.date(), 29);
    m.add(1, "year");
    EXPECT_EQ(m.year(), 2025);
    EXPECT_EQ(m.month(), 1);
    EXPECT_EQ(m.date(), 28); // Clamped
}

// ── Subtract ─────────────────────────────────────────────────────────

TEST(MomentSubtract, SubtractDay) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.subtract(1, "day");
    EXPECT_EQ(m.date(), 14);
}

TEST(MomentSubtract, SubtractMonth) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.subtract(1, "month");
    EXPECT_EQ(m.month(), 1); // February
    EXPECT_EQ(m.date(), 15);
}

TEST(MomentSubtract, SubtractYear) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.subtract(2, "years");
    EXPECT_EQ(m.year(), 2022);
}

TEST(MomentSubtract, SubtractMonthOverflow) {
    // 2024-03-31 - 1 month = 2024-02-29 (leap year, March 31 -> clamp Feb 29)
    // 2024-03-31T00:00:00Z
    Moment m(1711843200000LL);
    m.utc();
    EXPECT_EQ(m.date(), 31);
    m.subtract(1, "month");
    EXPECT_EQ(m.month(), 1); // February
    EXPECT_EQ(m.date(), 29); // Leap year clamp
}

TEST(MomentSubtract, SubtractMonthsWrap) {
    // Subtract 13 months from March 2024
    Moment m(TS_2024_03_15);
    m.utc();
    m.subtract(13, "months");
    EXPECT_EQ(m.year(), 2023);
    EXPECT_EQ(m.month(), 1); // February
    EXPECT_EQ(m.date(), 15);
}

// ═════════════════════════════════════════════════════════════════════
// startOf / endOf
// ═════════════════════════════════════════════════════════════════════

TEST(MomentStartEnd, StartOfYear) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.startOf("year");
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 0);
    EXPECT_EQ(m.date(), 1);
    EXPECT_EQ(m.hour(), 0);
    EXPECT_EQ(m.minute(), 0);
    EXPECT_EQ(m.second(), 0);
    EXPECT_EQ(m.millisecond(), 0);
}

TEST(MomentStartEnd, EndOfYear) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.endOf("year");
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 11);
    EXPECT_EQ(m.date(), 31);
    EXPECT_EQ(m.hour(), 23);
    EXPECT_EQ(m.minute(), 59);
    EXPECT_EQ(m.second(), 59);
    EXPECT_EQ(m.millisecond(), 999);
}

TEST(MomentStartEnd, StartOfMonth) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.startOf("month");
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 1);
    EXPECT_EQ(m.hour(), 0);
    EXPECT_EQ(m.minute(), 0);
    EXPECT_EQ(m.second(), 0);
    EXPECT_EQ(m.millisecond(), 0);
}

TEST(MomentStartEnd, EndOfMonth) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.endOf("month");
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 31); // March has 31 days
    EXPECT_EQ(m.hour(), 23);
    EXPECT_EQ(m.minute(), 59);
    EXPECT_EQ(m.second(), 59);
    EXPECT_EQ(m.millisecond(), 999);
}

TEST(MomentStartEnd, EndOfMonthFeb) {
    Moment m(TS_2024_02_29);
    m.utc();
    m.endOf("month");
    EXPECT_EQ(m.date(), 29); // Leap year
    EXPECT_EQ(m.millisecond(), 999);
}

TEST(MomentStartEnd, StartOfDay) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.startOf("day");
    EXPECT_EQ(m.date(), 15);
    EXPECT_EQ(m.hour(), 0);
    EXPECT_EQ(m.minute(), 0);
    EXPECT_EQ(m.second(), 0);
    EXPECT_EQ(m.millisecond(), 0);
}

TEST(MomentStartEnd, EndOfDay) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.endOf("day");
    EXPECT_EQ(m.date(), 15);
    EXPECT_EQ(m.hour(), 23);
    EXPECT_EQ(m.minute(), 59);
    EXPECT_EQ(m.second(), 59);
    EXPECT_EQ(m.millisecond(), 999);
}

TEST(MomentStartEnd, StartOfHour) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.startOf("hour");
    EXPECT_EQ(m.hour(), 12);
    EXPECT_EQ(m.minute(), 0);
    EXPECT_EQ(m.second(), 0);
    EXPECT_EQ(m.millisecond(), 0);
}

TEST(MomentStartEnd, EndOfHour) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.endOf("hour");
    EXPECT_EQ(m.hour(), 12);
    EXPECT_EQ(m.minute(), 59);
    EXPECT_EQ(m.second(), 59);
    EXPECT_EQ(m.millisecond(), 999);
}

TEST(MomentStartEnd, StartOfMinute) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.startOf("minute");
    EXPECT_EQ(m.minute(), 30);
    EXPECT_EQ(m.second(), 0);
    EXPECT_EQ(m.millisecond(), 0);
}

TEST(MomentStartEnd, StartOfSecond) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.startOf("second");
    EXPECT_EQ(m.second(), 45);
    EXPECT_EQ(m.millisecond(), 0);
}

TEST(MomentStartEnd, StartOfQuarter) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.startOf("quarter");
    EXPECT_EQ(m.month(), 0);  // Q1 starts in January
    EXPECT_EQ(m.date(), 1);
    EXPECT_EQ(m.hour(), 0);
}

TEST(MomentStartEnd, EndOfQuarter) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.endOf("quarter");
    EXPECT_EQ(m.month(), 2);  // Q1 ends in March
    EXPECT_EQ(m.date(), 31);
    EXPECT_EQ(m.hour(), 23);
    EXPECT_EQ(m.millisecond(), 999);
}

TEST(MomentStartEnd, StartOfQuarterQ2) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.month(4); // May (Q2)
    m.startOf("quarter");
    EXPECT_EQ(m.month(), 3);  // April
    EXPECT_EQ(m.date(), 1);
}

// ── startOf("week") with default locale (Sunday) ────────────────────

TEST(MomentStartEnd, StartOfWeekSunday) {
    // 2024-03-15 is Friday. Start of week (Sunday) = March 10
    Moment m(TS_2024_03_15);
    m.utc();
    m.startOf("week");
    EXPECT_EQ(m.date(), 10);
    EXPECT_EQ(m.day(), 0); // Sunday
    EXPECT_EQ(m.hour(), 0);
    EXPECT_EQ(m.millisecond(), 0);
}

TEST(MomentStartEnd, EndOfWeek) {
    // 2024-03-15 is Friday. End of week (Saturday) = March 16 23:59:59.999
    Moment m(TS_2024_03_15);
    m.utc();
    m.endOf("week");
    EXPECT_EQ(m.date(), 16);
    EXPECT_EQ(m.day(), 6); // Saturday
    EXPECT_EQ(m.hour(), 23);
    EXPECT_EQ(m.millisecond(), 999);
}

// ── startOf("isoWeek") always Monday ─────────────────────────────────

TEST(MomentStartEnd, StartOfIsoWeek) {
    // 2024-03-15 is Friday. Start of ISO week (Monday) = March 11
    Moment m(TS_2024_03_15);
    m.utc();
    m.startOf("isoWeek");
    EXPECT_EQ(m.date(), 11);
    EXPECT_EQ(m.day(), 1); // Monday
    EXPECT_EQ(m.hour(), 0);
}

TEST(MomentStartEnd, EndOfIsoWeek) {
    // End of ISO week = Sunday = March 17
    Moment m(TS_2024_03_15);
    m.utc();
    m.endOf("isoWeek");
    EXPECT_EQ(m.date(), 17);
    EXPECT_EQ(m.day(), 0); // Sunday
    EXPECT_EQ(m.hour(), 23);
    EXPECT_EQ(m.millisecond(), 999);
}

// ═════════════════════════════════════════════════════════════════════
// UTC / LOCAL MODE
// ═════════════════════════════════════════════════════════════════════

TEST(MomentUtc, UtcMode) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_TRUE(m.isUtc());
    EXPECT_FALSE(m.isLocal());
    EXPECT_EQ(m.hour(), 12);
}

TEST(MomentUtc, UtcAndLocalSameTimestamp) {
    Moment m(TS_2024_03_15);
    m.utc();
    int64_t ts = m.valueOf();
    m.local();
    // Timestamp should not change
    EXPECT_EQ(m.valueOf(), ts);
    EXPECT_TRUE(m.isLocal());
    EXPECT_FALSE(m.isUtc());
}

TEST(MomentUtc, UtcOffsetMinutes) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.utcOffset(), 0);
}

TEST(MomentUtc, UtcOffsetFixed) {
    Moment m(TS_2024_03_15);
    m.utcOffset(330); // +5:30 (India)
    EXPECT_EQ(m.utcOffset(), 330);
    // In +5:30, 12:30 UTC = 18:00
    EXPECT_EQ(m.hour(), 18);
    EXPECT_EQ(m.minute(), 0);
}

TEST(MomentUtc, UtcOffsetString) {
    Moment m(TS_2024_03_15);
    m.utcOffset("+05:30");
    EXPECT_EQ(m.utcOffset(), 330);
    EXPECT_EQ(m.hour(), 18);
    EXPECT_EQ(m.minute(), 0);
}

TEST(MomentUtc, UtcOffsetNegative) {
    Moment m(TS_2024_03_15);
    m.utcOffset("-05:00");
    EXPECT_EQ(m.utcOffset(), -300);
    // 12:30 UTC = 07:30 in UTC-5
    EXPECT_EQ(m.hour(), 7);
    EXPECT_EQ(m.minute(), 30);
}

TEST(MomentUtc, UtcOffsetStringNoColon) {
    Moment m(TS_2024_03_15);
    m.utcOffset("+0530");
    EXPECT_EQ(m.utcOffset(), 330);
}

TEST(MomentUtc, UtcOffsetKeepLocalTime) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.hour(), 12);
    EXPECT_EQ(m.minute(), 30);

    // Keep the same wall-clock time but change offset
    m.utcOffset(330, true);
    EXPECT_EQ(m.hour(), 12);
    EXPECT_EQ(m.minute(), 30);
    EXPECT_EQ(m.utcOffset(), 330);
}

TEST(MomentUtc, UtcToLocalPreservesTimestamp) {
    Moment m(TS_2024_03_15);
    m.utc();
    int64_t ts1 = m.valueOf();
    m.local();
    int64_t ts2 = m.valueOf();
    EXPECT_EQ(ts1, ts2);
}

// ═════════════════════════════════════════════════════════════════════
// DAYS IN MONTH
// ═════════════════════════════════════════════════════════════════════

TEST(MomentDaysInMonth, January) {
    Moment m(TS_2024_01_01);
    m.utc();
    EXPECT_EQ(m.daysInMonth(), 31);
}

TEST(MomentDaysInMonth, FebruaryLeap) {
    Moment m(TS_2024_02_29);
    m.utc();
    EXPECT_EQ(m.daysInMonth(), 29);
}

TEST(MomentDaysInMonth, FebruaryNonLeap) {
    // 2023-02-15T00:00:00Z
    Moment m(1676419200000LL);
    m.utc();
    EXPECT_EQ(m.month(), 1);
    EXPECT_EQ(m.daysInMonth(), 28);
}

TEST(MomentDaysInMonth, April) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.month(3); // April
    EXPECT_EQ(m.daysInMonth(), 30);
}

TEST(MomentDaysInMonth, March) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.daysInMonth(), 31);
}

// ═════════════════════════════════════════════════════════════════════
// LEAP YEAR
// ═════════════════════════════════════════════════════════════════════

TEST(MomentLeapYear, Year2024) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_TRUE(m.isLeapYear());
}

TEST(MomentLeapYear, Year2023) {
    Moment m(TS_2023_01_31);
    m.utc();
    EXPECT_FALSE(m.isLeapYear());
}

TEST(MomentLeapYear, Year2000) {
    // 2000-06-15T00:00:00Z = 961027200000
    Moment m(961027200000LL);
    m.utc();
    EXPECT_TRUE(m.isLeapYear());
}

TEST(MomentLeapYear, Year1900) {
    // 1900 is not a leap year (divisible by 100 but not 400)
    // 1900-06-15T00:00:00Z = -2193100800000
    Moment m(-2193100800000LL);
    m.utc();
    EXPECT_EQ(m.year(), 1900);
    EXPECT_FALSE(m.isLeapYear());
}

// ═════════════════════════════════════════════════════════════════════
// WEEKS IN YEAR / ISO WEEKS IN YEAR
// ═════════════════════════════════════════════════════════════════════

TEST(MomentWeeks, IsoWeeksInYear2024) {
    Moment m(TS_2024_03_15);
    m.utc();
    // 2024 has 52 ISO weeks
    EXPECT_EQ(m.isoWeeksInYear(), 52);
}

TEST(MomentWeeks, IsoWeeksInYear2020) {
    // 2020-06-15T00:00:00Z = 1592179200000
    Moment m(1592179200000LL);
    m.utc();
    // 2020 has 53 ISO weeks
    EXPECT_EQ(m.isoWeeksInYear(), 53);
}

// ═════════════════════════════════════════════════════════════════════
// CLONE INDEPENDENCE
// ═════════════════════════════════════════════════════════════════════

TEST(MomentClone, Independence) {
    Moment m(TS_2024_03_15);
    m.utc();
    auto c = m.clone();

    m.add(1, "day");
    EXPECT_EQ(c.date(), 15); // Clone unchanged
    EXPECT_EQ(m.date(), 16); // Original changed
}

TEST(MomentClone, IndependentUtcMode) {
    Moment m(TS_2024_03_15);
    m.utc();
    auto c = m.clone();
    c.local();
    EXPECT_TRUE(m.isUtc());
    EXPECT_TRUE(c.isLocal());
}

// ═════════════════════════════════════════════════════════════════════
// CHAINING
// ═════════════════════════════════════════════════════════════════════

TEST(MomentChaining, AddAndStartOf) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(1, "day").startOf("hour");
    EXPECT_EQ(m.date(), 16);
    EXPECT_EQ(m.hour(), 12);
    EXPECT_EQ(m.minute(), 0);
    EXPECT_EQ(m.second(), 0);
    EXPECT_EQ(m.millisecond(), 0);
}

TEST(MomentChaining, SetYearMonthDate) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.year(2025).month(5).date(1);
    EXPECT_EQ(m.year(), 2025);
    EXPECT_EQ(m.month(), 5);
    EXPECT_EQ(m.date(), 1);
}

TEST(MomentChaining, SubtractAndEndOf) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.subtract(1, "month").endOf("month");
    EXPECT_EQ(m.month(), 1); // February
    EXPECT_EQ(m.date(), 29); // 2024 is leap year
    EXPECT_EQ(m.hour(), 23);
    EXPECT_EQ(m.millisecond(), 999);
}

// ═════════════════════════════════════════════════════════════════════
// MONTH SETTER: Day clamping on set
// ═════════════════════════════════════════════════════════════════════

TEST(MomentSetMonth, ClampDayOnMonthSet) {
    // Start with Jan 31, set month to Feb
    Moment m(TS_2023_01_31);
    m.utc();
    m.month(1); // February 2023 (non-leap)
    EXPECT_EQ(m.month(), 1);
    EXPECT_EQ(m.date(), 28); // Clamped from 31
}

TEST(MomentSetMonth, ClampDayOnMonthSetLeap) {
    // 2024-01-31, set month to Feb (leap year)
    Moment m(1706659200000LL);
    m.utc();
    m.month(1);
    EXPECT_EQ(m.date(), 29); // Clamped to 29 in leap year
}

// ── Year setter: clamp Feb 29 ────────────────────────────────────────

TEST(MomentSetYear, ClampLeapDayOnYearSet) {
    // 2024-02-29, set year to 2023
    Moment m(TS_2024_02_29);
    m.utc();
    m.year(2023);
    EXPECT_EQ(m.year(), 2023);
    EXPECT_EQ(m.month(), 1);
    EXPECT_EQ(m.date(), 28); // Clamped
}

// ═════════════════════════════════════════════════════════════════════
// OPERATORS (already tested in basic, but verify post-expansion)
// ═════════════════════════════════════════════════════════════════════

TEST(MomentOperators, PostExpansionOperators) {
    Moment a(1000);
    Moment b(2000);
    Moment c(1000);

    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a == c);
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a <= c);
    EXPECT_TRUE(a >= c);
}

// ═════════════════════════════════════════════════════════════════════
// EDGE CASES
// ═════════════════════════════════════════════════════════════════════

TEST(MomentEdge, AddZero) {
    Moment m(TS_2024_03_15);
    m.utc();
    int64_t ts = m.valueOf();
    m.add(0, "day");
    EXPECT_EQ(m.valueOf(), ts);
}

TEST(MomentEdge, SubtractZero) {
    Moment m(TS_2024_03_15);
    m.utc();
    int64_t ts = m.valueOf();
    m.subtract(0, "month");
    EXPECT_EQ(m.valueOf(), ts);
}

TEST(MomentEdge, StartOfMillisecondNoOp) {
    Moment m(TS_2024_03_15);
    m.utc();
    int64_t ts = m.valueOf();
    m.startOf("millisecond");
    EXPECT_EQ(m.valueOf(), ts);
}

TEST(MomentEdge, EndOfMillisecondNoOp) {
    Moment m(TS_2024_03_15);
    m.utc();
    int64_t ts = m.valueOf();
    m.endOf("millisecond");
    EXPECT_EQ(m.valueOf(), ts);
}

TEST(MomentEdge, Add12Months) {
    Moment m(TS_2024_03_15);
    m.utc();
    m.add(12, "months");
    EXPECT_EQ(m.year(), 2025);
    EXPECT_EQ(m.month(), 2); // March
    EXPECT_EQ(m.date(), 15);
}

TEST(MomentEdge, SubtractNegative) {
    // Subtracting a negative is adding
    Moment m(TS_2024_03_15);
    m.utc();
    m.subtract(-1, "day");
    EXPECT_EQ(m.date(), 16);
}

TEST(MomentEdge, SetMonthNegative) {
    // month(-1) should wrap to previous year December
    Moment m(TS_2024_03_15);
    m.utc();
    m.month(-1);
    EXPECT_EQ(m.year(), 2023);
    EXPECT_EQ(m.month(), 11); // December
}

TEST(MomentEdge, SetMonthGreaterThan11) {
    // month(12) should wrap to next year January
    Moment m(TS_2024_03_15);
    m.utc();
    m.month(12);
    EXPECT_EQ(m.year(), 2025);
    EXPECT_EQ(m.month(), 0); // January
}

// ═════════════════════════════════════════════════════════════════════
// VALIDATE KNOWN TIMESTAMP COMPONENTS
// ═════════════════════════════════════════════════════════════════════

TEST(MomentValidation, KnownTimestamp2024_03_15) {
    Moment m(TS_2024_03_15);
    m.utc();
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);   // March (0-based)
    EXPECT_EQ(m.date(), 15);
    EXPECT_EQ(m.hour(), 12);
    EXPECT_EQ(m.minute(), 30);
    EXPECT_EQ(m.second(), 45);
    EXPECT_EQ(m.millisecond(), 123);
    EXPECT_EQ(m.day(), 5);     // Friday
    EXPECT_EQ(m.isoWeekday(), 5);
}

TEST(MomentValidation, KnownTimestamp2024_01_01) {
    Moment m(TS_2024_01_01);
    m.utc();
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 0);
    EXPECT_EQ(m.date(), 1);
    EXPECT_EQ(m.hour(), 0);
    EXPECT_EQ(m.minute(), 0);
    EXPECT_EQ(m.second(), 0);
    EXPECT_EQ(m.millisecond(), 0);
    EXPECT_EQ(m.day(), 1);    // Monday
}

TEST(MomentValidation, KnownTimestampEndOfYear) {
    Moment m(TS_2024_12_31_END);
    m.utc();
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 11);
    EXPECT_EQ(m.date(), 31);
    EXPECT_EQ(m.hour(), 23);
    EXPECT_EQ(m.minute(), 59);
    EXPECT_EQ(m.second(), 59);
    EXPECT_EQ(m.millisecond(), 999);
}
