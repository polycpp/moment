Format a single Moment in a different locale
============================================

**When to reach for this:** the global locale is set once at
startup, but a specific call site needs output in another language.

Use :cpp:func:`Moment::locale <polycpp::moment::Moment::locale>` on a
clone so the caller's value is not mutated:

.. code-block:: cpp

   namespace m = polycpp::moment;

   std::string formatIn(const m::Moment& when, const std::string& key,
                        const std::string& fmt) {
       auto localized = when.clone();
       localized.locale(key);
       return localized.format(fmt);
   }

Generated Moment.js locales are registered by default. Unknown keys
leave the clone's locale unchanged, so ``formatIn(when, "missing",
...)`` safely falls back to the original locale.

Use :cpp:func:`globalLocale <polycpp::moment::globalLocale>` only
when you want to change the default captured by newly-created moments
and durations.

For the multi-locale tutorial, see :doc:`../tutorials/locale-formatting`.
