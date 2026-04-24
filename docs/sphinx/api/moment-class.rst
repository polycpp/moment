Moment class
============

:cpp:class:`polycpp::moment::Moment` is the main data type — a
millisecond-precision instant in time plus UTC/local mode awareness.
Manipulation methods mutate in place and return ``*this`` for
chaining; use :cpp:func:`clone() <polycpp::moment::Moment::clone>`
when you need an independent copy.

.. doxygenclass:: polycpp::moment::Moment
   :members:
