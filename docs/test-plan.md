# Test Plan

## Unit tests

- `tests/test_units.cpp`: unit alias normalization, canonical strings, invalid units, and case-sensitive shorthand behavior.
- `tests/test_locale.cpp`: English locale data, standalone month/weekday listers, custom locale registration, global locale selection, locale fallback, preparse/postformat identity, calendar strings, ordinal, meridiem, and relative-time thresholds/rounding including upstream `s`/`ss` threshold coupling.
- `tests/test_generated_locales.cpp`: generated upstream locale corpus registration, locale week rules, non-English formatting and parsing, standalone-vs-format locale listers, format-vs-standalone month/weekday contexts, postformatted relative time, callback-backed calendar behavior, Japanese era behavior, and global/per-instance locale selection.
- `tests/test_moment_basic.cpp`: construction, current time, timestamp storage, and basic validity behavior.
- `tests/test_moment.cpp`: getters/setters, add/subtract, start/end, UTC/local mode, UTC offset, month/year overflow, week/year behavior, clone independence, comparisons, and edge cases.
- `tests/test_duration.cpp`: duration construction, bubbling, component getters, total conversions including quarters, arithmetic, ISO serialization/string aliases, invalid durations, locale data, and humanize including per-call threshold overrides.
- `tests/test_format.cpp`: format tokens including expanded years, day-of-year ordinals, week-years, timezone names, backslash escapes, HTML5 format constants, macro formats, literals, timezone and era tokens, invalid moment formatting, ISO serialization, and Unix timestamp output.
- `tests/test_parse.cpp`: ISO 8601, RFC 2822, explicit ISO/RFC sentinels, deterministic `polycpp::Date` fallback formats, custom format parsing, locale/ISO week parsing, weekday mismatch diagnostics, HTML5 format constants, strict parsing, parse diagnostics, creation metadata, multiple formats, UTC factories, fixed-offset parsing, invalid inputs, min/max helpers, and round trips.
- `tests/test_display.cpp`: relative time, calendar output/key selection including explicit format overrides, JSON/string display, `toDate`, `inspect`, arrays, objects, timezone convenience methods, localeData, and week-count helpers.
- `tests/test_query.cpp`: validation, comparison helpers, `isBetween`, `isDST`, and unit-granularity comparison behavior.
- `tests/test_json_integration.cpp`: `JsonValue`, `JsonObject`, `JsonArray`, `polycpp::Date`, direct `JSON::stringify`, and diagnostic-object integration for moment and duration conversion APIs.

## Integration tests

- Parse then format ISO and custom-format strings and assert round-trip component stability.
- Convert moments to JSON-style arrays/objects and reconstruct local or UTC moments through `JsonObject` and object-valued `JsonValue` factories.
- Combine moment arithmetic and duration arithmetic in examples such as countdowns, standup summaries, and log-time reports.
- Build example programs with `POLYCPP_MOMENT_BUILD_EXAMPLES=ON` to verify public headers and link behavior.
- Build Doxygen/Sphinx documentation so API references and examples stay in sync with public declarations.

## Compatibility tests adapted from upstream

- upstream compatibility layout: Moment.js QUnit tests under `src/test/moment/*.js`, English locale tests under `src/test/locale/en.js`, and helper behavior under `src/test/helpers/*.js`.
- upstream-to-local coverage map:
  - `src/test/moment/create.js` -> `tests/test_parse.cpp`, `tests/test_moment_basic.cpp`
  - `src/test/moment/format.js` -> `tests/test_format.cpp`
  - `src/test/moment/getters_setters.js` -> `tests/test_moment.cpp`
  - `src/test/moment/add_subtract.js` -> `tests/test_moment.cpp`, `tests/test_duration.cpp`
  - `src/test/moment/start_end_of.js` -> `tests/test_moment.cpp`
  - `src/test/moment/diff.js` -> `tests/test_moment.cpp`
  - `src/test/moment/duration.js` -> `tests/test_duration.cpp`
  - `src/test/moment/duration_invalid.js` -> `tests/test_duration.cpp`
  - `src/test/moment/from_to.js` and `relative_time.js` -> `tests/test_display.cpp`, `tests/test_duration.cpp`
  - `src/test/moment/calendar.js` -> `tests/test_display.cpp`
  - `src/test/moment/utc.js`, `utc_offset.js`, `zones.js`, and `zone_switching.js` -> `tests/test_parse.cpp`, `tests/test_moment.cpp`, `tests/test_format.cpp`
  - `src/test/moment/is_before.js`, `is_after.js`, `is_same.js`, `is_between.js`, `is_same_or_before.js`, and `is_same_or_after.js` -> `tests/test_query.cpp`
  - `src/test/moment/weeks.js`, `weeks_in_year.js`, `week_year.js`, `weekday.js`, and `quarter.js` -> `tests/test_units.cpp`, `tests/test_moment.cpp`, `tests/test_format.cpp`
  - `src/test/moment/min_max.js` -> `tests/test_parse.cpp`
  - `src/test/moment/mutable.js` -> mutation/chaining cases in `tests/test_moment.cpp`
  - `src/test/moment/invalid.js` and `is_valid.js` -> `tests/test_parse.cpp`, `tests/test_format.cpp`, `tests/test_query.cpp`
  - `src/test/locale/en.js` and representative non-English locale behavior -> `tests/test_locale.cpp`, `tests/test_generated_locales.cpp`, `tests/test_format.cpp`, `tests/test_display.cpp`
- omitted upstream cases:
  - exhaustive per-locale QUnit files under `src/test/locale/*.js` are not fully duplicated; the generated corpus is covered by representative non-English formatting, parsing, calendar, relative-time, postformat, and week-rule tests
  - deprecation tests are omitted because deprecation warning hooks are not exposed
  - JavaScript runtime type tests for arrays, dates, numbers, and `instanceof` are omitted or mapped to static C++ type behavior
  - browser/package-loader tests are not applicable to a C++ companion library
  - host-specific native `Date.parse` fallback cases are omitted because unsupported strings fail closed as invalid moments; deterministic `polycpp::Date` fallback formats are covered
  - named IANA timezone database tests are omitted because upstream implements them in `moment-timezone`, not core `moment`
  - era-specific formatting/parsing cases are covered for English AD/BC and generated Japanese era data
  - `moment.HTML5_FMT` parity is covered by constant-value, formatting, and strict parsing tests
  - diagnostic/interoperability parity is covered by `creationData`, `parsingFlags`, `invalidAt`, `inspect`, `toDate`, `JsonValue`, and direct `JSON::stringify` tests

## Security and fail-closed tests

- Invalid strings and empty strings return invalid moments.
- Strict parsing rejects extra input.
- Invalid moments format as `Invalid date` for normal formatting and throw `polycpp::RangeError` from `toISOString()`.
- Invalid ISO duration strings produce invalid durations.
- Unknown unit strings normalize to `Unit::Invalid` and do not silently map to a valid unit.
- Missing JSON object keys use documented defaults rather than reading uninitialized data.
- Locale lookup for unknown keys falls back to English instead of returning dangling state.

## Protocol/client tests

Not applicable because `moment` is not a database, cache, queue, cloud-service, wire-protocol client, or protocol server.

- service-backed e2e matrix: not applicable
- transport matrix, including TLS/compression decisions: not applicable
- auth or credential flow matrix: not applicable
- malformed packet / unsupported mode tests: not applicable
- binary payload type-mapping tests: not applicable
- stateful parser/session-state tests: not applicable
- server/listener response writer loopback tests: not applicable
- multi-result or unread-packet drain behavior: not applicable
- pool/session lifecycle tests: not applicable

## Release-blocking behaviors

- CMake configure and build must pass with local base `polycpp`.
- All GoogleTest tests discovered by CTest must pass.
- Example programs must build when `POLYCPP_MOMENT_BUILD_EXAMPLES=ON`.
- `python3 docs/build.py` must pass with warnings treated as errors.
- README usage snippets must compile against the public API.
- `THIRD_PARTY_LICENSES.md` must preserve the upstream Moment.js MIT notice.
- Remaining upstream divergences and omitted runtime-specific surfaces must be listed in `docs/divergences.md`.

## Current validation

Recorded during this retrofit pass:

- `python3 scripts/check-port-readiness.py --baseline <repo path>`
- `python3 scripts/intake-upstream.py <repo path> https://github.com/moment/moment.git`
- `python3 scripts/intake-npm-package.py <repo path> moment`
- `python3 scripts/analyze-upstream-js.py <repo path> <repo path>/.tmp/npm-package`
- `node tools/generate-locales.mjs <repo path>/.tmp/npm-package`
- `cmake -S . -B build-libgen -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_SOURCE_DIR=<repo path>/../polycpp -DPOLYCPP_MOMENT_BUILD_TESTS=ON -DPOLYCPP_MOMENT_BUILD_EXAMPLES=ON`
- `cmake --build build-libgen -j2`
- `ctest --test-dir build-libgen --output-on-failure -j2`
- `python3 docs/build.py`
- `python3 <libgen path>/scripts/check-port-validation.py <repo path>`
- `cmake -S . -B build-no-locales -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_SOURCE_DIR=<repo path>/../polycpp -DPOLYCPP_MOMENT_BUILD_TESTS=OFF -DPOLYCPP_MOMENT_BUILD_EXAMPLES=OFF -DPOLYCPP_MOMENT_ENABLE_ALL_LOCALES=OFF`
- `cmake --build build-no-locales -j2`
- `cmake -S . -B build-no-locales-tests -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_SOURCE_DIR=<repo path>/../polycpp -DPOLYCPP_MOMENT_BUILD_TESTS=ON -DPOLYCPP_MOMENT_BUILD_EXAMPLES=OFF -DPOLYCPP_MOMENT_ENABLE_ALL_LOCALES=OFF`
- `cmake --build build-no-locales-tests -j2`
- `ctest --test-dir build-no-locales-tests --output-on-failure -j2`
- `cmake -S . -B build-selected-locales -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_SOURCE_DIR=<repo path>/../polycpp -DPOLYCPP_MOMENT_BUILD_TESTS=OFF -DPOLYCPP_MOMENT_BUILD_EXAMPLES=OFF -DPOLYCPP_MOMENT_LOCALES=fr,ar,ja`
- `cmake --build build-selected-locales -j2`
- `cmake -S . -B build-selected-locales-tests -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_SOURCE_DIR=<repo path>/../polycpp -DPOLYCPP_MOMENT_BUILD_TESTS=ON -DPOLYCPP_MOMENT_BUILD_EXAMPLES=OFF -DPOLYCPP_MOMENT_LOCALES=fr,ar,ja`
- `cmake --build build-selected-locales-tests -j2`
- `ctest --test-dir build-selected-locales-tests --output-on-failure -j2`

Validation results:

- CMake configure completed with GNU 11.4.0; CMake reported the existing base `polycpp` recommendation for GCC 12+.
- Build completed for the library, all GoogleTest binaries, and all example programs.
- CTest passed: 461 tests passed, 0 failed.
- Documentation build passed with Sphinx warnings treated as errors.
- Libgen post-implementation validation passed.
- English/custom-locale-only build with generated locales disabled passed; matching test build passed: 447 tests passed, 0 failed.
- Curated generated-locale build with `fr`, `ar`, and `ja` passed.
- Curated generated-locale test build passed: 447 tests passed, 0 failed; full-corpus tests are only enabled when the full corpus is configured.
