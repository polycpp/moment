# Dependency and JavaScript API Analysis

`scripts/analyze-upstream-js.py` was run after upstream and npm artifact intake. The JSON report is stored at `.tmp/dependency-analysis.json`.

- package: moment
- package version: 2.30.1
- package root: `<repo path>/.tmp/npm-package`
- analyzer json: `.tmp/dependency-analysis.json`
- published npm artifact path: `<repo path>/.tmp/npm-package`
- published npm artifact analyzed: yes
- include dev dependencies: no
- dependency source install used: yes
- companion root checked: `<repo path>/../`

## Package entry metadata

- main: `./moment.js`
- module: not declared
- types: `./moment.d.ts`
- exports: none declared
- bin: none declared
- missing declared entries in repo clone: none
- TypeScript source files detected: 0
- TypeScript declarations reviewed: yes, `moment.d.ts` and package `typesVersions`
- declaration-source decision: use the published `moment.d.ts` as the public API reference because it is shipped to npm consumers
- source-vs-published artifact decision: analyze the published artifact for consumer entry points and use the upstream source tree for readable source modules and QUnit test mapping

## Direct dependencies

- none detected

## Dependency ownership decisions

No direct runtime dependencies are declared. The target package source is owned by this repo under the upstream MIT license notice.

| Package | Kind | Requested | Installed | License | License evidence | License impact | License strategy | Affects repo license | Deps | Source files | Node API calls | JS API calls | Recommendation | Rationale |
|---|---|---|---|---|---|---|---|---|---:|---:|---:|---:|---|---|
| moment | target | 2.30.1 | 2.30.1 | MIT | package.json license field and upstream LICENSE file | permissive | permissive dependency ok with notice | no | 0 | 1 | 0 | 186 | port in this repo | Runtime has no production dependencies; the C++ port can implement the selected surface directly with base `polycpp` primitives and preserve the MIT notice. |

## License impact summary

- upstream package license: MIT
- repo license decision: keep this companion repo MIT and include the upstream Moment.js notice in `THIRD_PARTY_LICENSES.md`
- GPL/AGPL dependencies: none detected
- LGPL/MPL dependencies: none detected
- permissive dependencies requiring notices: upstream Moment.js source notice only
- dev/test-only dependencies excluded from shipped artifacts: benchmark, Grunt, Rollup, TypeScript, QUnit, Karma, ESLint, Prettier, coverage, and related upstream build/test packages
- dependency license notices to add to `THIRD_PARTY_LICENSES.md`: upstream Moment.js MIT notice

## Transitive dependency summary

- none detected for runtime dependencies
- dev/test-only transitive dependencies are not shipped and are excluded from this companion dependency graph

## Runtime API usage

### Target package

- entry points analyzed: `moment.js`
- source files analyzed by analyzer: 1 published bundle file
- source files manually inspected: `moment.js`, `moment.d.ts`, `ts3.1-typings/moment.d.ts`, `src/moment.js`, `src/lib/create/*`, `src/lib/parse/*`, `src/lib/format/format.js`, `src/lib/moment/*`, `src/lib/duration/*`, `src/lib/units/*`, `src/lib/locale/*`, `src/test/moment/*`, and `src/test/locale/en.js`
- external imports seen from target: none

### Analyzer porting gates

- polycpp reuse hints consumed: yes; `Date`, `Math`, and `Number` hints were mapped to base `polycpp` core modules
- Node parity hints consumed: yes; callback-shaped signals were reviewed and mapped to typed locale callback hooks or omitted runtime surfaces
- security hints consumed: yes; analyzer did not classify the package as security-sensitive
- security-sensitive package: no

### Node.js API usage

- none detected in the runtime implementation

### Node parity surface usage

- callbacks: analyzer reported callback-shaped names; the meaningful public equivalents are locale formatting hooks, represented as typed `std::function` fields
- Promise APIs: none
- EventEmitter APIs: none
- server/listener APIs: none
- diagnostic/tracing APIs: none
- streams: none
- Buffer and binary data: none
- URL/timer/process/filesystem APIs: no URL, process, timer, or filesystem APIs; current-time behavior maps to `std::chrono` and `polycpp::Date`
- crypto/compression/TLS/network/HTTP APIs: none

### JavaScript API usage

Analyzer-detected JavaScript API families:

- `Date`, `Date.now`, `Date.UTC`, local and UTC date getters/setters
- `Math.abs`, `ceil`, `floor`, `max`, `min`, `pow`, and `round`
- `parseInt`, `parseFloat`, `isNaN`, and `isFinite`
- `RegExp`, `exec`, and `test`
- array operations such as `push`, `join`, `sort`, `indexOf`, and `slice`
- object operations such as `Object.assign`, `Object.getOwnPropertyNames`, `Object.isFrozen`, and `hasOwnProperty`
- string operations such as `indexOf`, `match`, `replace`, `slice`, `substr`, and `toLowerCase`
- `console.warn`, `Error`, `Number.prototype.toFixed`, and `Symbol.for`

C++ replacements:

- `polycpp::Date` for JavaScript date construction, component access, and UTC conversion
- `polycpp::Math` and `polycpp::Number` for JavaScript-style numeric behavior
- `std::regex` and explicit token parsing for selected parser behavior
- `std::vector`, `std::array`, `std::string`, `std::variant`, and typed structs for JavaScript arrays/objects
- `polycpp::JsonValue`, `polycpp::JsonObject`, and `polycpp::JsonArray` for JSON interop, object/array conversion APIs, and diagnostic serialization
- no replacement for JavaScript package/runtime deprecation warning hooks in v0

### Framework object boundary usage

- analyzer-reported target-package framework object accesses: 11 internal `res.*` reads/writes in the published bundle
- analyzer-reported dependency framework object accesses: none
- manual review decision: these hits are internal result-object mutations for parsing or duration difference helpers, not request/response/context framework boundaries

## Porting decisions

- Implement the selected v0 API directly in this repo because Moment has no runtime dependencies and depends heavily on date/time semantics rather than reusable companion packages.
- Use `polycpp::Date` for JavaScript Date parity instead of plain `std::chrono` wherever component normalization or local/UTC getters matter.
- Use typed factory overloads plus `JsonObject` and object-valued `JsonValue` input for object-style creation instead of mirroring every JavaScript dynamic overload.
- Preserve Moment's mutable, chainable instance style in C++ member methods.
- Preserve duration three-bucket storage and bubbling because total conversion and component display differ from simple millisecond storage.
- Use `polycpp::Date::parse` for deterministic JavaScript `Date` round-trip fallback formats; treat unsupported host-specific parse strings as invalid moments.
- Ship generated Moment.js locale data from the npm package and keep English locale plus AD/BC era data available as the base locale.
- Keep JavaScript runtime packaging outside v0; plugin/prototype mutation behavior and deprecated aliases are intentionally omitted rather than deferred.
- Keep full IANA timezone database behavior outside this core port because upstream provides named-zone support through the separate `moment-timezone` package; core `moment` only owns UTC, local-time, and fixed-offset behavior.

## Analyzer warnings

- none emitted by analyzer
