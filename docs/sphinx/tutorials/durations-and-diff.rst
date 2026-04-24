Durations and diff
==================

**You'll build:** a shift-scheduling helper that computes elapsed
time between two moments, splits it into human-sized chunks, and
round-trips through ISO 8601 for storage.

**You'll use:** :cpp:func:`Moment::diff
<polycpp::moment::Moment::diff>`, :cpp:class:`Duration
<polycpp::moment::Duration>`, and the
:cpp:func:`duration <polycpp::moment::duration>` factory overloads.

Step 1 — ask two moments how far apart they are
-----------------------------------------------

.. code-block:: cpp

   namespace m = polycpp::moment;
   auto start = m::parse("2026-05-04T09:00:00Z");
   auto end   = m::parse("2026-05-04T17:30:00Z");

   double hours = end.diff(start, "hour", true);   // 8.5

:cpp:func:`diff() <polycpp::moment::Moment::diff>` returns a
``double``; the last argument controls truncation. Pass ``true``
(precise) for fractional results, ``false`` (the default) for
truncation toward zero.

Step 2 — turn milliseconds into a Duration
------------------------------------------

When you want the bubbled components — "5 days 3 hours" — feed the
millisecond diff to :cpp:func:`duration
<polycpp::moment::duration>`:

.. code-block:: cpp

   const int64_t ms = static_cast<int64_t>(end.diff(start, "ms"));
   m::Duration d(ms);
   std::cout << d.hours() << "h " << d.minutes() << "m\n";   // 8h 30m
   std::cout << d.humanize(false) << '\n';                   // "9 hours"

The humaniser snaps to the nearest labelled unit — "9 hours" uses
the same thresholds as :cpp:func:`Moment::fromNow
<polycpp::moment::Moment::fromNow>`. Dial them with
:cpp:func:`relativeTimeThreshold
<polycpp::moment::relativeTimeThreshold>`.

Step 3 — round-trip through ISO 8601
------------------------------------

:cpp:func:`Duration::toISOString
<polycpp::moment::Duration::toISOString>` emits ``P...`` form;
constructing a :cpp:class:`Duration <polycpp::moment::Duration>`
from that string round-trips cleanly:

.. code-block:: cpp

   std::string stored = d.toISOString();     // "PT8H30M"
   m::Duration restored(stored);
   assert(restored.isValid());
   assert(restored.asMinutes() == 510);

The ISO form is lossless for the hour/minute/second range; for
durations that span months or years, store the components
separately (or accept that ``P1M`` is month-count, not
30-days-worth-of-seconds).

Step 4 — build from named fields
--------------------------------

Tests are cleaner when you can name the components:

.. code-block:: cpp

   m::Duration shift(m::DurationInput{.hours = 8, .minutes = 30});
   assert(shift.asHours() == 8.5);

   // Same thing via JsonObject (useful when values come from config):
   m::Duration fromCfg = m::duration(
       polycpp::JsonObject{{"hours", 8}, {"minutes", 30}}
   );
   assert(fromCfg.asHours() == 8.5);

What you learned
----------------

- :cpp:func:`Moment::diff <polycpp::moment::Moment::diff>` returns a
  ``double``; pass ``precise=true`` for fractional output.
- :cpp:class:`Duration <polycpp::moment::Duration>` "bubbles" a
  millisecond total into individual components; use either the
  getters or :cpp:func:`humanize
  <polycpp::moment::Duration::humanize>` for display.
- ISO 8601 round-trips via :cpp:func:`Duration::toISOString
  <polycpp::moment::Duration::toISOString>` and the
  single-string constructor.
- :cpp:struct:`DurationInput <polycpp::moment::DurationInput>` and
  the ``JsonObject`` overload make test and config loading
  readable.
