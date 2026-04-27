Installation
============

moment targets C++20 and builds with clang ≥ 14 or gcc ≥ 11. It depends
only on the base polycpp library and has no runtime package dependencies.

CMake FetchContent (recommended)
--------------------------------

Add the library to your ``CMakeLists.txt``:

.. code-block:: cmake

   include(FetchContent)

   FetchContent_Declare(
       polycpp_moment
       GIT_REPOSITORY https://github.com/polycpp/moment.git
       GIT_TAG        master
   )
   FetchContent_MakeAvailable(polycpp_moment)

   add_executable(my_app main.cpp)
   target_link_libraries(my_app PRIVATE polycpp::moment)

The first configure pulls ``polycpp`` transitively, so the build tree may be
large. Pin ``GIT_TAG`` to a specific commit for reproducible builds.

Using a local clone
-------------------

If you already have moment and polycpp checked out side by side, tell
CMake to use them instead of fetching from GitHub:

.. code-block:: bash

   cmake -B build -G Ninja \
       -DPOLYCPP_SOURCE_DIR=<repo path>/../polycpp \
       -DFETCHCONTENT_SOURCE_DIR_POLYCPP_MOMENT=<repo path>

Use the same pattern for local development, CI jobs that mount sibling
checkouts, or package-manager builds that provide polycpp separately.

Build options
-------------

``POLYCPP_MOMENT_BUILD_TESTS``
    Build the GoogleTest suite. Defaults to ``ON`` for standalone builds and
    can be disabled by consumers.

``POLYCPP_MOMENT_BUILD_EXAMPLES``
    Build example programs under ``examples/``. Defaults to ``OFF``.

``POLYCPP_MOMENT_ENABLE_ALL_LOCALES``
    Compile and register the generated Moment.js locale corpus. Defaults to
    ``ON`` when ``POLYCPP_MOMENT_LOCALES`` is empty; set to ``OFF`` for an
    English/custom-locale-only build.

``POLYCPP_MOMENT_LOCALES``
    Comma or semicolon separated Moment.js locale keys to compile and
    register, for example ``fr,ar,ja``. When this is non-empty it overrides
    ``POLYCPP_MOMENT_ENABLE_ALL_LOCALES``.

``POLYCPP_IO``
    ``asio`` (default) or ``libuv`` — inherited from polycpp.

``POLYCPP_SSL_BACKEND``
    ``boringssl`` (default) or ``openssl``.

``POLYCPP_UNICODE``
    ``icu`` (recommended) or ``builtin``. ICU enables the Intl surface that
    several polycpp headers pull into their public signatures.

Verifying the install
---------------------

.. code-block:: bash

   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   cmake --build build
   ctest --test-dir build --output-on-failure

All tests should pass on a supported toolchain — if they do not, open an
issue on the `repository <https://github.com/polycpp/moment/issues>`_
with the compiler version and the failing test name.
