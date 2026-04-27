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
     - Use case
     - APIs demonstrated
   * - :doc:`countdown`
     - Print one fixed event date and a relative countdown
     - ``utcFromDate``, ``format``, ``fromNow``
   * - :doc:`standup-summary`
     - Bucket timestamp streams by local calendar day
     - ``parse``, ``local``, ``format``, ``min``, ``max``
   * - :doc:`log-time-report`
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
   cmake --build build --target <example_name>
   ./build/examples/<example_name>

Examples are intentionally small. Use them as API sketches, then move
shared logic into your own tests where expected timezones and locale
settings are controlled.
