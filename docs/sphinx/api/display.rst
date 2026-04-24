Display and serialisation
=========================

Helpers for turning a :cpp:class:`Moment
<polycpp::moment::Moment>` into a string, a JSON value, or a numeric
timestamp. The method declarations live on :doc:`moment-class`; this
page is the task-oriented overview.

Formatters
----------

:cpp:func:`Moment::format <polycpp::moment::Moment::format>` is the
general-purpose token-driven formatter.
:cpp:func:`Moment::toISOString
<polycpp::moment::Moment::toISOString>` shortcuts to ISO 8601.
:cpp:func:`Moment::toString
<polycpp::moment::Moment::toString>` emits a fixed
``"Fri Mar 15 2024 14:30:45 GMT+0000"`` shape regardless of locale.

Relative and calendar
---------------------

:cpp:func:`Moment::fromNow <polycpp::moment::Moment::fromNow>`,
:cpp:func:`Moment::from <polycpp::moment::Moment::from>`,
:cpp:func:`Moment::toNow <polycpp::moment::Moment::toNow>`,
:cpp:func:`Moment::to <polycpp::moment::Moment::to>`, and
:cpp:func:`Moment::calendar <polycpp::moment::Moment::calendar>`
produce human phrasing — "3 days ago", "in 2 hours", "Today at
09:30". Configure thresholds with :cpp:func:`relativeTimeThreshold
<polycpp::moment::relativeTimeThreshold>`.

Numeric
-------

:cpp:func:`Moment::valueOf <polycpp::moment::Moment::valueOf>`
returns the raw millisecond timestamp;
:cpp:func:`Moment::unix <polycpp::moment::Moment::unix>` returns
seconds.

JSON
----

:cpp:func:`Moment::toJSON <polycpp::moment::Moment::toJSON>` is the
alias the serialisation layer should use — same output as
:cpp:func:`toISOString <polycpp::moment::Moment::toISOString>` with
no arguments. :cpp:func:`Moment::toArray
<polycpp::moment::Moment::toArray>` and
:cpp:func:`Moment::toObject <polycpp::moment::Moment::toObject>`
expose the decomposed components.

Low-level formatting API
------------------------

These free functions back :cpp:func:`Moment::format
<polycpp::moment::Moment::format>` and are available for tooling
that doesn't have a :cpp:class:`Moment <polycpp::moment::Moment>` in
hand:

.. doxygenfunction:: polycpp::moment::formatMoment
.. doxygenfunction:: polycpp::moment::defaultFormat

Format tokens
-------------

:cpp:func:`format() <polycpp::moment::Moment::format>` accepts the
full Moment.js token set:

========================= ==========================================
Token                     Output
========================= ==========================================
``YYYY`` / ``YY``         4- and 2-digit year
``Q``                     Quarter (1-4)
``M`` / ``MM``            Month number (1-12, zero-padded)
``MMM`` / ``MMMM``        Short / full month name (locale)
``D`` / ``DD`` / ``Do``   Day of month (with ordinal suffix)
``DDD`` / ``DDDD``        Day of year
``d`` / ``dd`` / ``ddd``  Day-of-week (number / min / short)
``dddd``                  Full weekday name
``e`` / ``E``             Locale / ISO weekday
``w`` / ``ww``            Locale week
``W`` / ``WW``            ISO week
``H`` / ``HH``            24-hour, 0-23
``h`` / ``hh``            12-hour, 1-12
``k`` / ``kk``            24-hour, 1-24
``m`` / ``mm``            Minute
``s`` / ``ss``            Second
``S`` / ``SS`` / ``SSS``  Fractional second (1/2/3 digits)
``a`` / ``A``             Lower/upper meridiem
``Z`` / ``ZZ``            ``+07:00`` / ``+0700``
``X`` / ``x``             Unix seconds / milliseconds
``[text]``                Literal escaped text
``LT`` ``LTS``            Locale time (short / with seconds)
``L`` ``LL`` ``LLL``      Locale date (numeric / long / with time)
``LLLL``                  Full locale date + time
========================= ==========================================

Unknown tokens render as literal text.
