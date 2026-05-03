Core model and conventions
==========================

Moment keeps the Moment.js mental model but exposes it through typed C++.
These conventions are the ones most likely to affect new code.

Mutable values
--------------

:cpp:class:`Moment <polycpp::moment::Moment>` values are mutable.
Setters and manipulation methods update the instance and return
``Moment&`` for chaining:

.. code-block:: cpp

   auto start = polycpp::moment::utcFromDate(2026, 3, 27, 9, 30);
   auto end   = start.clone().add(1, "week");

Use :cpp:func:`clone() <polycpp::moment::Moment::clone>` before
chaining when the original value must stay unchanged.

Invalid values
--------------

Parse failures, invalid ``polycpp::Date`` inputs, and
:cpp:func:`invalid <polycpp::moment::invalid>` produce moments where
:cpp:func:`isValid() <polycpp::moment::Moment::isValid>` is ``false``.
Formatting an invalid value uses the active locale's invalid-date text.
ISO serialization with :cpp:func:`toISOString
<polycpp::moment::Moment::toISOString>` throws ``polycpp::RangeError``,
so validate untrusted input before storing or emitting it.

Components and tokens
---------------------

Component APIs follow Moment.js and JavaScript ``Date`` conventions:

- :cpp:func:`month() <polycpp::moment::Moment::month>` and component
  factories use zero-based months: January is ``0`` and December is ``11``.
- Format and parse tokens are human-facing: ``M``/``MM`` are ``1`` through
  ``12``.
- :cpp:func:`date() <polycpp::moment::Moment::date>` is day-of-month,
  ``1`` through ``31``.
- :cpp:func:`day() <polycpp::moment::Moment::day>` is day-of-week,
  ``0`` for Sunday through ``6`` for Saturday. Use
  :cpp:func:`weekday() <polycpp::moment::Moment::weekday>` for
  locale-aware weekdays and :cpp:func:`isoWeekday()
  <polycpp::moment::Moment::isoWeekday>` for ISO ``1`` through ``7``.

Units
-----

Public manipulation APIs take Moment-style unit strings such as
``"year"``, ``"month"``, ``"day"``, ``"date"``, ``"hour"``, and ``"ms"``.
They normalize through :cpp:func:`normalizeUnit
<polycpp::moment::normalizeUnit>` to the :cpp:enum:`Unit
<polycpp::moment::Unit>` enum. Shorthand is case-sensitive:
``"M"`` means month and ``"m"`` means minute. Unknown strings normalize to
``Unit::Invalid``; manipulation methods treat them as no-ops.

Time modes
----------

A moment stores one millisecond timestamp plus a rendering mode:

- Local mode uses ``polycpp::Date`` local-time behavior. Use
  :cpp:func:`now <polycpp::moment::now>` or
  :cpp:func:`fromDate <polycpp::moment::fromDate>`.
- UTC mode renders components in UTC. Use
  :cpp:func:`utcNow <polycpp::moment::utcNow>`, UTC component/timestamp
  factories, no-offset ``utcFromString`` inputs, or
  :cpp:func:`Moment::utc <polycpp::moment::Moment::utc>`.
- Fixed-offset mode preserves a numeric ``+HH:mm``/``-HH:mm`` offset. Use
  :cpp:func:`parseZone <polycpp::moment::parseZone>` or
  :cpp:func:`utcOffset <polycpp::moment::Moment::utcOffset>`.

Switching modes normally keeps the timestamp and changes how components
format. Passing ``keepLocalTime=true`` shifts the timestamp so the displayed
wall-clock fields stay the same.

Timezone scope
--------------

Core moment does not ship an IANA timezone database and does not resolve
named zones such as ``America/New_York``. It supports local time through
``polycpp::Date``, UTC, and fixed numeric offsets. Named-zone behavior belongs
in a separate timezone layer.

Process-level configuration
---------------------------

Locale registration, :cpp:func:`globalLocale
<polycpp::moment::globalLocale>`, :cpp:func:`relativeTimeThreshold
<polycpp::moment::relativeTimeThreshold>`, and
:cpp:func:`relativeTimeRounding <polycpp::moment::relativeTimeRounding>` are
process-level settings. Set them during application startup or restore the
previous value around tests. Use :cpp:func:`Moment::locale
<polycpp::moment::Moment::locale>` when only one value should render with a
different locale.

Next steps
----------

- :doc:`quickstart` for a complete first program.
- :doc:`../guides/choose-input-factory` for choosing local, UTC, or
  fixed-offset construction.
- :doc:`../api/units` for the full unit alias table.
