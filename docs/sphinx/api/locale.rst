Locale
======

Locale data drives month and weekday names, relative-time wording,
calendar templates, week numbering, ordinal suffixes, meridiem
labels, era definitions, and pre/post formatting transforms. The
generated Moment.js locale corpus is registered by default, and the
registry remains thread-safe for concurrent reads.

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
