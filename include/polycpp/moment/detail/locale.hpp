/**
 * @file detail/locale.hpp
 * @brief Locale registry implementation — thread-safe storage and lookup.
 *
 * Uses std::shared_mutex for concurrent-read / exclusive-write access.
 * The registry is a process-global singleton; locales are registered via
 * defineLocale() and looked up via localeData().
 *
 * @since 1.0.0
 */
#pragma once

#include <polycpp/moment/locale.hpp>
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace polycpp {
namespace moment {
namespace detail {

/**
 * @brief Process-global locale registry singleton.
 *
 * Holds all registered LocaleData objects, the current global locale key,
 * and relative-time thresholds. All access is guarded by a shared_mutex.
 */
struct LocaleRegistry {
    std::shared_mutex mutex;
    std::unordered_map<std::string, LocaleData> locales;
    std::string globalKey = "en";

    // Relative-time thresholds (moment.js defaults)
    // ss: seconds → "X seconds" up to this value, then switch to "a minute"
    // s:  seconds threshold before "a minute"
    // m:  minutes threshold before "an hour"
    // h:  hours threshold before "a day"
    // d:  days threshold before "a month"
    // w:  weeks threshold (not used by default in moment.js, set to -1)
    // M:  months threshold before "a year"
    std::unordered_map<std::string, double> thresholds = {
        {"ss", 44.0},
        {"s",  45.0},
        {"m",  45.0},
        {"h",  22.0},
        {"d",  26.0},
        {"w",  -1.0},  // null/disabled in moment.js
        {"M",  11.0},
    };

    RelativeTimeRoundingFn rounding = [](double value) {
        return std::round(value);
    };

    /// @brief Get the singleton instance.
    static LocaleRegistry& instance() {
        static LocaleRegistry reg;
        return reg;
    }

private:
    LocaleRegistry() = default;
};

inline void ensureGeneratedLocalesLoaded() {
#ifndef POLYCPP_MOMENT_DISABLE_GENERATED_LOCALES
    ensureGeneratedMomentLocalesRegistered();
#endif
}

inline int positiveModulo(int value, int mod) {
    int result = value % mod;
    return result < 0 ? result + mod : result;
}

template <std::size_t N>
inline bool hasLocaleNames(const std::array<std::string, N>& values) {
    return std::any_of(values.begin(), values.end(), [](const std::string& value) {
        return !value.empty();
    });
}

template <std::size_t N>
inline std::vector<std::string> copyLocaleNames(const std::array<std::string, N>& values) {
    return std::vector<std::string>(values.begin(), values.end());
}

template <std::size_t N>
inline std::vector<std::string> copyLocaleNamesShifted(const std::array<std::string, N>& values,
                                                       int shift) {
    std::vector<std::string> out;
    out.reserve(N);
    for (std::size_t i = 0; i < N; ++i) {
        out.push_back(values[positiveModulo(static_cast<int>(i) + shift, static_cast<int>(N))]);
    }
    return out;
}

inline bool monthFormatUsesFormattingCase(const std::string& format) {
    for (std::size_t i = 0; i < format.size(); ++i) {
        if (format[i] != 'D') {
            continue;
        }

        std::size_t pos = i + 1;
        if (pos < format.size() && (format[pos] == 'D' || format[pos] == 'o')) {
            ++pos;
        }

        bool sawSeparator = false;
        while (pos < format.size()) {
            if (std::isspace(static_cast<unsigned char>(format[pos]))) {
                sawSeparator = true;
                do {
                    ++pos;
                } while (pos < format.size() &&
                         std::isspace(static_cast<unsigned char>(format[pos])));
                continue;
            }

            if (format[pos] == '[') {
                const std::size_t close = format.find(']', pos + 1);
                if (close == std::string::npos) {
                    break;
                }
                sawSeparator = true;
                pos = close + 1;
                continue;
            }

            break;
        }

        if (sawSeparator &&
            (format.compare(pos, 4, "MMMM") == 0 ||
             format.compare(pos, 3, "MMM") == 0)) {
            return true;
        }
    }

    return false;
}

inline bool containsAny(const std::string& value, std::initializer_list<const char*> needles) {
    return std::any_of(needles.begin(), needles.end(), [&](const char* needle) {
        return value.find(needle) != std::string::npos;
    });
}

inline bool weekdayFormatUsesFormattingCase(const LocaleData& locale, const std::string& format) {
    const std::string& source = locale.weekdaysFormatRegexSource;
    if (source.empty()) {
        return false;
    }

    if (source == "dddd HH:mm") {
        return format.find("dddd HH:mm") != std::string::npos;
    }

    if (source == "(წინა|შემდეგ)") {
        return containsAny(format, {"წინა", "შემდეგ"});
    }

    if (source.find("[Вв]") != std::string::npos) {
        return format.find("dddd") != std::string::npos &&
               containsAny(format, {"[В", "[в"});
    }

    if (source.find("[Ууў]") != std::string::npos) {
        return format.find("dddd") != std::string::npos &&
               containsAny(format, {"[У", "[у", "[ў"});
    }

    return false;
}

inline const std::array<std::string, 12>& selectMonthNames(const LocaleData& locale,
                                                           const std::string& format,
                                                           bool shortNames) {
    const bool formattingCase = monthFormatUsesFormattingCase(format);
    if (shortNames) {
        if (!formattingCase && hasLocaleNames(locale.monthsShortStandalone)) {
            return locale.monthsShortStandalone;
        }
        return locale.monthsShort;
    }

    if (!formattingCase && hasLocaleNames(locale.monthsStandalone)) {
        return locale.monthsStandalone;
    }
    return locale.months;
}

inline const std::array<std::string, 7>& selectWeekdayNames(const LocaleData& locale,
                                                            const std::string& format,
                                                            int width) {
    if (width == 0) {
        if (weekdayFormatUsesFormattingCase(locale, format) &&
            hasLocaleNames(locale.weekdaysFormat)) {
            return locale.weekdaysFormat;
        }
        if (hasLocaleNames(locale.weekdaysStandalone)) {
            return locale.weekdaysStandalone;
        }
        return locale.weekdays;
    }
    if (width == 1) {
        return locale.weekdaysShort;
    }
    if (width == 2) {
        return locale.weekdaysMin;
    }
    return locale.weekdays;
}

inline std::vector<std::string> listMonthNames(const std::string& format, bool shortNames) {
    const auto& names = selectMonthNames(localeData(), format, shortNames);
    return copyLocaleNames(names);
}

inline std::string getMonthName(const std::string& format, int index, bool shortNames) {
    const auto& names = selectMonthNames(localeData(), format, shortNames);
    return names[positiveModulo(index, 12)];
}

inline std::vector<std::string> listWeekdayNames(bool localeSorted,
                                                 const std::string& format,
                                                 int width) {
    const auto& locale = localeData();
    const auto& names = selectWeekdayNames(locale, format, width);
    const int shift = localeSorted ? locale.week.dow : 0;
    return copyLocaleNamesShifted(names, shift);
}

inline std::string getWeekdayName(bool localeSorted,
                                  const std::string& format,
                                  int index,
                                  int width) {
    const auto& locale = localeData();
    const auto& names = selectWeekdayNames(locale, format, width);
    const int shift = localeSorted ? locale.week.dow : 0;
    return names[positiveModulo(index + shift, 7)];
}

} // namespace detail

// ── Free function implementations ──────────────────────────────────────

inline void defineLocale(const std::string& name, const LocaleData& data) {
    auto& reg = detail::LocaleRegistry::instance();
    std::unique_lock lock(reg.mutex);
    LocaleData copy = data;
    copy.name = name;
    reg.locales[name] = std::move(copy);
}

inline void updateLocale(const std::string& name, const LocaleData& data) {
    // updateLocale behaves identically to defineLocale in this implementation.
    // In the future, field-level merging could be added.
    defineLocale(name, data);
}

inline const LocaleData& localeData(const std::string& key) {
    detail::ensureGeneratedLocalesLoaded();

    auto& reg = detail::LocaleRegistry::instance();
    std::shared_lock lock(reg.mutex);

    // Empty key → use the current global locale
    const std::string& lookupKey = key.empty() ? reg.globalKey : key;

    auto it = reg.locales.find(lookupKey);
    if (it != reg.locales.end()) {
        return it->second;
    }

    // Fallback to "en" if the requested locale is not registered
    it = reg.locales.find("en");
    if (it != reg.locales.end()) {
        return it->second;
    }

    // Should never happen if locale_en.hpp is included, but guard against it.
    // Return a reference to a static empty locale as last resort.
    static const LocaleData empty{};
    return empty;
}

inline std::vector<std::string> locales() {
    detail::ensureGeneratedLocalesLoaded();

    auto& reg = detail::LocaleRegistry::instance();
    std::shared_lock lock(reg.mutex);

    std::vector<std::string> keys;
    keys.reserve(reg.locales.size());
    for (const auto& [k, _] : reg.locales) {
        keys.push_back(k);
    }
    std::sort(keys.begin(), keys.end());
    return keys;
}

inline std::vector<std::string> months() {
    return detail::listMonthNames("", false);
}

inline std::string months(int index) {
    return detail::getMonthName("", index, false);
}

inline std::vector<std::string> months(const std::string& format) {
    return detail::listMonthNames(format, false);
}

inline std::vector<std::string> months(const char* format) {
    return months(format ? std::string(format) : std::string());
}

inline std::string months(const std::string& format, int index) {
    return detail::getMonthName(format, index, false);
}

inline std::string months(const char* format, int index) {
    return months(format ? std::string(format) : std::string(), index);
}

inline std::vector<std::string> monthsShort() {
    return detail::listMonthNames("", true);
}

inline std::string monthsShort(int index) {
    return detail::getMonthName("", index, true);
}

inline std::vector<std::string> monthsShort(const std::string& format) {
    return detail::listMonthNames(format, true);
}

inline std::vector<std::string> monthsShort(const char* format) {
    return monthsShort(format ? std::string(format) : std::string());
}

inline std::string monthsShort(const std::string& format, int index) {
    return detail::getMonthName(format, index, true);
}

inline std::string monthsShort(const char* format, int index) {
    return monthsShort(format ? std::string(format) : std::string(), index);
}

inline std::vector<std::string> weekdays() {
    return detail::listWeekdayNames(false, "", 0);
}

inline std::string weekdays(int index) {
    return detail::getWeekdayName(false, "", index, 0);
}

inline std::vector<std::string> weekdays(const std::string& format) {
    return detail::listWeekdayNames(false, format, 0);
}

inline std::vector<std::string> weekdays(const char* format) {
    return weekdays(format ? std::string(format) : std::string());
}

inline std::string weekdays(const std::string& format, int index) {
    return detail::getWeekdayName(false, format, index, 0);
}

inline std::string weekdays(const char* format, int index) {
    return weekdays(format ? std::string(format) : std::string(), index);
}

inline std::vector<std::string> weekdays(bool localeSorted) {
    return detail::listWeekdayNames(localeSorted, "", 0);
}

inline std::string weekdays(bool localeSorted, int index) {
    return detail::getWeekdayName(localeSorted, "", index, 0);
}

inline std::vector<std::string> weekdays(bool localeSorted, const std::string& format) {
    return detail::listWeekdayNames(localeSorted, format, 0);
}

inline std::vector<std::string> weekdays(bool localeSorted, const char* format) {
    return weekdays(localeSorted, format ? std::string(format) : std::string());
}

inline std::string weekdays(bool localeSorted, const std::string& format, int index) {
    return detail::getWeekdayName(localeSorted, format, index, 0);
}

inline std::string weekdays(bool localeSorted, const char* format, int index) {
    return weekdays(localeSorted, format ? std::string(format) : std::string(), index);
}

inline std::vector<std::string> weekdaysShort() {
    return detail::listWeekdayNames(false, "", 1);
}

inline std::string weekdaysShort(int index) {
    return detail::getWeekdayName(false, "", index, 1);
}

inline std::vector<std::string> weekdaysShort(const std::string& format) {
    return detail::listWeekdayNames(false, format, 1);
}

inline std::vector<std::string> weekdaysShort(const char* format) {
    return weekdaysShort(format ? std::string(format) : std::string());
}

inline std::string weekdaysShort(const std::string& format, int index) {
    return detail::getWeekdayName(false, format, index, 1);
}

inline std::string weekdaysShort(const char* format, int index) {
    return weekdaysShort(format ? std::string(format) : std::string(), index);
}

inline std::vector<std::string> weekdaysShort(bool localeSorted) {
    return detail::listWeekdayNames(localeSorted, "", 1);
}

inline std::string weekdaysShort(bool localeSorted, int index) {
    return detail::getWeekdayName(localeSorted, "", index, 1);
}

inline std::vector<std::string> weekdaysShort(bool localeSorted, const std::string& format) {
    return detail::listWeekdayNames(localeSorted, format, 1);
}

inline std::vector<std::string> weekdaysShort(bool localeSorted, const char* format) {
    return weekdaysShort(localeSorted, format ? std::string(format) : std::string());
}

inline std::string weekdaysShort(bool localeSorted, const std::string& format, int index) {
    return detail::getWeekdayName(localeSorted, format, index, 1);
}

inline std::string weekdaysShort(bool localeSorted, const char* format, int index) {
    return weekdaysShort(localeSorted, format ? std::string(format) : std::string(), index);
}

inline std::vector<std::string> weekdaysMin() {
    return detail::listWeekdayNames(false, "", 2);
}

inline std::string weekdaysMin(int index) {
    return detail::getWeekdayName(false, "", index, 2);
}

inline std::vector<std::string> weekdaysMin(const std::string& format) {
    return detail::listWeekdayNames(false, format, 2);
}

inline std::vector<std::string> weekdaysMin(const char* format) {
    return weekdaysMin(format ? std::string(format) : std::string());
}

inline std::string weekdaysMin(const std::string& format, int index) {
    return detail::getWeekdayName(false, format, index, 2);
}

inline std::string weekdaysMin(const char* format, int index) {
    return weekdaysMin(format ? std::string(format) : std::string(), index);
}

inline std::vector<std::string> weekdaysMin(bool localeSorted) {
    return detail::listWeekdayNames(localeSorted, "", 2);
}

inline std::string weekdaysMin(bool localeSorted, int index) {
    return detail::getWeekdayName(localeSorted, "", index, 2);
}

inline std::vector<std::string> weekdaysMin(bool localeSorted, const std::string& format) {
    return detail::listWeekdayNames(localeSorted, format, 2);
}

inline std::vector<std::string> weekdaysMin(bool localeSorted, const char* format) {
    return weekdaysMin(localeSorted, format ? std::string(format) : std::string());
}

inline std::string weekdaysMin(bool localeSorted, const std::string& format, int index) {
    return detail::getWeekdayName(localeSorted, format, index, 2);
}

inline std::string weekdaysMin(bool localeSorted, const char* format, int index) {
    return weekdaysMin(localeSorted, format ? std::string(format) : std::string(), index);
}

inline std::string globalLocale() {
    detail::ensureGeneratedLocalesLoaded();

    auto& reg = detail::LocaleRegistry::instance();
    std::shared_lock lock(reg.mutex);
    return reg.globalKey;
}

inline std::string globalLocale(const std::string& key) {
    detail::ensureGeneratedLocalesLoaded();

    auto& reg = detail::LocaleRegistry::instance();
    std::unique_lock lock(reg.mutex);

    // Only switch if the locale is registered
    if (reg.locales.count(key)) {
        reg.globalKey = key;
    }
    return reg.globalKey;
}

inline void relativeTimeThreshold(const std::string& unit, double limit) {
    auto& reg = detail::LocaleRegistry::instance();
    std::unique_lock lock(reg.mutex);
    reg.thresholds[unit] = limit;
    if (unit == "s") {
        reg.thresholds["ss"] = limit - 1.0;
    }
}

inline double relativeTimeThreshold(const std::string& unit) {
    auto& reg = detail::LocaleRegistry::instance();
    std::shared_lock lock(reg.mutex);
    auto it = reg.thresholds.find(unit);
    if (it != reg.thresholds.end()) {
        return it->second;
    }
    return -1.0;
}

inline bool relativeTimeRounding(RelativeTimeRoundingFn roundingFunction) {
    if (!roundingFunction) {
        return false;
    }

    auto& reg = detail::LocaleRegistry::instance();
    std::unique_lock lock(reg.mutex);
    reg.rounding = std::move(roundingFunction);
    return true;
}

inline RelativeTimeRoundingFn relativeTimeRounding() {
    auto& reg = detail::LocaleRegistry::instance();
    std::shared_lock lock(reg.mutex);
    return reg.rounding;
}

} // namespace moment
} // namespace polycpp
