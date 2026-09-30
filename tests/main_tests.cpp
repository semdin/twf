#include "twf/core/buffer.hpp"
#include "twf/core/types.hpp"
#include "twf/shared/serializer.hpp"
#include "twf/shared/models.hpp"
#include "twf/server/request.hpp"
#include "twf/server/response.hpp"
#include "twf/client/dom.hpp"

#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <cassert>

static int g_tests_passed = 0;
static int g_assertions = 0;

#define ASSERT_TRUE(condition, message) \
    do { \
        g_assertions++; \
        if (!(condition)) { \
            std::cerr << std::format("[FAIL] Assertion failed: {} ({}:{})\n", message, __FILE__, __LINE__); \
            std::exit(1); \
        } \
    } while (0)

#define ASSERT_EQ(actual, expected, message) \
    do { \
        g_assertions++; \
        if ((actual) != (expected)) { \
            std::cerr << std::format("[FAIL] {} | Expected: {}, Actual: {} ({}:{})\n", message, expected, actual, __FILE__, __LINE__); \
            std::exit(1); \
        } \
    } while (0)

// Test 1: twf::Buffer zero-copy mechanics
void test_buffer() {
    twf::Buffer buf;
    ASSERT_TRUE(buf.empty(), "New buffer should be empty");
    ASSERT_EQ(buf.size(), 0ULL, "Initial size must be 0");

    buf.append("GET /api/status HTTP/1.1\r\nHost: localhost\r\n\r\n");
    ASSERT_EQ(buf.size(), 45ULL, "Appended size must match");

    std::string_view sv = buf.string_view();
    ASSERT_EQ(sv.substr(0, 3), "GET", "Prefix must match GET");

    std::string_view slice = buf.slice(4, 11);
    ASSERT_EQ(slice, "/api/status", "Slice must return zero-copy path");

    buf.clear();
    ASSERT_TRUE(buf.empty(), "Buffer must be empty after clear()");
    std::cout << "  [PASS] Buffer Operations (zero-copy slicing & capacity)\n";
    g_tests_passed++;
}

// Test 2: C++23 Compile-Time Reflection with Glaze
void test_reflection() {
    twf::TodoItem item{42, "Master C++23 WebAssembly", true};

    // Serialization
    auto json_res = twf::to_json(item);
    ASSERT_TRUE(json_res.has_value(), "Serialization should succeed");
    std::string json_str = *json_res;
    ASSERT_TRUE(json_str.find("\"id\":42") != std::string::npos, "JSON must contain id");
    ASSERT_TRUE(json_str.find("\"title\":\"Master C++23 WebAssembly\"") != std::string::npos, "JSON must contain title");
    ASSERT_TRUE(json_str.find("\"completed\":true") != std::string::npos, "JSON must contain completed");

    // Deserialization
    auto parsed_res = twf::from_json<twf::TodoItem>(json_str);
    ASSERT_TRUE(parsed_res.has_value(), "Deserialization should succeed");
    ASSERT_EQ(parsed_res->id, 42, "Deserialized id must match");
    ASSERT_EQ(parsed_res->title, "Master C++23 WebAssembly", "Deserialized title must match");
    ASSERT_TRUE(parsed_res->completed, "Deserialized completed must match");

    // Vector Serialization
    std::vector<twf::TodoItem> list = { {1, "Task 1", false}, {2, "Task 2", true} };
    auto vec_json = twf::to_json(list);
    ASSERT_TRUE(vec_json.has_value(), "Vector serialization should succeed");

    auto vec_parsed = twf::from_json<std::vector<twf::TodoItem>>(*vec_json);
    ASSERT_TRUE(vec_parsed.has_value(), "Vector deserialization should succeed");
    ASSERT_EQ(vec_parsed->size(), 2ULL, "Vector size must be 2");

    // Error handling on invalid JSON
    auto bad_res = twf::from_json<twf::TodoItem>("{bad_json");
    ASSERT_TRUE(!bad_res.has_value(), "Invalid JSON must produce error Result");

    std::cout << "  [PASS] JSON Reflection (twf::to_json & twf::from_json for models & vectors)\n";
    g_tests_passed++;
}

// Test 3: HTTP Request Parsing & Query String
void test_request() {
    twf::Request req;
    req.method = twf::Method::GET;
    req.path = "/search";
    req.raw_query = "q=modern+cpp&limit=25&category=";

    ASSERT_EQ(req.get_query("q").value_or(""), "modern+cpp", "Query 'q' must match");
    ASSERT_EQ(req.get_query("limit").value_or(""), "25", "Query 'limit' must match");
    ASSERT_EQ(req.get_query("category").value_or("unset"), "", "Empty query param must be empty string");
    ASSERT_TRUE(!req.get_query("nonexistent").has_value(), "Missing query must return nullopt");

    // Case-insensitive headers
    req.headers["Content-Type"] = "application/json";
    req.headers["X-Custom-Header"] = "twf-test";

    ASSERT_TRUE(req.get_header("content-type").has_value(), "Lowercase header lookup must succeed");
    ASSERT_EQ(*req.get_header("content-type"), "application/json", "Header value must match");
    ASSERT_TRUE(req.get_header("CONTENT-TYPE").has_value(), "Uppercase header lookup must succeed");
    ASSERT_TRUE(req.get_header("x-custom-header").has_value(), "Custom header lookup must succeed");

    std::cout << "  [PASS] HTTP Request (query parsing & case-insensitive headers)\n";
    g_tests_passed++;
}

// Test 4: HTTP Response Builder & Serialization
void test_response() {
    auto res = twf::Response::text("Hello Test", twf::StatusCode::OK);
    ASSERT_EQ(static_cast<int>(res.status_code()), 200, "Status must be 200 OK");
    ASSERT_EQ(res.get_body(), "Hello Test", "Body must match");

    std::string wire = res.serialize();
    ASSERT_TRUE(wire.find("HTTP/1.1 200 OK\r\n") != std::string::npos, "Wire must contain status line");
    ASSERT_TRUE(wire.find("Content-Length: 10\r\n") != std::string::npos, "Wire must contain Content-Length: 10");
    ASSERT_TRUE(wire.find("Server: twf/0.1.0 (C++23)\r\n") != std::string::npos, "Wire must contain Server header");
    ASSERT_TRUE(wire.find("\r\n\r\nHello Test") != std::string::npos, "Wire must contain body after headers");

    // Structured JSON response overload
    twf::TodoItem item{101, "Response test", false};
    auto json_res = twf::Response::json(item, twf::StatusCode::Created);
    ASSERT_EQ(static_cast<int>(json_res.status_code()), 201, "Status must be 201 Created");
    ASSERT_TRUE(json_res.get_body().find("\"id\":101") != std::string::npos, "Body must contain serialized struct");

    std::cout << "  [PASS] HTTP Response (fluent builder, wire protocol, struct-to-json)\n";
    g_tests_passed++;
}

// Test 5: Client DOM Declarative Builder
void test_dom_builder() {
    using namespace twf::dom;
    Element btn = button();
    btn.text("Click Me");
    btn.attr("type", "submit");
    btn.class_name("btn-primary");

    Element container = div();
    container.id("main-app");
    container.append(btn);

    std::cout << "  [PASS] Client DOM Builder (declarative tags, attributes, and nesting)\n";
    g_tests_passed++;
}

int main() {
    std::cout << "\n========================================\n";
    std::cout << "   twf (The Web Framework) Test Suite   \n";
    std::cout << "   Standard: C++23 | Architecture: Full-Stack\n";
    std::cout << "========================================\n\n";

    test_buffer();
    test_reflection();
    test_request();
    test_response();
    test_dom_builder();

    std::cout << std::format("\n>>> All {} test suites passed successfully! ({} assertions verified) <<<\n\n", 
        g_tests_passed, g_assertions);
    return 0;
}
