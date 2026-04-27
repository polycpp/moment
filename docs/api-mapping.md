# API Mapping

| Upstream symbol | C++ symbol | Status | Notes |
|---|---|---|---|
| `moment()` | `polycpp::moment::now()` and `Moment()` | adapted | C++ exposes explicit construction and factory names instead of a callable namespace object. |
| `moment(string)` | `parse(const std::string&)` | adapted | Auto-detects ISO 8601 and RFC 2822, then uses deterministic `polycpp::Date::parse` fallback formats. |
| `moment(string, format)` | `parse(input, format)` | direct | Uses Moment-style format tokens for the implemented token set. |
| `moment(string, format, strict)` | `parse(input, format, strict)` | direct | Strict mode rejects extra unmatched input. |
| `moment(string, string[])` | `parse(input, std::vector<std::string>)` | direct | Tries formats in order and returns the first valid parse. |
| `moment.ISO_8601` / `moment.RFC_2822` | `ISO_8601` / `RFC_2822` | direct | Explicit parser sentinels for `parse`, `utcFromFormat`, and `parseZone` format overloads. |
| `moment.parseTwoDigitYear` | `parseTwoDigitYear` | direct | Uses the upstream pivot: `00`-`68` -> `2000`-`2068`, `69`-`99` -> `1969`-`1999`. |
| `moment.utc(...)` | `utcNow`, `utcFromString`, `utcFromFormat`, `utcFromMs`, `utcFromDate`, `utcFromObject` | adapted | Dynamic overloads are split into typed factories; formatted inputs honor parsed offsets before converting to UTC. |
| `moment.unix(seconds)` | `fromUnixTimestamp(int64_t)` | direct | Converts seconds to milliseconds. |
| `moment.parseZone(...)` | `parseZone(input)` and `parseZone(input, format)` | direct | Preserves parsed fixed offset for supported string formats; assumes UTC when no offset is present. |
| `moment.duration(...)` | `duration(...)` and `Duration` constructors | adapted | Dynamic arguments map to typed overloads, ISO string input, `DurationInput`, or `JsonObject`. |
| `moment.invalid()` | `invalid()` | direct | Produces an invalid `Moment`. |
| `moment.min` / `moment.max` | `min` / `max` | direct | Supports initializer-list and vector overloads. |
| `moment.defineLocale` | `defineLocale` | direct | Uses typed `LocaleData`. |
| `moment.updateLocale` | `updateLocale` | direct | Uses typed `LocaleData`. |
| `moment.locale()` | `globalLocale()` | adapted | Getter/setter split into overloads; sets the default locale for new moments and durations. |
| `moment.localeData()` | `localeData()` | direct | Returns registered locale data. |
| `moment.locales()` | `locales()` | direct | Lists registered locale keys, including the generated Moment.js locale corpus when enabled. |
| `moment.relativeTimeThreshold` | `relativeTimeThreshold` | direct | Getter/setter overloads. |
| `moment.relativeTimeRounding` | `relativeTimeRounding` | direct | Getter/setter overloads for the relative-time rounding callback. |
| `moment.normalizeUnits` | `normalizeUnit` | direct | Returns a typed `Unit` enum. |
| `moment.fn.year/month/date/hour/minute/second/millisecond` | `Moment::year/month/date/hour/minute/second/millisecond` | direct | Getter and setter overloads preserve mutable chaining. |
| `moment.fn.day/weekday/isoWeekday/dayOfYear/week/isoWeek/weekYear/isoWeekYear/quarter` | matching `Moment` member methods | direct | Calendar-unit methods implemented as typed member functions. |
| `moment.fn.get` / `moment.fn.set` | `Moment::get` / `Moment::set` | direct | Unit strings are normalized through `normalizeUnit`. |
| `moment.fn.add` / `moment.fn.subtract` | `Moment::add` / `Moment::subtract` | direct | Mutates in place and returns `Moment&`. |
| `moment.fn.startOf` / `moment.fn.endOf` | `Moment::startOf` / `Moment::endOf` | direct | Supports implemented units. |
| `moment.fn.utc` / `moment.fn.local` | `Moment::utc` / `Moment::local` | direct | Supports `keepLocalTime`. |
| `moment.fn.utcOffset` | `Moment::utcOffset` | direct | Supports numeric minutes and offset strings. |
| `moment.fn.parseZone` | `Moment::parseZone` | direct | Re-applies the creation input offset, or switches to UTC with local wall-clock preservation when no offset was present. |
| `moment.fn.locale` | `Moment::locale` | direct | Getter/setter overloads select the locale for one `Moment`. |
| `moment.fn.localeData` | `Moment::localeData` | direct | Returns the locale data for one `Moment`. |
| `moment.fn.format` | `Moment::format` and `formatMoment` | direct | Supports implemented Moment-style tokens and macro formats. |
| `moment.fn.eraName/eraNarrow/eraAbbr/eraYear` | matching `Moment` member methods | direct | Uses locale era data, with English AD/BC fallback and generated Japanese era data. |
| `moment.fn.toISOString` | `Moment::toISOString` | direct | Throws `polycpp::RangeError` for invalid moments. |
| `moment.fn.unix` | `Moment::unix` | direct | Returns seconds since epoch. |
| `moment.fn.fromNow/from/toNow/to` | matching `Moment` member methods | direct | Uses locale relative-time data. |
| `moment.fn.calendar` | `Moment::calendar` | direct | Supports locale calendar formats, explicit `CalendarFormats` overrides, and generated callback-backed locale entries. |
| `moment.calendarFormat` | `calendarFormat` | direct | Returns the Moment.js calendar key for a moment relative to a reference. |
| `moment.fn.diff` | `Moment::diff` | direct | Supports truncating or precise unit output. |
| `moment.fn.toJSON` / `toString` / `toDate` / `inspect` | matching `Moment` member methods | direct | Uses Moment-style display and diagnostic behavior; `toDate` returns a `polycpp::Date` copy. |
| `moment.fn.toArray` / `toObject` | `Moment::toArray` / `Moment::toObject` | adapted | Uses `polycpp::JsonArray` and `polycpp::JsonObject`. |
| `moment.fn.valueOf` | `Moment::valueOf` | direct | Returns millisecond timestamp. |
| `moment.fn.isValid/isLeapYear/isUtc/isUTC/isLocal/isUtcOffset/isDST` | matching `Moment` member methods | direct | Query methods return typed booleans. |
| `moment.fn.hasAlignedHourOffset/zoneAbbr/zoneName` | matching `Moment` member methods | direct | Matches core Moment fixed-offset behavior; named-zone data remains outside core Moment scope. |
| `moment.fn.creationData/parsingFlags/invalidAt` | matching `Moment` member methods | adapted | JavaScript diagnostic objects become typed `MomentCreationData` and `MomentParsingFlags` structs. |
| `moment.fn.weeksInWeekYear/isoWeeksInISOWeekYear` | matching `Moment` member methods | direct | Complements `weeksInYear` and `isoWeeksInYear`. |
| `moment.fn.isBefore/isAfter/isSame/isBetween/isSameOrBefore/isSameOrAfter` | matching `Moment` member methods | direct | Supports raw and unit-granularity comparisons. |
| `moment.fn.clone` | `Moment::clone` | direct | Returns an independent copy. |
| `duration.clone/abs/add/subtract/humanize/as/get` | matching `Duration` member methods | direct | Preserves mutable duration behavior; `humanize` supports typed per-call threshold overrides. |
| `duration.years/months/days/weeks/hours/minutes/seconds/milliseconds` | matching `Duration` member methods | direct | Component getters expose bubbled values. |
| `duration.asMilliseconds/asSeconds/asMinutes/asHours/asDays/asWeeks/asMonths/asQuarters/asYears` | matching `Duration` member methods | direct | Total conversion uses Moment bucket constants. |
| `duration.toISOString/toString/toJSON/valueOf/isValid/locale/localeData` | matching `Duration` member methods | direct | Locale setter returns `Duration&`; `toString` aliases ISO output. |
| Typed duration object conversion | `Duration::toObject` | extension | Upstream Moment.js durations do not expose `toObject`; the C++ port provides a structured `polycpp::JsonObject` convenience. |
| Locale callback specs | `OrdinalFn`, `MeridiemFn`, `IsPMFn`, `MeridiemHourFn`, `RelativeTimeFn`, `CalendarFn`, `PrePostFormatFn`, `EraYearOrdinalParseFn` | adapted | JavaScript callbacks become typed `std::function` fields. |
| Per-call duration thresholds | `RelativeTimeThresholds` | adapted | JavaScript threshold option objects become an optional-field C++ struct. |
| Locale parser hooks | `LocaleParseEntry` vectors on `LocaleData` | adapted | Upstream locale `preparse`, month/weekday parse, meridiem parse, and ordinal parse behavior is generated into C++ parser tables. |
| Locale era specs | `EraSpec` and `LocaleData::eras` | direct | Supports `since`, `until`, `offset`, `name`, `narrow`, and `abbr` era entries. |
| JavaScript object creation input | `fromObject`, `utcFromObject`, `Duration(JsonObject)`, `duration(JsonObject)` | adapted | Plain JS object overloads become explicit JSON object APIs. |
| `moment.months`, `moment.monthsShort`, `moment.weekdays`, `moment.weekdaysShort`, `moment.weekdaysMin` | matching free functions | direct | Supports array, index, format-context, and locale-sorted weekday overloads as typed C++ functions. |
| `moment.HTML5_FMT` | `HTML5_FMT` constants namespace | direct | Exposes upstream HTML5 input format strings, with `html5_fmt` as a C++-style namespace alias. |
| `moment.isDate` / `moment.isMoment` / `moment.isDuration` | static C++ types | omitted | Runtime type predicates are not meaningful for statically typed C++; date values use the `polycpp::Date` type directly. |
| Deprecated aliases `lang`, `zone`, `dates`, `months`, `years`, `isDSTShifted` | none | omitted | Deprecated JavaScript compatibility aliases are intentionally not exposed. |
| UMD/CJS/AMD/global packaging | CMake target `polycpp::moment` | adapted | C++ consumption is through CMake and headers. |

## TypeScript Declaration Review

- Declaration source used: published `moment.d.ts` from version 2.30.1, with `ts3.1-typings/moment.d.ts` noted from package `typesVersions`
- Public APIs, overloads, options, callbacks, streams, or literal unions found only or most clearly in declarations: dynamic creation overloads, `MomentInputObject`, `DurationInputObject`, `LocaleSpecification`, calendar/relative-time/era callbacks, `unitOfTime` string unions, deprecated aliases, and global lister helpers
- Declaration-only globals, caches, deprecated fields, or runtime-specific surfaces mapped as unsupported/not-applicable: CommonJS `export = moment`, JavaScript namespace callability, deprecated `lang`/`zone` aliases, runtime predicates, and plugin/prototype-oriented surfaces

## Framework object boundary review

- Upstream reads or mutates framework/request/response/context objects: no
- Upstream fields or methods read: analyzer hits read internal helper result fields `res.months`, `res.milliseconds`, and `res._nextDay`
- Upstream fields or methods written: analyzer hits write internal helper result fields `res.months`, `res.milliseconds`, and `res._nextDay`
- C++ adapter boundary: no framework adapter is needed; equivalent state is represented by local structs and typed member fields
- Partial mutation risk on validation failure: public parse failures return invalid `Moment`/`Duration` values and do not mutate caller-owned framework objects

## Node parity surface review

- Callback APIs: no Node-style callbacks; locale callbacks are represented by typed `std::function` fields
- Promise APIs: none
- EventEmitter APIs: none
- Server/listener APIs: none
- Diagnostic/tracing APIs: none
- Stream APIs: none
- Buffer and binary APIs: none
- URL, timer, process, and filesystem APIs: no URL, process, or filesystem APIs; current-time behavior maps to `std::chrono` and `polycpp::Date`
- Crypto, compression, TLS, network, and HTTP APIs: none
- Unsupported or non-meaningful Node-specific APIs and audit reason: JavaScript package loaders, globals, console deprecation hooks, and prototype mutation are runtime/package concerns rather than C++ library APIs
