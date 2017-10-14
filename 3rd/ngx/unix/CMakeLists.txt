set(LIB_SRC 
    ngx_alloc.c
    ngx_channel.c
    ngx_daemon.c
    ngx_dlopen.c
    ngx_errno.c
    ngx_files.c
    ngx_linux_init.c
    ngx_linux_sendfile_chain.c
    ngx_posix_init.c
    ngx_process.c
    ngx_process_cycle.c
    ngx_readv_chain.c
    ngx_recv.c
    ngx_send.c
    ngx_setaffinity.c
    ngx_setproctitle.c
    ngx_shmem.c
    ngx_socket.c
    ngx_time.c
    ngx_udp_recv.c
    ngx_udp_send.c
    ngx_udp_sendmsg_chain.c
    ngx_user.c
    ngx_writev_chain.c
)

add_library(ngxos ${LIB_SRC})

