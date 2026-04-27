/**
 * @file detail/json.hpp
 * @brief Shared JsonValue/JsonObject helpers for moment interop.
 *
 * @since 1.0.0
 */
#pragma once

#include <string>
#include <vector>
#include <polycpp/core/json.hpp>

namespace polycpp {
namespace moment {
namespace detail {

/// @brief Extract an integer field from a JsonObject, returning a default when absent or non-numeric.
inline int jsonGetInt(const polycpp::JsonObject& obj, const std::string& key, int def) {
    auto it = obj.find(key);
    if (it != obj.end() && it->second.isNumber()) {
        return it->second.asInt();
    }
    return def;
}

/// @brief Extract an integer field, trying a primary key before a fallback key.
inline int jsonGetInt(const polycpp::JsonObject& obj,
                      const std::string& primary_key,
                      const std::string& fallback_key,
                      int def) {
    auto it = obj.find(primary_key);
    if (it != obj.end() && it->second.isNumber()) {
        return it->second.asInt();
    }
    return jsonGetInt(obj, fallback_key, def);
}

/// @brief Convert a vector of strings to a JsonArray.
inline polycpp::JsonArray jsonArrayFromStrings(const std::vector<std::string>& values) {
    polycpp::JsonArray arr;
    arr.reserve(values.size());
    for (const auto& value : values) {
        arr.emplace_back(value);
    }
    return arr;
}

/// @brief Convert a vector of integers to a JsonArray.
inline polycpp::JsonArray jsonArrayFromInts(const std::vector<int>& values) {
    polycpp::JsonArray arr;
    arr.reserve(values.size());
    for (int value : values) {
        arr.emplace_back(value);
    }
    return arr;
}

/// @brief Convert an optional string sentinel to a JSON value.
inline polycpp::JsonValue jsonOptionalString(const std::string& value) {
    if (value.empty()) {
        return polycpp::JsonValue(nullptr);
    }
    return polycpp::JsonValue(value);
}

} // namespace detail
} // namespace moment
} // namespace polycpp
