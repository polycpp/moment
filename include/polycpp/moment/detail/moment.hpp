/**
 * @file detail/moment.hpp
 * @brief Moment class inline implementations.
 */
#pragma once

#include <polycpp/moment/moment.hpp>
#include <chrono>

namespace polycpp {
namespace moment {

inline Moment::Moment() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    timestamp_ms_ = ms;
}

inline Moment::Moment(int64_t timestamp_ms) : timestamp_ms_(timestamp_ms) {}

inline int64_t Moment::valueOf() const { return timestamp_ms_; }

inline bool Moment::isValid() const { return is_valid_; }

inline Moment Moment::clone() const { return *this; }

inline bool Moment::operator==(const Moment& other) const { return timestamp_ms_ == other.timestamp_ms_; }
inline bool Moment::operator!=(const Moment& other) const { return timestamp_ms_ != other.timestamp_ms_; }
inline bool Moment::operator<(const Moment& other) const { return timestamp_ms_ < other.timestamp_ms_; }
inline bool Moment::operator<=(const Moment& other) const { return timestamp_ms_ <= other.timestamp_ms_; }
inline bool Moment::operator>(const Moment& other) const { return timestamp_ms_ > other.timestamp_ms_; }
inline bool Moment::operator>=(const Moment& other) const { return timestamp_ms_ >= other.timestamp_ms_; }

inline Moment now() {
    return Moment();
}

inline int64_t nowMs() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
}

} // namespace moment
} // namespace polycpp
