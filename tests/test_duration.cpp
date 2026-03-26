/**
 * @file test_duration.cpp
 * @brief Tests for the Duration class, ported from moment.js duration.js.
 */

#include <polycpp/moment/detail/aggregator.hpp>
#include <gtest/gtest.h>
#include <cmath>

using namespace polycpp::moment;

// =========================================================================
// Creation tests
// =========================================================================

TEST(Duration, ObjectInstantiation) {
    auto d = duration(DurationInput{
        .years = 2, .months = 3, .weeks = 2, .days = 1,
        .hours = 8, .minutes = 9, .seconds = 20, .milliseconds = 12
    });
    EXPECT_EQ(d.years(), 2);
    EXPECT_EQ(d.months(), 3);
    EXPECT_EQ(d.weeks(), 2);      // floor(15/7) = 2
    EXPECT_EQ(d.days(), 15);      // 2 weeks + 1 day
    EXPECT_EQ(d.hours(), 8);
    EXPECT_EQ(d.minutes(), 9);
    EXPECT_EQ(d.seconds(), 20);
    EXPECT_EQ(d.milliseconds(), 12);
}

TEST(Duration, MillisecondsInstantiation) {
    EXPECT_EQ(duration(static_cast<int64_t>(72)).milliseconds(), 72);
    EXPECT_EQ(duration(static_cast<int64_t>(72)).humanize(), "a few seconds");
}

TEST(Duration, ZeroDuration) {
    auto d = duration(static_cast<int64_t>(0));
    EXPECT_EQ(d.milliseconds(), 0);
    EXPECT_TRUE(d.isValid());
    EXPECT_EQ(d.humanize(), "a few seconds");
}

TEST(Duration, InstantiationByType) {
    EXPECT_EQ(duration(1, "years").years(), 1);
    EXPECT_EQ(duration(1, "y").years(), 1);
    EXPECT_EQ(duration(2, "months").months(), 2);
    EXPECT_EQ(duration(2, "M").months(), 2);
    EXPECT_EQ(duration(3, "weeks").weeks(), 3);
    EXPECT_EQ(duration(3, "w").weeks(), 3);
    EXPECT_EQ(duration(4, "days").days(), 4);
    EXPECT_EQ(duration(4, "d").days(), 4);
    EXPECT_EQ(duration(5, "hours").hours(), 5);
    EXPECT_EQ(duration(5, "h").hours(), 5);
    EXPECT_EQ(duration(6, "minutes").minutes(), 6);
    EXPECT_EQ(duration(6, "m").minutes(), 6);
    EXPECT_EQ(duration(7, "seconds").seconds(), 7);
    EXPECT_EQ(duration(7, "s").seconds(), 7);
    EXPECT_EQ(duration(8, "milliseconds").milliseconds(), 8);
    EXPECT_EQ(duration(8, "ms").milliseconds(), 8);
}

TEST(Duration, GenericGetter) {
    EXPECT_EQ(duration(1, "years").get("years"), 1);
    EXPECT_EQ(duration(1, "years").get("year"), 1);
    EXPECT_EQ(duration(1, "years").get("y"), 1);
    EXPECT_EQ(duration(2, "months").get("months"), 2);
    EXPECT_EQ(duration(2, "months").get("month"), 2);
    EXPECT_EQ(duration(2, "months").get("M"), 2);
    EXPECT_EQ(duration(3, "weeks").get("weeks"), 3);
    EXPECT_EQ(duration(3, "weeks").get("week"), 3);
    EXPECT_EQ(duration(3, "weeks").get("w"), 3);
    EXPECT_EQ(duration(4, "days").get("days"), 4);
    EXPECT_EQ(duration(4, "days").get("day"), 4);
    EXPECT_EQ(duration(4, "days").get("d"), 4);
    EXPECT_EQ(duration(5, "hours").get("hours"), 5);
    EXPECT_EQ(duration(5, "hours").get("hour"), 5);
    EXPECT_EQ(duration(5, "hours").get("h"), 5);
    EXPECT_EQ(duration(6, "minutes").get("minutes"), 6);
    EXPECT_EQ(duration(6, "minutes").get("minute"), 6);
    EXPECT_EQ(duration(6, "minutes").get("m"), 6);
    EXPECT_EQ(duration(7, "seconds").get("seconds"), 7);
    EXPECT_EQ(duration(7, "seconds").get("second"), 7);
    EXPECT_EQ(duration(7, "seconds").get("s"), 7);
    EXPECT_EQ(duration(8, "milliseconds").get("milliseconds"), 8);
    EXPECT_EQ(duration(8, "milliseconds").get("millisecond"), 8);
    EXPECT_EQ(duration(8, "milliseconds").get("ms"), 8);
}

// =========================================================================
// ISO 8601 string parsing
// =========================================================================

TEST(Duration, ISOStringParsing) {
    auto d = duration(std::string("P1Y2M3DT4H5M6S"));
    EXPECT_EQ(d.years(), 1);
    EXPECT_EQ(d.months(), 2);
    EXPECT_EQ(d.days(), 3);
    EXPECT_EQ(d.hours(), 4);
    EXPECT_EQ(d.minutes(), 5);
    EXPECT_EQ(d.seconds(), 6);
    EXPECT_EQ(d.milliseconds(), 0);
    EXPECT_TRUE(d.isValid());
}

TEST(Duration, ISOStringParsingWeeks) {
    auto d = duration(std::string("P3W"));
    EXPECT_EQ(d.days(), 21);
    EXPECT_EQ(d.weeks(), 3);
}

TEST(Duration, ISOStringParsingNegative) {
    auto d = duration(std::string("-P1Y2M"));
    EXPECT_EQ(d.years(), -1);
    EXPECT_EQ(d.months(), -2);
}

TEST(Duration, ISOStringParsingFractionalSeconds) {
    auto d = duration(std::string("PT1.5S"));
    EXPECT_EQ(d.seconds(), 1);
    EXPECT_EQ(d.milliseconds(), 500);
}

TEST(Duration, ISOStringInvalid) {
    auto d = duration(std::string("not a duration"));
    EXPECT_FALSE(d.isValid());
}

TEST(Duration, ISOStringMinimal) {
    auto d = duration(std::string("PT0S"));
    EXPECT_TRUE(d.isValid());
    EXPECT_EQ(d.seconds(), 0);
}

// =========================================================================
// Clone
// =========================================================================

TEST(Duration, ExplicitCloning) {
    auto a = duration(5, "milliseconds");
    auto b = a.clone();
    a.add(5, "milliseconds");
    EXPECT_NE(a.milliseconds(), b.milliseconds());
}

// =========================================================================
// Humanize
// =========================================================================

TEST(Duration, Humanize) {
    EXPECT_EQ(duration(DurationInput{.seconds = 44}).humanize(), "a few seconds");
    EXPECT_EQ(duration(DurationInput{.seconds = 45}).humanize(), "a minute");
    EXPECT_EQ(duration(DurationInput{.seconds = 89}).humanize(), "a minute");
    EXPECT_EQ(duration(DurationInput{.seconds = 90}).humanize(), "2 minutes");
    EXPECT_EQ(duration(DurationInput{.minutes = 44}).humanize(), "44 minutes");
    EXPECT_EQ(duration(DurationInput{.minutes = 45}).humanize(), "an hour");
    EXPECT_EQ(duration(DurationInput{.minutes = 89}).humanize(), "an hour");
    EXPECT_EQ(duration(DurationInput{.minutes = 90}).humanize(), "2 hours");
    EXPECT_EQ(duration(DurationInput{.hours = 5}).humanize(), "5 hours");
    EXPECT_EQ(duration(DurationInput{.hours = 21}).humanize(), "21 hours");
    EXPECT_EQ(duration(DurationInput{.hours = 22}).humanize(), "a day");
    EXPECT_EQ(duration(DurationInput{.hours = 35}).humanize(), "a day");
    EXPECT_EQ(duration(DurationInput{.hours = 36}).humanize(), "2 days");
    EXPECT_EQ(duration(DurationInput{.days = 1}).humanize(), "a day");
    EXPECT_EQ(duration(DurationInput{.days = 5}).humanize(), "5 days");
    EXPECT_EQ(duration(DurationInput{.weeks = 1}).humanize(), "7 days");
    EXPECT_EQ(duration(DurationInput{.days = 25}).humanize(), "25 days");
    EXPECT_EQ(duration(DurationInput{.days = 26}).humanize(), "a month");
    EXPECT_EQ(duration(DurationInput{.days = 30}).humanize(), "a month");
    EXPECT_EQ(duration(DurationInput{.days = 45}).humanize(), "a month");
    EXPECT_EQ(duration(DurationInput{.days = 46}).humanize(), "2 months");
    EXPECT_EQ(duration(DurationInput{.days = 74}).humanize(), "2 months");
    EXPECT_EQ(duration(DurationInput{.days = 77}).humanize(), "3 months");
    EXPECT_EQ(duration(DurationInput{.months = 1}).humanize(), "a month");
    EXPECT_EQ(duration(DurationInput{.months = 5}).humanize(), "5 months");
    EXPECT_EQ(duration(DurationInput{.days = 344}).humanize(), "a year");
    EXPECT_EQ(duration(DurationInput{.days = 345}).humanize(), "a year");
    EXPECT_EQ(duration(DurationInput{.days = 547}).humanize(), "a year");
    EXPECT_EQ(duration(DurationInput{.days = 548}).humanize(), "2 years");
    EXPECT_EQ(duration(DurationInput{.years = 1}).humanize(), "a year");
    EXPECT_EQ(duration(DurationInput{.years = 5}).humanize(), "5 years");
    EXPECT_EQ(duration(static_cast<int64_t>(7200000)).humanize(), "2 hours");
}

TEST(Duration, HumanizeWithSuffix) {
    EXPECT_EQ(duration(DurationInput{.seconds = 44}).humanize(true), "in a few seconds");
    EXPECT_EQ(duration(DurationInput{.seconds = -44}).humanize(true), "a few seconds ago");
    EXPECT_EQ(duration(DurationInput{.seconds = 44}).humanize(true), "in a few seconds");
}

// =========================================================================
// Total getters (as*)
// =========================================================================

TEST(Duration, AsGettersYear) {
    EXPECT_DOUBLE_EQ(duration(1, "year").asYears(), 1.0);
    EXPECT_DOUBLE_EQ(duration(1, "year").asMonths(), 12.0);
    EXPECT_DOUBLE_EQ(duration(400, "year").asMonths(), 4800.0);
    EXPECT_DOUBLE_EQ(duration(1, "year").asDays(), 365.0);
    EXPECT_DOUBLE_EQ(duration(2, "year").asDays(), 730.0);
    EXPECT_DOUBLE_EQ(duration(3, "year").asDays(), 1096.0);
    EXPECT_DOUBLE_EQ(duration(4, "year").asDays(), 1461.0);
    EXPECT_DOUBLE_EQ(duration(400, "year").asDays(), 146097.0);
    EXPECT_DOUBLE_EQ(duration(1, "year").asHours(), 8760.0);
    EXPECT_DOUBLE_EQ(duration(1, "year").asMinutes(), 525600.0);
    EXPECT_DOUBLE_EQ(duration(1, "year").asSeconds(), 31536000.0);
    EXPECT_DOUBLE_EQ(duration(1, "year").asMilliseconds(), 31536000000.0);
}

TEST(Duration, AsGettersMonth) {
    EXPECT_DOUBLE_EQ(duration(1, "month").asMonths(), 1.0);
    EXPECT_DOUBLE_EQ(duration(1, "month").asDays(), 30.0);
    EXPECT_DOUBLE_EQ(duration(2, "month").asDays(), 61.0);
    EXPECT_DOUBLE_EQ(duration(3, "month").asDays(), 91.0);
    EXPECT_DOUBLE_EQ(duration(4, "month").asDays(), 122.0);
    EXPECT_DOUBLE_EQ(duration(5, "month").asDays(), 152.0);
    EXPECT_DOUBLE_EQ(duration(6, "month").asDays(), 183.0);
    EXPECT_DOUBLE_EQ(duration(7, "month").asDays(), 213.0);
    EXPECT_DOUBLE_EQ(duration(8, "month").asDays(), 243.0);
    EXPECT_DOUBLE_EQ(duration(9, "month").asDays(), 274.0);
    EXPECT_DOUBLE_EQ(duration(10, "month").asDays(), 304.0);
    EXPECT_DOUBLE_EQ(duration(11, "month").asDays(), 335.0);
    EXPECT_DOUBLE_EQ(duration(12, "month").asDays(), 365.0);
    EXPECT_DOUBLE_EQ(duration(24, "month").asDays(), 730.0);
    EXPECT_DOUBLE_EQ(duration(36, "month").asDays(), 1096.0);
    EXPECT_DOUBLE_EQ(duration(48, "month").asDays(), 1461.0);
    EXPECT_DOUBLE_EQ(duration(4800, "month").asDays(), 146097.0);
    EXPECT_DOUBLE_EQ(duration(1, "month").asHours(), 720.0);
    EXPECT_DOUBLE_EQ(duration(1, "month").asMinutes(), 43200.0);
    EXPECT_DOUBLE_EQ(duration(1, "month").asSeconds(), 2592000.0);
    EXPECT_DOUBLE_EQ(duration(1, "month").asMilliseconds(), 2592000000.0);
}

TEST(Duration, AsGettersWeek) {
    EXPECT_DOUBLE_EQ(duration(1, "week").asWeeks(), 1.0);
    EXPECT_DOUBLE_EQ(duration(1, "week").asDays(), 7.0);
    EXPECT_DOUBLE_EQ(duration(1, "week").asHours(), 168.0);
    EXPECT_DOUBLE_EQ(duration(1, "week").asMinutes(), 10080.0);
    EXPECT_DOUBLE_EQ(duration(1, "week").asSeconds(), 604800.0);
    EXPECT_DOUBLE_EQ(duration(1, "week").asMilliseconds(), 604800000.0);
}

TEST(Duration, AsGettersDay) {
    EXPECT_DOUBLE_EQ(duration(1, "day").asDays(), 1.0);
    EXPECT_DOUBLE_EQ(duration(1, "day").asHours(), 24.0);
    EXPECT_DOUBLE_EQ(duration(1, "day").asMinutes(), 1440.0);
    EXPECT_DOUBLE_EQ(duration(1, "day").asSeconds(), 86400.0);
    EXPECT_DOUBLE_EQ(duration(1, "day").asMilliseconds(), 86400000.0);
}

TEST(Duration, AsGettersHour) {
    EXPECT_DOUBLE_EQ(duration(1, "hour").asHours(), 1.0);
    EXPECT_DOUBLE_EQ(duration(1, "hour").asMinutes(), 60.0);
    EXPECT_DOUBLE_EQ(duration(1, "hour").asSeconds(), 3600.0);
    EXPECT_DOUBLE_EQ(duration(1, "hour").asMilliseconds(), 3600000.0);
}

TEST(Duration, AsGettersMinute) {
    EXPECT_DOUBLE_EQ(duration(1, "minute").asMinutes(), 1.0);
    EXPECT_DOUBLE_EQ(duration(1, "minute").asSeconds(), 60.0);
    EXPECT_DOUBLE_EQ(duration(1, "minute").asMilliseconds(), 60000.0);
}

TEST(Duration, AsGettersSecond) {
    EXPECT_DOUBLE_EQ(duration(1, "second").asSeconds(), 1.0);
    EXPECT_DOUBLE_EQ(duration(1, "second").asMilliseconds(), 1000.0);
}

TEST(Duration, AsGettersSmallUnits) {
    EXPECT_DOUBLE_EQ(duration(1, "milliseconds").as("milliseconds"), 1.0);
    EXPECT_DOUBLE_EQ(duration(1, "milliseconds").asMilliseconds(), 1.0);
    EXPECT_DOUBLE_EQ(duration(3, "seconds").as("seconds"), 3.0);
    EXPECT_DOUBLE_EQ(duration(3, "seconds").asSeconds(), 3.0);
    EXPECT_DOUBLE_EQ(duration(13, "minutes").as("minutes"), 13.0);
    EXPECT_DOUBLE_EQ(duration(13, "minutes").asMinutes(), 13.0);
}

TEST(Duration, ValueOfAndAsMilliseconds) {
    auto t1 = duration(2, "months").valueOf();
    auto t2 = static_cast<int64_t>(duration(2, "months").asMilliseconds());
    EXPECT_EQ(t1, t2);
}

// =========================================================================
// toISOString
// =========================================================================

TEST(Duration, ToISOStringAllFields) {
    auto d = duration(DurationInput{
        .years = 1, .months = 2, .days = 3,
        .hours = 4, .minutes = 5, .seconds = 6
    });
    EXPECT_EQ(d.toISOString(), "P1Y2M3DT4H5M6S");
}

TEST(Duration, ToISOStringNegativeMonth) {
    EXPECT_EQ(duration(DurationInput{.months = -1}).toISOString(), "-P1M");
}

TEST(Duration, ToISOStringNegativeMinute) {
    EXPECT_EQ(duration(DurationInput{.minutes = -1}).toISOString(), "-PT1M");
}

TEST(Duration, ToISOStringNegativeHalfSecond) {
    EXPECT_EQ(duration(DurationInput{.seconds = 0, .milliseconds = -500}).toISOString(), "-PT0.5S");
}

TEST(Duration, ToISOStringPositiveMonth) {
    EXPECT_EQ(duration(DurationInput{.months = 1}).toISOString(), "P1M");
}

TEST(Duration, ToISOStringPositiveMinute) {
    EXPECT_EQ(duration(DurationInput{.minutes = 1}).toISOString(), "PT1M");
}

TEST(Duration, ToISOStringPositiveHalfSecond) {
    EXPECT_EQ(duration(DurationInput{.milliseconds = 500}).toISOString(), "PT0.5S");
}

TEST(Duration, ToISOStringZero) {
    EXPECT_EQ(duration(DurationInput{}).toISOString(), "P0D");
}

TEST(Duration, ToISOStringBubblesMonthsAndMs) {
    // M: 16, d: 40, s: 86465 => P1Y4M40DT24H1M5S
    auto d = duration(DurationInput{.months = 16, .days = 40, .seconds = 86465});
    EXPECT_EQ(d.toISOString(), "P1Y4M40DT24H1M5S");
}

TEST(Duration, ToISOStringFloatingPoint) {
    // 123456789 ms = 34h 17m 36.789s
    EXPECT_EQ(duration(DurationInput{.milliseconds = 123456789}).toISOString(), "PT34H17M36.789S");
    // 31952 ms = 31.952s
    EXPECT_EQ(duration(DurationInput{.milliseconds = 31952}).toISOString(), "PT31.952S");
}

TEST(Duration, ToISOStringPositiveYearAndMonth) {
    EXPECT_EQ(duration(DurationInput{.years = 1, .months = 1}).toISOString(), "P1Y1M");
}

// =========================================================================
// Add / Subtract
// =========================================================================

TEST(Duration, AddAndBubble) {
    EXPECT_EQ(duration(1, "second").add(1000, "milliseconds").seconds(), 2);
    EXPECT_EQ(duration(1, "minute").add(60, "second").minutes(), 2);
    EXPECT_EQ(duration(1, "hour").add(60, "minutes").hours(), 2);
    EXPECT_EQ(duration(1, "day").add(24, "hours").days(), 2);
}

TEST(Duration, AddMixedSigns) {
    auto d1 = duration(-1, "day");
    d1.add(1, "hour");
    EXPECT_EQ(d1.hours(), -23);
    EXPECT_DOUBLE_EQ(d1.asHours(), -23.0);

    auto d2 = duration(1, "day");
    d2.add(1, "hour");
    EXPECT_EQ(d2.hours(), 1);
    EXPECT_DOUBLE_EQ(d2.asHours(), 25.0);
}

TEST(Duration, AddYearAndDay) {
    auto d = duration(1, "year");
    d.add(1, "day");
    EXPECT_EQ(d.days(), 1);
    EXPECT_EQ(d.months(), 0);
    EXPECT_EQ(d.years(), 1);
    EXPECT_DOUBLE_EQ(d.asDays(), 366.0);
}

TEST(Duration, SubtractAndBubble) {
    EXPECT_EQ(duration(2, "second").subtract(1000, "milliseconds").seconds(), 1);
    EXPECT_EQ(duration(2, "minute").subtract(60, "second").minutes(), 1);
    EXPECT_EQ(duration(2, "hour").subtract(60, "minutes").hours(), 1);
    EXPECT_EQ(duration(2, "day").subtract(24, "hours").days(), 1);
}

TEST(Duration, SubtractDayHour) {
    auto d = duration(1, "day");
    d.subtract(1, "hour");
    EXPECT_EQ(d.hours(), 23);
    EXPECT_DOUBLE_EQ(d.asHours(), 23.0);
}

TEST(Duration, SubtractYearDay) {
    auto d = duration(1, "year");
    d.subtract(1, "day");
    EXPECT_EQ(d.days(), 30);
    EXPECT_EQ(d.months(), 11);
    EXPECT_EQ(d.years(), 0);
    EXPECT_DOUBLE_EQ(d.asDays(), 364.0);
}

TEST(Duration, SubtractYearHour) {
    auto d = duration(1, "year");
    d.subtract(1, "hour");
    EXPECT_EQ(d.hours(), 23);
    EXPECT_EQ(d.days(), 30);
    EXPECT_EQ(d.months(), 11);
    EXPECT_EQ(d.years(), 0);
}

TEST(Duration, AddDuration) {
    auto d1 = duration(1, "hours");
    auto d2 = duration(30, "minutes");
    d1.add(d2);
    EXPECT_EQ(d1.hours(), 1);
    EXPECT_EQ(d1.minutes(), 30);
}

TEST(Duration, SubtractDuration) {
    auto d1 = duration(2, "hours");
    auto d2 = duration(30, "minutes");
    d1.subtract(d2);
    EXPECT_EQ(d1.hours(), 1);
    EXPECT_EQ(d1.minutes(), 30);
}

// =========================================================================
// Abs
// =========================================================================

TEST(Duration, Abs) {
    auto d = duration(-5, "hours");
    EXPECT_EQ(d.hours(), -5);
    d.abs();
    EXPECT_EQ(d.hours(), 5);
}

TEST(Duration, AbsNegativeDays) {
    auto d = duration(-3, "days");
    d.abs();
    EXPECT_EQ(d.days(), 3);
}

TEST(Duration, AbsNegativeMonths) {
    auto d = duration(-6, "months");
    d.abs();
    EXPECT_EQ(d.months(), 6);
}

// =========================================================================
// isValid
// =========================================================================

TEST(Duration, IsValidForNormal) {
    EXPECT_TRUE(duration(static_cast<int64_t>(1000)).isValid());
    EXPECT_TRUE(duration(1, "day").isValid());
    EXPECT_TRUE(duration(DurationInput{.years = 1}).isValid());
}

TEST(Duration, IsValidForBadISO) {
    auto d = duration(std::string("garbage"));
    EXPECT_FALSE(d.isValid());
}

// =========================================================================
// valueOf
// =========================================================================

TEST(Duration, ValueOf) {
    EXPECT_EQ(duration(static_cast<int64_t>(1000)).valueOf(), 1000);
    EXPECT_EQ(duration(1, "second").valueOf(), 1000);
}

// =========================================================================
// toJSON
// =========================================================================

TEST(Duration, ToJSON) {
    auto d = duration(DurationInput{.hours = 1, .minutes = 30});
    EXPECT_EQ(d.toJSON(), d.toISOString());
}

// =========================================================================
// Locale
// =========================================================================

TEST(Duration, LocaleGetSet) {
    auto d = duration(1, "day");
    EXPECT_EQ(d.locale(), "en");
    d.locale("fr");
    EXPECT_EQ(d.locale(), "fr");
}

// =========================================================================
// Factory functions
// =========================================================================

TEST(Duration, FactoryMilliseconds) {
    auto d = duration(static_cast<int64_t>(5000));
    EXPECT_EQ(d.seconds(), 5);
}

TEST(Duration, FactoryAmountUnit) {
    auto d = duration(2, "hours");
    EXPECT_EQ(d.hours(), 2);
}

TEST(Duration, FactoryISOString) {
    auto d = duration(std::string("P1Y"));
    EXPECT_EQ(d.years(), 1);
}

TEST(Duration, FactoryDurationInput) {
    auto d = duration(DurationInput{.days = 10});
    EXPECT_EQ(d.days(), 10);
}

// =========================================================================
// Negative year + positive components (bubble with mixed signs)
// =========================================================================

TEST(Duration, NegativeYearPlusDay) {
    auto d = duration(-1, "year");
    d.add(1, "day");
    EXPECT_EQ(d.days(), -30);
    EXPECT_EQ(d.months(), -11);
    EXPECT_EQ(d.years(), 0);
    EXPECT_DOUBLE_EQ(d.asDays(), -364.0);
}

TEST(Duration, NegativeYearPlusHour) {
    auto d = duration(-1, "year");
    d.add(1, "hour");
    EXPECT_EQ(d.hours(), -23);
    EXPECT_EQ(d.days(), -30);
    EXPECT_EQ(d.months(), -11);
    EXPECT_EQ(d.years(), 0);
}

// =========================================================================
// Weeks getter
// =========================================================================

TEST(Duration, WeeksFloorOfDays) {
    EXPECT_EQ(duration(DurationInput{.days = 13}).weeks(), 1);
    EXPECT_EQ(duration(DurationInput{.days = 14}).weeks(), 2);
    EXPECT_EQ(duration(DurationInput{.days = 6}).weeks(), 0);
}

// =========================================================================
// asYears / asMonths with rounded precision
// =========================================================================

TEST(Duration, AsYearsOneYear) {
    auto val = duration(1, "year").asWeeks();
    // 1 year as weeks = 52.143 (to 3 decimal places)
    std::ostringstream oss;
    oss << std::fixed;
    oss.precision(3);
    oss << val;
    EXPECT_EQ(oss.str(), "52.143");
}

TEST(Duration, AsMonthsOneYear) {
    auto val = duration(1, "month").asYears();
    // 1 month as years = 0.0833 (to 4 decimal places)
    std::ostringstream oss;
    oss << std::fixed;
    oss.precision(4);
    oss << val;
    EXPECT_EQ(oss.str(), "0.0833");
}
