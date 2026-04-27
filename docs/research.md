# Research

- package: moment
- npm url: https://www.npmjs.com/package/moment
- source url: https://github.com/moment/moment.git
- upstream version basis: 2.30.1
- upstream revision analyzed: 18aba135ab927ffe7f868ee09276979bed6993a6
- upstream default branch: develop
- license: MIT
- license evidence: package.json license field and upstream LICENSE file
- category: date and time parsing, formatting, manipulation, durations, and locale data

## Package purpose

`moment` parses, validates, manipulates, compares, and displays date/time values. It also provides duration math, relative-time display, locale data, and formatting/parsing tokens that are widely used by existing JavaScript applications.

## Runtime assumptions

- browser: upstream ships browser-ready UMD artifacts; this C++ port has no browser runtime surface
- node.js: upstream runtime is usable in Node but does not depend on Node built-ins; this C++ port has no Node runtime dependency
- filesystem: no runtime filesystem access
- network: no runtime network access
- crypto: no runtime crypto access
- terminal: upstream deprecation paths can call `console.warn`; this C++ port does not expose terminal or console behavior

## Dependency summary

- package.json present: yes
- package main: `./moment.js`
- package types: `./moment.d.ts`
- package exports: none declared
- package bin: none declared
- hard dependencies: none detected in package.json
- peer dependencies: none detected in package.json
- optional dependencies: none detected in package.json
- published artifact analyzed: `<repo path>/.tmp/npm-package`
- dependency analysis report: `docs/dependency-analysis.md`

## Upstream repo layout summary

Clone path used for analysis: `<repo path>/.tmp/upstream/moment`

Top files and directories inspected:

- `package.json`
- `LICENSE`
- `README.md`
- `CHANGELOG.md`
- `moment.js`
- `moment.d.ts`
- `ts3.1-typings/moment.d.ts`
- `src/moment.js`
- `src/lib/`
- `src/locale/`
- `src/test/`
- `locale/`
- `dist/`
- `min/`
- `benchmarks/`

The published npm artifact was also inspected because consumers load `moment.js`, `moment.d.ts`, generated locale files, and minified bundles from the package tarball.

## Entry points used by consumers

- `./moment.js`
- `./moment.d.ts`
- `ts3.1-typings/moment.d.ts` through package `typesVersions`
- `locale/*.js`, `dist/locale/*.js`, and `min/moment-with-locales*.js` for full JavaScript locale distribution
- TypeScript declarations inspected: yes, `moment.d.ts` and the package `typesVersions` target

## Important files and why they matter

- `moment.js`: shipped runtime entry point analyzed by libgen; includes the public prototype wiring and bundled implementation.
- `moment.d.ts`: most compact source for public overloads, option shapes, callback-like locale hooks, and deprecated surfaces.
- `src/moment.js`: source entry point that wires creation, duration, locale, and utility exports.
- `src/lib/create/*.js`: parsing and factory behavior for strings, arrays, objects, UTC/local mode, strict parsing, and invalid moments.
- `src/lib/parse/*.js`: token and regex registration used by custom format parsing.
- `src/lib/format/format.js`: format-token expansion and escaped literal handling.
- `src/lib/moment/*.js`: instance methods for getters/setters, comparison, display, diff, start/end, cloning, and UTC/offset switching.
- `src/lib/duration/*.js`: duration bucket model, bubbling, total conversion, ISO serialization, and humanize behavior.
- `src/lib/units/*.js`: unit aliases, priorities, week/year algorithms, and offset parsing.
- `src/lib/locale/*.js`: locale registry, English base data, calendar/relative-time formats, preparse/postformat hooks, and week configuration.
- `src/locale/*.js`: upstream locale corpus used as generated data input for locale formatting, calendar, relative-time, ordinal, meridiem, era, postformat, and week-rule behavior.
- `src/test/moment/*.js`: compatibility cases for the implemented core behavior.
- `src/test/locale/en.js`, selected locale tests, and `src/test/helpers/*.js`: locale expectations and QUnit helpers.

## Files likely irrelevant to the C++ port

- `dist/` and `min/`: generated JavaScript bundles; useful for artifact verification, not direct port input.
- `locale/*.js` and `src/locale/*.js`: data input for the generated C++ locale corpus; JavaScript module wrappers and runtime loader behavior are irrelevant.
- `benchmarks/`: performance harnesses, useful only for later optimization work.
- `Gruntfile.js`, `tasks/`, build configs, package-manager metadata, and dev dependencies: upstream build/test tooling only.
- `ender.js`, `package.js`, `bower.json`, `component.json`, and other legacy packaging files: JavaScript package ecosystem integration not meaningful in the C++ port.

## Test directories worth mining first

- `src/test/moment/create.js`: creation overloads, parsing, invalid inputs, and native Date fallback expectations.
- `src/test/moment/format.js`: formatting tokens, literals, invalid date formatting, and ISO output.
- `src/test/moment/getters_setters.js`: mutable unit getters and setters.
- `src/test/moment/add_subtract.js`: date math and month/year overflow behavior.
- `src/test/moment/start_end_of.js`: start/end unit normalization.
- `src/test/moment/diff.js`: unit conversion and truncation behavior.
- `src/test/moment/duration.js` and `src/test/moment/duration_invalid.js`: duration construction, bubbling, humanize, and invalid duration handling.
- `src/test/moment/utc.js`, `utc_offset.js`, `zones.js`, and `zone_switching.js`: UTC/local/fixed-offset behavior.
- `src/test/moment/is_before.js`, `is_after.js`, `is_same.js`, `is_between.js`, `is_same_or_before.js`, and `is_same_or_after.js`: comparison semantics.
- `src/test/moment/week_year.js`, `weeks.js`, `weeks_in_year.js`, `weekday.js`, and `quarter.js`: calendar-unit edge cases.
- `src/test/locale/en.js`, selected locale files, and `src/test/moment/relative_time.js`: locale and relative-time behavior.

## Implementation risks discovered from the source layout

- Upstream has a very large dynamic overload surface; v0 needs explicit C++ factories and typed objects instead of trying to mirror all JavaScript duck typing.
- Moment objects are intentionally mutable and chainable; C++ methods must preserve mutation semantics while making invalid states explicit.
- JavaScript `Date` local-time and DST behavior is platform-sensitive; the port depends on `polycpp::Date` for these semantics.
- Upstream parsing includes forgiving behavior and a deprecated fallback to native `Date`; the C++ port uses `polycpp::Date::parse` for deterministic `Date` round-trip formats and returns invalid moments for host-specific inputs.
- Full locale distribution is large and data-heavy; generated data is compiled once into the library instead of being included into every consumer translation unit.
- Analyzer framework-object hits are internal result-object mutations (`res.months`, `res.milliseconds`, `res._nextDay`), not HTTP or framework boundaries.
- TypeScript declarations expose deprecated and plugin-oriented surfaces such as `lang`, `zone`, and `isDSTShifted`; deprecated aliases are intentionally omitted, while non-deprecated global listers are mapped to typed C++ helpers.

## Companion repo alignment

- companion repos inspected: `content-type`, `picomatch`, and `range-parser`
- CMake target and alias pattern: `polycpp_moment` and `polycpp::moment`, matching companion naming conventions
- public header layout: top-level public headers live under `include/polycpp/moment/` with separate headers for `moment`, `duration`, `format`, `locale`, `parse`, and `units`
- detail/private header strategy: inline implementation and helper internals live under `include/polycpp/moment/detail/`
- aggregator header strategy: `include/polycpp/moment/detail/aggregator.hpp` wires the implementation, and `include/polycpp/moment/moment.hpp` remains the main public entry
- examples strategy: keep focused examples under `examples/` and build them with `POLYCPP_MOMENT_BUILD_EXAMPLES`
- README structure aligned with companion repos: status, build, usage, API summary, and license sections
- documentation site strategy: use Sphinx plus Doxygen through `python3 docs/build.py`
- deliberate deviations from existing companions: this port has a broader multi-header API and more documentation pages because Moment's surface is much larger than the smaller string/parser companions

## Polycpp ecosystem reuse analysis

- polycpp core paths inspected: `<repo path>/../polycpp/include/polycpp/core/date.hpp`, `<repo path>/../polycpp/include/polycpp/core/math.hpp`, `<repo path>/../polycpp/include/polycpp/core/number.hpp`, `<repo path>/../polycpp/include/polycpp/core/json.hpp`, and `<repo path>/../polycpp/include/polycpp/core/error.hpp`
- polycpp core types/functions selected: `polycpp::Date`, `polycpp::Math`, `polycpp::Number`, `polycpp::JsonValue`, `polycpp::JsonObject`, `polycpp::JsonArray`, and `polycpp::RangeError`
- polycpp core types/functions rejected: `polycpp::Temporal` is not used for v0 because upstream Moment semantics are mutable, `Date`-based, and duration-bucket-based rather than Temporal-style immutable value types
- companion libs inspected for reusable APIs: `content-type`, `range-parser`, `picomatch`, and nearby HTTP/string companions
- companion libs selected for reuse: none; Moment is a standalone date/time library with no runtime companion dependency
- companion libs rejected or deferred: HTTP, URL, buffer, stream, and parser companions are unrelated to the v0 surface
- new local abstractions introduced: `Moment`, `Duration`, `DurationInput`, `LocaleData`, locale callback types, unit normalization, parse components, and format-token helpers
- reuse risks or integration gaps: correctness depends on `polycpp::Date` local-time/DST behavior and on manually translated Moment algorithms for week/year and duration bubbling

## Node parity surface audit

- callback APIs: no Node-style async callbacks; analyzer callback signals map to locale callback-like hooks that are represented as typed `std::function` fields
- Promise APIs: none
- EventEmitter APIs: none
- server/listener APIs: none
- stream APIs: none
- Buffer and binary APIs: none
- URL, timer, process, and filesystem APIs: no URL, process, or filesystem APIs; current-time behavior maps to `std::chrono` and `polycpp::Date`
- crypto, compression, TLS, network, and HTTP APIs: none
- unsupported Node-specific APIs and audit reason: package loading, UMD/CJS/AMD wrappers, deprecation console hooks, and JavaScript global mutation are packaging/runtime concerns rather than C++ library APIs

## External SDK and native driver strategy

- upstream external services/protocols: none
- native SDKs/client libraries to use: none
- SDKs/protocols explicitly not reimplemented: none
- adapter/linking strategy: link only against base `polycpp`
- test environment needs: C++20 compiler, CMake, Ninja, GoogleTest, Doxygen, and Sphinx dependencies for documentation

## Compatibility foundation review

- downstream dependency role: foundational date/time companion that application and future companion libraries may use for Moment-like formatting, parsing, and duration behavior
- native substitution risk: medium; substituting C++ date/time primitives can silently diverge from Moment's mutable `Date`-based overflow, DST, invalid, and duration-bucket semantics
- upstream implementation data to preserve: unit aliases, parse/format token behavior, locale strings, relative-time thresholds, week-year algorithms, duration bubbling constants, ISO/RFC parse behavior, and invalid-date behavior
- generated or vendored data plan: `tools/generate-locales.mjs` generates `include/polycpp/moment/detail/generated/locales.hpp` and `cmake/PolycppMomentLocales.cmake` from the Moment.js 2.30.1 npm locale files; registration is compiled through `src/generated_locales.cpp`, selected locales are controlled by `POLYCPP_MOMENT_LOCALES`, English locale and AD/BC era data remain available through `include/polycpp/moment/detail/locale_en.hpp`, and the upstream MIT notice is preserved in `THIRD_PARTY_LICENSES.md`
- compatibility fixture strategy: adapt representative upstream QUnit cases into GoogleTest files under `tests/`, prioritizing implemented behavior and documenting JavaScript-runtime or parser-specific omissions

## Security and fail-closed review

- security-sensitive behavior: not a security package, but it parses user-controlled date strings, format strings, locale keys, and JSON-like component objects
- trust boundary: callers may pass arbitrary strings and numeric component values
- supported protocol or algorithm matrix: ISO 8601 parsing, RFC 2822 parsing, selected custom format-token parsing, UTC/local/fixed-offset calculations, Moment duration bubbling, generated locale formatting/relative-time/calendar/era behavior, and JSON component conversion
- unsupported behavior and fail-closed policy: unsupported parse inputs return invalid `Moment` values; invalid `toISOString()` throws `polycpp::RangeError`; unsupported runtime-only APIs are omitted rather than approximated
- result-set/framing drain policy, if protocol client: not applicable because this is not a protocol client
- binary payload type-mapping policy, if protocol client: not applicable because this is not a protocol client
- stateful parser/session-state policy, if protocol client/server: not applicable because this is not a protocol client or server
- server/listener response writer matrix, if protocol server surface exists: not applicable because no server surface exists
- key, secret, credential, or user-controlled input handling: no secrets or credentials; user-controlled parse/format strings must not mutate global locale state unless the caller explicitly invokes locale registry APIs
- misuse cases that must be tested: invalid strings, empty strings, strict parsing failures, invalid dates, invalid durations, unsupported unit strings, boundary month/year overflow, invalid ISO serialization, and missing JSON keys

## Core use cases

- Create local, UTC, fixed-offset, and invalid moments.
- Parse ISO 8601, RFC 2822, selected custom format-token inputs, and deterministic `polycpp::Date` fallback formats.
- Format moments with Moment.js-style tokens, era tokens, and generated locale data.
- Mutate moments with getters, setters, add/subtract, start/end, UTC/local, and offset operations.
- Compare moments and compute differences.
- Use duration objects for arithmetic, total conversion, humanize output, and ISO serialization.
- Register/update locale data, list localized month/weekday names, and adjust relative-time thresholds and rounding.
- Inspect parse diagnostics, creation metadata, invalid overflow fields, and calendar-format keys.
- Convert moments to JSON-style objects and arrays, and durations to JSON-style objects.

## Key features to port first

- `Moment` construction, validity, cloning, timestamp access, and UTC/local/fixed-offset state.
- Unit getters/setters and unit normalization.
- `parse`, `utc`, `parseZone`, `fromDate`, `fromObject`, `min`, and `max` factories.
- Format tokens, default format, ISO serialization, `toString`, `toDate`, `inspect`, `toJSON`, `toArray`, and `toObject`.
- Add/subtract, start/end, comparison, diff, relative time, and calendar formatting.
- `Duration` construction, bubbling, arithmetic, total conversions, humanize, JSON object output, and ISO serialization.
- English locale and AD/BC era data, generated upstream locale corpus, generated locale parser tables, generated Japanese era data, locale registration/update/global selection, standalone month/weekday listers, relative-time thresholds and rounding, and per-instance locale selection.

## Features to defer

No core Moment.js parity items are currently tracked as deferred.

## v0 scope

- port version: 1.0.0
- versioning note: port version is independent from upstream versioning
- supported APIs: `Moment`, `Duration`, `DurationInput`, `LocaleData`, `LocaleParseEntry`, `EraSpec`, `RelativeTimeThresholds`, `MomentParsingFlags`, `MomentCreationData`, unit normalization, parsing factories including deterministic `polycpp::Date` fallback formats and explicit ISO/RFC sentinels, UTC/fixed-offset factories, formatting, HTML5 format constants, era display/parsing, diagnostics/interoperability helpers, comparison, diff, manipulation, duration math including per-call humanize thresholds, generated locale registration, generated locale-specific parser tables, standalone month/weekday listers, relative-time thresholds and rounding, calendar key selection and explicit calendar format overrides, JSON object/array conversion, and min/max helpers
- unsupported APIs: JavaScript runtime packaging, exact host-engine native `Date.parse` quirks, exact TypeScript overload parity, and mutable runtime-only package globals such as deprecation hooks, fallback hooks, default-format globals, version metadata, and plugin/prototype mutation points; deprecated aliases such as `lang`, `zone`, `dates`, `months`, `years`, and `isDSTShifted` are intentionally omitted and not tracked as deferred work
- out-of-scope companion package behavior: full IANA timezone database support is provided upstream by the separate `moment-timezone` package, not core `moment`; this port keeps core Moment's UTC, local-time, and fixed-offset scope
- dependency plan: no runtime dependency repos are needed; use base `polycpp` only
- polycpp modules to use: `Date`, `Math`, `Number`, `JsonValue`, `JsonObject`, `JsonArray`, and `RangeError`
- missing polycpp primitives: no blocking missing primitive for v0; a future `moment-timezone` companion would need a dedicated timezone database abstraction or generated tzdb data
