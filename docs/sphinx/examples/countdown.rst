Countdown to a fixed event
==========================

A four-line program that prints a formatted event date and a
human-readable relative phrase — handy as a shell prompt helper or a
CI notice.

Use this when you need a single known event expressed as both an
absolute date and relative text. The event is created with
``utcFromDate`` so the display is stable across machines. Month
components are zero-based, so September is ``8``.

.. literalinclude:: ../../../examples/countdown.cpp
   :language: cpp
   :linenos:

What to notice
--------------

- ``format("LLLL")`` uses the active locale's long date/time macro.
- ``fromNow()`` is evaluated at runtime, so the exact wording changes
  as the event approaches.
- The example uses only public headers and the ``polycpp::moment``
  namespace alias.

Build and run:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_MOMENT_BUILD_EXAMPLES=ON
   cmake --build build --target countdown
   ./build/examples/countdown
