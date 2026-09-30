#include "twf/server/server.hpp"
#include <iostream>
#include <format>

int main() {
    twf::Server app;

    // Route: Root "/" returning a modern dark-mode welcome page
    app.get("/", [](const twf::Request&) {
        return twf::Response::html(R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>twf - The Web Framework (C++23)</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            background: #090a0f;
            color: #f1f5f9;
            display: flex;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
        }
        .container {
            background: rgba(255, 255, 255, 0.04);
            border: 1px solid rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(16px);
            padding: 2.5rem;
            border-radius: 16px;
            max-width: 600px;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.6);
        }
        .badge {
            background: linear-gradient(135deg, #06b6d4, #3b82f6);
            color: #fff;
            padding: 4px 12px;
            border-radius: 9999px;
            font-size: 0.75rem;
            font-weight: 700;
            letter-spacing: 0.05em;
            text-transform: uppercase;
            display: inline-block;
            margin-bottom: 1rem;
        }
        h1 {
            font-size: 2.2rem;
            margin-bottom: 0.5rem;
            background: linear-gradient(135deg, #ffffff, #94a3b8);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        p { color: #94a3b8; line-height: 1.6; margin-bottom: 1.5rem; }
        .endpoints {
            background: #050608;
            border: 1px solid rgba(255, 255, 255, 0.06);
            border-radius: 8px;
            padding: 1rem;
        }
        .endpoint {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 0.5rem 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
        }
        .endpoint:last-child { border-bottom: none; }
        .method {
            font-family: monospace;
            background: #10b981;
            color: #000;
            padding: 2px 6px;
            border-radius: 4px;
            font-weight: bold;
            font-size: 0.8rem;
        }
        a {
            color: #38bdf8;
            text-decoration: none;
            font-family: monospace;
        }
        a:hover { text-decoration: underline; }
    </style>
</head>
<body>
    <div class="container">
        <div class="badge">twf &bull; The Web Framework</div>
        <h1>Welcome to twf (C++23)</h1>
        <p>A full-stack, zero-copy, modern C++ web framework running natively on your server.</p>
        
        <div class="endpoints">
            <div class="endpoint">
                <span class="method">GET</span>
                <a href="/api/status">/api/status</a>
                <span style="color:#64748b; font-size:0.8rem">JSON Status</span>
            </div>
            <div class="endpoint">
                <span class="method">GET</span>
                <a href="/api/greet?name=Mehmet">/api/greet?name=Mehmet</a>
                <span style="color:#64748b; font-size:0.8rem">Query Params</span>
            </div>
        </div>
    </div>
</body>
</html>)");
    });

    // Route: JSON API status
    app.get("/api/status", [](const twf::Request&) {
        return twf::Response::json(R"({"framework":"twf","standard":"C++23","status":"running","speed":"nanoseconds"})");
    });

    // Route: Dynamic greeting using query string parameter
    app.get("/api/greet", [](const twf::Request& req) {
        auto name = req.get_query("name").value_or("World");
        return twf::Response::json(std::format(R"({{"message":"Hello, {}! Welcome to twf."}})", name));
    });

    std::cout << "[twf] Starting server on port 8080...\n";
    auto result = app.listen("127.0.0.1", 8080);
    if (!result) {
        std::cerr << std::format("[twf Error] {}\n", result.error().message);
        return 1;
    }

    return 0;
}
