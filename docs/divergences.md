# Divergences From Upstream

## Deferred Features

No core Moment.js parity items are currently tracked as deferred.

## Deliberate Behavior Changes

- The JavaScript callable namespace `moment(...)` is split into explicit C++ constructors and factory functions such as `now`, `parse`, `utcFromString`, `fromDate`, and `duration`.
- Dynamic JavaScript input overloads are represented by typed C++ overloads, `polycpp::Date` factories, and `polycpp::JsonObject`/object-valued `polycpp::JsonValue` APIs for object-style construction.
- Unformatted string parsing uses Moment-style ISO/RFC detection first, then the deterministic `polycpp::Date::parse` fallback for JavaScript `Date` round-trip formats.
- Unsupported or implementation-specific native `Date.parse` inputs return invalid `Moment` values instead of inheriting host-engine parsing quirks.
- Locale configuration uses typed `LocaleData` fields and `std::function` callbacks rather than arbitrary JavaScript objects.
- The upstream locale corpus is generated into C++ data and registered through the library target instead of loading JavaScript locale files at runtime.
- Locale parser hooks such as `preparse`, month/weekday parse regexes, meridiem parse regexes, and ordinal parse regexes are converted into generated C++ parse tables instead of executing JavaScript regex functions at runtime.
- `Duration::toObject()` is a typed C++ convenience for structured duration inspection; upstream Moment.js durations do not expose a `duration.toObject()` method.
- Invalid `toISOString()` throws `polycpp::RangeError`, matching JavaScript error semantics through the polycpp error model.
- Package/runtime globals, CommonJS/AMD/UMD wrappers, and browser global mutation are replaced by CMake target consumption and headers.

## Unsupported Runtime-Specific Features

- JavaScript package loaders, browser globals, Ender/Dojo/SPM/Bower metadata, and minified bundle artifacts are not C++ APIs.
- Mutable JavaScript hook globals such as `moment.now`, `moment.updateOffset`, `moment.createFromInputFallback`, `moment.defaultFormat`, `moment.defaultFormatUtc`, `moment.version`, and `moment.momentProperties` are not exposed as runtime mutation points; equivalent behavior is represented by typed factories, fixed default formatting, deterministic fallback parsing, and package/build metadata.
- Console deprecation hooks such as `suppressDeprecationWarnings` and `deprecationHandler` are not exposed.
- Runtime predicates such as `moment.isDate()`, `moment.isMoment()`, and `moment.isDuration()` are not exposed because C++ static types cover the intended use.
- Deprecated JavaScript aliases such as `lang`, `zone`, `dates`, `months`, `years`, and `isDSTShifted` are not exposed.
- Plugin hooks, JavaScript object freezing, symbol markers, arbitrary object mutation, and prototype mutation behavior are not modeled.
- Full IANA timezone database behavior is not part of core `moment`; upstream named-zone behavior belongs to the separate `moment-timezone` package. This port supports UTC, local time through `polycpp::Date`, and fixed offsets parsed from strings.
