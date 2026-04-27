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
<polycpp::moment::relativeTimeThreshold>` and rounding with
:cpp:func:`relativeTimeRounding <polycpp::moment::relativeTimeRounding>`.

:cpp:func:`calendarFormat <polycpp::moment::calendarFormat>` exposes
the Moment.js calendar key selection helper directly.

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
:cpp:func:`Moment::toDate <polycpp::moment::Moment::toDate>` returns
a ``polycpp::Date`` copy, and
:cpp:func:`Moment::inspect <polycpp::moment::Moment::inspect>` returns
an eval-like diagnostic string.

Low-level formatting API
------------------------

These free functions back :cpp:func:`Moment::format
<polycpp::moment::Moment::format>` and are available for tooling
that doesn't have a :cpp:class:`Moment <polycpp::moment::Moment>` in
hand:

.. doxygenfunction:: polycpp::moment::formatMoment
.. doxygenfunction:: polycpp::moment::defaultFormat
.. doxygenfunction:: polycpp::moment::calendarFormat

HTML5 format constants
----------------------

``polycpp::moment::HTML5_FMT`` mirrors upstream ``moment.HTML5_FMT``.
The lowercase ``polycpp::moment::html5_fmt`` namespace alias is also
available for C++-style call sites.

============================== ===========================
Constant                       Format string
============================== ===========================
``DATETIME_LOCAL``             ``YYYY-MM-DDTHH:mm``
``DATETIME_LOCAL_SECONDS``     ``YYYY-MM-DDTHH:mm:ss``
``DATETIME_LOCAL_MS``          ``YYYY-MM-DDTHH:mm:ss.SSS``
``DATE``                       ``YYYY-MM-DD``
``TIME``                       ``HH:mm``
``TIME_SECONDS``               ``HH:mm:ss``
``TIME_MS``                    ``HH:mm:ss.SSS``
``WEEK``                       ``GGGG-[W]WW``
``MONTH``                      ``YYYY-MM``
============================== ===========================

Format tokens
-------------

:cpp:func:`format() <polycpp::moment::Moment::format>` accepts the
Moment.js tokens implemented by this port:

========================= ==========================================
Token                     Output
========================= ==========================================
``Y``                     Expanded signed year when needed
``YYYYYY`` / ``YYYYY``    Signed 6- / 5-digit year
``YYYY`` / ``YY``         4- and 2-digit year
``N`` / ``NN`` / ``NNN``  Era abbreviation
``NNNN`` / ``NNNNN``      Full / narrow era label
``y`` / ``yy``            Era year / zero-padded era year
``yyy`` / ``yyyy``        3- / 4-digit zero-padded era year
``yo``                    Era year ordinal
``Q``                     Quarter (1-4)
``M`` / ``MM``            Month number (1-12, zero-padded)
``MMM`` / ``MMMM``        Short / full month name (locale)
``D`` / ``DD`` / ``Do``   Day of month (with ordinal suffix)
``DDD`` / ``DDDD``        Day of year
``DDDo``                  Day-of-year ordinal
``d`` / ``dd`` / ``ddd``  Day-of-week (number / min / short)
``dddd``                  Full weekday name
``do``                    Day-of-week ordinal
``e`` / ``E``             Locale / ISO weekday
``w`` / ``ww`` / ``wo``   Locale week
``W`` / ``WW`` / ``Wo``   ISO week
``g`` / ``gg``            Locale week-year
``gggg`` / ``ggggg``      Locale week-year, padded
``G`` / ``GG``            ISO week-year
``GGGG`` / ``GGGGG``      ISO week-year, padded
``H`` / ``HH``            24-hour, 0-23
``h`` / ``hh``            12-hour, 1-12
``k`` / ``kk``            24-hour, 1-24
``Hmm`` / ``Hmmss``       Compact 24-hour time
``hmm`` / ``hmmss``       Compact 12-hour time
``m`` / ``mm``            Minute
``s`` / ``ss``            Second
``S`` ... ``SSSSSSSSS``   Fractional second (1-9 digits)
``a`` / ``A``             Lower/upper meridiem
``Z`` / ``ZZ``            ``+07:00`` / ``+0700``
``z`` / ``zz``            Zone abbreviation / name
``X`` / ``x``             Unix seconds / milliseconds
``[text]``                Literal escaped text
``\X``                    Backslash-escaped next character
``LT`` ``LTS``            Locale time (short / with seconds)
``L`` ``LL`` ``LLL``      Locale date (numeric / long / with time)
``LLLL``                  Full locale date + time
========================= ==========================================

Unknown tokens render as literal text.
