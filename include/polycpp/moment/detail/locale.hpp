/**
 * @file detail/locale.hpp
 * @brief Locale registry implementation — thread-safe storage and lookup.
 *
 * Uses std::shared_mutex for concurrent-read / exclusive-write access.
 * The registry is a process-global singleton; locales are registered via
 * defineLocale() and looked up via localeData().
 *
 * @since 0.1.0
 */
#pragma once

#include <polycpp/moment/locale.hpp>
#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
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

    /// @brief Get the singleton instance.
    static LocaleRegistry& instance() {
        static LocaleRegistry reg;
        return reg;
    }

private:
    LocaleRegistry() = default;
};

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

inline std::string globalLocale() {
    auto& reg = detail::LocaleRegistry::instance();
    std::shared_lock lock(reg.mutex);
    return reg.globalKey;
}

inline std::string globalLocale(const std::string& key) {
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

} // namespace moment
} // namespace polycpp
