/**
 * @file test_display.cpp
 * @brief Tests for Moment display methods: diff, from, to, calendar, toJSON,
 *        toString, toArray, unix.
 *
 * All tests use UTC moments for deterministic, timezone-independent results.
 */
#include <gtest/gtest.h>
#include <polycpp/moment/detail/aggregator.hpp>
#include <cmath>

using namespace polycpp::moment;

// ── Known timestamps ────────────────────────────────────────────────
// 2024-03-15T14:30:45.123Z
static constexpr int64_t TS_2024_03_15 = 1710510645123LL;

// ═════════════════════════════════════════════════════════════════════
// diff
// ═════════════════════════════════════════════════════════════════════

TEST(DisplayTest, DiffMilliseconds) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(5000LL); b.utc();
    EXPECT_DOUBLE_EQ(b.diff(a), 5000.0);
    EXPECT_DOUBLE_EQ(a.diff(b), -5000.0);
}

TEST(DisplayTest, DiffSeconds) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(90000LL); b.utc(); // 90 seconds
    EXPECT_DOUBLE_EQ(b.diff(a, "seconds"), 90.0);
    EXPECT_DOUBLE_EQ(b.diff(a, "second"), 90.0);
    EXPECT_DOUBLE_EQ(b.diff(a, "s"), 90.0);
}

TEST(DisplayTest, DiffMinutes) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(150 * 60000LL); b.utc(); // 150 minutes
    EXPECT_DOUBLE_EQ(b.diff(a, "minutes"), 150.0);
}

TEST(DisplayTest, DiffHours) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(36 * 3600000LL); b.utc(); // 36 hours
    EXPECT_DOUBLE_EQ(b.diff(a, "hours"), 36.0);
}

TEST(DisplayTest, DiffDays) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(10 * 86400000LL); b.utc(); // 10 days
    EXPECT_DOUBLE_EQ(b.diff(a, "days"), 10.0);
}

TEST(DisplayTest, DiffWeeks) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(21 * 86400000LL); b.utc(); // 21 days = 3 weeks
    EXPECT_DOUBLE_EQ(b.diff(a, "weeks"), 3.0);
}

TEST(DisplayTest, DiffMonths) {
    // Jan 15 to Mar 15 = 2 months
    auto jan15 = Moment::InternalAccess::fromUtcComponents(2024, 0, 15, 0, 0, 0, 0);
    auto mar15 = Moment::InternalAccess::fromUtcComponents(2024, 2, 15, 0, 0, 0, 0);
    EXPECT_DOUBLE_EQ(mar15.diff(jan15, "months"), 2.0);
}

TEST(DisplayTest, DiffYears) {
    auto a = Moment::InternalAccess::fromUtcComponents(2020, 0, 1, 0, 0, 0, 0);
    auto b = Moment::InternalAccess::fromUtcComponents(2024, 0, 1, 0, 0, 0, 0);
    EXPECT_DOUBLE_EQ(b.diff(a, "years"), 4.0);
}

TEST(DisplayTest, DiffPrecise) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(1500LL); b.utc(); // 1.5 seconds
    EXPECT_DOUBLE_EQ(b.diff(a, "seconds"), 1.0);  // truncated
    EXPECT_DOUBLE_EQ(b.diff(a, "seconds", true), 1.5);  // precise
}

TEST(DisplayTest, DiffTruncateTowardZero) {
    // -1.5 seconds should truncate to -1, not floor to -2
    auto a = Moment(1500LL); a.utc();
    auto b = Moment(0LL); b.utc();
    EXPECT_DOUBLE_EQ(b.diff(a, "seconds"), -1.0);
}

TEST(DisplayTest, DiffQuarters) {
    auto jan = Moment::InternalAccess::fromUtcComponents(2024, 0, 1, 0, 0, 0, 0);
    auto jul = Moment::InternalAccess::fromUtcComponents(2024, 6, 1, 0, 0, 0, 0);
    EXPECT_DOUBLE_EQ(jul.diff(jan, "quarters"), 2.0);
}

// ═════════════════════════════════════════════════════════════════════
// from / to (relative time with known references)
// ═════════════════════════════════════════════════════════════════════

TEST(DisplayTest, FromKnownMomentHourAgo) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(3600000LL); b.utc(); // 1 hour later
    // a.from(b) = a is 1 hour before b = "an hour ago"
    EXPECT_EQ(a.from(b), "an hour ago");
    EXPECT_EQ(a.from(b, true), "an hour");
}

TEST(DisplayTest, FromKnownMomentFewSeconds) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(10000LL); b.utc(); // 10 seconds later
    // a.from(b) = a is 10 seconds before b = "a few seconds ago"
    EXPECT_EQ(a.from(b), "a few seconds ago");
}

TEST(DisplayTest, FromKnownMomentMinutes) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(5 * 60000LL); b.utc(); // 5 minutes later
    EXPECT_EQ(a.from(b), "5 minutes ago");
}

TEST(DisplayTest, FromKnownMomentFuture) {
    auto a = Moment(3600000LL); a.utc(); // 1 hour later
    auto b = Moment(0LL); b.utc();
    // a.from(b) = a is 1 hour after b = "in an hour"
    EXPECT_EQ(a.from(b), "in an hour");
}

TEST(DisplayTest, FromKnownMomentDays) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(5 * 86400000LL); b.utc(); // 5 days later
    EXPECT_EQ(a.from(b), "5 days ago");
}

TEST(DisplayTest, FromKnownMomentMonth) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(30 * 86400000LL); b.utc(); // ~30 days
    EXPECT_EQ(a.from(b), "a month ago");
}

TEST(DisplayTest, FromKnownMomentYear) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(365LL * 86400000LL); b.utc(); // ~1 year
    EXPECT_EQ(a.from(b), "a year ago");
}

TEST(DisplayTest, FromKnownMomentYears) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(730LL * 86400000LL); b.utc(); // ~2 years
    EXPECT_EQ(a.from(b), "2 years ago");
}

TEST(DisplayTest, ToKnownMoment) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(3600000LL); b.utc();
    // a.to(b) = from a's perspective, b is in the future = "in an hour"
    EXPECT_EQ(a.to(b), "in an hour");
    EXPECT_EQ(a.to(b, true), "an hour");
}

TEST(DisplayTest, ToKnownMomentPast) {
    auto a = Moment(3600000LL); a.utc();
    auto b = Moment(0LL); b.utc();
    // a.to(b) = from a's perspective, b is in the past = "an hour ago"
    EXPECT_EQ(a.to(b), "an hour ago");
}

// ═════════════════════════════════════════════════════════════════════
// calendar (with known reference)
// ═════════════════════════════════════════════════════════════════════

TEST(DisplayTest, CalendarSameDay) {
    // Both at 2024-03-15 in UTC, same day
    auto ref = Moment::InternalAccess::fromUtcComponents(2024, 2, 15, 8, 0, 0, 0);
    auto m   = Moment::InternalAccess::fromUtcComponents(2024, 2, 15, 14, 30, 0, 0);
    std::string result = m.calendar(ref);
    // Should contain "Today at" and the time
    EXPECT_NE(result.find("Today at"), std::string::npos);
}

TEST(DisplayTest, CalendarYesterday) {
    auto ref = Moment::InternalAccess::fromUtcComponents(2024, 2, 15, 12, 0, 0, 0);
    auto m   = Moment::InternalAccess::fromUtcComponents(2024, 2, 14, 14, 30, 0, 0);
    std::string result = m.calendar(ref);
    EXPECT_NE(result.find("Yesterday at"), std::string::npos);
}

TEST(DisplayTest, CalendarTomorrow) {
    auto ref = Moment::InternalAccess::fromUtcComponents(2024, 2, 15, 12, 0, 0, 0);
    auto m   = Moment::InternalAccess::fromUtcComponents(2024, 2, 16, 14, 30, 0, 0);
    std::string result = m.calendar(ref);
    EXPECT_NE(result.find("Tomorrow at"), std::string::npos);
}

TEST(DisplayTest, CalendarLastWeek) {
    auto ref = Moment::InternalAccess::fromUtcComponents(2024, 2, 15, 12, 0, 0, 0);
    auto m   = Moment::InternalAccess::fromUtcComponents(2024, 2, 11, 14, 30, 0, 0);
    std::string result = m.calendar(ref);
    // Should contain "Last" and "at"
    EXPECT_NE(result.find("Last"), std::string::npos);
    EXPECT_NE(result.find("at"), std::string::npos);
}

TEST(DisplayTest, CalendarNextWeek) {
    auto ref = Moment::InternalAccess::fromUtcComponents(2024, 2, 15, 12, 0, 0, 0);
    auto m   = Moment::InternalAccess::fromUtcComponents(2024, 2, 18, 14, 30, 0, 0);
    std::string result = m.calendar(ref);
    // Should contain a day name and "at" (e.g., "Monday at 2:30 PM")
    EXPECT_NE(result.find("at"), std::string::npos);
}

TEST(DisplayTest, CalendarSameElse) {
    auto ref = Moment::InternalAccess::fromUtcComponents(2024, 2, 15, 12, 0, 0, 0);
    auto m   = Moment::InternalAccess::fromUtcComponents(2024, 0, 1, 14, 30, 0, 0);
    std::string result = m.calendar(ref);
    // Should be formatted as L (MM/DD/YYYY in English)
    EXPECT_NE(result.find("01/01/2024"), std::string::npos);
}

TEST(DisplayTest, CalendarFormatKeys) {
    auto reference = utcFromDate(2024, 2, 15, 12, 0, 0);
    auto start = reference.clone().startOf("day");

    EXPECT_EQ(calendarFormat(utcFromDate(2024, 2, 7), start), "sameElse");
    EXPECT_EQ(calendarFormat(utcFromDate(2024, 2, 11), start), "lastWeek");
    EXPECT_EQ(calendarFormat(utcFromDate(2024, 2, 14), start), "lastDay");
    EXPECT_EQ(calendarFormat(utcFromDate(2024, 2, 15), start), "sameDay");
    EXPECT_EQ(calendarFormat(utcFromDate(2024, 2, 16), start), "nextDay");
    EXPECT_EQ(calendarFormat(utcFromDate(2024, 2, 18), start), "nextWeek");
    EXPECT_EQ(calendarFormat(utcFromDate(2024, 2, 22), start), "sameElse");
}

TEST(DisplayTest, CalendarAcceptsExplicitFormatOverrides) {
    CalendarFormats formats;
    formats.sameDay = std::string("[same] HH:mm");
    formats.nextDay = std::string("[next]");
    formats.nextWeek = std::string("[next week]");
    formats.lastDay = std::string("[last]");
    formats.lastWeek = std::string("[last week]");
    formats.sameElse = std::string("YYYY-MM-DD");

    auto ref = utcFromDate(2024, 2, 15, 12, 0, 0);
    auto same = utcFromDate(2024, 2, 15, 14, 30, 0);
    auto far = utcFromDate(2024, 0, 1, 14, 30, 0);

    EXPECT_EQ(same.calendar(ref, formats), "same 14:30");
    EXPECT_EQ(far.calendar(ref, formats), "2024-01-01");
}

// ═════════════════════════════════════════════════════════════════════
// toJSON, toString, toArray, unix
// ═════════════════════════════════════════════════════════════════════

TEST(DisplayTest, ToJSON) {
    auto m = Moment(TS_2024_03_15); m.utc();
    EXPECT_EQ(m.toJSON(), m.toISOString());
}

TEST(DisplayTest, ToJSONInvalidReturnsNull) {
    auto m = invalid();
    EXPECT_EQ(m.toJSON(), "null");
}

TEST(DisplayTest, ToString) {
    auto m = Moment::InternalAccess::fromUtcComponents(2024, 2, 15, 14, 30, 45, 0);
    std::string s = m.toString();
    EXPECT_NE(s.find("2024"), std::string::npos);
    EXPECT_NE(s.find("14:30:45"), std::string::npos);
    EXPECT_NE(s.find("GMT"), std::string::npos);
    EXPECT_NE(s.find("Fri"), std::string::npos);
    EXPECT_NE(s.find("Mar"), std::string::npos);
}

TEST(DisplayTest, ToStringInvalid) {
    auto m = invalid();
    EXPECT_EQ(m.toString(), "Invalid date");
}

TEST(DisplayTest, ToDateReturnsDateCopy) {
    auto m = utcFromDate(2024, 2, 15, 14, 30, 45, 123);
    auto d = m.toDate();
    EXPECT_TRUE(d.isValid());
    EXPECT_DOUBLE_EQ(d.getTime(), static_cast<double>(m.valueOf()));

    auto invalidDate = invalid().toDate();
    EXPECT_FALSE(invalidDate.isValid());
}

TEST(DisplayTest, UpstreamTimezoneAndLocaleConvenienceMethods) {
    auto halfHour = parseZone("2024-03-15T14:30:45+05:30");
    ASSERT_TRUE(halfHour.isValid());
    EXPECT_FALSE(halfHour.isUTC());
    EXPECT_TRUE(halfHour.isUtcOffset());
    EXPECT_FALSE(halfHour.hasAlignedHourOffset());
    EXPECT_EQ(halfHour.zoneAbbr(), "UTC");
    EXPECT_EQ(halfHour.zoneName(), "Coordinated Universal Time");
    EXPECT_EQ(halfHour.localeData().name, "en");

    auto fullHour = parseZone("2024-03-15T14:30:45+05:00");
    ASSERT_TRUE(fullHour.isValid());
    EXPECT_TRUE(fullHour.hasAlignedHourOffset());
    EXPECT_TRUE(fullHour.hasAlignedHourOffset(utcFromDate(2024, 2, 15)));

    auto m = utcFromDate(2024, 11, 31);
    EXPECT_EQ(m.weeksInWeekYear(), m.weeksInYear());
    EXPECT_EQ(m.isoWeeksInISOWeekYear(), m.isoWeeksInYear());
}

TEST(DisplayTest, InspectUsesMomentFactorySyntax) {
    auto utc = utcFromDate(2024, 2, 15, 14, 30, 45, 123);
    EXPECT_EQ(utc.inspect(), "moment.utc(\"2024-03-15T14:30:45.123+00:00\")");

    auto zoned = parseZone("2024-03-15T14:30:45.123+05:30");
    ASSERT_TRUE(zoned.isValid());
    EXPECT_EQ(zoned.inspect(), "moment.parseZone(\"2024-03-15T14:30:45.123+05:30\")");
}

TEST(DisplayTest, ToArray) {
    auto m = Moment::InternalAccess::fromUtcComponents(2024, 2, 15, 14, 30, 45, 123);
    auto arr = m.toArray();
    EXPECT_EQ(arr.size(), 7u);
    EXPECT_EQ(arr[0].asInt(), 2024);  // year
    EXPECT_EQ(arr[1].asInt(), 2);     // month (0-based)
    EXPECT_EQ(arr[2].asInt(), 15);    // day
    EXPECT_EQ(arr[3].asInt(), 14);    // hour
    EXPECT_EQ(arr[4].asInt(), 30);    // minute
    EXPECT_EQ(arr[5].asInt(), 45);    // second
    EXPECT_EQ(arr[6].asInt(), 123);   // ms
}

TEST(DisplayTest, Unix) {
    auto m = Moment(1710510645123LL); m.utc();
    EXPECT_EQ(m.unix(), 1710510645);
}

TEST(DisplayTest, UnixEpoch) {
    auto m = Moment(0LL); m.utc();
    EXPECT_EQ(m.unix(), 0);
}

// ═════════════════════════════════════════════════════════════════════
// from/to with withoutSuffix variations
// ═════════════════════════════════════════════════════════════════════

TEST(DisplayTest, FromWithoutSuffix) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(5 * 60000LL); b.utc();
    EXPECT_EQ(a.from(b, true), "5 minutes");
}

TEST(DisplayTest, ToWithoutSuffix) {
    auto a = Moment(0LL); a.utc();
    auto b = Moment(5 * 60000LL); b.utc();
    EXPECT_EQ(a.to(b, true), "5 minutes");
}

// ═════════════════════════════════════════════════════════════════════
// diff with invalid moments
// ═════════════════════════════════════════════════════════════════════

TEST(DisplayTest, DiffInvalid) {
    auto a = invalid();
    auto b = Moment(0LL); b.utc();
    EXPECT_TRUE(std::isnan(a.diff(b)));
    EXPECT_TRUE(std::isnan(b.diff(a)));
}

TEST(DisplayTest, FromInvalid) {
    auto a = invalid();
    auto b = Moment(0LL); b.utc();
    EXPECT_EQ(a.from(b), "Invalid date");
}
