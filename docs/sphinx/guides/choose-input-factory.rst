Choose the right input factory
==============================

**When to reach for this:** you know what shape your input has, but
you are not sure which construction function preserves the semantics
you want.

The factories are intentionally explicit. Pick the one that matches
your source data instead of relying on a single overloaded constructor.

.. list-table::
   :header-rows: 1

   * - Input shape
     - Use
     - Why
   * - Current local time
     - :cpp:func:`now <polycpp::moment::now>`
     - Captures the current wall-clock time in local mode.
   * - Current UTC time
     - :cpp:func:`utcNow <polycpp::moment::utcNow>`
     - Captures the current instant in UTC mode.
   * - ISO 8601, RFC 2822, or deterministic date string
     - :cpp:func:`parse(input) <polycpp::moment::parse>`
     - Auto-detects supported string formats and returns invalid on failure.
   * - Known custom format
     - :cpp:func:`parse(input, format, strict) <polycpp::moment::parse>`
     - Keeps validation tied to the exact token pattern you expected.
   * - String that should become UTC
     - :cpp:func:`utcFromString <polycpp::moment::utcFromString>`
     - Converts the parsed instant to UTC output mode.
   * - String whose fixed offset must be preserved
     - :cpp:func:`parseZone <polycpp::moment::parseZone>`
     - Keeps the parsed ``+HH:mm`` or ``-HH:mm`` offset instead of normalizing display to UTC/local time.
   * - Year/month/day components
     - :cpp:func:`fromDate <polycpp::moment::fromDate>` or :cpp:func:`utcFromDate <polycpp::moment::utcFromDate>`
     - Avoids string parsing when components are already structured.
   * - Existing ``polycpp::Date``
     - :cpp:func:`fromDate <polycpp::moment::fromDate>` or :cpp:func:`utcFromDate <polycpp::moment::utcFromDate>`
     - Copies the timestamp from another polycpp JavaScript-style date object.
   * - Epoch seconds or milliseconds
     - :cpp:func:`fromUnixTimestamp <polycpp::moment::fromUnixTimestamp>`, :cpp:func:`fromMilliseconds <polycpp::moment::fromMilliseconds>`, or :cpp:func:`utcFromMs <polycpp::moment::utcFromMs>`
     - Makes the timestamp unit explicit at the call site.
   * - JSON-style object or ``JsonValue`` object
     - :cpp:func:`fromObject <polycpp::moment::fromObject>` or :cpp:func:`utcFromObject <polycpp::moment::utcFromObject>`
     - Maps object keys such as ``year``, ``month``, ``date``, ``hour``, and ``minute`` to typed construction.

Common choices
--------------

Parse a user-supplied ISO string and check validity:

.. code-block:: cpp

   namespace m = polycpp::moment;

   auto submitted = m::parse(input);
   if (!submitted.isValid()) {
       throw std::invalid_argument("invalid date");
   }

Parse an exact UI field:

.. code-block:: cpp

   auto day = m::parse("04/27/2026", "MM/DD/YYYY", true);
   assert(day.isValid());

Preserve a fixed-offset timestamp for display:

.. code-block:: cpp

   auto remote = m::parseZone("2026-04-27T09:30:00-05:00");
   std::cout << remote.format("YYYY-MM-DD HH:mm Z") << '\n';

Build from components when you already have structured data:

.. code-block:: cpp

   auto local = m::fromDate(2026, 3, 27, 9, 30);      // Apr 27, local mode
   auto utc   = m::utcFromDate(2026, 3, 27, 9, 30);   // Apr 27, UTC mode

Bridge from other polycpp date/JSON APIs without parsing strings:

.. code-block:: cpp

   polycpp::Date date(1777282200000.0);
   auto fromDate = m::utcFromDate(date);

   polycpp::JsonValue value(polycpp::JsonObject{
       {"year", 2026}, {"month", 3}, {"date", 27}, {"hour", 9}
   });
   auto fromJson = m::fromObject(value);

Month numbers follow Moment.js and JavaScript ``Date``: January is
``0`` and December is ``11``. Format tokens such as ``MM`` remain
human-facing and print months as ``01`` through ``12``.

Rules of thumb
--------------

- Use ``parse`` for broad accepted input and ``parse(..., true)`` for
  exact input.
- Use ``utcFrom...`` when later formatting should default to UTC.
- Use ``parseZone`` when the source offset is meaningful to the user.
- Use component or object factories when your data is already
  structured; they make month indexing and missing defaults explicit.
- Use ``JsonValue`` overloads when input is already in polycpp's
  dynamic JSON representation.
- Always check ``isValid`` on untrusted input.
