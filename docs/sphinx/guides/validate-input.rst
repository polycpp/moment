Validate input and handle invalid values
========================================

**When to reach for this:** you took a string, ``JsonValue``,
``polycpp::Date``, unit name, or duration from user input or the
network and need to know what fails closed.

String parse factories do not throw on malformed input. They return a
value whose :cpp:func:`isValid
<polycpp::moment::Moment::isValid>` is ``false``. Gate on it before
you use the result:

.. code-block:: cpp

   namespace m = polycpp::moment;

   auto parsed = m::parse(userInput);
   if (!parsed.isValid()) {
       throw std::invalid_argument("not a recognised date: " + userInput);
   }

For programmatic invalids, call :cpp:func:`invalid
<polycpp::moment::invalid>`:

.. code-block:: cpp

   m::Moment sentinel = m::invalid();
   assert(!sentinel.isValid());

Invalid ``Moment`` behavior
---------------------------

An invalid :cpp:class:`Moment <polycpp::moment::Moment>` stays
invalid through chains. Mutating methods still return ``Moment&`` so
the chain compiles, but they do not make the value valid again.

Display and serialization methods are intentionally explicit:

.. list-table::
   :header-rows: 1

   * - API
     - Invalid result
   * - ``format()``, ``from()``, ``to()``, ``calendar()``
     - The active locale's :cpp:member:`invalidDate
       <polycpp::moment::LocaleData::invalidDate>` string, usually
       ``"Invalid date"``.
   * - ``toISOString()``
     - Throws ``polycpp::RangeError("Invalid time value")``.
   * - ``toJSON()``
     - ``"null"``.
   * - ``toString()``
     - ``"Invalid date"``.
   * - ``toDate()``
     - An invalid ``polycpp::Date``.
   * - ``diff()``
     - ``NaN`` when either side is invalid.
   * - Comparison predicates
     - ``false`` when either side is invalid.
   * - ``toArray()`` and ``toObject()``
     - Component snapshots, not validation signals.

Use :cpp:func:`parsingFlags
<polycpp::moment::Moment::parsingFlags>` and :cpp:func:`invalidAt
<polycpp::moment::Moment::invalidAt>` when you need diagnostics for a
failed parse. Use :cpp:func:`creationData
<polycpp::moment::Moment::creationData>` when logging which input and
format produced the value.

Invalid ``Duration`` behavior
-----------------------------

:cpp:class:`Duration <polycpp::moment::Duration>` follows the same
``isValid`` convention for malformed ISO strings and non-object
``JsonValue`` input:

.. code-block:: cpp

   m::Duration wait("not a duration");
   if (!wait.isValid()) {
       throw std::invalid_argument("invalid duration");
   }

Invalid durations do not throw from formatting methods.
``humanize()``, ``toISOString()``, ``toJSON()``, and ``toString()``
return the active locale's ``invalidDate`` string. Component getters
and ``as(...)`` conversions return ``0``.

Unknown unit strings
--------------------

Unit-taking APIs first call :cpp:func:`normalizeUnit
<polycpp::moment::normalizeUnit>`. Unknown strings become
:cpp:enumerator:`Unit::Invalid <polycpp::moment::Unit::Invalid>`.

For :cpp:class:`Moment <polycpp::moment::Moment>`:

- ``get("unknown")`` returns ``0``.
- ``set``, ``add``, ``subtract``, ``startOf``, and ``endOf`` leave the
  value unchanged for unknown units.
- Unit-granularity comparisons treat an unknown unit like raw
  millisecond comparison.

For :cpp:class:`Duration <polycpp::moment::Duration>`:

- ``Duration(1, "unknown")`` creates an invalid duration.
- ``add`` and ``subtract`` ignore unknown units on an existing duration.
- ``get("unknown")`` and ``as("unknown")`` return ``0``.

When to validate
----------------

Validate every value that crosses a trust boundary: user strings,
network payloads, file/config values, dynamic ``JsonValue`` values,
and ``polycpp::Date`` values from outside your component. Also
validate component ranges yourself when overflow normalization would
hide a bad field; component and object factories use Moment.js /
JavaScript ``Date`` month numbering and overflow rules rather than
rejecting every out-of-range integer.
