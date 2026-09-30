#include "twf/client/dom.hpp"
#include "twf/shared/models.hpp"
#include <iostream>
#include <string>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>

// Global reactive counter state in C++
static int g_counter = 0;
static twf::dom::Element* g_counter_btn = nullptr;

// C++ click handler called by browser DOM
extern "C" {
EMSCRIPTEN_KEEPALIVE
void on_increment_click() {
    g_counter++;
    if (g_counter_btn) {
        g_counter_btn->text("Clicked from C++: " + std::to_string(g_counter) + " times!");
    }
}
}

int main() {
    std::cout << "[twf Wasm] Initializing C++ Frontend in WebAssembly...\n";

    using namespace twf::dom;

    // Build the UI tree declaratively in pure C++
    Element card = div();
    card.attr("style", 
        "background: linear-gradient(135deg, rgba(30, 41, 59, 0.7), rgba(15, 23, 42, 0.9)); "
        "border: 1px solid rgba(56, 189, 248, 0.3); "
        "border-radius: 12px; "
        "padding: 1.5rem; "
        "margin-top: 1.5rem; "
        "box-shadow: 0 10px 25px rgba(0, 0, 0, 0.5); "
        "color: #e2e8f0;");

    Element badge = span();
    badge.text("PURE C++23 WEBASSEMBLY ENGINE");
    badge.attr("style", 
        "background: #0284c7; color: #fff; padding: 3px 8px; border-radius: 4px; "
        "font-size: 0.7rem; font-weight: bold; letter-spacing: 0.05em; display: inline-block; margin-bottom: 0.75rem;");

    Element title = h2();
    title.text("Client UI rendered entirely from C++");
    title.attr("style", "margin-bottom: 0.5rem; font-size: 1.25rem; color: #38bdf8;");

    Element desc = p();
    desc.text("This interactive widget runs compiled C++ machine instructions inside your browser's WebAssembly sandbox. Memory and state are managed with C++ RAII.");
    desc.attr("style", "font-size: 0.9rem; color: #94a3b8; line-height: 1.5; margin-bottom: 1rem;");

    // Interactive button with C++ event binding
    static Element btn = button();
    btn.text("Click Me (C++ State: 0)");
    btn.attr("style", 
        "background: linear-gradient(135deg, #0ea5e9, #2563eb); "
        "color: white; border: none; padding: 10px 18px; border-radius: 8px; "
        "font-weight: bold; cursor: pointer; transition: transform 0.1s; font-size: 0.9rem;");
    btn.attr("onclick", "Module._on_increment_click()");
    g_counter_btn = &btn;

    // Assemble the component tree
    card.append(badge);
    card.append(title);
    card.append(desc);
    card.append(btn);

    // Mount into browser DOM
    card.mount("#wasm-mount-point");

    std::cout << "[twf Wasm] C++ Frontend successfully mounted to DOM!\n";
    return 0;
}

#else

int main() {
    std::cout << "Compile with Emscripten to run in WebAssembly.\n";
    return 0;
}

#endif
