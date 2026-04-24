Check for parse failure
=======================

**When to reach for this:** you took a string from user input or
the network and need to know whether it parsed.

Moment factory functions never throw on malformed input — they
return a value whose :cpp:func:`isValid
<polycpp::moment::Moment::isValid>` is ``false``. Gate on it before
you use the result:

.. code-block:: cpp

   namespace m = polycpp::moment;

   auto parsed = m::parse(userInput);
   if (!parsed.isValid()) {
       throw std::invalid_argument("not a recognised date: " + userInput);
   }

Invalid Moments format as ``"Invalid date"`` by default — that's
the :cpp:member:`invalidDate
<polycpp::moment::LocaleData::invalidDate>` field on the active
locale — and propagate through chains (``add()`` on an invalid
Moment stays invalid), so you'll usually notice even if you forget
the check.

For programmatic invalids, call :cpp:func:`invalid
<polycpp::moment::invalid>`:

.. code-block:: cpp

   m::Moment sentinel = m::invalid();
   assert(!sentinel.isValid());

:cpp:class:`Duration <polycpp::moment::Duration>` follows the same
convention — construct from a malformed ISO string and
:cpp:func:`Duration::isValid
<polycpp::moment::Duration::isValid>` is ``false`` instead of an
exception.
