Construction and parsing
========================

The free functions in this page are the idiomatic way to create a
:cpp:class:`Moment <polycpp::moment::Moment>`. Prefer the factory
matching your input shape over calling the constructors directly —
the factories pick the right mode (local vs. UTC, fixed-offset
preservation) for you.

Current time
------------

.. doxygenfunction:: polycpp::moment::now
.. doxygenfunction:: polycpp::moment::utcNow
.. doxygenfunction:: polycpp::moment::nowMs

Parse from string
-----------------

.. doxygenfunction:: polycpp::moment::parse(const std::string&)
.. doxygenfunction:: polycpp::moment::parse(const std::string&, const std::string&)
.. doxygenfunction:: polycpp::moment::parse(const std::string&, const std::string&, bool)
.. doxygenfunction:: polycpp::moment::parse(const std::string&, const std::vector<std::string>&)
.. doxygenfunction:: polycpp::moment::parseTwoDigitYear

.. doxygenfunction:: polycpp::moment::utcFromString
.. doxygenfunction:: polycpp::moment::utcFromFormat

.. doxygenfunction:: polycpp::moment::parseZone(const std::string&)
.. doxygenfunction:: polycpp::moment::parseZone(const std::string&, const std::string&)

From components
---------------

Use these when the input is already structured and should not go
through string parsing. Component overloads use Moment.js month
numbering, where January is ``0``. ``polycpp::Date`` overloads copy
the timestamp from an existing JavaScript-style date object.

.. doxygenfunction:: polycpp::moment::fromDate(const polycpp::Date&)
.. doxygenfunction:: polycpp::moment::fromDate(int, int, int, int, int, int, int)
.. doxygenfunction:: polycpp::moment::utcFromDate(const polycpp::Date&)
.. doxygenfunction:: polycpp::moment::utcFromDate(int, int, int, int, int, int, int)
.. doxygenfunction:: polycpp::moment::fromObject(const polycpp::JsonObject&)
.. doxygenfunction:: polycpp::moment::fromObject(const polycpp::JsonValue&)
.. doxygenfunction:: polycpp::moment::utcFromObject(const polycpp::JsonObject&)
.. doxygenfunction:: polycpp::moment::utcFromObject(const polycpp::JsonValue&)

From timestamps
---------------

.. doxygenfunction:: polycpp::moment::fromUnixTimestamp
.. doxygenfunction:: polycpp::moment::fromMilliseconds
.. doxygenfunction:: polycpp::moment::utcFromMs

Special values
--------------

.. doxygenfunction:: polycpp::moment::invalid()
.. doxygenfunction:: polycpp::moment::invalid(const MomentParsingFlags&)

Aggregation
-----------

.. doxygenfunction:: polycpp::moment::min(std::initializer_list<Moment>)
.. doxygenfunction:: polycpp::moment::max(std::initializer_list<Moment>)
.. doxygenfunction:: polycpp::moment::min(const std::vector<Moment>&)
.. doxygenfunction:: polycpp::moment::max(const std::vector<Moment>&)
