Locale
======

Locale data drives month and weekday names, relative-time wording,
calendar templates, week numbering, and ordinal suffixes. The
registry is global and thread-safe for concurrent reads.

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

.. doxygenstruct:: polycpp::moment::WeekConfig
   :members:

Callbacks
---------

.. doxygentypedef:: polycpp::moment::OrdinalFn
.. doxygentypedef:: polycpp::moment::MeridiemFn
.. doxygentypedef:: polycpp::moment::IsPMFn
.. doxygentypedef:: polycpp::moment::RelativeTimeFn
.. doxygentypedef:: polycpp::moment::RelativeTimeValue
.. doxygentypedef:: polycpp::moment::CalendarFn
.. doxygentypedef:: polycpp::moment::CalendarValue
.. doxygentypedef:: polycpp::moment::PrePostFormatFn

Registry
--------

.. doxygenfunction:: polycpp::moment::defineLocale
.. doxygenfunction:: polycpp::moment::updateLocale
.. doxygenfunction:: polycpp::moment::localeData
.. doxygenfunction:: polycpp::moment::locales
.. doxygenfunction:: polycpp::moment::globalLocale()
.. doxygenfunction:: polycpp::moment::globalLocale(const std::string&)

Thresholds
----------

.. doxygenfunction:: polycpp::moment::relativeTimeThreshold(const std::string&, double)
.. doxygenfunction:: polycpp::moment::relativeTimeThreshold(const std::string&)
