How-to guides
=============

Short, problem-oriented recipes. Unlike the :doc:`../tutorials/index`,
guides assume you already know the basics and just want the answer to a
specific question.

Choose by task
--------------

.. list-table::
   :header-rows: 1

   * - Task
     - Guide
     - Main APIs
   * - Pick the correct parser or factory
     - :doc:`choose-input-factory`
     - ``parse``, ``utcFromString``, ``parseZone``, ``fromDate``
   * - Reject inputs that do not match one format
     - :doc:`parse-strict`
     - ``parse(input, format, true)``
   * - Check user or network input before using it
     - :doc:`validate-input`
     - ``isValid``, ``invalid``, parsing flags
   * - Port Moment.js usage to typed C++
     - :doc:`compatibility`
     - factories, zero-based months, locales, time modes
   * - Store values in a stable string form
     - :doc:`iso-roundtrip`
     - ``toISOString``, ``toJSON``
   * - Format one value in another language
     - :doc:`switch-locale`
     - ``Moment::locale``, ``globalLocale``
   * - Align a date to a reporting bucket
     - :doc:`start-of-week`
     - ``startOf``, ``endOf``, ``WeekConfig``

.. toctree::
   :maxdepth: 1

   choose-input-factory
   parse-strict
   validate-input
   compatibility
   iso-roundtrip
   switch-locale
   start-of-week
