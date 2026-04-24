moment
======

**Date/time formatting and manipulation for polycpp**

A C++ port of Moment.js with the same chainable API — construct from strings, components, or timestamps; manipulate years, months, days, or durations; format with Moment tokens or locale macros; humanise relative time; and round-trip through JSON.

.. code-block:: cpp

   #include <iostream>
   #include <polycpp/moment/moment.hpp>

   int main() {
       namespace m = polycpp::moment;

       auto due = m::utcFromDate(2026, 11, 25, 9, 0, 0);      // 25 Dec 2026 09:00 UTC
       due.subtract(3, "days").endOf("day");

       std::cout << due.format("dddd, MMMM Do YYYY [at] HH:mm")
                 << '\n'
                 << due.fromNow() << '\n';
   }

.. grid:: 2

   .. grid-item-card:: Drop-in familiarity
      :margin: 1

      Mirrors Moment.js — now(), parse(), format(), add(), diff(), humanize(), plus the full format token set (YYYY/MM/DD/HH/mm/ss, LT/L/LL/LLL/LLLL, escaped [brackets], etc.).

   .. grid-item-card:: C++20 native
      :margin: 1

      Header-only where possible, zero-overhead abstractions, ``constexpr``
      and ``std::string_view`` throughout.

   .. grid-item-card:: Tested
      :margin: 1

      Ported from Moment.js's own test suites — hundreds of assertions across parsing, formatting, locale data, duration arithmetic, and ISO 8601 / RFC 2822 edge cases.

   .. grid-item-card:: Plays well with polycpp
      :margin: 1

      Uses the same JSON value, error, and typed-event types as the rest of
      the polycpp ecosystem — no impedance mismatch.

Getting started
---------------

.. code-block:: bash

   # With FetchContent (recommended)
   FetchContent_Declare(
       polycpp_moment
       GIT_REPOSITORY https://github.com/polycpp/moment.git
       GIT_TAG        master
   )
   FetchContent_MakeAvailable(polycpp_moment)
   target_link_libraries(my_app PRIVATE polycpp::moment)

:doc:`Installation <getting-started/installation>` · :doc:`Quickstart <getting-started/quickstart>` · :doc:`Tutorials <tutorials/index>` · :doc:`API reference <api/index>`

.. toctree::
   :hidden:
   :caption: Getting started

   getting-started/installation
   getting-started/quickstart

.. toctree::
   :hidden:
   :caption: Tutorials

   tutorials/index

.. toctree::
   :hidden:
   :caption: How-to guides

   guides/index

.. toctree::
   :hidden:
   :caption: API reference

   api/index

.. toctree::
   :hidden:
   :caption: Examples

   examples/index
