/**
 * @file test_query.cpp
 * @brief Tests for Moment query methods: isBefore, isAfter, isSame,
 *        isSameOrBefore, isSameOrAfter, isBetween, isValid, isLeapYear,
 *        isDST, isUtc, isLocal, isUtcOffset.
 *
 * All tests use UTC moments for deterministic, timezone-independent results.
 */
#include <gtest/gtest.h>
#include <polycpp/moment/detail/aggregator.hpp>

using namespace polycpp::moment;

// ── Helper: create UTC moments from date components ──────────────────
static Moment utcDate(int year, int month, int day = 1, int hour = 0,
                      int minute = 0, int second = 0, int ms = 0) {
    return Moment::InternalAccess::fromUtcComponents(year, month, day, hour, minute, second, ms);
}

// ═════════════════════════════════════════════════════════════════════
// isBefore
// ═════════════════════════════════════════════════════════════════════

TEST(QueryTest, IsBefore) {
    auto jan = utcDate(2024, 0, 15);
    auto feb = utcDate(2024, 1, 15);
    EXPECT_TRUE(jan.isBefore(feb));
    EXPECT_FALSE(feb.isBefore(jan));
    EXPECT_FALSE(jan.isBefore(jan.clone()));
}

TEST(QueryTest, IsBeforeUnit) {
    auto jan15 = utcDate(2024, 0, 15, 10, 0, 0);
    auto jan20 = utcDate(2024, 0, 20, 5, 0, 0);
    // Same month, so isBefore("month") should be false
    EXPECT_FALSE(jan15.isBefore(jan20, "month"));
    // Different days
    EXPECT_TRUE(jan15.isBefore(jan20, "day"));
    // Different month
    auto feb5 = utcDate(2024, 1, 5);
    EXPECT_TRUE(jan15.isBefore(feb5, "month"));
}

TEST(QueryTest, IsBeforeYear) {
    auto a = utcDate(2023, 5, 15);
    auto b = utcDate(2024, 0, 1);
    EXPECT_TRUE(a.isBefore(b, "year"));
    EXPECT_FALSE(b.isBefore(a, "year"));
}

TEST(QueryTest, IsBeforeInvalid) {
    auto a = invalid();
    auto b = utcDate(2024, 0, 1);
    EXPECT_FALSE(a.isBefore(b));
    EXPECT_FALSE(b.isBefore(a));
}

// ═════════════════════════════════════════════════════════════════════
// isAfter
// ═════════════════════════════════════════════════════════════════════

TEST(QueryTest, IsAfter) {
    auto jan = utcDate(2024, 0, 15);
    auto feb = utcDate(2024, 1, 15);
    EXPECT_TRUE(feb.isAfter(jan));
    EXPECT_FALSE(jan.isAfter(feb));
    EXPECT_FALSE(jan.isAfter(jan.clone()));
}

TEST(QueryTest, IsAfterUnit) {
    auto jan15 = utcDate(2024, 0, 15, 10, 0, 0);
    auto jan20 = utcDate(2024, 0, 20, 5, 0, 0);
    EXPECT_FALSE(jan20.isAfter(jan15, "month")); // same month
    EXPECT_TRUE(jan20.isAfter(jan15, "day"));
}

TEST(QueryTest, IsAfterInvalid) {
    auto a = invalid();
    auto b = utcDate(2024, 0, 1);
    EXPECT_FALSE(a.isAfter(b));
    EXPECT_FALSE(b.isAfter(a));
}

// ═════════════════════════════════════════════════════════════════════
// isSame
// ═════════════════════════════════════════════════════════════════════

TEST(QueryTest, IsSame) {
    auto jan = utcDate(2024, 0, 15);
    EXPECT_TRUE(jan.isSame(jan.clone()));
    auto feb = utcDate(2024, 1, 15);
    EXPECT_FALSE(jan.isSame(feb));
}

TEST(QueryTest, IsSameUnit) {
    auto a = utcDate(2024, 0, 15);
    auto b = utcDate(2024, 0, 20);
    EXPECT_TRUE(a.isSame(b, "month"));   // same month
    EXPECT_FALSE(a.isSame(b, "day"));    // different day
    EXPECT_TRUE(a.isSame(b, "year"));    // same year
}

TEST(QueryTest, IsSameInvalid) {
    auto a = invalid();
    auto b = utcDate(2024, 0, 1);
    EXPECT_FALSE(a.isSame(b));
    EXPECT_FALSE(a.isSame(a.clone()));
}

// ═════════════════════════════════════════════════════════════════════
// isSameOrBefore
// ═════════════════════════════════════════════════════════════════════

TEST(QueryTest, IsSameOrBefore) {
    auto jan = utcDate(2024, 0, 15);
    auto feb = utcDate(2024, 1, 15);
    EXPECT_TRUE(jan.isSameOrBefore(feb));
    EXPECT_TRUE(jan.isSameOrBefore(jan.clone()));
    EXPECT_FALSE(feb.isSameOrBefore(jan));
}

TEST(QueryTest, IsSameOrBeforeUnit) {
    auto a = utcDate(2024, 0, 15);
    auto b = utcDate(2024, 0, 20);
    EXPECT_TRUE(a.isSameOrBefore(b, "month"));  // same month = same = true
    EXPECT_TRUE(a.isSameOrBefore(b, "day"));     // a before b
}

// ═════════════════════════════════════════════════════════════════════
// isSameOrAfter
// ═════════════════════════════════════════════════════════════════════

TEST(QueryTest, IsSameOrAfter) {
    auto jan = utcDate(2024, 0, 15);
    auto feb = utcDate(2024, 1, 15);
    EXPECT_TRUE(feb.isSameOrAfter(jan));
    EXPECT_TRUE(jan.isSameOrAfter(jan.clone()));
    EXPECT_FALSE(jan.isSameOrAfter(feb));
}

TEST(QueryTest, IsSameOrAfterUnit) {
    auto a = utcDate(2024, 0, 15);
    auto b = utcDate(2024, 0, 20);
    EXPECT_TRUE(b.isSameOrAfter(a, "month"));  // same month
    EXPECT_TRUE(b.isSameOrAfter(a, "day"));
}

// ═════════════════════════════════════════════════════════════════════
// isBetween
// ═════════════════════════════════════════════════════════════════════

TEST(QueryTest, IsBetweenExclusive) {
    auto jan = utcDate(2024, 0, 15);
    auto feb = utcDate(2024, 1, 15);
    auto mar = utcDate(2024, 2, 15);
    EXPECT_TRUE(feb.isBetween(jan, mar));
    EXPECT_FALSE(jan.isBetween(jan, mar));  // exclusive start
    EXPECT_FALSE(mar.isBetween(jan, mar));  // exclusive end
}

TEST(QueryTest, IsBetweenInclusive) {
    auto jan = utcDate(2024, 0, 15);
    auto feb = utcDate(2024, 1, 15);
    auto mar = utcDate(2024, 2, 15);
    EXPECT_TRUE(jan.isBetween(jan, mar, "", "[]"));
    EXPECT_TRUE(mar.isBetween(jan, mar, "", "[]"));
    EXPECT_TRUE(feb.isBetween(jan, mar, "", "[]"));
}

TEST(QueryTest, IsBetweenHalfOpenStart) {
    auto jan = utcDate(2024, 0, 15);
    auto feb = utcDate(2024, 1, 15);
    auto mar = utcDate(2024, 2, 15);
    // [) inclusive start, exclusive end
    EXPECT_TRUE(jan.isBetween(jan, mar, "", "[)"));
    EXPECT_FALSE(mar.isBetween(jan, mar, "", "[)"));
    EXPECT_TRUE(feb.isBetween(jan, mar, "", "[)"));
}

TEST(QueryTest, IsBetweenHalfOpenEnd) {
    auto jan = utcDate(2024, 0, 15);
    auto feb = utcDate(2024, 1, 15);
    auto mar = utcDate(2024, 2, 15);
    // (] exclusive start, inclusive end
    EXPECT_FALSE(jan.isBetween(jan, mar, "", "(]"));
    EXPECT_TRUE(mar.isBetween(jan, mar, "", "(]"));
    EXPECT_TRUE(feb.isBetween(jan, mar, "", "(]"));
}

TEST(QueryTest, IsBetweenWithUnit) {
    auto jan15 = utcDate(2024, 0, 15);
    auto feb15 = utcDate(2024, 1, 15);
    auto mar15 = utcDate(2024, 2, 15);
    // At month granularity
    EXPECT_TRUE(feb15.isBetween(jan15, mar15, "month"));
}

TEST(QueryTest, IsBetweenInvalid) {
    auto a = invalid();
    auto b = utcDate(2024, 0, 15);
    auto c = utcDate(2024, 2, 15);
    EXPECT_FALSE(a.isBetween(b, c));
}

// ═════════════════════════════════════════════════════════════════════
// isValid
// ═════════════════════════════════════════════════════════════════════

TEST(QueryTest, IsValid) {
    auto m = utcDate(2024, 0, 1);
    EXPECT_TRUE(m.isValid());
}

TEST(QueryTest, IsInvalid) {
    auto m = invalid();
    EXPECT_FALSE(m.isValid());
}

// ═════════════════════════════════════════════════════════════════════
// isLeapYear
// ═════════════════════════════════════════════════════════════════════

TEST(QueryTest, IsLeapYear) {
    EXPECT_TRUE(utcDate(2024, 0, 1).isLeapYear());
    EXPECT_TRUE(utcDate(2000, 0, 1).isLeapYear());
    EXPECT_FALSE(utcDate(2023, 0, 1).isLeapYear());
    EXPECT_FALSE(utcDate(1900, 0, 1).isLeapYear());
    EXPECT_TRUE(utcDate(2400, 0, 1).isLeapYear());
}

// ═════════════════════════════════════════════════════════════════════
// isUtc / isLocal / isUtcOffset
// ═════════════════════════════════════════════════════════════════════

TEST(QueryTest, IsUtc) {
    auto m = Moment(0LL);
    m.utc();
    EXPECT_TRUE(m.isUtc());
}

TEST(QueryTest, IsNotUtcByDefault) {
    auto m = Moment(0LL);
    // Default is local mode
    EXPECT_FALSE(m.isUtc());
}

TEST(QueryTest, IsLocal) {
    auto m = now();
    EXPECT_TRUE(m.isLocal());
}

TEST(QueryTest, IsNotLocalWhenUtc) {
    auto m = Moment(0LL);
    m.utc();
    EXPECT_FALSE(m.isLocal());
}

TEST(QueryTest, IsUtcOffset) {
    auto m = Moment(0LL);
    m.utcOffset(330); // +05:30
    EXPECT_TRUE(m.isUtcOffset());
}

TEST(QueryTest, IsUtcOffsetForUtcMoment) {
    auto m = Moment(0LL);
    m.utc();
    EXPECT_TRUE(m.isUtcOffset());
}

// ═════════════════════════════════════════════════════════════════════
// isDST
// ═════════════════════════════════════════════════════════════════════

TEST(QueryTest, IsDSTForUtcMoment) {
    // UTC moments should never have DST
    auto m = utcDate(2024, 6, 15);
    EXPECT_FALSE(m.isDST());
}
