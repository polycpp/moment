/**
 * @file test_parse.cpp
 * @brief Tests for the parse factory functions.
 *
 * Tests cover ISO 8601 parsing, RFC 2822 parsing, custom format parsing,
 * fromUnixTimestamp, fromMilliseconds, fromDate, UTC creation, parseZone,
 * invalid() factory, and min/max aggregation.
 */
#include <gtest/gtest.h>
#include <polycpp/moment/detail/aggregator.hpp>

using namespace polycpp::moment;

// Known timestamp: 2024-03-15T14:30:45.123Z = 1710510645123
static constexpr int64_t TS_KNOWN = 1710513045123LL;

// ═════════════════════════════════════════════════════════════════════
// ISO 8601 parsing
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, ISOFull) {
    auto m = parse("2024-03-15T14:30:45.123Z");
    ASSERT_TRUE(m.isValid());
    m.utc();
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2); // 0-based: March
    EXPECT_EQ(m.date(), 15);
    EXPECT_EQ(m.hour(), 14);
    EXPECT_EQ(m.minute(), 30);
    EXPECT_EQ(m.second(), 45);
    EXPECT_EQ(m.millisecond(), 123);
}

TEST(ParseTest, ISODateOnly) {
    auto m = parse("2024-03-15");
    ASSERT_TRUE(m.isValid());
    // Date-only ISO should default to midnight
    // Note: When no timezone is provided, it's treated as local time in moment.js,
    // but in our implementation the exact behavior depends on the host timezone.
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
}

TEST(ParseTest, ISOYearMonth) {
    auto m = parse("2024-03");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
}

TEST(ParseTest, ISOYearOnly) {
    auto m = parse("2024");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
}

TEST(ParseTest, ISOWithPositiveOffset) {
    auto m = parse("2024-03-15T14:30:45+05:30");
    ASSERT_TRUE(m.isValid());
    // The moment should have the correct UTC time:
    // 14:30:45+05:30 = 09:00:45 UTC
    Moment utcView = m.clone();
    utcView.utc();
    EXPECT_EQ(utcView.hour(), 9);
    EXPECT_EQ(utcView.minute(), 0);
    EXPECT_EQ(utcView.second(), 45);
}

TEST(ParseTest, ISOWithNegativeOffset) {
    auto m = parse("2024-03-15T14:30:45-05:00");
    ASSERT_TRUE(m.isValid());
    // 14:30:45-05:00 = 19:30:45 UTC
    Moment utcView = m.clone();
    utcView.utc();
    EXPECT_EQ(utcView.hour(), 19);
    EXPECT_EQ(utcView.minute(), 30);
}

TEST(ParseTest, ISOWithSpace) {
    auto m = parse("2024-03-15 14:30:45");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
}

TEST(ParseTest, ISONoSeconds) {
    auto m = parse("2024-03-15T14:30");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.hour(), 14);
    EXPECT_EQ(m.minute(), 30);
}

TEST(ParseTest, ISOFractionalSeconds) {
    auto m = parse("2024-03-15T14:30:45.1Z");
    ASSERT_TRUE(m.isValid());
    m.utc();
    EXPECT_EQ(m.millisecond(), 100);

    auto m2 = parse("2024-03-15T14:30:45.12Z");
    ASSERT_TRUE(m2.isValid());
    m2.utc();
    EXPECT_EQ(m2.millisecond(), 120);

    auto m3 = parse("2024-03-15T14:30:45.123456Z");
    ASSERT_TRUE(m3.isValid());
    m3.utc();
    EXPECT_EQ(m3.millisecond(), 123); // truncated to ms
}

// ═════════════════════════════════════════════════════════════════════
// RFC 2822 parsing
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, RFC2822Full) {
    auto m = parse("Fri, 15 Mar 2024 14:30:45 +0000");
    ASSERT_TRUE(m.isValid());
    Moment utcView = m.clone();
    utcView.utc();
    EXPECT_EQ(utcView.year(), 2024);
    EXPECT_EQ(utcView.month(), 2);
    EXPECT_EQ(utcView.date(), 15);
    EXPECT_EQ(utcView.hour(), 14);
    EXPECT_EQ(utcView.minute(), 30);
    EXPECT_EQ(utcView.second(), 45);
}

TEST(ParseTest, RFC2822WithOffset) {
    auto m = parse("Fri, 15 Mar 2024 14:30:45 +0530");
    ASSERT_TRUE(m.isValid());
    Moment utcView = m.clone();
    utcView.utc();
    EXPECT_EQ(utcView.hour(), 9);
    EXPECT_EQ(utcView.minute(), 0);
}

TEST(ParseTest, RFC2822NoDayOfWeek) {
    auto m = parse("15 Mar 2024 14:30:45 +0000");
    ASSERT_TRUE(m.isValid());
    Moment utcView = m.clone();
    utcView.utc();
    EXPECT_EQ(utcView.year(), 2024);
    EXPECT_EQ(utcView.date(), 15);
}

// ═════════════════════════════════════════════════════════════════════
// Custom format parsing
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, CustomFormatDDMMYYYY) {
    auto m = parse("15/03/2024", "DD/MM/YYYY");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2); // March
    EXPECT_EQ(m.date(), 15);
}

TEST(ParseTest, CustomFormatTime12h) {
    auto m = parse("2:30 PM", "h:mm A");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.hour(), 14);
    EXPECT_EQ(m.minute(), 30);
}

TEST(ParseTest, CustomFormatTime12hAM) {
    auto m = parse("9:15 AM", "h:mm A");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.hour(), 9);
    EXPECT_EQ(m.minute(), 15);
}

TEST(ParseTest, CustomFormatYYYYMMDD) {
    auto m = parse("2024-03-15", "YYYY-MM-DD");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
}

TEST(ParseTest, CustomFormatWithMonthName) {
    auto m = parse("March 15, 2024", "MMMM D, YYYY");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
}

TEST(ParseTest, CustomFormatShortMonthName) {
    auto m = parse("Mar 15, 2024", "MMM D, YYYY");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
}

TEST(ParseTest, CustomFormatFullDateTime) {
    auto m = parse("2024-03-15 14:30:45", "YYYY-MM-DD HH:mm:ss");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
    EXPECT_EQ(m.hour(), 14);
    EXPECT_EQ(m.minute(), 30);
    EXPECT_EQ(m.second(), 45);
}

TEST(ParseTest, CustomFormatTwoDigitYear) {
    auto m = parse("15/03/24", "DD/MM/YY");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
}

TEST(ParseTest, CustomFormatTwoDigitYear68) {
    auto m = parse("15/03/68", "DD/MM/YY");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 1968);  // 68-99 -> 1968-1999 (moment.js convention)
}

TEST(ParseTest, CustomFormatTwoDigitYear69) {
    auto m = parse("15/03/69", "DD/MM/YY");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 1969);
}

// ═════════════════════════════════════════════════════════════════════
// Strict mode
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, StrictModeValid) {
    auto m = parse("2024-03-15", "YYYY-MM-DD", true);
    ASSERT_TRUE(m.isValid());
}

TEST(ParseTest, StrictModeExtraChars) {
    auto m = parse("2024-03-15 extra", "YYYY-MM-DD", true);
    EXPECT_FALSE(m.isValid());
}

// ═════════════════════════════════════════════════════════════════════
// Multiple formats
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, MultipleFormats) {
    std::vector<std::string> formats = {"DD/MM/YYYY", "YYYY-MM-DD", "MM-DD-YYYY"};
    auto m = parse("15/03/2024", formats);
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
}

TEST(ParseTest, MultipleFormatsSecondMatches) {
    std::vector<std::string> formats = {"DD/MM/YYYY", "YYYY-MM-DD"};
    auto m = parse("2024-03-15", formats);
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
}

// ═════════════════════════════════════════════════════════════════════
// fromUnixTimestamp
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, FromUnixTimestamp) {
    auto m = fromUnixTimestamp(1710513045);
    ASSERT_TRUE(m.isValid());
    // Should be approximately 2024-03-15T14:30:45
    Moment utcView = m.clone();
    utcView.utc();
    EXPECT_EQ(utcView.year(), 2024);
    EXPECT_EQ(utcView.month(), 2);
    EXPECT_EQ(utcView.date(), 15);
    EXPECT_EQ(utcView.hour(), 14);
    EXPECT_EQ(utcView.minute(), 30);
    EXPECT_EQ(utcView.second(), 45);
}

// ═════════════════════════════════════════════════════════════════════
// fromMilliseconds
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, FromMilliseconds) {
    auto m = fromMilliseconds(TS_KNOWN);
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.valueOf(), TS_KNOWN);
}

// ═════════════════════════════════════════════════════════════════════
// fromDate
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, FromDate) {
    auto m = fromDate(2024, 2, 15); // month 0-based: March
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
}

TEST(ParseTest, FromDateWithTime) {
    auto m = fromDate(2024, 2, 15, 14, 30, 45, 123);
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
    EXPECT_EQ(m.hour(), 14);
    EXPECT_EQ(m.minute(), 30);
    EXPECT_EQ(m.second(), 45);
    EXPECT_EQ(m.millisecond(), 123);
}

// ═════════════════════════════════════════════════════════════════════
// UTC creation
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, UtcNow) {
    auto m = utcNow();
    ASSERT_TRUE(m.isValid());
    EXPECT_TRUE(m.isUtc());
}

TEST(ParseTest, UtcFromString) {
    auto m = utcFromString("2024-03-15T14:30:45Z");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
    EXPECT_EQ(m.hour(), 14);
}

TEST(ParseTest, UtcFromMs) {
    auto m = utcFromMs(TS_KNOWN);
    ASSERT_TRUE(m.isValid());
    EXPECT_TRUE(m.isUtc());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
}

TEST(ParseTest, UtcFromDate) {
    auto m = utcFromDate(2024, 2, 15, 14, 30, 45, 123);
    ASSERT_TRUE(m.isValid());
    EXPECT_TRUE(m.isUtc());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
    EXPECT_EQ(m.hour(), 14);
    EXPECT_EQ(m.minute(), 30);
    EXPECT_EQ(m.second(), 45);
    EXPECT_EQ(m.millisecond(), 123);
}

// ═════════════════════════════════════════════════════════════════════
// parseZone — preserve parsed offset
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, ParseZonePreservesOffset) {
    auto m = parseZone("2024-03-15T14:30:45+05:30");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.utcOffset(), 330); // +05:30 = 330 minutes
}

TEST(ParseTest, ParseZoneUtc) {
    auto m = parseZone("2024-03-15T14:30:45Z");
    ASSERT_TRUE(m.isValid());
    EXPECT_TRUE(m.isUtc());
    EXPECT_EQ(m.utcOffset(), 0);
}

TEST(ParseTest, ParseZoneNegativeOffset) {
    auto m = parseZone("2024-03-15T14:30:45-05:00");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.utcOffset(), -300); // -05:00 = -300 minutes
}

// ═════════════════════════════════════════════════════════════════════
// Invalid input
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, InvalidInput) {
    auto m = parse("not a date");
    EXPECT_FALSE(m.isValid());
}

TEST(ParseTest, EmptyInput) {
    auto m = parse("");
    EXPECT_FALSE(m.isValid());
}

// ═════════════════════════════════════════════════════════════════════
// invalid() factory
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, InvalidFactory) {
    auto m = invalid();
    EXPECT_FALSE(m.isValid());
}

// ═════════════════════════════════════════════════════════════════════
// min/max
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, MinInitializerList) {
    auto m1 = utcFromMs(1000);
    auto m2 = utcFromMs(2000);
    auto m3 = utcFromMs(3000);
    auto result = min({m1, m2, m3});
    EXPECT_EQ(result.valueOf(), 1000);
}

TEST(ParseTest, MaxInitializerList) {
    auto m1 = utcFromMs(1000);
    auto m2 = utcFromMs(2000);
    auto m3 = utcFromMs(3000);
    auto result = max({m1, m2, m3});
    EXPECT_EQ(result.valueOf(), 3000);
}

TEST(ParseTest, MinVector) {
    std::vector<Moment> moments = {utcFromMs(5000), utcFromMs(1000), utcFromMs(3000)};
    auto result = min(moments);
    EXPECT_EQ(result.valueOf(), 1000);
}

TEST(ParseTest, MaxVector) {
    std::vector<Moment> moments = {utcFromMs(5000), utcFromMs(1000), utcFromMs(3000)};
    auto result = max(moments);
    EXPECT_EQ(result.valueOf(), 5000);
}

TEST(ParseTest, MinWithInvalid) {
    auto m1 = utcFromMs(1000);
    auto m2 = invalid();
    auto result = min({m1, m2});
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.valueOf(), 1000);
}

TEST(ParseTest, MinEmpty) {
    auto result = min(std::vector<Moment>{});
    EXPECT_FALSE(result.isValid());
}

// ═════════════════════════════════════════════════════════════════════
// Round-trip: format then parse
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, RoundTripISO) {
    auto m1 = utcFromMs(TS_KNOWN);
    std::string iso = m1.toISOString();
    auto m2 = parse(iso);
    ASSERT_TRUE(m2.isValid());
    m2.utc();
    EXPECT_EQ(m1.valueOf(), m2.valueOf());
}

TEST(ParseTest, RoundTripCustomFormat) {
    auto m1 = utcFromDate(2024, 2, 15, 14, 30, 45, 0);
    std::string formatted = m1.format("YYYY-MM-DD HH:mm:ss");
    auto m2 = parse(formatted, "YYYY-MM-DD HH:mm:ss");
    ASSERT_TRUE(m2.isValid());
    // The parsed time should match (it will be in local time unless we force UTC)
    EXPECT_EQ(m2.year(), 2024);
    EXPECT_EQ(m2.month(), 2);
    EXPECT_EQ(m2.date(), 15);
}

// ═════════════════════════════════════════════════════════════════════
// UTC from format
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, UtcFromFormat) {
    auto m = utcFromFormat("15/03/2024", "DD/MM/YYYY");
    ASSERT_TRUE(m.isValid());
    EXPECT_TRUE(m.isUtc());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 2);
    EXPECT_EQ(m.date(), 15);
}
