Locale-aware formatting
=======================

**You'll build:** date-formatting helpers that use the generated
Moment.js locale corpus, switch one value to French, and set a
process-wide default for newly-created values.

**You'll use:** :cpp:func:`locales <polycpp::moment::locales>`,
:cpp:func:`globalLocale <polycpp::moment::globalLocale>`,
:cpp:func:`defineLocale <polycpp::moment::defineLocale>`, and
:cpp:func:`Moment::locale <polycpp::moment::Moment::locale>`.

Step 1 — confirm which locales exist
------------------------------------

The generated locale corpus is registered by default:

.. code-block:: cpp

   #include <polycpp/moment/locale.hpp>

   for (const auto& key : polycpp::moment::locales())
       std::cout << key << '\n';
   // ar
   // en
   // fr
   // ...

Unknown keys still fall back to English through
:cpp:func:`localeData <polycpp::moment::localeData>`. Register a
custom key with :cpp:func:`defineLocale
<polycpp::moment::defineLocale>` when your application needs a
locale variant that Moment.js does not ship.

Step 2 — format one value in another locale
-------------------------------------------

Use :cpp:func:`Moment::locale <polycpp::moment::Moment::locale>` for
per-value output without touching the global default:

.. code-block:: cpp

   namespace m = polycpp::moment;

   auto bastille = m::utcFromDate(2026, 6, 14);  // 14 Jul 2026
   bastille.locale("fr");

   std::cout << bastille.format("LL") << '\n';     // 14 juillet 2026

The setter leaves the current key unchanged when the requested
locale is not registered.

Step 3 — set the default for new values
---------------------------------------

:cpp:func:`globalLocale <polycpp::moment::globalLocale>` controls
the locale captured by moments and durations created after the
switch:

.. code-block:: cpp

   namespace m = polycpp::moment;

   const std::string previous = m::globalLocale("fr");

   auto when = m::utcFromDate(2026, 6, 14);
   auto wait = m::duration(1, "day");

   std::cout << when.format("LL") << '\n';       // 14 juillet 2026
   std::cout << wait.humanize() << '\n';         // un jour

   m::globalLocale(previous);

Existing moments keep their own locale key. Use
:cpp:func:`Moment::locale <polycpp::moment::Moment::locale>` when
you need to switch an already-created value.

Step 4 — tweak relative-time thresholds
---------------------------------------

The relative-time thresholds (``"a few seconds ago"`` vs ``"a minute
ago"``) are global, not per-locale:

.. code-block:: cpp

   m::relativeTimeThreshold("s", 60);     // keep <60s below the minute boundary
   m::relativeTimeThreshold("m", 90);     // extend the minute boundary

Set these once at startup if your application wants non-default
humanize boundaries. For one-off duration formatting, pass a
``RelativeTimeThresholds`` value to ``Duration::humanize`` instead.

Step 5 — list localized month and weekday names
-----------------------------------------------

Use the standalone listers when building dropdowns, report headers, or
calendar grids. They read from the active global locale, matching
Moment.js's ``moment.months()`` and ``moment.weekdays()`` helpers:

.. code-block:: cpp

   m::globalLocale("fr");

   for (const auto& label : m::monthsShort()) {
       std::cout << label << ' ';
   }
   std::cout << '\n';

   for (const auto& label : m::weekdaysMin()) {
       std::cout << label << ' ';
   }
   std::cout << '\n';

Pass an index when you need a single label:

.. code-block:: cpp

   std::cout << m::months(6) << '\n';       // juillet
   std::cout << m::weekdaysShort(1) << '\n';

What you learned
----------------

- Generated Moment.js locales are available by default and appear in
  :cpp:func:`locales <polycpp::moment::locales>`.
- :cpp:func:`Moment::locale <polycpp::moment::Moment::locale>` is the
  right tool for one formatted value.
- :cpp:func:`globalLocale <polycpp::moment::globalLocale>` sets the
  default for newly-created moments and durations.
- Standalone listers such as :cpp:func:`months
  <polycpp::moment::months>` and :cpp:func:`weekdaysMin
  <polycpp::moment::weekdaysMin>` are useful for UI and report labels.
- Relative-time thresholds are global and independent of the active
  locale.

Next: :doc:`durations-and-diff` pairs :cpp:class:`Duration
<polycpp::moment::Duration>` with :cpp:func:`Moment::diff
<polycpp::moment::Moment::diff>` for timeline maths.
