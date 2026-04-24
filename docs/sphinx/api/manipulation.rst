Manipulation
============

Every manipulation method on :cpp:class:`Moment
<polycpp::moment::Moment>` mutates the instance in place and returns
``*this``, so chaining is cheap and the ``clone()`` call is how you
preserve the original. All unit arguments are normalised through
:cpp:func:`normalizeUnit <polycpp::moment::normalizeUnit>` — see
:doc:`units`.

.. note::

   The full method list lives on :doc:`moment-class`. This page
   groups the same methods by task for readers who already know what
   the type looks like.

Add and subtract
----------------

:cpp:func:`Moment::add <polycpp::moment::Moment::add>` and
:cpp:func:`Moment::subtract <polycpp::moment::Moment::subtract>`
advance or rewind by ``amount`` units. Both accept the full alias
set; see :doc:`units`.

.. code-block:: cpp

   moment.add(1, "month").subtract(15, "days");

Align to unit boundaries
------------------------

:cpp:func:`Moment::startOf <polycpp::moment::Moment::startOf>` and
:cpp:func:`Moment::endOf <polycpp::moment::Moment::endOf>` snap to
the boundary of the given unit:

.. code-block:: cpp

   auto thisMonday = moment.clone().startOf("week");
   auto endOfYear  = moment.clone().endOf("year");

Mode toggles
------------

:cpp:func:`Moment::utc <polycpp::moment::Moment::utc>` and
:cpp:func:`Moment::local <polycpp::moment::Moment::local>` switch
rendering mode without changing the underlying timestamp. Pass
``keepLocalTime=true`` to shift the timestamp so the displayed
clock-time stays the same. :cpp:func:`Moment::utcOffset
<polycpp::moment::Moment::utcOffset>` (the setter overloads) fixes
a specific offset.

Generic accessors
-----------------

:cpp:func:`Moment::get <polycpp::moment::Moment::get>` and
:cpp:func:`Moment::set <polycpp::moment::Moment::set>` accept a
string unit alias — handy when the unit is data-driven and you
don't want a switch statement over every named accessor.

.. code-block:: cpp

   for (auto unit : {"year", "month", "date"}) {
       std::cout << unit << " = " << moment.get(unit) << '\n';
   }

Queries
-------

Comparison methods (:cpp:func:`isBefore
<polycpp::moment::Moment::isBefore>`, :cpp:func:`isAfter
<polycpp::moment::Moment::isAfter>`, :cpp:func:`isSame
<polycpp::moment::Moment::isSame>`, :cpp:func:`isBetween
<polycpp::moment::Moment::isBetween>`, :cpp:func:`isSameOrBefore
<polycpp::moment::Moment::isSameOrBefore>`,
:cpp:func:`isSameOrAfter
<polycpp::moment::Moment::isSameOrAfter>`) accept an optional unit
string — drop the sub-unit noise from the comparison without
pre-snapping the moments:

.. code-block:: cpp

   if (a.isSame(b, "day"))       { /* same calendar day */ }
   if (a.isBefore(b, "month"))   { /* strictly earlier month */ }

:cpp:func:`Moment::diff <polycpp::moment::Moment::diff>` is the
flip-side of :cpp:func:`add <polycpp::moment::Moment::add>`, taking
a second moment and returning a ``double`` in the named unit.
