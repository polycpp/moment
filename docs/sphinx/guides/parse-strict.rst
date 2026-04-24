Parse with a specific format (strict mode)
==========================================

**When to reach for this:** the input is supposed to match exactly
one format and anything else should fail, not be best-effort parsed.

Pass ``strict=true`` to the three-argument :cpp:func:`parse
<polycpp::moment::parse>` overload:

.. code-block:: cpp

   namespace m = polycpp::moment;

   auto good = m::parse("2026-04-23", "YYYY-MM-DD", true);
   assert(good.isValid());

   auto bad  = m::parse("4/23/2026", "YYYY-MM-DD", true);
   assert(!bad.isValid());                      // no silent fallback

In non-strict mode the parser accepts reasonable variations (missing
zeros, swapped separators). Strict mode requires the input to match
the format tokens exactly, with no trailing characters.

If the input might be one of several shapes, pass a vector:

.. code-block:: cpp

   auto any = m::parse("23/04/2026",
                       {"YYYY-MM-DD", "DD/MM/YYYY", "MM-DD-YYYY"});
   assert(any.isValid());
   // The first successful format wins.

Always call :cpp:func:`isValid() <polycpp::moment::Moment::isValid>`
on the result — a failed parse returns an invalid Moment, not an
exception. See :doc:`validate-input` for the idiom.
