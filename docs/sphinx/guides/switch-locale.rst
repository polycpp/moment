Format a single Moment in a different locale
============================================

**When to reach for this:** the global locale shouldn't change (it's
set once at startup), but a specific call site needs output in a
different language.

:cpp:func:`globalLocale <polycpp::moment::globalLocale>` returns the
previous key, which makes save-and-restore a one-liner:

.. code-block:: cpp

   namespace m = polycpp::moment;

   std::string formatIn(const m::Moment& w, const std::string& key,
                        const std::string& fmt) {
       const std::string prev = m::globalLocale(key);
       const std::string out = w.format(fmt);
       m::globalLocale(prev);
       return out;
   }

The locale must already be registered via :cpp:func:`defineLocale
<polycpp::moment::defineLocale>`. Unknown keys leave the global
unchanged — :cpp:func:`globalLocale
<polycpp::moment::globalLocale>` returns its current value rather
than failing. That makes ``formatIn(m, "es", ...)`` a no-op if
``es`` wasn't defined, which is usually what you want.

For the multi-locale tutorial, see :doc:`../tutorials/locale-formatting`.
