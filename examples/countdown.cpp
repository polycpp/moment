// Print a human-readable countdown to a fixed event.
//
//   $ ./countdown
//   Launch is at: Saturday, September 19, 2026 9:00 AM
//   That's in 4 months

#include <iostream>
#include <polycpp/moment/moment.hpp>

int main() {
    namespace m = polycpp::moment;

    auto launch = m::utcFromDate(2026, 8, 19, 9, 0, 0);   // Sep is month 8

    std::cout << "Launch is at: " << launch.format("LLLL") << '\n'
              << "That's " << launch.fromNow() << '\n';
}
