#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <memory>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#endif

namespace twf::dom {

#ifdef __EMSCRIPTEN__
using JsVal = emscripten::val;
#else
// Mock JsVal when compiling on native desktop (for syntax verification and testing)
struct JsVal {
    static JsVal global(std::string_view) { return {}; }
    template <typename T>
    T call(std::string_view, auto...) const { return T{}; }
    void set(std::string_view, auto...) const {}
};
#endif

/// Declarative DOM Element wrapper
class Element {
public:
    Element(std::string_view tag) : tag_(tag) {
#ifdef __EMSCRIPTEN__
        auto document = emscripten::val::global("document");
        raw_ = document.call<emscripten::val>("createElement", std::string(tag_));
#endif
    }

    /// Set element inner text
    Element& text(std::string_view content) {
#ifdef __EMSCRIPTEN__
        raw_.set("textContent", std::string(content));
#endif
        return *this;
    }

    /// Set element inner HTML
    Element& html(std::string_view content) {
#ifdef __EMSCRIPTEN__
        raw_.set("innerHTML", std::string(content));
#endif
        return *this;
    }

    /// Set attribute
    Element& attr(std::string_view name, std::string_view value) {
#ifdef __EMSCRIPTEN__
        raw_.call<void>("setAttribute", std::string(name), std::string(value));
#endif
        return *this;
    }

    /// Set CSS class
    Element& class_name(std::string_view cls) {
        return attr("class", cls);
    }

    /// Set element ID
    Element& id(std::string_view id_name) {
        return attr("id", id_name);
    }

    /// Set inline CSS style property
    Element& style(std::string_view prop, std::string_view val) {
#ifdef __EMSCRIPTEN__
        auto s = raw_["style"];
        s.set(std::string(prop), std::string(val));
#endif
        return *this;
    }

    /// Append a child element
    Element& append(const Element& child) {
#ifdef __EMSCRIPTEN__
        raw_.call<void>("appendChild", child.raw());
#endif
        return *this;
    }

    /// Clear children
    Element& clear() {
#ifdef __EMSCRIPTEN__
        raw_.set("innerHTML", std::string(""));
#endif
        return *this;
    }

    /// Get value from input elements
    std::string value() const {
#ifdef __EMSCRIPTEN__
        return raw_["value"].as<std::string>();
#else
        return "";
#endif
    }

    /// Set value for input elements
    Element& set_value(std::string_view val) {
#ifdef __EMSCRIPTEN__
        raw_.set("value", std::string(val));
#endif
        return *this;
    }

    /// Mount to a DOM parent selector (e.g. "#app" or "body")
    void mount(std::string_view selector = "#app") const {
#ifdef __EMSCRIPTEN__
        auto document = emscripten::val::global("document");
        auto parent = document.call<emscripten::val>("querySelector", std::string(selector));
        if (!parent.isNull() && !parent.isUndefined()) {
            parent.call<void>("appendChild", raw_);
        }
#endif
    }

#ifdef __EMSCRIPTEN__
    emscripten::val raw() const noexcept { return raw_; }
#endif

private:
    std::string tag_;
#ifdef __EMSCRIPTEN__
    emscripten::val raw_{emscripten::val::null()};
#endif
};

/// Helper creation functions
inline Element create(std::string_view tag) { return Element(tag); }
inline Element div() { return Element("div"); }
inline Element span() { return Element("span"); }
inline Element h1() { return Element("h1"); }
inline Element h2() { return Element("h2"); }
inline Element p() { return Element("p"); }
inline Element button() { return Element("button"); }
inline Element input(std::string_view type = "text") {
    Element el("input");
    el.attr("type", type);
    return el;
}
inline Element ul() { return Element("ul"); }
inline Element li() { return Element("li"); }

} // namespace twf::dom
