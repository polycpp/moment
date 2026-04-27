#include <mutex>

#ifndef POLYCPP_MOMENT_DISABLE_GENERATED_LOCALES

#include <polycpp/moment/detail/aggregator.hpp>
#include <polycpp/moment/detail/generated/locales.hpp>

namespace polycpp {
namespace moment {
namespace detail {

void ensureGeneratedMomentLocalesRegistered() {
    static std::once_flag once;
    std::call_once(once, [] {
        registerGeneratedMomentLocales();
    });
}

} // namespace detail
} // namespace moment
} // namespace polycpp

#endif
