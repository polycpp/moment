Sum logged durations
====================

Reads ISO 8601 duration strings on stdin (``PT1H30M`` style),
accumulates them in a single :cpp:class:`Duration
<polycpp::moment::Duration>`, and prints the humanised total plus the
bubbled components.

.. literalinclude:: ../../../examples/log_time_report.cpp
   :language: cpp
   :linenos:

Build and run:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_MOMENT_BUILD_EXAMPLES=ON
   cmake --build build --target log_time_report
   printf 'PT1H30M\nPT45M\nPT2H15M\n' | ./build/examples/log_time_report
