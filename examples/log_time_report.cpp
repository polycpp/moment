// Sum a list of ISO 8601 durations on stdin, report the total humanised.
//
//   $ printf 'PT1H30M\nPT45M\nPT2H15M\n' | ./log_time_report
//   Total: 5 hours (components: 4h 30m 0s)

#include <iostream>
#include <string>
#include <polycpp/moment/duration.hpp>
#include <polycpp/moment/moment.hpp>

int main() {
    namespace m = polycpp::moment;

    m::Duration total(0);
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        m::Duration d(line);
        if (!d.isValid()) continue;
        total.add(d);
    }

    std::cout << "Total: " << total.humanize(false)
              << " (components: " << total.hours() << "h "
              << total.minutes() << "m "
              << total.seconds() << "s)\n";
}
