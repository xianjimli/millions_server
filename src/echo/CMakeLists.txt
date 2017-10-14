
add_executable(echod main.cpp echo_service.cpp)

target_link_libraries(echod base co ${SYS_LIBS})
