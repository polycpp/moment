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

TEST(ParseTest, CreationDataAndParsingFlagsForISO) {
    auto m = parse("2024-03-15T14:30:45Z");
    ASSERT_TRUE(m.isValid());

    auto flags = m.parsingFlags();
    EXPECT_TRUE(flags.iso);
    EXPECT_EQ(flags.overflow, -1);
    ASSERT_GE(flags.parsedDateParts.size(), 6U);

    auto data = m.creationData();
    EXPECT_EQ(data.input, "2024-03-15T14:30:45Z");
    EXPECT_TRUE(data.isUTC);
    EXPECT_FALSE(data.strict);
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

TEST(ParseTest, ExplicitIsoAndRfcSentinelFormats) {
    auto iso = parse("2024-03-15T14:30:45Z", ISO_8601, true);
    ASSERT_TRUE(iso.isValid());
    EXPECT_TRUE(iso.parsingFlags().iso);
    EXPECT_EQ(iso.creationData().format, ISO_8601);
    iso.utc();
    EXPECT_EQ(iso.format("YYYY-MM-DDTHH:mm:ss[Z]"), "2024-03-15T14:30:45Z");

    auto rfc = parse("Fri, 15 Mar 2024 14:30:45 +0000", RFC_2822, true);
    ASSERT_TRUE(rfc.isValid());
    EXPECT_TRUE(rfc.parsingFlags().rfc2822);
    EXPECT_EQ(rfc.creationData().format, RFC_2822);

    EXPECT_FALSE(parse("Fri, 15 Mar 2024 14:30:45 +0000", ISO_8601, true).isValid());
    EXPECT_FALSE(parse("2024-03-15T14:30:45Z", RFC_2822, true).isValid());
}

// ═════════════════════════════════════════════════════════════════════
// polycpp::Date fallback parsing
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, PolycppDateToStringFallback) {
    auto m = parse("Tue Nov 14 2023 22:13:20 GMT+0000 (UTC)");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.valueOf(), 1700000000000LL);
}

TEST(ParseTest, PolycppDateToStringFallbackWithOffset) {
    auto m = parse("Tue Nov 14 2023 22:13:20 GMT+0530");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.valueOf(), 1699980200000LL);
}

TEST(ParseTest, PolycppDateFallbackRejectsImplementationSpecificInputs) {
    EXPECT_FALSE(parse("March 5 2024").isValid());
    EXPECT_FALSE(parse("2024/03/15").isValid());
    EXPECT_FALSE(parse("2024-1-1").isValid());
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

TEST(ParseTest, Html5FormatConstants) {
    auto datetime = parse("2024-03-15T14:30", HTML5_FMT::DATETIME_LOCAL, true);
    ASSERT_TRUE(datetime.isValid());
    EXPECT_EQ(datetime.year(), 2024);
    EXPECT_EQ(datetime.month(), 2);
    EXPECT_EQ(datetime.date(), 15);
    EXPECT_EQ(datetime.hour(), 14);
    EXPECT_EQ(datetime.minute(), 30);

    auto datetimeMs = parse("2024-03-15T14:30:45.123", HTML5_FMT::DATETIME_LOCAL_MS, true);
    ASSERT_TRUE(datetimeMs.isValid());
    EXPECT_EQ(datetimeMs.second(), 45);
    EXPECT_EQ(datetimeMs.millisecond(), 123);

    auto datetimeSeconds = parse("2024-03-15T14:30:45", HTML5_FMT::DATETIME_LOCAL_SECONDS, true);
    ASSERT_TRUE(datetimeSeconds.isValid());
    EXPECT_EQ(datetimeSeconds.second(), 45);

    auto date = parse("2024-03-15", HTML5_FMT::DATE, true);
    ASSERT_TRUE(date.isValid());
    EXPECT_EQ(date.year(), 2024);
    EXPECT_EQ(date.month(), 2);
    EXPECT_EQ(date.date(), 15);

    auto time = parse("14:30", HTML5_FMT::TIME, true);
    ASSERT_TRUE(time.isValid());
    EXPECT_EQ(time.hour(), 14);
    EXPECT_EQ(time.minute(), 30);

    auto timeMs = parse("14:30:45.123", HTML5_FMT::TIME_MS, true);
    ASSERT_TRUE(timeMs.isValid());
    EXPECT_EQ(timeMs.hour(), 14);
    EXPECT_EQ(timeMs.minute(), 30);
    EXPECT_EQ(timeMs.second(), 45);
    EXPECT_EQ(timeMs.millisecond(), 123);

    auto timeSeconds = parse("14:30:45", HTML5_FMT::TIME_SECONDS, true);
    ASSERT_TRUE(timeSeconds.isValid());
    EXPECT_EQ(timeSeconds.hour(), 14);
    EXPECT_EQ(timeSeconds.minute(), 30);
    EXPECT_EQ(timeSeconds.second(), 45);

    auto month = parse("2024-03", html5_fmt::MONTH, true);
    ASSERT_TRUE(month.isValid());
    EXPECT_EQ(month.year(), 2024);
    EXPECT_EQ(month.month(), 2);
    EXPECT_EQ(month.date(), 1);

    auto week = parse("2024-W11", HTML5_FMT::WEEK, true);
    ASSERT_TRUE(week.isValid());
    EXPECT_EQ(week.format(HTML5_FMT::DATE), "2024-03-11");
    EXPECT_EQ(week.isoWeekYear(), 2024);
    EXPECT_EQ(week.isoWeek(), 11);
    EXPECT_EQ(week.isoWeekday(), 1);
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

TEST(ParseTest, ParsesEnglishEraYear) {
    auto m = parse("AD 2024-04-27", "N y-MM-DD", true);
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 2024);
    EXPECT_EQ(m.month(), 3);
    EXPECT_EQ(m.date(), 27);
}

TEST(ParseTest, ParsesEnglishBeforeChristEraYear) {
    auto m = parse("BC 1-12-31", "N y-MM-DD", true);
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 0);
    EXPECT_EQ(m.month(), 11);
    EXPECT_EQ(m.date(), 31);
    EXPECT_EQ(m.eraAbbr(), "BC");
    EXPECT_EQ(m.eraYear(), 1);
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
    EXPECT_EQ(m.year(), 2068);  // 00-68 -> 2000-2068 (moment.js convention)
}

TEST(ParseTest, CustomFormatTwoDigitYear69) {
    auto m = parse("15/03/69", "DD/MM/YY");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.year(), 1969);
}

TEST(ParseTest, ParseTwoDigitYearHelperUsesUpstreamPivot) {
    EXPECT_EQ(parseTwoDigitYear("00"), 2000);
    EXPECT_EQ(parseTwoDigitYear("68"), 2068);
    EXPECT_EQ(parseTwoDigitYear("69"), 1969);
}

TEST(ParseTest, ParsesDayOfYearAndExpandedFormatTokens) {
    auto doy = parse("2024-075", "YYYY-DDD", true);
    ASSERT_TRUE(doy.isValid());
    EXPECT_EQ(doy.format("YYYY-MM-DD"), "2024-03-15");

    auto combined = parse("2024-03-15 143045.1234", "YYYY-MM-DD Hmmss.SSSS", true);
    ASSERT_TRUE(combined.isValid());
    EXPECT_EQ(combined.format("HH:mm:ss.SSS"), "14:30:45.123");

    auto expanded = parse("+002024-03-15", "YYYYYY-MM-DD", true);
    ASSERT_TRUE(expanded.isValid());
    EXPECT_EQ(expanded.format("YYYY-MM-DD"), "2024-03-15");
}

TEST(ParseTest, ParsesLocaleAndIsoWeekFormats) {
    auto localeWeek = parse("2024-w11-5", "gggg-[w]ww-e", true);
    ASSERT_TRUE(localeWeek.isValid());
    EXPECT_EQ(localeWeek.format("YYYY-MM-DD"), "2024-03-15");
    EXPECT_EQ(localeWeek.weekYear(), 2024);
    EXPECT_EQ(localeWeek.week(), 11);
    EXPECT_EQ(localeWeek.weekday(), 5);

    auto isoWeek = parse("2024-W11-5", "GGGG-[W]WW-E", true);
    ASSERT_TRUE(isoWeek.isValid());
    EXPECT_EQ(isoWeek.format("YYYY-MM-DD"), "2024-03-15");
    EXPECT_EQ(isoWeek.isoWeekYear(), 2024);
    EXPECT_EQ(isoWeek.isoWeek(), 11);
    EXPECT_EQ(isoWeek.isoWeekday(), 5);
}

TEST(ParseTest, WeekdayMismatchInvalidatesStrictParse) {
    auto bad = parse("Thursday, 2024-03-15", "dddd, YYYY-MM-DD", true);
    EXPECT_FALSE(bad.isValid());
    EXPECT_TRUE(bad.parsingFlags().weekdayMismatch);

    auto good = parse("Friday, 2024-03-15", "dddd, YYYY-MM-DD", true);
    ASSERT_TRUE(good.isValid());
    EXPECT_FALSE(good.parsingFlags().weekdayMismatch);
}

// ═════════════════════════════════════════════════════════════════════
// Strict mode
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, StrictModeValid) {
    auto m = parse("2024-03-15", "YYYY-MM-DD", true);
    ASSERT_TRUE(m.isValid());
}

TEST(ParseTest, ParsingFlagsCaptureDateOverflow) {
    auto m = parse("2024-02-31", "YYYY-MM-DD", true);
    EXPECT_FALSE(m.isValid());
    EXPECT_EQ(m.invalidAt(), 2);

    auto flags = m.parsingFlags();
    EXPECT_EQ(flags.overflow, 2);
    ASSERT_EQ(flags.parsedDateParts.size(), 3U);
    EXPECT_EQ(flags.parsedDateParts[0], 2024);
    EXPECT_EQ(flags.parsedDateParts[1], 1);
    EXPECT_EQ(flags.parsedDateParts[2], 31);
}

TEST(ParseTest, StrictModeExtraChars) {
    auto m = parse("2024-03-15 extra", "YYYY-MM-DD", true);
    EXPECT_FALSE(m.isValid());

    auto flags = m.parsingFlags();
    EXPECT_EQ(flags.charsLeftOver, 6);
    ASSERT_EQ(flags.unusedInput.size(), 1U);
    EXPECT_EQ(flags.unusedInput[0], " extra");
    EXPECT_EQ(m.creationData().input, "2024-03-15 extra");
    EXPECT_EQ(m.creationData().format, "YYYY-MM-DD");
    EXPECT_TRUE(m.creationData().strict);
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

TEST(ParseTest, UtcFromStringUsesPolycppDateFallback) {
    auto m = utcFromString("Tue Nov 14 2023 22:13:20 GMT+0000 (UTC)");
    ASSERT_TRUE(m.isValid());
    EXPECT_TRUE(m.isUtc());
    EXPECT_EQ(m.valueOf(), 1700000000000LL);
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
    EXPECT_EQ(m.hour(), 14);
    EXPECT_EQ(m.minute(), 30);
    EXPECT_EQ(m.format("YYYY-MM-DD[T]HH:mm:ssZ"), "2024-03-15T14:30:45+05:30");
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

TEST(ParseTest, ParseZoneUsesPolycppDateFallbackOffset) {
    auto m = parseZone("Tue Nov 14 2023 22:13:20 GMT+0530");
    ASSERT_TRUE(m.isValid());
    EXPECT_EQ(m.utcOffset(), 330);
    EXPECT_EQ(m.valueOf(), 1699980200000LL);
}

TEST(ParseTest, ParseZoneAssumesUtcWhenNoOffsetIsPresent) {
    auto m = parseZone("2024-03-15T14:30:45");
    ASSERT_TRUE(m.isValid());
    EXPECT_TRUE(m.isUtc());
    EXPECT_EQ(m.utcOffset(), 0);
    EXPECT_EQ(m.format("YYYY-MM-DD[T]HH:mm:ssZ"), "2024-03-15T14:30:45+00:00");
}

TEST(ParseTest, InstanceParseZoneUsesCreationInputOffset) {
    auto withOffset = parse("2024-03-15T14:30:45+05:30");
    ASSERT_TRUE(withOffset.isValid());
    withOffset.parseZone();
    EXPECT_EQ(withOffset.utcOffset(), 330);
    EXPECT_EQ(withOffset.format("YYYY-MM-DD[T]HH:mm:ssZ"), "2024-03-15T14:30:45+05:30");

    auto withoutOffset = parse("2024-03-15T14:30:45");
    ASSERT_TRUE(withoutOffset.isValid());
    withoutOffset.parseZone();
    EXPECT_TRUE(withoutOffset.isUtc());
    EXPECT_EQ(withoutOffset.format("YYYY-MM-DD[T]HH:mm:ssZ"), "2024-03-15T14:30:45+00:00");
}

TEST(ParseTest, ParseZoneSupportsFormatAndSentinelInputs) {
    auto formatted = parseZone("2024-03-15T14:30:45+05:30", "YYYY-MM-DDTHH:mm:ssZ");
    ASSERT_TRUE(formatted.isValid());
    EXPECT_EQ(formatted.utcOffset(), 330);
    EXPECT_EQ(formatted.format("YYYY-MM-DD[T]HH:mm:ssZ"), "2024-03-15T14:30:45+05:30");

    auto sentinel = parseZone("2024-03-15T14:30:45", ISO_8601);
    ASSERT_TRUE(sentinel.isValid());
    EXPECT_TRUE(sentinel.isUtc());
    EXPECT_EQ(sentinel.creationData().format, ISO_8601);
}

// ═════════════════════════════════════════════════════════════════════
// Invalid input
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, InvalidInput) {
    auto m = parse("not a date");
    EXPECT_FALSE(m.isValid());
    EXPECT_EQ(m.creationData().input, "not a date");
    EXPECT_EQ(m.inspect(), "moment.invalid(/* not a date */)");
}

TEST(ParseTest, EmptyInput) {
    auto m = parse("");
    EXPECT_FALSE(m.isValid());
    EXPECT_TRUE(m.parsingFlags().nullInput);
}

// ═════════════════════════════════════════════════════════════════════
// invalid() factory
// ═════════════════════════════════════════════════════════════════════

TEST(ParseTest, InvalidFactory) {
    auto m = invalid();
    EXPECT_FALSE(m.isValid());
    EXPECT_TRUE(m.parsingFlags().userInvalidated);
    EXPECT_EQ(m.inspect(), "moment.invalid(/* NaN */)");
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

TEST(ParseTest, UtcFromFormatHonorsParsedOffsetsAndSentinels) {
    auto withOffset = utcFromFormat("2024-03-15T14:30:45+05:30", "YYYY-MM-DDTHH:mm:ssZ");
    ASSERT_TRUE(withOffset.isValid());
    EXPECT_TRUE(withOffset.isUtc());
    EXPECT_EQ(withOffset.format("YYYY-MM-DD[T]HH:mm:ss[Z]"), "2024-03-15T09:00:45Z");

    auto iso = utcFromFormat("2024-03-15T14:30:45+05:30", ISO_8601);
    ASSERT_TRUE(iso.isValid());
    EXPECT_TRUE(iso.isUtc());
    EXPECT_EQ(iso.creationData().format, ISO_8601);
    EXPECT_EQ(iso.format("YYYY-MM-DD[T]HH:mm:ss[Z]"), "2024-03-15T09:00:45Z");
}
