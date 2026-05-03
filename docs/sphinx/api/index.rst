API reference
=============

The complete public surface of moment, generated from the Doxygen
comments in the headers. If a symbol is missing here, either it's internal
(under a ``detail`` namespace) or it needs a public Doxygen comment.
Please open an issue for missing public symbols.

The library has two main data types — :cpp:class:`Moment
<polycpp::moment::Moment>` (a specific instant in time) and
:cpp:class:`Duration <polycpp::moment::Duration>` (a length of time)
— along with locale and unit-alias support functions. The pages below
group the public API by task rather than by header.

The generated namespace entry is intentionally only an overview. Use
the task pages for recommended public entry points; implementation
names in ``detail`` headers are not part of the supported API.

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
