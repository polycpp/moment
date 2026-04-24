Duration
========

A :cpp:class:`polycpp::moment::Duration` represents a length of time
rather than a specific instant. Like :cpp:class:`Moment
<polycpp::moment::Moment>`, durations mutate in place and chain.

Duration class
--------------

.. doxygenclass:: polycpp::moment::Duration
   :members:

Input helper
------------

.. doxygenstruct:: polycpp::moment::DurationInput
   :members:

Factory functions
-----------------

.. doxygenfunction:: polycpp::moment::duration(int64_t)
.. doxygenfunction:: polycpp::moment::duration(int, const std::string&)
.. doxygenfunction:: polycpp::moment::duration(const std::string&)
.. doxygenfunction:: polycpp::moment::duration(const DurationInput&)
.. doxygenfunction:: polycpp::moment::duration(const polycpp::JsonObject&)
