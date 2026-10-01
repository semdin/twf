add_executable(hello_server
    main.cpp
    services/todo_service.cpp
    controllers/todo_controller.cpp
    controllers/status_controller.cpp
)
target_include_directories(hello_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(hello_server PRIVATE twf)
