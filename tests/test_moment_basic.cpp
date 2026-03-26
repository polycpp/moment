#include <gtest/gtest.h>
#include <polycpp/moment/detail/aggregator.hpp>

using namespace polycpp::moment;

TEST(MomentBasicTest, NowReturnsValidMoment) {
    auto m = now();
    EXPECT_TRUE(m.isValid());
    EXPECT_GT(m.valueOf(), 0);
}

TEST(MomentBasicTest, FromTimestamp) {
    Moment m(1000000);
    EXPECT_EQ(m.valueOf(), 1000000);
    EXPECT_TRUE(m.isValid());
}

TEST(MomentBasicTest, CloneCreatesIndependentCopy) {
    Moment m(12345678);
    auto c = m.clone();
    EXPECT_EQ(m.valueOf(), c.valueOf());
    EXPECT_EQ(m, c);
}

TEST(MomentBasicTest, ComparisonOperators) {
    Moment a(1000);
    Moment b(2000);
    Moment c(1000);

    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(b >= a);
    EXPECT_TRUE(a == c);
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a <= c);
    EXPECT_TRUE(a >= c);
}

TEST(MomentBasicTest, NowMsReturnsPositive) {
    auto ms = nowMs();
    EXPECT_GT(ms, 0);
}
