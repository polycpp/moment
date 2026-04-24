API reference
=============

The complete public surface of moment, generated from the Doxygen
comments in the headers. If a symbol is missing here, either it's internal
(under a ``detail`` namespace) or its header comment needs more love —
please open an issue.

The library has two main data types — :cpp:class:`Moment
<polycpp::moment::Moment>` (a specific instant in time) and
:cpp:class:`Duration <polycpp::moment::Duration>` (a length of time)
— along with locale and unit-alias support functions. The pages below
group the public API by task rather than by header.

Module index
------------

.. toctree::
   :maxdepth: 1

   construction
   moment-class
   manipulation
   display
   duration
   locale
   units

Namespace overview
------------------

.. doxygennamespace:: polycpp::moment
   :desc-only:
