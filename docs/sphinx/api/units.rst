Units
=====

Every string accepted by :cpp:func:`Moment::add
<polycpp::moment::Moment::add>`, :cpp:func:`Moment::subtract
<polycpp::moment::Moment::subtract>`, :cpp:func:`Moment::get
<polycpp::moment::Moment::get>`, :cpp:func:`Moment::set
<polycpp::moment::Moment::set>`, :cpp:func:`Moment::startOf
<polycpp::moment::Moment::startOf>`, :cpp:func:`Moment::endOf
<polycpp::moment::Moment::endOf>`, :cpp:func:`Moment::diff
<polycpp::moment::Moment::diff>`, and the :cpp:class:`Duration
<polycpp::moment::Duration>` arithmetic methods runs through
:cpp:func:`normalizeUnit <polycpp::moment::normalizeUnit>`.

Enum
----

.. doxygenenum:: polycpp::moment::Unit

Helpers
-------

.. doxygenfunction:: polycpp::moment::normalizeUnit
.. doxygenfunction:: polycpp::moment::unitToString

Accepted aliases
----------------

The normaliser accepts singular, plural, and (where unambiguous)
shorthand forms — case-sensitive for shorthand only. A non-exhaustive
cheat sheet:

====================  ===========================================
Canonical             Accepted aliases
====================  ===========================================
``year``              ``year``, ``years``, ``y``
``quarter``           ``quarter``, ``quarters``, ``Q``
``month``             ``month``, ``months``, ``M``
``week``              ``week``, ``weeks``, ``w``
``isoWeek``           ``isoWeek``, ``isoWeeks``, ``W``
``day``               ``day``, ``days``, ``d`` (day of week)
``date``              ``date``, ``dates``, ``D`` (day of month)
``hour``              ``hour``, ``hours``, ``h``
``minute``            ``minute``, ``minutes``, ``m``
``second``            ``second``, ``seconds``, ``s``
``millisecond``       ``millisecond``, ``milliseconds``, ``ms``
====================  ===========================================

Pass an unrecognised string and ``normalizeUnit`` returns
:cpp:enumerator:`Unit::Invalid <polycpp::moment::Unit::Invalid>`;
downstream callers then noop the manipulation rather than throwing.
