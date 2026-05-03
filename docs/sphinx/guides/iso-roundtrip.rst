Round-trip through ISO 8601
===========================

**When to reach for this:** storage, APIs, and anything else that
traffics in strings. ISO 8601 is Moment's default string shape for
stable interchange; by default serialization emits the same instant in
UTC, while ``toISOString(true)`` keeps a fixed numeric offset.

.. code-block:: cpp

   namespace m = polycpp::moment;

   // Parse: any ISO 8601 input, UTC or with offset.
   auto utc      = m::parse("2026-04-23T10:30:00Z");
   auto offset   = m::parse("2026-04-23T10:30:00+02:00");
   auto zoned    = m::parseZone("2026-04-23T10:30:00+02:00");

   // Serialize: round-trip-stable.
   std::string s1 = utc.toISOString();                // "2026-04-23T10:30:00.000Z"
   std::string s2 = offset.toISOString(true);         // keeps "+02:00"
   std::string s3 = zoned.toISOString();              // normalizes to "Z"
   std::string s4 = utc.toJSON();                     // identical to toISOString()

Three rules of thumb:

- :cpp:func:`parse <polycpp::moment::parse>` and
  :cpp:func:`parseZone <polycpp::moment::parseZone>` both preserve an
  explicit numeric offset such as ``+02:00`` for later formatting.
  Their no-offset behavior differs: ``parse`` creates a local moment,
  while ``parseZone`` assumes UTC/fixed offset ``+00:00``.
- Leave :cpp:func:`toISOString
  <polycpp::moment::Moment::toISOString>` at its default to serialize
  the instant in UTC with ``Z``. Pass ``true`` to
  :cpp:func:`toISOString
  <polycpp::moment::Moment::toISOString>` to preserve the offset
  ``+HH:MM``.
- :cpp:func:`toJSON <polycpp::moment::Moment::toJSON>` is the alias
  you expose to serialization layers — same output as
  :cpp:func:`toISOString <polycpp::moment::Moment::toISOString>`
  with no arguments.

Same applies to :cpp:class:`Duration
<polycpp::moment::Duration>`:

.. code-block:: cpp

   m::Duration d = m::duration(m::DurationInput{.hours = 5, .minutes = 30});
   std::string iso = d.toISOString();    // "PT5H30M"
   m::Duration back(iso);                // exact round-trip
   assert(back.asMinutes() == 330);
