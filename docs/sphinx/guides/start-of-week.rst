Snap to the start or end of a period
====================================

**When to reach for this:** reporting windows, rate-limit buckets,
or date-range queries that must begin at midnight on a specific
boundary.

:cpp:func:`startOf <polycpp::moment::Moment::startOf>` and
:cpp:func:`endOf <polycpp::moment::Moment::endOf>` snap forward and
backward through the hierarchy of units:

.. code-block:: cpp

   namespace m = polycpp::moment;
   auto now = m::utcNow();

   auto dayStart  = now.clone().startOf("day");      // 00:00:00.000
   auto weekStart = now.clone().startOf("week");     // locale-aware Sunday or Monday
   auto monthEnd  = now.clone().endOf("month");      // last day, 23:59:59.999

Week handling follows the active locale —
:cpp:func:`globalLocale <polycpp::moment::globalLocale>` switches
the first day, and :cpp:member:`WeekConfig::dow
<polycpp::moment::WeekConfig::dow>` inside a custom
:cpp:class:`LocaleData <polycpp::moment::LocaleData>` picks the
day-of-week index.

For ISO-weeks specifically, ``startOf("isoWeek")`` is independent of
locale — always Monday.

.. code-block:: cpp

   auto quarterly = now.clone().startOf("quarter");
   auto yearly    = now.clone().startOf("year");

Remember to :cpp:func:`clone <polycpp::moment::Moment::clone>` —
these methods mutate in place.
