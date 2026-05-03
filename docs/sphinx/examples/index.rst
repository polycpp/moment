Examples
========

Self-contained programs exercising the main features of moment. Each
example compiles against the public API only — no private headers, no
non-exported targets.

Choose an example
-----------------

.. list-table::
   :header-rows: 1

   * - Program
     - CMake target
     - Source
     - Use case
     - APIs demonstrated
   * - :doc:`countdown`
     - ``countdown``
     - `countdown.cpp <https://raw.githubusercontent.com/polycpp/moment/master/examples/countdown.cpp>`__
     - Print one fixed event date and a relative countdown
     - ``utcFromDate``, ``format``, ``fromNow``
   * - :doc:`standup-summary`
     - ``standup_summary``
     - `standup_summary.cpp <https://raw.githubusercontent.com/polycpp/moment/master/examples/standup_summary.cpp>`__
     - Bucket timestamp streams by local calendar day
     - ``parse``, ``local``, ``format``, ``min``, ``max``
   * - :doc:`log-time-report`
     - ``log_time_report``
     - `log_time_report.cpp <https://raw.githubusercontent.com/polycpp/moment/master/examples/log_time_report.cpp>`__
     - Sum ISO 8601 duration strings from stdin
     - ``Duration``, ``isValid``, ``add``, ``humanize``

.. toctree::
   :maxdepth: 1

   countdown
   standup-summary
   log-time-report

Running an example
------------------

From the repository root:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_MOMENT_BUILD_EXAMPLES=ON
   cmake --build build --target countdown
   ./build/examples/countdown

Use ``standup_summary`` or ``log_time_report`` in the build target and
binary path to run those examples instead.

Examples are intentionally small. Use them as API sketches, then move
shared logic into your own tests where expected timezones and locale
settings are controlled.
