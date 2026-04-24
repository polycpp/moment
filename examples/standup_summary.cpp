// Parse a list of commit timestamps on stdin and print a per-day summary.
//
//   $ printf '2026-04-23T09:05:00Z\n2026-04-23T14:20:00Z\n2026-04-24T10:00:00Z\n' |
//       ./standup_summary
//   Fri Apr 24 2026: 1 commit  (first: 10:00, last: 10:00)
//   Thu Apr 23 2026: 2 commits (first: 09:05, last: 14:20)

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <polycpp/moment/moment.hpp>

int main() {
    namespace m = polycpp::moment;

    std::map<std::string, std::vector<m::Moment>> byDay;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        auto stamp = m::parse(line);
        if (!stamp.isValid()) continue;
        auto local = stamp.clone().local();
        byDay[local.format("YYYY-MM-DD")].push_back(local);
    }

    for (auto it = byDay.rbegin(); it != byDay.rend(); ++it) {
        auto& items = it->second;
        auto first = m::min(items);
        auto last  = m::max(items);
        std::cout << first.format("ddd MMM D YYYY") << ": "
                  << items.size() << (items.size() == 1 ? " commit  " : " commits ")
                  << "(first: " << first.format("HH:mm")
                  << ", last: "  << last.format("HH:mm") << ")\n";
    }
}
