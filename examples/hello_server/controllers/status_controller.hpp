#pragma once

#include "twf/server/router.hpp"

struct StatusInfo {
    std::string framework{"twf"};
    std::string standard{"C++23"};
    std::string architecture{"Modular MVC (Controllers + Services + Static Mounts)"};
    std::string serializer{"Glaze (Compile-Time Reflection)"};
    std::string status{"running"};
};

class StatusController {
public:
    StatusController();

    const twf::Router& router() const noexcept { return router_; }

private:
    void init_routes();

    twf::Router router_;
};
