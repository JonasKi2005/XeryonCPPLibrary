# XeryonCppLibrary

XeryonCppLibrary is a C++ library to control Xeryon motion controllers and stages. It provides a `Controller` class, `Axis` abstractions, and helpers for unit conversions and stage configuration.

## Features

- Serial communication using standalone Asio (`asio` header-only)
- Axis/Stage abstractions with unit-aware conversions
- Thread-safe command queue and asynchronous serial I/O
- CMake-based build (C++20)

## Requirements

- CMake 3.15+
- A C++20 compiler (g++/clang++ supporting C++20)
- Internet access during configure (FetchContent will download Asio) or preinstalled Asio/Boost

## Building

From the project root:

```bash
# Create a build directory and configure
mkdir -p build
cd build
# Build the library and example app (optional)
cmake -S .. -B . -DBUILD_EXAMPLES=ON
cmake --build . --config Release
```

By default the project fetches the standalone Asio headers via CMake `FetchContent`. If you prefer to use a system Boost installation instead, update `CMakeLists.txt` to use `find_package(Boost COMPONENTS system)` and change `#include <asio.hpp>` back to `#include <boost/asio.hpp>` in `Communication.hpp` / `Communication.cpp`.

## Example

A small example `main` is included (built when `-DBUILD_EXAMPLES=ON`). It demonstrates creating a `Controller`, adding axes and sending simple commands.

## Notes

- `log.txt` and the `build/` directory are ignored by `.gitignore`.
- The project uses `std::unique_ptr<Axis>` ownership inside `Controller` to keep axis instances stable.

## License

Add a `LICENSE` file to specify a license (e.g., MIT).

## Contributing

Pull requests are welcome. For major changes, open an issue first to discuss the change.
