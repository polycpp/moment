# polycpp/moment

A C++ port of [Moment.js](https://momentjs.com/) for the [polycpp](https://github.com/enricohuang/polycpp) ecosystem.

Parse, validate, manipulate, and display dates and times in C++20 using the familiar Moment.js API.

## Prerequisites

- CMake 3.20+
- GCC 13+ or Clang 16+
- C++20 support required

## Building

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_MOMENT_BUILD_TESTS=ON
cmake --build build -j$(nproc)
```

## Running Tests

```bash
cd build && ctest --output-on-failure
```

## License

MIT License. See [LICENSE](LICENSE) for details.
