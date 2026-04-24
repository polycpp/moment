Round-trip through ISO 8601
===========================

**When to reach for this:** storage, APIs, and anything else that
traffics in strings. ISO 8601 is Moment's default string shape and
survives every parse/format pass untouched.

.. code-block:: cpp

   namespace m = polycpp::moment;

   // Parse: any ISO 8601 input, UTC or with offset.
   auto utc      = m::parse("2026-04-23T10:30:00Z");
   auto offset   = m::parseZone("2026-04-23T10:30:00+02:00");

   // Serialize: round-trip-stable.
   std::string s1 = utc.toISOString();                // "2026-04-23T10:30:00.000Z"
   std::string s2 = offset.toISOString(true);         // keeps "+02:00"
   std::string s3 = utc.toJSON();                     // identical to toISOString()

Three rules of thumb:

- :cpp:func:`parseZone <polycpp::moment::parseZone>` preserves the
  string's offset; :cpp:func:`parse <polycpp::moment::parse>`
  normalises to UTC.
- Pass ``true`` to :cpp:func:`toISOString
  <polycpp::moment::Moment::toISOString>` to preserve the offset
  ``+HH:MM``; leave it ``false`` (the default) to emit ``Z``.
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
