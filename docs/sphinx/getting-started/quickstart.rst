Quickstart
==========

This page walks through a minimal moment program end-to-end. Copy the
snippet, run it, then jump to :doc:`../tutorials/index` for task-oriented
walkthroughs or :doc:`../api/index` for the full reference.

We'll parse an ISO 8601 string, move forward by one business-ish
week, format with a mix of macro and raw tokens, and print the result
relative to the current wall clock — enough to hit the parser,
:cpp:class:`Moment <polycpp::moment::Moment>` manipulation, formatting,
and relative-time humanisation.

Full example
------------

.. code-block:: cpp

   #include <iostream>
   #include <polycpp/moment/moment.hpp>

   int main() {
       namespace m = polycpp::moment;

       auto kickoff = m::parse("2026-04-27T09:00:00Z");
       auto review  = kickoff.clone().add(7, "days").hour(15);

       std::cout << "kickoff : " << kickoff.format("LLLL") << '\n';
       std::cout << "review  : " << review.format("ddd, MMM D [at] HH:mm")
                 << "  (" << review.fromNow() << ")\n";
       std::cout << "gap     : "
                 << m::duration(review.diff(kickoff, "ms"))
                       .humanize(false) << '\n';
   }

Compile it with the same CMake wiring from :doc:`installation`:

.. code-block:: bash

   cmake -B build -G Ninja
   cmake --build build
   ./build/my_app

Expected output (with the current time close to 23 Apr 2026):

.. code-block:: text

   kickoff : Monday, April 27, 2026 9:00 AM
   review  : Mon, May 4 at 15:00  (in 11 days)
   gap     : 7 days

What just happened
------------------

1. :cpp:func:`polycpp::moment::parse` auto-detects ISO 8601 (and
   RFC 2822 as a fallback). A timezone offset in the input is
   honoured, so the result is in UTC mode here.

2. :cpp:func:`clone() <polycpp::moment::Moment::clone>` is important:
   :cpp:class:`Moment <polycpp::moment::Moment>` mutates in place,
   matching Moment.js, so chaining ``add(...).hour(...)`` on
   ``kickoff`` would overwrite it. Clone first, chain second.

3. The format string mixes a macro token (``LLLL`` — "Monday, April
   27, 2026 9:00 AM" for the ``en`` locale) with literal tokens
   (``ddd``, ``MMM``, ``D``) and escaped text (``[at]``). All three
   shapes compose freely — see :doc:`../api/display` for the complete
   token table.

4. :cpp:func:`fromNow() <polycpp::moment::Moment::fromNow>` returns a
   human-readable string relative to the current wall clock. The
   thresholds come from the active locale and can be adjusted with
   :cpp:func:`relativeTimeThreshold
   <polycpp::moment::relativeTimeThreshold>`.

5. :cpp:func:`diff() <polycpp::moment::Moment::diff>` gives you an
   integer-or-double in whatever unit you name. Feed that into
   :cpp:func:`duration() <polycpp::moment::duration>` and you can
   humanise it the same way.

Next steps
----------

- :doc:`../tutorials/index` — step-by-step walkthroughs of common tasks.
- :doc:`../guides/index` — short how-tos for specific problems.
- :doc:`../api/index` — every public type, function, and option.
- :doc:`../examples/index` — runnable programs you can drop into a sandbox.
