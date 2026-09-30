# twf (The Web Framework)

A modern, fast, full-stack **C++23** web framework supporting both native backend services and WebAssembly frontends.

---

## Features

- **Modern C++23**: Built with coroutines, `std::expected` for zero-cost error handling, and `std::format`.
- **Zero-Copy I/O**: Efficient memory buffers with non-allocating `std::string_view` slicing.
- **Express-like Routing**: Intuitive, expressive API for defining endpoints and middleware.
- **Isomorphic Architecture**: Share data structures and validation logic between backend and frontend.
- **Blazing Fast**: Compiles directly to native machine code on the backend and WebAssembly in the browser.

---

## Requirements

- **C++ Compiler**: Clang 18+ or GCC 14+ with C++23 support
- **Build System**: [CMake](https://cmake.org/) (3.25+) and [Ninja](https://ninja-build.org/)

---

## Quick Start

### 1. Build the Framework & Examples

```bash
# Configure the build with Ninja and Clang
cmake -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang

# Compile
cmake --build build
```

### 2. Run the Example Server

```bash
./build/examples/hello_server/hello_server.exe
```

Open your browser at **`http://localhost:8080/`**.

---

## Code Example

```cpp
#include "twf/server/server.hpp"

int main() {
    twf::Server app;

    // HTML route
    app.get("/", [](const twf::Request&) {
        return twf::Response::html("<h1>Hello from twf (C++23)!</h1>");
    });

    // JSON API route
    app.get("/api/status", [](const twf::Request&) {
        return twf::Response::json(R"({"status":"ok","framework":"twf"})");
    });

    // Query parameters (/greet?name=Mehmet)
    app.get("/greet", [](const twf::Request& req) {
        auto name = req.get_query("name").value_or("World");
        return twf::Response::text("Hello, " + name + "!");
    });

    // Start listening on port 8080
    app.listen("127.0.0.1", 8080);
    return 0;
}
```

---

## Project Structure

```
twf/
├── CMakeLists.txt        # Root CMake configuration
├── include/twf/
│   ├── core/             # Zero-copy buffer and core types
│   ├── server/           # HTTP server, request/response, router
│   └── shared/           # Shared models & contracts (isomorphic)
├── src/                  # Implementation files
└── examples/             # Sample applications
```

---

## License

MIT License.
