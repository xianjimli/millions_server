set(LIB_SRC 
    src/co_epoll.cpp 
    src/co_hook_sys_call.cpp 
    src/co_routine.cpp 
    src/coctx.cpp 
    src/coctx_swap.S)

set_source_files_properties(src/coctx_swap.S PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")

add_library(co ${LIB_SRC})
