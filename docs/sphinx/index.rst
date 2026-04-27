moment
======

**Date/time formatting and manipulation for polycpp**

A C++ port of Moment.js with a typed, chainable API — construct from
strings, components, or timestamps; manipulate years, months, days, or
durations; format with Moment tokens or locale macros; humanize relative
time; and round-trip through JSON.

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

      Mirrors Moment.js — now(), parse(), format(), add(), diff(), humanize(), plus Moment-style format tokens (YYYY/MM/DD/HH/mm/ss, LT/L/LL/LLL/LLLL, escaped [brackets], etc.).

   .. grid-item-card:: C++20 native
      :margin: 1

      Uses typed factories, enums, structs, and callbacks around Moment-style
      behavior instead of JavaScript's dynamic overload surface.

   .. grid-item-card:: Tested
      :margin: 1

      Ported from Moment.js's own test suites — hundreds of assertions across parsing, formatting, locale data, duration arithmetic, and ISO 8601 / RFC 2822 edge cases.

   .. grid-item-card:: Plays well with polycpp
      :margin: 1

      Uses the same JSON value and error types as the rest of the polycpp
      ecosystem, so parsed dates, object inputs, and failures stay in the same
      type system as the calling code.

Find your use case
------------------

.. list-table::
   :header-rows: 1

   * - Need
     - Start here
     - APIs to look for
   * - Add Moment-style date/time handling to a project
     - :doc:`getting-started/installation`
     - CMake target ``polycpp::moment``
   * - Parse user input safely
     - :doc:`guides/choose-input-factory`
     - ``parse``, ``utcFromString``, ``parseZone``, ``isValid``
   * - Enforce one exact input format
     - :doc:`guides/parse-strict`
     - ``parse(input, format, true)``
   * - Store or exchange date/time values
     - :doc:`guides/iso-roundtrip`
     - ``toISOString``, ``toJSON``, ``Duration::toISOString``
   * - Compute reporting windows
     - :doc:`guides/start-of-week`
     - ``startOf``, ``endOf``, locale week config
   * - Format for a user's locale
     - :doc:`tutorials/locale-formatting`
     - ``locale``, ``globalLocale``, ``months``, ``weekdays``
   * - Add business-day behavior
     - :doc:`tutorials/business-days`
     - ``add``, ``day``, ``isSame``, ``clone``
   * - Sum or display elapsed time
     - :doc:`tutorials/durations-and-diff`
     - ``diff``, ``Duration``, ``humanize``
   * - Study runnable programs
     - :doc:`examples/index`
     - Countdown, log totals, per-day summaries

Getting started
---------------

.. code-block:: cmake

   # With FetchContent (recommended)
   include(FetchContent)

   FetchContent_Declare(
       polycpp_moment
       GIT_REPOSITORY https://github.com/polycpp/moment.git
       GIT_TAG        master
   )
   FetchContent_MakeAvailable(polycpp_moment)

   add_executable(my_app main.cpp)
   target_link_libraries(my_app PRIVATE polycpp::moment)

:doc:`Installation <getting-started/installation>` · :doc:`Quickstart <getting-started/quickstart>` · :doc:`How-to guides <guides/index>` · :doc:`Tutorials <tutorials/index>` · :doc:`Examples <examples/index>` · :doc:`API reference <api/index>`

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
