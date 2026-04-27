Per-day commit summary
======================

Reads ISO 8601 timestamps on stdin (one per line), buckets them by
local date, and prints the count plus first/last commit time per day.

Use this when you need to group timestamp streams into local calendar
days, such as activity summaries, audit logs, or report exports.

.. literalinclude:: ../../../examples/standup_summary.cpp
   :language: cpp
   :linenos:

What to notice
--------------

- ``parse`` accepts ISO 8601 timestamps and returns invalid for bad
  lines.
- ``local`` converts the parsed instant to local display mode before
  bucketing.
- ``min`` and ``max`` make first/last timestamp selection independent
  of input order.

Build and run:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_MOMENT_BUILD_EXAMPLES=ON
   cmake --build build --target standup_summary
   git log --since='1 week ago' --format='%aI' | ./build/examples/standup_summary
