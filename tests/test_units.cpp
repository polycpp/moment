/**
 * @file test_units.cpp
 * @brief Tests for unit normalization (normalizeUnit / unitToString).
 */
#include <gtest/gtest.h>
#include <polycpp/moment/detail/aggregator.hpp>

using namespace polycpp::moment;

// ── Year ─────────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeYear) {
    EXPECT_EQ(normalizeUnit("year"),  Unit::Year);
    EXPECT_EQ(normalizeUnit("years"), Unit::Year);
    EXPECT_EQ(normalizeUnit("y"),     Unit::Year);
}

TEST(UnitsTest, NormalizeYearCaseInsensitive) {
    EXPECT_EQ(normalizeUnit("Year"),  Unit::Year);
    EXPECT_EQ(normalizeUnit("YEARS"), Unit::Year);
}

// ── Quarter ──────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeQuarter) {
    EXPECT_EQ(normalizeUnit("quarter"),  Unit::Quarter);
    EXPECT_EQ(normalizeUnit("quarters"), Unit::Quarter);
    EXPECT_EQ(normalizeUnit("Q"),        Unit::Quarter);
}

// ── Month ────────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeMonth) {
    EXPECT_EQ(normalizeUnit("month"),  Unit::Month);
    EXPECT_EQ(normalizeUnit("months"), Unit::Month);
    EXPECT_EQ(normalizeUnit("M"),      Unit::Month);
}

// ── Week ─────────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeWeek) {
    EXPECT_EQ(normalizeUnit("week"),  Unit::Week);
    EXPECT_EQ(normalizeUnit("weeks"), Unit::Week);
    EXPECT_EQ(normalizeUnit("w"),     Unit::Week);
}

// ── IsoWeek ──────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeIsoWeek) {
    EXPECT_EQ(normalizeUnit("isoWeek"),  Unit::IsoWeek);
    EXPECT_EQ(normalizeUnit("isoWeeks"), Unit::IsoWeek);
    EXPECT_EQ(normalizeUnit("isoweek"),  Unit::IsoWeek);
    EXPECT_EQ(normalizeUnit("isoweeks"), Unit::IsoWeek);
    EXPECT_EQ(normalizeUnit("W"),        Unit::IsoWeek);
}

// ── Day ──────────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeDay) {
    EXPECT_EQ(normalizeUnit("day"),  Unit::Day);
    EXPECT_EQ(normalizeUnit("days"), Unit::Day);
    EXPECT_EQ(normalizeUnit("d"),    Unit::Day);
}

// ── Date ─────────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeDate) {
    EXPECT_EQ(normalizeUnit("date"),  Unit::Date);
    EXPECT_EQ(normalizeUnit("dates"), Unit::Date);
    EXPECT_EQ(normalizeUnit("D"),     Unit::Date);
}

// ── Hour ─────────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeHour) {
    EXPECT_EQ(normalizeUnit("hour"),  Unit::Hour);
    EXPECT_EQ(normalizeUnit("hours"), Unit::Hour);
    EXPECT_EQ(normalizeUnit("h"),     Unit::Hour);
}

// ── Minute ───────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeMinute) {
    EXPECT_EQ(normalizeUnit("minute"),  Unit::Minute);
    EXPECT_EQ(normalizeUnit("minutes"), Unit::Minute);
    EXPECT_EQ(normalizeUnit("m"),       Unit::Minute);
}

// ── Second ───────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeSecond) {
    EXPECT_EQ(normalizeUnit("second"),  Unit::Second);
    EXPECT_EQ(normalizeUnit("seconds"), Unit::Second);
    EXPECT_EQ(normalizeUnit("s"),       Unit::Second);
}

// ── Millisecond ──────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeMillisecond) {
    EXPECT_EQ(normalizeUnit("millisecond"),  Unit::Millisecond);
    EXPECT_EQ(normalizeUnit("milliseconds"), Unit::Millisecond);
    EXPECT_EQ(normalizeUnit("ms"),           Unit::Millisecond);
}

// ── WeekYear ─────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeWeekYear) {
    EXPECT_EQ(normalizeUnit("weekYear"),  Unit::WeekYear);
    EXPECT_EQ(normalizeUnit("weekYears"), Unit::WeekYear);
    EXPECT_EQ(normalizeUnit("weekyear"),  Unit::WeekYear);
    EXPECT_EQ(normalizeUnit("weekyears"), Unit::WeekYear);
    EXPECT_EQ(normalizeUnit("gg"),        Unit::WeekYear);
}

// ── IsoWeekYear ──────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeIsoWeekYear) {
    EXPECT_EQ(normalizeUnit("isoWeekYear"),  Unit::IsoWeekYear);
    EXPECT_EQ(normalizeUnit("isoWeekYears"), Unit::IsoWeekYear);
    EXPECT_EQ(normalizeUnit("isoweekyear"),  Unit::IsoWeekYear);
    EXPECT_EQ(normalizeUnit("isoweekyears"), Unit::IsoWeekYear);
    EXPECT_EQ(normalizeUnit("GG"),           Unit::IsoWeekYear);
}

// ── DayOfYear ────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeDayOfYear) {
    EXPECT_EQ(normalizeUnit("dayOfYear"),  Unit::DayOfYear);
    EXPECT_EQ(normalizeUnit("dayOfYears"), Unit::DayOfYear);
    EXPECT_EQ(normalizeUnit("dayofyear"),  Unit::DayOfYear);
    EXPECT_EQ(normalizeUnit("dayofyears"), Unit::DayOfYear);
    EXPECT_EQ(normalizeUnit("DDD"),        Unit::DayOfYear);
}

// ── Weekday ──────────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeWeekday) {
    EXPECT_EQ(normalizeUnit("weekday"),  Unit::Weekday);
    EXPECT_EQ(normalizeUnit("weekdays"), Unit::Weekday);
    EXPECT_EQ(normalizeUnit("e"),        Unit::Weekday);
}

// ── IsoWeekday ───────────────────────────────────────────────────────

TEST(UnitsTest, NormalizeIsoWeekday) {
    EXPECT_EQ(normalizeUnit("isoWeekday"),  Unit::IsoWeekday);
    EXPECT_EQ(normalizeUnit("isoWeekdays"), Unit::IsoWeekday);
    EXPECT_EQ(normalizeUnit("isoweekday"),  Unit::IsoWeekday);
    EXPECT_EQ(normalizeUnit("isoweekdays"), Unit::IsoWeekday);
    EXPECT_EQ(normalizeUnit("E"),           Unit::IsoWeekday);
}

// ── Invalid ──────────────────────────────────────────────────────────

TEST(UnitsTest, InvalidUnitReturnsInvalid) {
    EXPECT_EQ(normalizeUnit(""),         Unit::Invalid);
    EXPECT_EQ(normalizeUnit("foo"),      Unit::Invalid);
    EXPECT_EQ(normalizeUnit("century"),  Unit::Invalid);
    EXPECT_EQ(normalizeUnit("YY"),       Unit::Invalid);
}

// ── Case sensitivity for shorthand ───────────────────────────────────

TEST(UnitsTest, ShorthandCaseSensitivity) {
    // "M" is Month, "m" is Minute — they must not be confused
    EXPECT_EQ(normalizeUnit("M"), Unit::Month);
    EXPECT_EQ(normalizeUnit("m"), Unit::Minute);

    // "D" is Date, "d" is Day
    EXPECT_EQ(normalizeUnit("D"), Unit::Date);
    EXPECT_EQ(normalizeUnit("d"), Unit::Day);

    // "W" is IsoWeek, "w" is Week
    EXPECT_EQ(normalizeUnit("W"), Unit::IsoWeek);
    EXPECT_EQ(normalizeUnit("w"), Unit::Week);

    // "E" is IsoWeekday, "e" is Weekday
    EXPECT_EQ(normalizeUnit("E"), Unit::IsoWeekday);
    EXPECT_EQ(normalizeUnit("e"), Unit::Weekday);

    // "Q" is Quarter, "s" is Second
    EXPECT_EQ(normalizeUnit("Q"), Unit::Quarter);
    EXPECT_EQ(normalizeUnit("s"), Unit::Second);
}

// ── unitToString round-trip ──────────────────────────────────────────

TEST(UnitsTest, UnitToString) {
    EXPECT_EQ(unitToString(Unit::Year),        "year");
    EXPECT_EQ(unitToString(Unit::Quarter),     "quarter");
    EXPECT_EQ(unitToString(Unit::Month),       "month");
    EXPECT_EQ(unitToString(Unit::Week),        "week");
    EXPECT_EQ(unitToString(Unit::IsoWeek),     "isoWeek");
    EXPECT_EQ(unitToString(Unit::Day),         "day");
    EXPECT_EQ(unitToString(Unit::Date),        "date");
    EXPECT_EQ(unitToString(Unit::Hour),        "hour");
    EXPECT_EQ(unitToString(Unit::Minute),      "minute");
    EXPECT_EQ(unitToString(Unit::Second),      "second");
    EXPECT_EQ(unitToString(Unit::Millisecond), "millisecond");
    EXPECT_EQ(unitToString(Unit::WeekYear),    "weekYear");
    EXPECT_EQ(unitToString(Unit::IsoWeekYear), "isoWeekYear");
    EXPECT_EQ(unitToString(Unit::DayOfYear),   "dayOfYear");
    EXPECT_EQ(unitToString(Unit::Weekday),     "weekday");
    EXPECT_EQ(unitToString(Unit::IsoWeekday),  "isoWeekday");
    EXPECT_EQ(unitToString(Unit::Invalid),     "invalid");
}

TEST(UnitsTest, RoundTrip) {
    // normalizeUnit(unitToString(X)) == X for all valid units
    for (auto u : {Unit::Year, Unit::Quarter, Unit::Month, Unit::Week,
                   Unit::IsoWeek, Unit::Day, Unit::Date, Unit::Hour,
                   Unit::Minute, Unit::Second, Unit::Millisecond,
                   Unit::WeekYear, Unit::IsoWeekYear, Unit::DayOfYear,
                   Unit::Weekday, Unit::IsoWeekday}) {
        EXPECT_EQ(normalizeUnit(unitToString(u)), u)
            << "Round-trip failed for " << unitToString(u);
    }
}
