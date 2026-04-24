Countdown to a fixed event
==========================

A four-line program that prints a formatted event date and a
human-readable relative phrase — handy as a shell prompt helper or a
CI notice.

.. literalinclude:: ../../../examples/countdown.cpp
   :language: cpp
   :linenos:

Build and run:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_MOMENT_BUILD_EXAMPLES=ON
   cmake --build build --target countdown
   ./build/examples/countdown
