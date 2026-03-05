
add_executable(omplayer WIN32 main.cpp)
target_link_libraries(omplayer PRIVATE SDL3::SDL3 OpenMedia::OpenMedia)
