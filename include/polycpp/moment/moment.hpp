/**
 * @file moment.hpp
 * @brief Core Moment class — C++ port of Moment.js.
 * @see https://momentjs.com/docs/
 */
#pragma once

#include <cstdint>
#include <string>
#include <chrono>

namespace polycpp {
namespace moment {

/**
 * @brief A mutable date/time wrapper inspired by Moment.js.
 *
 * Wraps a millisecond-precision timestamp with UTC/local mode awareness.
 * Manipulation methods mutate in-place and return *this for chaining.
 * Use clone() when you need an independent copy.
 *
 * @see https://momentjs.com/docs/
 * @since 0.1.0
 */
class Moment {
public:
    /// @brief Construct a Moment representing the current time.
    Moment();

    /// @brief Construct a Moment from a millisecond timestamp (epoch).
    explicit Moment(int64_t timestamp_ms);

    /// @brief Get the millisecond timestamp (ms since Unix epoch).
    int64_t valueOf() const;

    /// @brief Check if this Moment represents a valid date/time.
    bool isValid() const;

    /// @brief Deep copy this Moment.
    Moment clone() const;

    /// @brief Equality comparison (same timestamp).
    bool operator==(const Moment& other) const;
    bool operator!=(const Moment& other) const;
    bool operator<(const Moment& other) const;
    bool operator<=(const Moment& other) const;
    bool operator>(const Moment& other) const;
    bool operator>=(const Moment& other) const;

private:
    int64_t timestamp_ms_ = 0;
    bool is_utc_ = false;
    int utc_offset_minutes_ = 0;
    bool has_fixed_offset_ = false;
    bool is_valid_ = true;
    std::string locale_key_ = "en";
};

/**
 * @brief Create a Moment representing the current time.
 * @return A valid Moment set to now.
 * @since 0.1.0
 */
Moment now();

/**
 * @brief Get the current timestamp in milliseconds since epoch.
 * @return Milliseconds since Unix epoch.
 * @since 0.1.0
 */
int64_t nowMs();

} // namespace moment
} // namespace polycpp
