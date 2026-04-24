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

.. doxygenfunction:: polycpp::moment::utcFromString
.. doxygenfunction:: polycpp::moment::utcFromFormat

.. doxygenfunction:: polycpp::moment::parseZone(const std::string&)
.. doxygenfunction:: polycpp::moment::parseZone(const std::string&, const std::string&)

From components
---------------

.. doxygenfunction:: polycpp::moment::fromDate
.. doxygenfunction:: polycpp::moment::utcFromDate
.. doxygenfunction:: polycpp::moment::fromObject
.. doxygenfunction:: polycpp::moment::utcFromObject

From timestamps
---------------

.. doxygenfunction:: polycpp::moment::fromUnixTimestamp
.. doxygenfunction:: polycpp::moment::fromMilliseconds
.. doxygenfunction:: polycpp::moment::utcFromMs

Special values
--------------

.. doxygenfunction:: polycpp::moment::invalid

Aggregation
-----------

.. doxygenfunction:: polycpp::moment::min(std::initializer_list<Moment>)
.. doxygenfunction:: polycpp::moment::max(std::initializer_list<Moment>)
.. doxygenfunction:: polycpp::moment::min(const std::vector<Moment>&)
.. doxygenfunction:: polycpp::moment::max(const std::vector<Moment>&)
