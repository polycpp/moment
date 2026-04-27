Business-day arithmetic
=======================

**You'll build:** a helper that takes a start date and a number of
business days ``N`` and returns the date ``N`` weekdays later —
skipping Saturdays and Sundays — then a variant that also skips a
list of holidays.

**You'll use:** :cpp:func:`Moment::add
<polycpp::moment::Moment::add>`, :cpp:func:`Moment::day
<polycpp::moment::Moment::day>`, :cpp:func:`Moment::isSame
<polycpp::moment::Moment::isSame>`, and
:cpp:func:`Moment::clone <polycpp::moment::Moment::clone>`.

**Prerequisites:** a working ``polycpp::moment`` install — see
:doc:`../getting-started/installation`.

Moment component factories use JavaScript and Moment.js month
numbering: January is ``0`` and December is ``11``. The examples below
use ``3`` for April.

Step 1 — write the basic weekday loop
-------------------------------------

.. code-block:: cpp

   #include <polycpp/moment/moment.hpp>
   using polycpp::moment::Moment;

   Moment addBusinessDays(Moment m, int n) {
       m = m.clone();                       // never mutate the caller's value
       int direction = (n >= 0) ? 1 : -1;
       int remaining = std::abs(n);
       while (remaining > 0) {
           m.add(direction, "day");
           const int dow = m.day();         // 0=Sun, 6=Sat
           if (dow != 0 && dow != 6) --remaining;
       }
       return m;
   }

:cpp:func:`day() <polycpp::moment::Moment::day>` returns a
locale-agnostic 0–6 value where Sunday is 0 — perfect for this kind
of weekend test. Use :cpp:func:`isoWeekday
<polycpp::moment::Moment::isoWeekday>` if you prefer 1–7 with
Monday=1.

Step 2 — accept a holiday set
-----------------------------

Compare at day granularity by passing ``"day"`` to
:cpp:func:`isSame <polycpp::moment::Moment::isSame>`:

.. code-block:: cpp

   #include <vector>
   Moment addBusinessDays(Moment start, int n,
                          const std::vector<Moment>& holidays) {
       Moment m = start.clone();
       int direction = (n >= 0) ? 1 : -1;
       int remaining = std::abs(n);
       while (remaining > 0) {
           m.add(direction, "day");
           if (m.day() == 0 || m.day() == 6) continue;
           bool isHoliday = false;
           for (const auto& h : holidays)
               if (m.isSame(h, "day")) { isHoliday = true; break; }
           if (!isHoliday) --remaining;
       }
       return m;
   }

With unit-aware comparison, differences in hour/minute/second don't
matter — all that counts is the calendar date.

Step 3 — test it
----------------

.. code-block:: cpp

   #include <cassert>
   namespace m = polycpp::moment;
   auto fri = m::utcFromDate(2026, 3, 10);              // Fri 10 Apr 2026
   auto mon = addBusinessDays(fri, 1, {});              // Mon 13 Apr 2026
   assert(mon.format("YYYY-MM-DD") == "2026-04-13");

   auto easterMon = m::utcFromDate(2026, 3, 6);         // Mon 6 Apr 2026
   auto goodFri   = m::utcFromDate(2026, 3, 3);         // Fri 3 Apr 2026
   auto afterHol  = addBusinessDays(goodFri, 1, {easterMon});
   assert(afterHol.format("YYYY-MM-DD") == "2026-04-07");

Step 4 — subtract across a weekend
----------------------------------

Because the loop respects the direction sign, negative ``n`` walks
backward:

.. code-block:: cpp

   auto mon   = m::utcFromDate(2026, 3, 13);            // Mon 13 Apr
   auto prev  = addBusinessDays(mon, -1, {});           // Fri 10 Apr
   assert(prev.isSame(m::utcFromDate(2026, 3, 10), "day"));

What you learned
----------------

- :cpp:func:`day() <polycpp::moment::Moment::day>` gives 0=Sunday,
  6=Saturday — a clean weekend test.
- :cpp:func:`isSame(other, "day")
  <polycpp::moment::Moment::isSame>` makes
  hour/minute/second irrelevant when matching calendar dates.
- Always :cpp:func:`clone() <polycpp::moment::Moment::clone>` before
  mutating a caller-supplied Moment — the class mutates in place by
  design.

Next: :doc:`locale-formatting` covers switching locales for month /
weekday names and macro-token output.
