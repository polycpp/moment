/**
 * @file test_format.cpp
 * @brief Tests for the format token system.
 *
 * All tests use a known UTC moment: 2024-03-15T14:30:45.123Z = 1710510645123 ms.
 * Tests cover all major format tokens, macro tokens, escaped text, and edge cases.
 */
#include <gtest/gtest.h>
#include <polycpp/moment/detail/aggregator.hpp>
#include <polycpp/core/error.hpp>

using namespace polycpp::moment;

// ── Known UTC timestamp: 2024-03-15T14:30:45.123Z ───────────────────
// Calculated: 2024-03-15 is day 75 of 2024, Friday (day=5)
// Unix seconds: 1710510645
// Unix ms:      1710510645123
static constexpr int64_t TS_KNOWN = 1710513045123LL;

// Helper to create our known UTC moment
static Moment knownUtc() {
    return utcFromMs(TS_KNOWN);
}

// ═════════════════════════════════════════════════════════════════════
// Year tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, YearTokens) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("YYYY"), "2024");
    EXPECT_EQ(m.format("YY"), "24");
}

// ═════════════════════════════════════════════════════════════════════
// Month tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, MonthTokens) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("M"), "3");
    EXPECT_EQ(m.format("MM"), "03");
    EXPECT_EQ(m.format("MMM"), "Mar");
    EXPECT_EQ(m.format("MMMM"), "March");
}

TEST(FormatTest, MonthOrdinal) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("Mo"), "3rd");
}

// ═════════════════════════════════════════════════════════════════════
// Day of month tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, DayTokens) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("D"), "15");
    EXPECT_EQ(m.format("DD"), "15");
    EXPECT_EQ(m.format("Do"), "15th");
}

TEST(FormatTest, DaySingleDigit) {
    // 2024-03-01T00:00:00Z
    auto m = utcFromDate(2024, 2, 1); // month 0-based, so 2 = March
    EXPECT_EQ(m.format("D"), "1");
    EXPECT_EQ(m.format("DD"), "01");
}

// ═════════════════════════════════════════════════════════════════════
// Day of year tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, DayOfYearTokens) {
    auto m = knownUtc();
    // 2024-03-15: Jan(31) + Feb(29, leap year) + 15 = 75
    EXPECT_EQ(m.format("DDD"), "75");
    EXPECT_EQ(m.format("DDDD"), "075");
}

// ═════════════════════════════════════════════════════════════════════
// Day of week tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, DayOfWeekTokens) {
    auto m = knownUtc();
    // 2024-03-15 is a Friday
    EXPECT_EQ(m.format("d"), "5");
    EXPECT_EQ(m.format("dd"), "Fr");
    EXPECT_EQ(m.format("ddd"), "Fri");
    EXPECT_EQ(m.format("dddd"), "Friday");
}

TEST(FormatTest, IsoWeekday) {
    auto m = knownUtc();
    // Friday = ISO weekday 5
    EXPECT_EQ(m.format("E"), "5");
}

TEST(FormatTest, LocaleWeekday) {
    auto m = knownUtc();
    // English locale: dow=0 (Sunday), so Friday = weekday 5
    EXPECT_EQ(m.format("e"), "5");
}

// ═════════════════════════════════════════════════════════════════════
// Time tokens (24h)
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, TimeTokens24h) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("H"), "14");
    EXPECT_EQ(m.format("HH"), "14");
}

TEST(FormatTest, TimeTokens24hMidnight) {
    auto m = utcFromDate(2024, 0, 1, 0, 0, 0, 0); // midnight
    EXPECT_EQ(m.format("H"), "0");
    EXPECT_EQ(m.format("HH"), "00");
}

// ═════════════════════════════════════════════════════════════════════
// Time tokens (12h)
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, TimeTokens12h) {
    auto m = knownUtc(); // 14:30 -> 2 PM
    EXPECT_EQ(m.format("h"), "2");
    EXPECT_EQ(m.format("hh"), "02");
}

TEST(FormatTest, TimeTokens12hNoon) {
    auto m = utcFromDate(2024, 0, 1, 12, 0, 0, 0); // noon
    EXPECT_EQ(m.format("h"), "12");
    EXPECT_EQ(m.format("hh"), "12");
}

TEST(FormatTest, TimeTokens12hMidnight) {
    auto m = utcFromDate(2024, 0, 1, 0, 0, 0, 0); // midnight
    EXPECT_EQ(m.format("h"), "12");
    EXPECT_EQ(m.format("hh"), "12");
}

// ═════════════════════════════════════════════════════════════════════
// Time tokens (1-24h, k/kk)
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, TimeTokensK) {
    auto m = knownUtc(); // 14:30
    EXPECT_EQ(m.format("k"), "14");
    EXPECT_EQ(m.format("kk"), "14");
}

TEST(FormatTest, TimeTokensKMidnight) {
    auto m = utcFromDate(2024, 0, 1, 0, 0, 0, 0);
    EXPECT_EQ(m.format("k"), "24");
    EXPECT_EQ(m.format("kk"), "24");
}

// ═════════════════════════════════════════════════════════════════════
// Minute and second tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, MinuteTokens) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("m"), "30");
    EXPECT_EQ(m.format("mm"), "30");
}

TEST(FormatTest, SecondTokens) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("s"), "45");
    EXPECT_EQ(m.format("ss"), "45");
}

TEST(FormatTest, MinuteSingleDigit) {
    auto m = utcFromDate(2024, 0, 1, 14, 5, 3, 0);
    EXPECT_EQ(m.format("m"), "5");
    EXPECT_EQ(m.format("mm"), "05");
    EXPECT_EQ(m.format("s"), "3");
    EXPECT_EQ(m.format("ss"), "03");
}

// ═════════════════════════════════════════════════════════════════════
// Fractional second tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, FractionalSecondTokens) {
    auto m = knownUtc(); // .123
    EXPECT_EQ(m.format("S"), "1");     // tenths
    EXPECT_EQ(m.format("SS"), "12");   // hundredths
    EXPECT_EQ(m.format("SSS"), "123"); // milliseconds
}

// ═════════════════════════════════════════════════════════════════════
// Meridiem tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, MeridiemTokens) {
    auto m = knownUtc(); // 14:30 = PM
    EXPECT_EQ(m.format("A"), "PM");
    EXPECT_EQ(m.format("a"), "pm");
}

TEST(FormatTest, MeridiemAM) {
    auto m = utcFromDate(2024, 0, 1, 9, 0, 0, 0);
    EXPECT_EQ(m.format("A"), "AM");
    EXPECT_EQ(m.format("a"), "am");
}

// ═════════════════════════════════════════════════════════════════════
// Timezone offset tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, TimezoneTokensUtc) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("Z"), "+00:00");
    EXPECT_EQ(m.format("ZZ"), "+0000");
}

TEST(FormatTest, TimezoneTokensOffset) {
    auto m = knownUtc();
    m.utcOffset(330); // +05:30 (India)
    EXPECT_EQ(m.format("Z"), "+05:30");
    EXPECT_EQ(m.format("ZZ"), "+0530");
}

TEST(FormatTest, TimezoneTokensNegativeOffset) {
    auto m = knownUtc();
    m.utcOffset(-300); // -05:00 (EST)
    EXPECT_EQ(m.format("Z"), "-05:00");
    EXPECT_EQ(m.format("ZZ"), "-0500");
}

// ═════════════════════════════════════════════════════════════════════
// Unix timestamp tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, UnixTimestampTokens) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("x"), std::to_string(TS_KNOWN));
}

TEST(FormatTest, UnixTimestampSecondsToken) {
    auto m = knownUtc();
    // X should produce "seconds.fraction"
    std::string x = m.format("X");
    // Should start with the seconds value
    EXPECT_TRUE(x.find("1710513045") != std::string::npos);
}

// ═════════════════════════════════════════════════════════════════════
// Escaped text
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, EscapedText) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("[Today is] dddd"), "Today is Friday");
}

TEST(FormatTest, EscapedTextWithTokenChars) {
    auto m = knownUtc();
    // Characters inside [] should not be interpreted as tokens
    EXPECT_EQ(m.format("[YYYY]"), "YYYY");
}

TEST(FormatTest, MultipleBrackets) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("[Date: ]YYYY-MM-DD"), "Date: 2024-03-15");
}

// ═════════════════════════════════════════════════════════════════════
// Macro tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, MacroTokenLT) {
    auto m = knownUtc();
    // LT = "h:mm A" in English locale
    EXPECT_EQ(m.format("LT"), "2:30 PM");
}

TEST(FormatTest, MacroTokenL) {
    auto m = knownUtc();
    // L = "MM/DD/YYYY" in English locale
    EXPECT_EQ(m.format("L"), "03/15/2024");
}

TEST(FormatTest, MacroTokenLL) {
    auto m = knownUtc();
    // LL = "MMMM D, YYYY" in English locale
    EXPECT_EQ(m.format("LL"), "March 15, 2024");
}

TEST(FormatTest, MacroTokenLLL) {
    auto m = knownUtc();
    // LLL = "MMMM D, YYYY h:mm A"
    EXPECT_EQ(m.format("LLL"), "March 15, 2024 2:30 PM");
}

TEST(FormatTest, MacroTokenLLLL) {
    auto m = knownUtc();
    // LLLL = "dddd, MMMM D, YYYY h:mm A"
    EXPECT_EQ(m.format("LLLL"), "Friday, March 15, 2024 2:30 PM");
}

// ═════════════════════════════════════════════════════════════════════
// Complex format strings
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, ComplexFormat) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("YYYY-MM-DD HH:mm:ss"), "2024-03-15 14:30:45");
}

TEST(FormatTest, FullISOLikeFormat) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("YYYY-MM-DD[T]HH:mm:ss.SSS[Z]"), "2024-03-15T14:30:45.123Z");
}

// ═════════════════════════════════════════════════════════════════════
// Default format (empty string)
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, EmptyFormatUsesDefault) {
    auto m = knownUtc();
    std::string result = m.format();
    // UTC moments use default format: YYYY-MM-DDTHH:mm:ss[Z] (no ms, literal Z)
    EXPECT_EQ(result, "2024-03-15T14:30:45Z");
}

// ═════════════════════════════════════════════════════════════════════
// Week tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, WeekTokens) {
    auto m = knownUtc();
    // ISO week: 2024-03-15 is Friday of ISO week 11
    std::string W = m.format("W");
    std::string WW = m.format("WW");
    EXPECT_EQ(W, "11");
    EXPECT_EQ(WW, "11");
}

TEST(FormatTest, LocaleWeekTokens) {
    auto m = knownUtc();
    std::string w = m.format("w");
    std::string ww = m.format("ww");
    // Locale week number should be a reasonable value
    EXPECT_FALSE(w.empty());
    EXPECT_FALSE(ww.empty());
}

// ═════════════════════════════════════════════════════════════════════
// Quarter token
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, QuarterToken) {
    auto m = knownUtc(); // March = Q1
    EXPECT_EQ(m.format("Q"), "1");
}

TEST(FormatTest, QuarterTokenQ2) {
    auto m = utcFromDate(2024, 3, 15); // April = Q2
    EXPECT_EQ(m.format("Q"), "2");
}

// ═════════════════════════════════════════════════════════════════════
// Invalid moment
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, InvalidMomentFormat) {
    auto m = invalid();
    EXPECT_EQ(m.format("YYYY-MM-DD"), "Invalid date");
    EXPECT_EQ(m.format(), "Invalid date");
}

// ═════════════════════════════════════════════════════════════════════
// toISOString
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, ToISOString) {
    auto m = knownUtc();
    EXPECT_EQ(m.toISOString(), "2024-03-15T14:30:45.123Z");
}

TEST(FormatTest, ToISOStringThrowsRangeErrorOnInvalid) {
    EXPECT_THROW(invalid().toISOString(), polycpp::RangeError);
}

TEST(FormatTest, ToISOStringKeepOffset) {
    auto m = knownUtc();
    m.utcOffset(330); // +05:30
    std::string iso = m.toISOString(true);
    EXPECT_TRUE(iso.find("+05:30") != std::string::npos);
}

// ═════════════════════════════════════════════════════════════════════
// Unix method
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, UnixMethod) {
    auto m = knownUtc();
    EXPECT_EQ(m.unix(), TS_KNOWN / 1000);
}

// ═════════════════════════════════════════════════════════════════════
// Week year tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, IsoWeekYearTokens) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("GGGG"), "2024");
}

// ═════════════════════════════════════════════════════════════════════
// LTS macro token
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, MacroTokenLTS) {
    auto m = knownUtc();
    // LTS = "h:mm:ss A" in English locale
    EXPECT_EQ(m.format("LTS"), "2:30:45 PM");
}

// ═════════════════════════════════════════════════════════════════════
// Mixed literal and tokens
// ═════════════════════════════════════════════════════════════════════

TEST(FormatTest, MixedLiteralsAndTokens) {
    auto m = knownUtc();
    EXPECT_EQ(m.format("YYYY/MM/DD"), "2024/03/15");
    EXPECT_EQ(m.format("HH:mm:ss.SSS"), "14:30:45.123");
}
