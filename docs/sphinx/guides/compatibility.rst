Moment.js compatibility and divergences
=======================================

**When to reach for this:** you are porting Moment.js usage to C++
and need to know what is compatible, what is adapted, and what is
intentionally outside core ``polycpp/moment``.

``polycpp/moment`` is a C++20 port of Moment.js behavior, based on
the upstream Moment.js 2.30.1 surface. It is not a JavaScript runtime
compatibility layer. The public API keeps Moment's mutable,
chainable model, but exposes typed C++ factories, structs, callbacks,
and CMake targets instead of dynamic JavaScript overloads.

Supported core behavior
-----------------------

- :cpp:class:`Moment <polycpp::moment::Moment>` and
  :cpp:class:`Duration <polycpp::moment::Duration>` are mutable and
  chainable.
- Parsing covers ISO 8601, RFC 2822, explicit ISO/RFC sentinels,
  custom Moment-style format strings, UTC construction, fixed-offset
  ``parseZone`` inputs, and deterministic ``polycpp::Date`` fallback
  formats.
- Formatting uses Moment-style tokens, escaped ``[brackets]``, long
  date macros such as ``L`` / ``LL`` / ``LLLL``, relative time,
  calendar output, era data, and generated Moment.js locale data.
- Date math, unit aliases, start/end of units, comparison, ``diff``,
  ``min`` / ``max``, parsing flags, creation metadata, invalid
  moments, and invalid durations are modeled in the C++ API.
- Core time modes are local time, UTC, and fixed UTC offsets.

Adapted for C++
---------------

- The JavaScript callable namespace ``moment(...)`` is split into
  explicit factories such as :cpp:func:`parse
  <polycpp::moment::parse>`, :cpp:func:`utcFromString
  <polycpp::moment::utcFromString>`, :cpp:func:`fromDate
  <polycpp::moment::fromDate>`, :cpp:func:`fromObject
  <polycpp::moment::fromObject>`, and :cpp:func:`duration
  <polycpp::moment::duration>`.
- Dynamic overloads become typed overloads. Plain JavaScript object
  input becomes ``polycpp::JsonObject`` or object-valued
  ``polycpp::JsonValue`` input.
- JavaScript ``Date`` interop becomes ``polycpp::Date`` interop.
  Invalid ``polycpp::Date`` values produce invalid moments.
- Locale definitions use typed :cpp:struct:`LocaleData
  <polycpp::moment::LocaleData>` fields and ``std::function``
  callbacks rather than arbitrary JavaScript objects.
- Moment.js locale files are generated into C++ data and compiled into
  the library target. CMake options select all, none, or a requested
  set of generated locales.
- ``Moment::toArray`` and ``Moment::toObject`` return
  ``polycpp::JsonArray`` and ``polycpp::JsonObject``. ``Duration`` also
  exposes a C++ convenience ``toObject()``.
- Invalid ``Moment::toISOString()`` throws ``polycpp::RangeError``,
  matching JavaScript's error shape through the polycpp error model.

Important indexing and mode rules
---------------------------------

Moment.js and JavaScript ``Date`` month numbering is preserved:
January is ``0`` and December is ``11`` in component APIs,
``toArray()``, and object keys such as ``months``. Human-facing format
tokens such as ``MM`` still print ``01`` through ``12``.

``parse`` and ``parseZone`` preserve explicit parsed fixed offsets.
For strings without an offset, ``parse`` creates a local moment while
``parseZone`` assumes UTC/fixed offset ``+00:00``. ``utcFromString``
treats no-offset parsed input as UTC, but explicit offsets remain
fixed-offset unless you call ``.utc()`` afterwards. Local mode
delegates local-time and DST behavior to ``polycpp::Date``.

Generated locales
-----------------

The generated locale corpus is registered by default. Builds can
disable generated locales or compile a selected set with
``POLYCPP_MOMENT_ENABLE_ALL_LOCALES`` and ``POLYCPP_MOMENT_LOCALES``.
English locale data remains available as the base fallback.

Locale parser hooks from Moment.js are represented as generated parse
tables and typed callbacks. The result is data-driven C++ behavior, not
runtime execution of JavaScript locale files.

Omitted or out of scope
-----------------------

- Full IANA timezone database behavior is not in core
  ``polycpp/moment``. Core supports local time, UTC, and fixed offsets;
  named-zone behavior belongs to a separate timezone database layer,
  just as upstream Moment.js uses the separate ``moment-timezone``
  package.
- JavaScript package/runtime artifacts are not C++ APIs: CommonJS,
  AMD, UMD, browser globals, minified bundles, and package-loader
  behavior are replaced by headers and the CMake target
  ``polycpp::moment``.
- Host-engine-specific ``Date.parse`` quirks are not reproduced.
  Unsupported fallback parse inputs return invalid moments.
- Mutable JavaScript runtime hooks such as ``moment.now``,
  ``moment.updateOffset``, ``moment.createFromInputFallback``,
  ``moment.defaultFormat``, deprecation handlers, plugin hooks, and
  prototype mutation are not exposed.
- Runtime type predicates such as ``moment.isDate()``,
  ``moment.isMoment()``, and ``moment.isDuration()`` are omitted because
  C++ static types cover those use cases.
- Deprecated JavaScript aliases such as ``lang``, ``zone``, ``dates``,
  ``months``, ``years``, and ``isDSTShifted`` are intentionally omitted.

See also
--------

- :doc:`choose-input-factory` for choosing typed construction APIs.
- :doc:`validate-input` for invalid values, exceptions, and unknown
  unit strings.
- :doc:`../tutorials/locale-formatting` for generated locale data and
  process-wide locale settings.
