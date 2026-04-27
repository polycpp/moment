/**
 * @file test_locale.cpp
 * @brief Tests for locale infrastructure: registry, English locale data, thresholds.
 */
#include <gtest/gtest.h>
#include <polycpp/moment/detail/aggregator.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <variant>
#include <vector>

using namespace polycpp::moment;

// ── Default locale ───────────────────────────────────────────────────

TEST(LocaleTest, DefaultLocaleIsEn) {
    EXPECT_EQ(globalLocale(), "en");
}

// ── English month names ──────────────────────────────────────────────

TEST(LocaleTest, EnglishMonthNames) {
    const auto& en = localeData("en");
    EXPECT_EQ(en.months[0],  "January");
    EXPECT_EQ(en.months[1],  "February");
    EXPECT_EQ(en.months[2],  "March");
    EXPECT_EQ(en.months[3],  "April");
    EXPECT_EQ(en.months[4],  "May");
    EXPECT_EQ(en.months[5],  "June");
    EXPECT_EQ(en.months[6],  "July");
    EXPECT_EQ(en.months[7],  "August");
    EXPECT_EQ(en.months[8],  "September");
    EXPECT_EQ(en.months[9],  "October");
    EXPECT_EQ(en.months[10], "November");
    EXPECT_EQ(en.months[11], "December");

    EXPECT_EQ(en.monthsShort[0],  "Jan");
    EXPECT_EQ(en.monthsShort[5],  "Jun");
    EXPECT_EQ(en.monthsShort[11], "Dec");
}

// ── English weekday names ────────────────────────────────────────────

TEST(LocaleTest, EnglishWeekdayNames) {
    const auto& en = localeData("en");
    EXPECT_EQ(en.weekdays[0], "Sunday");
    EXPECT_EQ(en.weekdays[1], "Monday");
    EXPECT_EQ(en.weekdays[2], "Tuesday");
    EXPECT_EQ(en.weekdays[3], "Wednesday");
    EXPECT_EQ(en.weekdays[4], "Thursday");
    EXPECT_EQ(en.weekdays[5], "Friday");
    EXPECT_EQ(en.weekdays[6], "Saturday");

    EXPECT_EQ(en.weekdaysShort[0], "Sun");
    EXPECT_EQ(en.weekdaysShort[6], "Sat");

    EXPECT_EQ(en.weekdaysMin[0], "Su");
    EXPECT_EQ(en.weekdaysMin[1], "Mo");
    EXPECT_EQ(en.weekdaysMin[6], "Sa");
}

// ── English ordinal ──────────────────────────────────────────────────

TEST(LocaleTest, EnglishOrdinal) {
    const auto& en = localeData("en");
    ASSERT_TRUE(en.ordinal);

    EXPECT_EQ(en.ordinal(1, "D"),  "1st");
    EXPECT_EQ(en.ordinal(2, "D"),  "2nd");
    EXPECT_EQ(en.ordinal(3, "D"),  "3rd");
    EXPECT_EQ(en.ordinal(4, "D"),  "4th");
    EXPECT_EQ(en.ordinal(5, "D"),  "5th");
    EXPECT_EQ(en.ordinal(10, "D"), "10th");

    // Teens are always "th"
    EXPECT_EQ(en.ordinal(11, "D"), "11th");
    EXPECT_EQ(en.ordinal(12, "D"), "12th");
    EXPECT_EQ(en.ordinal(13, "D"), "13th");

    // 21st, 22nd, 23rd
    EXPECT_EQ(en.ordinal(21, "D"), "21st");
    EXPECT_EQ(en.ordinal(22, "D"), "22nd");
    EXPECT_EQ(en.ordinal(23, "D"), "23rd");
    EXPECT_EQ(en.ordinal(24, "D"), "24th");

    // 111th, 112th, 113th (teens rule applies to hundreds)
    EXPECT_EQ(en.ordinal(111, "D"), "111th");
    EXPECT_EQ(en.ordinal(112, "D"), "112th");
    EXPECT_EQ(en.ordinal(113, "D"), "113th");

    // 101st, 102nd, 103rd
    EXPECT_EQ(en.ordinal(101, "D"), "101st");
    EXPECT_EQ(en.ordinal(102, "D"), "102nd");
    EXPECT_EQ(en.ordinal(103, "D"), "103rd");
}

// ── English meridiem ─────────────────────────────────────────────────

TEST(LocaleTest, EnglishMeridiem) {
    const auto& en = localeData("en");
    ASSERT_TRUE(en.meridiem);

    // Uppercase
    EXPECT_EQ(en.meridiem(0,  0, false), "AM");
    EXPECT_EQ(en.meridiem(11, 59, false), "AM");
    EXPECT_EQ(en.meridiem(12, 0, false), "PM");
    EXPECT_EQ(en.meridiem(23, 59, false), "PM");

    // Lowercase
    EXPECT_EQ(en.meridiem(0,  0, true), "am");
    EXPECT_EQ(en.meridiem(11, 59, true), "am");
    EXPECT_EQ(en.meridiem(12, 0, true), "pm");
    EXPECT_EQ(en.meridiem(23, 59, true), "pm");
}

// ── English isPM ─────────────────────────────────────────────────────

TEST(LocaleTest, EnglishIsPM) {
    const auto& en = localeData("en");
    ASSERT_TRUE(en.isPM);

    EXPECT_TRUE(en.isPM("PM"));
    EXPECT_TRUE(en.isPM("pm"));
    EXPECT_TRUE(en.isPM("P"));
    EXPECT_FALSE(en.isPM("AM"));
    EXPECT_FALSE(en.isPM("am"));
    EXPECT_FALSE(en.isPM(""));
}

// ── English relative time ────────────────────────────────────────────

TEST(LocaleTest, EnglishRelativeTime) {
    const auto& en = localeData("en");

    EXPECT_EQ(en.relativeTime.future, "in %s");
    EXPECT_EQ(en.relativeTime.past,   "%s ago");

    // Verify string variant values
    EXPECT_EQ(std::get<std::string>(en.relativeTime.s),  "a few seconds");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.ss), "%d seconds");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.m),  "a minute");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.mm), "%d minutes");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.h),  "an hour");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.hh), "%d hours");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.d),  "a day");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.dd), "%d days");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.w),  "a week");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.ww), "%d weeks");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.M),  "a month");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.MM), "%d months");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.y),  "a year");
    EXPECT_EQ(std::get<std::string>(en.relativeTime.yy), "%d years");
}

// ── English calendar ─────────────────────────────────────────────────

TEST(LocaleTest, EnglishCalendar) {
    const auto& en = localeData("en");

    EXPECT_EQ(std::get<std::string>(en.calendar.sameDay),  "[Today at] LT");
    EXPECT_EQ(std::get<std::string>(en.calendar.nextDay),  "[Tomorrow at] LT");
    EXPECT_EQ(std::get<std::string>(en.calendar.nextWeek), "dddd [at] LT");
    EXPECT_EQ(std::get<std::string>(en.calendar.lastDay),  "[Yesterday at] LT");
    EXPECT_EQ(std::get<std::string>(en.calendar.lastWeek), "[Last] dddd [at] LT");
    EXPECT_EQ(std::get<std::string>(en.calendar.sameElse), "L");
}

// ── English long date formats ────────────────────────────────────────

TEST(LocaleTest, EnglishLongDateFormats) {
    const auto& en = localeData("en");

    EXPECT_EQ(en.longDateFormat.LT,   "h:mm A");
    EXPECT_EQ(en.longDateFormat.LTS,  "h:mm:ss A");
    EXPECT_EQ(en.longDateFormat.L,    "MM/DD/YYYY");
    EXPECT_EQ(en.longDateFormat.LL,   "MMMM D, YYYY");
    EXPECT_EQ(en.longDateFormat.LLL,  "MMMM D, YYYY h:mm A");
    EXPECT_EQ(en.longDateFormat.LLLL, "dddd, MMMM D, YYYY h:mm A");
}

// ── English week config ──────────────────────────────────────────────

TEST(LocaleTest, EnglishWeekConfig) {
    const auto& en = localeData("en");
    EXPECT_EQ(en.week.dow, 0);
    EXPECT_EQ(en.week.doy, 6);
}

// ── Global locale get/set ────────────────────────────────────────────

TEST(LocaleTest, GlobalLocaleGetSet) {
    // Default is "en"
    EXPECT_EQ(globalLocale(), "en");

    // Setting to a non-existent locale should not change it
    std::string result = globalLocale("nonexistent");
    EXPECT_EQ(result, "en");
    EXPECT_EQ(globalLocale(), "en");
}

// ── Define locale ────────────────────────────────────────────────────

TEST(LocaleTest, DefineLocale) {
    LocaleData custom;
    custom.name = "xx";
    custom.months = {{
        "Mes1", "Mes2", "Mes3", "Mes4", "Mes5", "Mes6",
        "Mes7", "Mes8", "Mes9", "Mes10", "Mes11", "Mes12"
    }};
    custom.weekdays = {{
        "Dia1", "Dia2", "Dia3", "Dia4", "Dia5", "Dia6", "Dia7"
    }};
    custom.invalidDate = "Fecha invalida";

    defineLocale("xx", custom);

    const auto& retrieved = localeData("xx");
    EXPECT_EQ(retrieved.name, "xx");
    EXPECT_EQ(retrieved.months[0], "Mes1");
    EXPECT_EQ(retrieved.months[11], "Mes12");
    EXPECT_EQ(retrieved.weekdays[0], "Dia1");
    EXPECT_EQ(retrieved.invalidDate, "Fecha invalida");

    // Now we can switch to it
    std::string result = globalLocale("xx");
    EXPECT_EQ(result, "xx");
    EXPECT_EQ(globalLocale(), "xx");

    // Restore to "en" for other tests
    globalLocale("en");
    EXPECT_EQ(globalLocale(), "en");
}

// ── List locales ─────────────────────────────────────────────────────

TEST(LocaleTest, ListLocales) {
    auto list = locales();
    EXPECT_FALSE(list.empty());

    // "en" must be present
    EXPECT_NE(std::find(list.begin(), list.end(), "en"), list.end());

    // The list must be sorted
    EXPECT_TRUE(std::is_sorted(list.begin(), list.end()));
}

// ── Global month/weekday listers ────────────────────────────────────

TEST(LocaleTest, GlobalMonthListersUseEnglishLocale) {
    auto full = months();
    ASSERT_EQ(full.size(), 12U);
    EXPECT_EQ(full[0], "January");
    EXPECT_EQ(full[11], "December");
    EXPECT_EQ(months(0), "January");
    EXPECT_EQ(months(12), "January");
    EXPECT_EQ(months(-1), "December");

    auto shortNames = monthsShort();
    ASSERT_EQ(shortNames.size(), 12U);
    EXPECT_EQ(shortNames[0], "Jan");
    EXPECT_EQ(shortNames[11], "Dec");
    EXPECT_EQ(monthsShort(5), "Jun");

    auto formatted = months("D MMMM");
    ASSERT_EQ(formatted.size(), 12U);
    EXPECT_EQ(formatted[2], "March");
    EXPECT_EQ(months("D MMMM", 2), "March");
}

TEST(LocaleTest, GlobalWeekdayListersUseEnglishLocale) {
    auto full = weekdays();
    ASSERT_EQ(full.size(), 7U);
    EXPECT_EQ(full[0], "Sunday");
    EXPECT_EQ(full[6], "Saturday");
    EXPECT_EQ(weekdays(0), "Sunday");
    EXPECT_EQ(weekdays(7), "Sunday");
    EXPECT_EQ(weekdays(-1), "Saturday");

    auto sorted = weekdays(true);
    ASSERT_EQ(sorted.size(), 7U);
    EXPECT_EQ(sorted[0], "Sunday");
    EXPECT_EQ(weekdays(true, 0), "Sunday");

    auto shortNames = weekdaysShort();
    ASSERT_EQ(shortNames.size(), 7U);
    EXPECT_EQ(shortNames[0], "Sun");
    EXPECT_EQ(weekdaysShort(6), "Sat");

    auto minNames = weekdaysMin();
    ASSERT_EQ(minNames.size(), 7U);
    EXPECT_EQ(minNames[0], "Su");
    EXPECT_EQ(weekdaysMin("dddd", 1), "Mo");
}

// ── Locale fallback ──────────────────────────────────────────────────

TEST(LocaleTest, FallbackToEnForUnknown) {
    const auto& data = localeData("zzz_nonexistent");
    EXPECT_EQ(data.name, "en");
    EXPECT_EQ(data.months[0], "January");
}

// ── localeData with empty key uses global ────────────────────────────

TEST(LocaleTest, EmptyKeyUsesGlobal) {
    const auto& data = localeData("");
    EXPECT_EQ(data.name, globalLocale());
}

// ── Relative time threshold get/set ──────────────────────────────────

TEST(LocaleTest, RelativeTimeThresholdGetSet) {
    // Default thresholds
    EXPECT_DOUBLE_EQ(relativeTimeThreshold("ss"), 44.0);
    EXPECT_DOUBLE_EQ(relativeTimeThreshold("s"),  45.0);
    EXPECT_DOUBLE_EQ(relativeTimeThreshold("m"),  45.0);
    EXPECT_DOUBLE_EQ(relativeTimeThreshold("h"),  22.0);
    EXPECT_DOUBLE_EQ(relativeTimeThreshold("d"),  26.0);
    EXPECT_DOUBLE_EQ(relativeTimeThreshold("M"),  11.0);

    // Unknown threshold returns -1
    EXPECT_DOUBLE_EQ(relativeTimeThreshold("unknown"), -1.0);

    // Set a custom threshold
    relativeTimeThreshold("ss", 50.0);
    EXPECT_DOUBLE_EQ(relativeTimeThreshold("ss"), 50.0);

    // Moment.js keeps ss in sync when the seconds threshold changes.
    relativeTimeThreshold("s", 60.0);
    EXPECT_DOUBLE_EQ(relativeTimeThreshold("s"), 60.0);
    EXPECT_DOUBLE_EQ(relativeTimeThreshold("ss"), 59.0);

    // Restore default
    relativeTimeThreshold("s", 45.0);
    relativeTimeThreshold("ss", 44.0);
    EXPECT_DOUBLE_EQ(relativeTimeThreshold("s"), 45.0);
    EXPECT_DOUBLE_EQ(relativeTimeThreshold("ss"), 44.0);
}

TEST(LocaleTest, RelativeTimeRoundingGetSet) {
    auto previous = relativeTimeRounding();

    EXPECT_EQ(static_cast<int>(previous(1.5)), 2);
    EXPECT_FALSE(relativeTimeRounding(RelativeTimeRoundingFn{}));

    EXPECT_TRUE(relativeTimeRounding([](double value) {
        return std::floor(value);
    }));
    EXPECT_EQ(duration(DurationInput{.seconds = 90}).humanize(), "a minute");
    EXPECT_EQ(parse("2024-03-15T00:00:00Z").from(parse("2024-03-15T00:01:30Z")),
              "a minute ago");

    EXPECT_TRUE(relativeTimeRounding(previous));
    EXPECT_EQ(duration(DurationInput{.seconds = 90}).humanize(), "2 minutes");
}

// ── Pre/post format identity ─────────────────────────────────────────

TEST(LocaleTest, EnglishPrePostFormatIdentity) {
    const auto& en = localeData("en");
    ASSERT_TRUE(en.preparse);
    ASSERT_TRUE(en.postformat);

    EXPECT_EQ(en.preparse("hello"), "hello");
    EXPECT_EQ(en.postformat("world"), "world");
}

// ── Invalid date string ──────────────────────────────────────────────

TEST(LocaleTest, EnglishInvalidDate) {
    const auto& en = localeData("en");
    EXPECT_EQ(en.invalidDate, "Invalid date");
}
