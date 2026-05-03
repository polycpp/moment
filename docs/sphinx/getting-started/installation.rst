Installation
============

moment targets C++20 and is consumed as the CMake target
``polycpp::moment``. The root ``CMakeLists.txt`` requires CMake 3.20.

CMake FetchContent (recommended)
--------------------------------

For reproducible builds, pin the moment revision you consume. Replace
``<commit-or-release-tag>`` with a release tag or full commit SHA:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.20)
   project(moment_hello LANGUAGES CXX)

   include(FetchContent)

   FetchContent_Declare(
       polycpp_moment
       GIT_REPOSITORY https://github.com/polycpp/moment.git
       GIT_TAG        <commit-or-release-tag>
   )
   FetchContent_MakeAvailable(polycpp_moment)

   add_executable(moment_hello src/main.cpp)
   target_compile_features(moment_hello PRIVATE cxx_std_20)
   target_link_libraries(moment_hello PRIVATE polycpp::moment)

Tracking a branch is useful for local experiments, but avoid branch names
such as ``master`` as the default in production projects. The first
configure pulls the base ``polycpp`` library transitively unless you provide
a local checkout; for fully reproducible application builds, control that
``polycpp`` revision too.

Minimal project
---------------

Use this shape when trying the library in a new directory:

.. code-block:: text

   moment-hello/
   |-- CMakeLists.txt
   `-- src/
       `-- main.cpp

``src/main.cpp``:

.. code-block:: cpp

   #include <iostream>
   #include <polycpp/moment/moment.hpp>

   int main() {
       namespace m = polycpp::moment;

       auto release = m::utcFromDate(2026, 3, 27, 9, 30); // Apr 27, 2026
       auto review  = release.clone().add(3, "days").hour(16);

       std::cout << "release: " << release.format("YYYY-MM-DD HH:mm [UTC]") << '\n';
       std::cout << "review : " << review.format("dddd, MMMM D [at] HH:mm [UTC]") << '\n';
   }

Configure, build, and run:

.. code-block:: bash

   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ./build/moment_hello

Representative output:

.. code-block:: text

   release: 2026-04-27 09:30 UTC
   review : Thursday, April 30 at 16:30 UTC

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

Install and build support
-------------------------

- Language standard: C++20.
- CMake minimum: 3.20, from the repository root ``CMakeLists.txt``.
- Compiler baseline discoverable in this checkout: the existing docs named
  clang 14+ and gcc 11+. The GitHub Actions workflow present here builds
  documentation only, so no broader C++ compiler or platform matrix is
  declared locally.
- Ninja is optional. The examples use ``-G Ninja`` when convenient, but any
  CMake generator supported by your environment can be used.
- Source inclusion through ``FetchContent`` or an equivalent
  ``add_subdirectory``/package-manager wrapper is the documented consumption
  path today.
- Installed-package consumption is not documented by this repository today:
  the current CMake file does not define install/export package rules for a
  ``find_package`` workflow.
- ``polycpp::moment`` links publicly to the base ``polycpp`` target. If
  ``POLYCPP_SOURCE_DIR`` is not set, this repository's fallback FetchContent
  declaration fetches ``https://github.com/enricohuang/polycpp.git`` at
  ``master``. Pin or provide that dependency yourself when your build needs a
  locked dependency graph.

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
