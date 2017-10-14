set(LIB_SRC co_epoll.cpp co_hook_sys_call.cpp co_routine.cpp coctx.cpp coctx_swap.S)

set_source_files_properties(coctx_swap.S PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")

add_library(co ${LIB_SRC})
add_executable(echosvr echosvr.cpp)

if(WIN32)
    target_link_libraries(echosvr co dl pthread wsock32 ws2_32) 
else()
    target_link_libraries(echosvr co dl pthread) 
endif()
