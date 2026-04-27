Sum logged durations
====================

Reads ISO 8601 duration strings on stdin (``PT1H30M`` style),
accumulates them in a single :cpp:class:`Duration
<polycpp::moment::Duration>`, and prints the humanised total plus the
bubbled components.

Use this when logs, configs, or APIs already carry durations in ISO
8601 form. Invalid lines are ignored in the example; production code
would normally count or report them.

.. literalinclude:: ../../../examples/log_time_report.cpp
   :language: cpp
   :linenos:

What to notice
--------------

- ``Duration(line)`` parses each ISO 8601 duration string.
- ``isValid()`` gates untrusted input without exceptions.
- ``add`` mutates the running total, and ``hours`` / ``minutes`` /
  ``seconds`` expose bubbled components rather than a raw millisecond
  total.

Build and run:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_MOMENT_BUILD_EXAMPLES=ON
   cmake --build build --target log_time_report
   printf 'PT1H30M\nPT45M\nPT2H15M\n' | ./build/examples/log_time_report
