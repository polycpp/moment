/**
 * @file detail/locale_en.hpp
 * @brief English locale data — auto-registered at static-init time.
 *
 * Translates the Moment.js English locale (src/lib/locale/en.js) and the
 * base-config defaults into C++ LocaleData. Included by the aggregator
 * header so that the English locale is always available.
 *
 * @since 1.0.0
 */
#pragma once

#include <polycpp/moment/locale.hpp>
#include <polycpp/moment/detail/locale.hpp>
#include <polycpp/core/date.hpp>
#include <cctype>
#include <cstdint>
#include <limits>
#include <string>

namespace polycpp {
namespace moment {
namespace detail {

inline int64_t englishEraUtcDateMs(int year, int month, int day) {
    double ms = polycpp::Date::UTC(year, month, day);
    if (year >= 0 && year <= 99) {
        polycpp::Date d(ms);
        d.setUTCFullYear(year);
        ms = d.getTime();
    }
    return static_cast<int64_t>(ms);
}

/**
 * @brief Build the complete English LocaleData.
 *
 * This is a standalone function so the data can be inspected in tests
 * independently of the static-init registration path.
 *
 * @return A fully populated LocaleData for the "en" locale.
 */
inline LocaleData buildEnglishLocale() {
    LocaleData en;
    en.name = "en";

    // ── Month names ──────────────────────────────────────────────────
    en.months = {{
        "January", "February", "March", "April",
        "May", "June", "July", "August",
        "September", "October", "November", "December"
    }};
    en.monthsShort = {{
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    }};
    en.monthsStandalone = en.months;
    en.monthsShortStandalone = en.monthsShort;

    // ── Weekday names (0 = Sunday) ───────────────────────────────────
    en.weekdays = {{
        "Sunday", "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
    }};
    en.weekdaysShort = {{
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    }};
    en.weekdaysMin = {{
        "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"
    }};
    en.weekdaysStandalone = en.weekdays;
    en.weekdaysFormat = en.weekdays;

    // ── Long date formats ────────────────────────────────────────────
    en.longDateFormat.LT   = "h:mm A";
    en.longDateFormat.LTS  = "h:mm:ss A";
    en.longDateFormat.L    = "MM/DD/YYYY";
    en.longDateFormat.LL   = "MMMM D, YYYY";
    en.longDateFormat.LLL  = "MMMM D, YYYY h:mm A";
    en.longDateFormat.LLLL = "dddd, MMMM D, YYYY h:mm A";

    // ── Calendar formats ─────────────────────────────────────────────
    en.calendar.sameDay  = std::string("[Today at] LT");
    en.calendar.nextDay  = std::string("[Tomorrow at] LT");
    en.calendar.nextWeek = std::string("dddd [at] LT");
    en.calendar.lastDay  = std::string("[Yesterday at] LT");
    en.calendar.lastWeek = std::string("[Last] dddd [at] LT");
    en.calendar.sameElse = std::string("L");

    // ── Relative time ────────────────────────────────────────────────
    en.relativeTime.future = "in %s";
    en.relativeTime.past   = "%s ago";
    en.relativeTime.s  = std::string("a few seconds");
    en.relativeTime.ss = std::string("%d seconds");
    en.relativeTime.m  = std::string("a minute");
    en.relativeTime.mm = std::string("%d minutes");
    en.relativeTime.h  = std::string("an hour");
    en.relativeTime.hh = std::string("%d hours");
    en.relativeTime.d  = std::string("a day");
    en.relativeTime.dd = std::string("%d days");
    en.relativeTime.w  = std::string("a week");
    en.relativeTime.ww = std::string("%d weeks");
    en.relativeTime.M  = std::string("a month");
    en.relativeTime.MM = std::string("%d months");
    en.relativeTime.y  = std::string("a year");
    en.relativeTime.yy = std::string("%d years");

    // ── Week config (US defaults) ────────────────────────────────────
    en.week.dow = 0; // Sunday is the first day of the week
    en.week.doy = 6; // The week containing Jan 6 is the first week of the year

    // ── Ordinal ──────────────────────────────────────────────────────
    // Mirrors en.js: 1st, 2nd, 3rd, 4th, 11th, 12th, 13th, 21st, ...
    en.ordinal = [](int number, const std::string& /*token*/) -> std::string {
        int b = number % 10;
        // The "teens" override: 11th, 12th, 13th (not 11st, 12nd, 13rd)
        int tens = (number % 100) / 10;
        const char* suffix =
            (tens == 1) ? "th"
            : (b == 1)  ? "st"
            : (b == 2)  ? "nd"
            : (b == 3)  ? "rd"
                         : "th";
        return std::to_string(number) + suffix;
    };

    // ── Meridiem ─────────────────────────────────────────────────────
    // hours > 11 → PM/pm, else AM/am
    en.meridiem = [](int hours, int /*minutes*/, bool isLower) -> std::string {
        if (hours > 11) {
            return isLower ? "pm" : "PM";
        }
        return isLower ? "am" : "AM";
    };

    // ── isPM ─────────────────────────────────────────────────────────
    // First character (lowered) == 'p'
    en.isPM = [](const std::string& input) -> bool {
        if (input.empty()) return false;
        return std::tolower(static_cast<unsigned char>(input[0])) == 'p';
    };

    // ── Pre/post format (identity) ───────────────────────────────────
    en.preparse   = [](const std::string& s) -> std::string { return s; };
    en.postformat = [](const std::string& s) -> std::string { return s; };

    // ── Eras ────────────────────────────────────────────────────────
    en.eras = {
        EraSpec{
            englishEraUtcDateMs(1, 0, 1),
            std::numeric_limits<int64_t>::max(),
            1,
            "Anno Domini",
            "AD",
            "AD"
        },
        EraSpec{
            englishEraUtcDateMs(0, 11, 31),
            std::numeric_limits<int64_t>::min(),
            1,
            "Before Christ",
            "BC",
            "BC"
        },
    };

    // ── Invalid date string ──────────────────────────────────────────
    en.invalidDate = "Invalid date";

    return en;
}

/**
 * @brief Static initializer that registers the English locale at program start.
 *
 * Uses the "construct on first use" idiom via an inline variable so that
 * the locale is available before main() regardless of translation-unit
 * inclusion order.
 */
inline const bool kEnglishLocaleRegistered = [] {
    defineLocale("en", buildEnglishLocale());
    return true;
}();

} // namespace detail
} // namespace moment
} // namespace polycpp
