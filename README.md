# polycpp/moment

C++20 companion port of [Moment.js](https://www.npmjs.com/package/moment) for the polycpp ecosystem.

`moment` parses, validates, manipulates, compares, and displays dates and durations with a Moment.js-style API adapted to typed C++.

## Status

Port version: `1.0.0`

Upstream repository: [moment/moment](https://github.com/moment/moment)

Upstream package: [moment](https://www.npmjs.com/package/moment)

Initial port based on upstream version: `2.30.1`

Compatibility note:

- This repo does not imply full parity with upstream `moment`.
- Implemented, adapted, and intentionally omitted behavior is tracked in `docs/research.md`, `docs/api-mapping.md`, and `docs/divergences.md`.

Implemented:

- `polycpp::moment::Moment`
- `polycpp::moment::Duration`
- parsing factories for ISO 8601, RFC 2822, explicit ISO/RFC sentinels, custom formats, UTC, fixed-offset inputs, and deterministic `polycpp::Date` fallback formats
- Moment-style formatting tokens and generated Moment.js locale data, including locale-specific parse tables
- mutable getters, setters, add/subtract, start/end, UTC/local, and offset APIs
- comparison, diff, relative-time, calendar, JSON object/array, and min/max helpers
- locale-backed era formatting and parsing, including generated Japanese era data
- Moment.js-compatible HTML5 input format constants
- typed locale registration, relative-time thresholds, and per-call duration humanize threshold overrides
- relative-time rounding configuration and calendar key selection
- parse diagnostics and interoperability helpers such as `parsingFlags`, `creationData`, `invalidAt`, `inspect`, and `toDate`
- per-moment and global locale selection
- standalone month and weekday locale listers

Deferred core parity items:

- none currently tracked

Out of scope for core `moment` parity:

- full IANA timezone database behavior; upstream implements named-zone support in the separate `moment-timezone` package, while core `moment` supports local time, UTC, and fixed offsets
- JavaScript package/runtime artifacts and exact host-engine `Date.parse` quirks

## Prerequisites

- C++20 compiler
- CMake 3.20+
- Ninja recommended

## Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_MOMENT_BUILD_TESTS=ON
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

To use a local base `polycpp` checkout:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DPOLYCPP_SOURCE_DIR=<repo path>/../polycpp \
  -DPOLYCPP_MOMENT_BUILD_TESTS=ON
```

All generated Moment.js locales are enabled by default. Configure with
`-DPOLYCPP_MOMENT_ENABLE_ALL_LOCALES=OFF` for an English/custom-locale-only build,
or select a curated set with:

```bash
cmake -B build -G Ninja -DPOLYCPP_MOMENT_LOCALES=fr,ar,ja
```

## Usage

```cpp
#include <iostream>
#include <polycpp/moment/moment.hpp>

int main() {
    auto report = polycpp::moment::parse("2024-03-15T14:30:45Z");
    report.utc();
    report.add(2, "hours");

    std::cout << report.format("YYYY-MM-DD HH:mm:ss") << '\n';
}
```

Durations:

```cpp
auto elapsed = polycpp::moment::duration(90, "minutes");
std::cout << elapsed.humanize() << '\n';
std::cout << elapsed.toISOString() << '\n';
```

## API

- `Moment` represents a mutable date/time value.
- `Duration` represents Moment-style three-bucket duration data.
- `parse`, `utcFromString`, `utcFromDate`, `parseZone`, `fromObject`, `duration`, `min`, and `max` create values.
- `Moment::locale`, `Duration::locale`, `defineLocale`, `updateLocale`, `localeData`, `globalLocale`, `locales`, and `relativeTimeThreshold` manage locale behavior.
- `normalizeUnit` maps Moment-style unit aliases to typed `Unit` values.

## License

MIT License. See [LICENSE](LICENSE) and [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).
