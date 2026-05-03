Locale
======

Locale data drives month and weekday names, relative-time wording,
calendar templates, week numbering, ordinal suffixes, meridiem
labels, era definitions, and pre/post formatting transforms. The
generated Moment.js locale corpus is registered by default, and the
registry protects reads and writes with a shared mutex.

Thread-safety and global state
------------------------------

Locale data, the current global locale key, relative-time thresholds,
and the relative-time rounding callback live in a process-global
registry. Access is mutex-protected, including writes through
:cpp:func:`defineLocale <polycpp::moment::defineLocale>`,
:cpp:func:`updateLocale <polycpp::moment::updateLocale>`,
:cpp:func:`globalLocale <polycpp::moment::globalLocale>`,
:cpp:func:`relativeTimeThreshold
<polycpp::moment::relativeTimeThreshold>`, and
:cpp:func:`relativeTimeRounding
<polycpp::moment::relativeTimeRounding>`.

That synchronization prevents unsynchronized registry access; it does
not make these settings request-local or thread-local. Configure
global locale, custom locales, thresholds, and rounding at startup or
another controlled initialization point. Use
:cpp:func:`Moment::locale <polycpp::moment::Moment::locale>` or
:cpp:func:`Duration::locale <polycpp::moment::Duration::locale>` when
one value needs a different locale.

For compatibility context, see :doc:`../guides/compatibility`.

Data structures
---------------

.. doxygenstruct:: polycpp::moment::LocaleData
   :members:

.. doxygenstruct:: polycpp::moment::LongDateFormats
   :members:

.. doxygenstruct:: polycpp::moment::CalendarFormats
   :members:

.. doxygenstruct:: polycpp::moment::RelativeTimeFormats
   :members:

.. doxygenstruct:: polycpp::moment::RelativeTimeThresholds
   :members:

.. doxygenstruct:: polycpp::moment::WeekConfig
   :members:

.. doxygenstruct:: polycpp::moment::EraSpec
   :members:

.. doxygenstruct:: polycpp::moment::LocaleParseEntry
   :members:

Callbacks
---------

.. doxygentypedef:: polycpp::moment::OrdinalFn
.. doxygentypedef:: polycpp::moment::MeridiemFn
.. doxygentypedef:: polycpp::moment::IsPMFn
.. doxygentypedef:: polycpp::moment::MeridiemHourFn
.. doxygentypedef:: polycpp::moment::RelativeTimeFn
.. doxygentypedef:: polycpp::moment::RelativeTimeRoundingFn
.. doxygentypedef:: polycpp::moment::RelativeTimeValue
.. doxygentypedef:: polycpp::moment::CalendarFn
.. doxygentypedef:: polycpp::moment::CalendarValue
.. doxygentypedef:: polycpp::moment::PrePostFormatFn
.. doxygentypedef:: polycpp::moment::EraYearOrdinalParseFn

Registry
--------

.. doxygenfunction:: polycpp::moment::defineLocale
.. doxygenfunction:: polycpp::moment::updateLocale
.. doxygenfunction:: polycpp::moment::localeData
.. doxygenfunction:: polycpp::moment::locales
.. doxygenfunction:: polycpp::moment::globalLocale()
.. doxygenfunction:: polycpp::moment::globalLocale(const std::string&)

Month and Weekday Listers
-------------------------

.. doxygenfunction:: polycpp::moment::months()
.. doxygenfunction:: polycpp::moment::months(int)
.. doxygenfunction:: polycpp::moment::monthsShort()
.. doxygenfunction:: polycpp::moment::monthsShort(int)
.. doxygenfunction:: polycpp::moment::weekdays()
.. doxygenfunction:: polycpp::moment::weekdays(int)
.. doxygenfunction:: polycpp::moment::weekdays(bool)
.. doxygenfunction:: polycpp::moment::weekdaysShort()
.. doxygenfunction:: polycpp::moment::weekdaysMin()

Thresholds
----------

.. doxygenfunction:: polycpp::moment::relativeTimeThreshold(const std::string&, double)
.. doxygenfunction:: polycpp::moment::relativeTimeThreshold(const std::string&)
.. doxygenfunction:: polycpp::moment::relativeTimeRounding(RelativeTimeRoundingFn)
.. doxygenfunction:: polycpp::moment::relativeTimeRounding()
