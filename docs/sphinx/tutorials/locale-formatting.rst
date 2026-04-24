Locale-aware formatting
=======================

**You'll build:** a date-formatting helper that speaks English by
default, French on demand, and gracefully falls back to English if
the requested locale isn't registered.

**You'll use:** :cpp:func:`defineLocale
<polycpp::moment::defineLocale>`, :cpp:func:`globalLocale
<polycpp::moment::globalLocale>`, and the
:cpp:class:`LocaleData <polycpp::moment::LocaleData>` struct.

Step 1 — confirm which locales exist
------------------------------------

Only ``en`` is built in; everything else comes from your code:

.. code-block:: cpp

   #include <polycpp/moment/locale.hpp>
   for (const auto& key : polycpp::moment::locales())
       std::cout << key << '\n';
   // en

The fallback is permanent — even after :cpp:func:`defineLocale
<polycpp::moment::defineLocale>` adds ``fr``, looking up ``fr_CA``
via :cpp:func:`localeData <polycpp::moment::localeData>` returns
``en`` until you register ``fr_CA`` explicitly.

Step 2 — register a French locale
---------------------------------

Fill in the fields you care about; the rest fall back to the
English defaults captured by the constructor:

.. code-block:: cpp

   namespace m = polycpp::moment;

   m::LocaleData fr{};
   fr.name = "fr";
   fr.months = {
       "janvier", "fevrier", "mars", "avril", "mai", "juin",
       "juillet", "aout", "septembre", "octobre", "novembre", "decembre",
   };
   fr.monthsShort = {
       "janv.", "fevr.", "mars", "avr.", "mai", "juin",
       "juil.", "aout", "sept.", "oct.", "nov.", "dec.",
   };
   fr.weekdays = {
       "dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi"
   };
   fr.week = {.dow = 1, .doy = 4};        // Monday-start, ISO week rule
   fr.longDateFormat = {
       "HH:mm",          // LT
       "HH:mm:ss",       // LTS
       "DD/MM/YYYY",     // L
       "D MMMM YYYY",    // LL
       "D MMMM YYYY HH:mm",
       "dddd D MMMM YYYY HH:mm",
   };
   m::defineLocale("fr", fr);

Step 3 — switch locales temporarily
-----------------------------------

:cpp:func:`globalLocale <polycpp::moment::globalLocale>` returns the
*previous* key, so you can save and restore it:

.. code-block:: cpp

   std::string saveAndFormat(const m::Moment& when, const std::string& key) {
       const std::string prev = m::globalLocale(key);
       const std::string s = when.format("LL");
       m::globalLocale(prev);                 // restore for subsequent callers
       return s;
   }

   auto bastille = m::utcFromDate(2026, 6, 14);  // 14 Jul 2026
   std::cout << saveAndFormat(bastille, "en") << '\n';  // July 14, 2026
   std::cout << saveAndFormat(bastille, "fr") << '\n';  // 14 juillet 2026

If the requested key isn't registered, :cpp:func:`globalLocale
<polycpp::moment::globalLocale>` leaves the global unchanged and
returns its current value — perfect for graceful degradation.

Step 4 — tweak relative-time thresholds
---------------------------------------

The relative-time thresholds (``"a few seconds ago"`` vs ``"a minute
ago"``) are global, not per-locale:

.. code-block:: cpp

   m::relativeTimeThreshold("s", 60);     // treat <60s as "seconds ago"
   m::relativeTimeThreshold("m", 90);     // extend the minute boundary

Both thresholds survive across
:cpp:func:`globalLocale <polycpp::moment::globalLocale>` switches;
set them once at startup.

What you learned
----------------

- Only ``en`` ships; every other locale comes from your
  :cpp:func:`defineLocale <polycpp::moment::defineLocale>` calls.
- :cpp:func:`globalLocale <polycpp::moment::globalLocale>` returns
  the *previous* key, which makes the save-and-restore pattern a
  one-liner.
- Unknown keys gracefully fall back to English without throwing —
  safe for user-supplied values.
- Relative-time thresholds are global and independent of the active
  locale; set them at startup once.

Next: :doc:`durations-and-diff` pairs :cpp:class:`Duration
<polycpp::moment::Duration>` with :cpp:func:`Moment::diff
<polycpp::moment::Moment::diff>` for timeline maths.
