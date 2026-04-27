/**
 * @file test_generated_locales.cpp
 * @brief Tests for the generated Moment.js locale corpus.
 */
#include <gtest/gtest.h>
#include <polycpp/moment/detail/aggregator.hpp>

#include <algorithm>
#include <string>
#include <vector>

using namespace polycpp::moment;

namespace {

bool hasLocale(const std::vector<std::string>& values, const std::string& key) {
    return std::find(values.begin(), values.end(), key) != values.end();
}

struct LocaleGuard {
    std::string previous = globalLocale();
    ~LocaleGuard() {
        globalLocale(previous);
    }
};

} // namespace

TEST(GeneratedLocaleTest, RegistersUpstreamLocaleCorpus) {
    const auto values = locales();

    EXPECT_GE(values.size(), 138U);
    EXPECT_TRUE(hasLocale(values, "en"));
    EXPECT_TRUE(hasLocale(values, "fr"));
    EXPECT_TRUE(hasLocale(values, "ar"));
    EXPECT_TRUE(hasLocale(values, "ja"));
    EXPECT_TRUE(hasLocale(values, "pl"));
    EXPECT_TRUE(hasLocale(values, "ru"));
    EXPECT_TRUE(hasLocale(values, "zh-cn"));
}

TEST(GeneratedLocaleTest, ProvidesLocaleDataAndWeekRules) {
    const auto& fr = localeData("fr");
    EXPECT_EQ(fr.months[2], "mars");
    EXPECT_EQ(fr.weekdays[5], "vendredi");
    EXPECT_EQ(fr.week.dow, 1);
    EXPECT_EQ(fr.week.doy, 4);

    const auto& ar = localeData("ar");
    ASSERT_TRUE(ar.postformat);
    EXPECT_EQ(ar.postformat("2024-03-15 14:30"), "٢٠٢٤-٠٣-١٥ ١٤:٣٠");
}

TEST(GeneratedLocaleTest, GlobalListersUseGeneratedLocaleData) {
    LocaleGuard guard;
    ASSERT_EQ(globalLocale("ru"), "ru");

    EXPECT_EQ(months(2), "март");
    EXPECT_EQ(months("D MMMM", 2), "марта");
    EXPECT_EQ(monthsShort(2), "март");
    EXPECT_EQ(monthsShort("D MMM", 2), "мар.");

    auto sorted = weekdays(true);
    ASSERT_EQ(sorted.size(), 7U);
    EXPECT_EQ(sorted[0], "понедельник");
    EXPECT_EQ(sorted[6], "воскресенье");
    EXPECT_EQ(weekdays(true, 0), "понедельник");
    EXPECT_EQ(weekdays(true, 6), "воскресенье");
    EXPECT_EQ(weekdays("dddd", 3), "среда");
    EXPECT_EQ(weekdays("[В] dddd", 3), "среду");
    EXPECT_EQ(weekdays(true, "[В] dddd", 2), "среду");
    EXPECT_EQ(weekdaysShort(true, 0), "пн");
    EXPECT_EQ(weekdaysMin(true, 6), "вс");
}

TEST(GeneratedLocaleTest, FormatsMomentsWithGeneratedLocales) {
    auto fr = utcFromDate(2024, 2, 15, 14, 30, 45);
    fr.locale("fr");
    EXPECT_EQ(fr.format("LLLL"), "vendredi 15 mars 2024 14:30");

    auto ar = utcFromDate(2024, 2, 15, 14, 30, 45);
    ar.locale("ar");
    EXPECT_EQ(ar.format("YYYY-MM-DD HH:mm"), "٢٠٢٤-٠٣-١٥ ١٤:٣٠");
}

TEST(GeneratedLocaleTest, AppliesStandaloneAndFormatNameContexts) {
    LocaleGuard guard;
    ASSERT_EQ(globalLocale("ru"), "ru");

    auto m = utcFromDate(2024, 2, 15, 14, 30, 0);
    m.locale("ru");

    EXPECT_EQ(m.format("MMMM"), "март");
    EXPECT_EQ(m.format("D MMMM"), "15 марта");
    EXPECT_EQ(m.format("[В] dddd"), "В пятницу");
}

TEST(GeneratedLocaleTest, SupportsJapaneseEraFormatting) {
    auto reiwa = utcFromDate(2024, 3, 27, 9, 0, 0);
    reiwa.locale("ja");

    EXPECT_EQ(reiwa.eraName(), "令和");
    EXPECT_EQ(reiwa.eraNarrow(), "㋿");
    EXPECT_EQ(reiwa.eraAbbr(), "R");
    EXPECT_EQ(reiwa.eraYear(), 6);
    EXPECT_EQ(reiwa.format("NNNN NNNNN N y yy yyy yyyy yo"),
              "令和 ㋿ R 6 06 006 0006 6年");

    auto heisei = utcFromDate(1989, 0, 8, 0, 0, 0);
    heisei.locale("ja");
    EXPECT_EQ(heisei.format("NNNN y yo"), "平成 1 元年");
}

TEST(GeneratedLocaleTest, ParsesJapaneseEraDates) {
    const std::string previous = globalLocale();
    ASSERT_EQ(globalLocale("ja"), "ja");

    auto reiwa = parse("令和6年4月27日", "NNNNyoM月D日", true);
    ASSERT_TRUE(reiwa.isValid());
    EXPECT_EQ(reiwa.year(), 2024);
    EXPECT_EQ(reiwa.month(), 3);
    EXPECT_EQ(reiwa.date(), 27);

    auto firstYear = parse("令和元年5月1日", "NNNNyoM月D日", true);
    ASSERT_TRUE(firstYear.isValid());
    EXPECT_EQ(firstYear.year(), 2019);
    EXPECT_EQ(firstYear.month(), 4);
    EXPECT_EQ(firstYear.date(), 1);

    globalLocale(previous);
}

TEST(GeneratedLocaleTest, ParsesLocalePreparseAndMeridiem) {
    LocaleGuard guard;
    ASSERT_EQ(globalLocale("ar"), "ar");

    auto iso = parse("٢٠٢٤-٠٣-١٥T١٤:٣٠:٤٥");
    ASSERT_TRUE(iso.isValid());
    EXPECT_EQ(iso.year(), 2024);
    EXPECT_EQ(iso.month(), 2);
    EXPECT_EQ(iso.date(), 15);
    EXPECT_EQ(iso.hour(), 14);
    EXPECT_EQ(iso.minute(), 30);
    EXPECT_EQ(iso.second(), 45);

    auto custom = parse("٢٠٢٤-٠٣-١٥ ٢:٣٠ م", "YYYY-MM-DD h:mm A", true);
    ASSERT_TRUE(custom.isValid());
    EXPECT_EQ(custom.year(), 2024);
    EXPECT_EQ(custom.month(), 2);
    EXPECT_EQ(custom.date(), 15);
    EXPECT_EQ(custom.hour(), 14);
    EXPECT_EQ(custom.minute(), 30);
}

TEST(GeneratedLocaleTest, ParsesLocaleSpecificMeridiemHourRules) {
    LocaleGuard guard;
    ASSERT_EQ(globalLocale("zh-cn"), "zh-cn");

    auto afternoon = parse("2024-03-15 下午 2:30", "YYYY-MM-DD A h:mm", true);
    ASSERT_TRUE(afternoon.isValid());
    EXPECT_EQ(afternoon.hour(), 14);
    EXPECT_EQ(afternoon.minute(), 30);

    auto midnight = parse("2024-03-15 凌晨 12:30", "YYYY-MM-DD A h:mm", true);
    ASSERT_TRUE(midnight.isValid());
    EXPECT_EQ(midnight.hour(), 0);
    EXPECT_EQ(midnight.minute(), 30);

    auto noon = parse("2024-03-15 中午 12:30", "YYYY-MM-DD A h:mm", true);
    ASSERT_TRUE(noon.isValid());
    EXPECT_EQ(noon.hour(), 12);
    EXPECT_EQ(noon.minute(), 30);
}

TEST(GeneratedLocaleTest, ParsesLocaleSpecificMonthAndWeekdayNames) {
    LocaleGuard guard;
    ASSERT_EQ(globalLocale("ru"), "ru");

    auto full = parse("15 марта 2024", "D MMMM YYYY", true);
    ASSERT_TRUE(full.isValid());
    EXPECT_EQ(full.year(), 2024);
    EXPECT_EQ(full.month(), 2);
    EXPECT_EQ(full.date(), 15);

    auto shortName = parse("15 мар. 2024", "D MMM YYYY", true);
    ASSERT_TRUE(shortName.isValid());
    EXPECT_EQ(shortName.month(), 2);
    EXPECT_EQ(shortName.date(), 15);

    auto weekday = parse("пятница, 15 марта 2024", "dddd, D MMMM YYYY", true);
    ASSERT_TRUE(weekday.isValid());
    EXPECT_EQ(weekday.year(), 2024);
    EXPECT_EQ(weekday.month(), 2);
    EXPECT_EQ(weekday.date(), 15);
}

TEST(GeneratedLocaleTest, ParsesGeneratedOrdinalForms) {
    LocaleGuard guard;
    ASSERT_EQ(globalLocale("fr"), "fr");

    auto dayOrdinal = parse("1er mars 2024", "Do MMMM YYYY", true);
    ASSERT_TRUE(dayOrdinal.isValid());
    EXPECT_EQ(dayOrdinal.year(), 2024);
    EXPECT_EQ(dayOrdinal.month(), 2);
    EXPECT_EQ(dayOrdinal.date(), 1);
}

TEST(GeneratedLocaleTest, HumanizesDurationsWithGeneratedLocales) {
    EXPECT_EQ(duration(5, "minutes").locale("ru").humanize(), "5 минут");
    EXPECT_EQ(duration(5, "minutes").locale("ar").humanize(true), "بعد ٥ دقائق");
}

TEST(GeneratedLocaleTest, UsesGeneratedCalendarCallbacks) {
    auto reference = utcFromDate(2024, 2, 15, 12, 0, 0);

    auto ja = utcFromDate(2024, 2, 16, 9, 0, 0);
    ja.locale("ja");
    EXPECT_EQ(ja.calendar(reference), "明日 09:00");

    auto pl = utcFromDate(2024, 2, 13, 9, 0, 0);
    pl.locale("pl");
    EXPECT_EQ(pl.calendar(reference), "W zeszłą środę o 09:00");
}

TEST(GeneratedLocaleTest, GlobalLocaleAppliesToNewMomentsAndDurations) {
    const std::string previous = globalLocale();
    ASSERT_EQ(globalLocale("fr"), "fr");

    auto m = utcFromDate(2024, 2, 15, 14, 30, 45);
    EXPECT_EQ(m.locale(), "fr");
    EXPECT_EQ(m.format("LL"), "15 mars 2024");

    EXPECT_EQ(duration(1, "day").locale(), "fr");

    globalLocale(previous);
}
