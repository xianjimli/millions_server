sysctl -w net.core.somaxconn=262144
sysctl -w net.ipv4.tcp_mem='4096 4096 4096'
sysctl -w net.ipv4.tcp_rmem='4096 4096 4096'
sysctl -w net.ipv4.tcp_wmem='4096 4096 4096'
sysctl -w net.ipv4.ip_local_port_range="10000 61000"
sysctl -w net.ipv4.tcp_fin_timeout=30
sysctl -w net.nf_conntrack_max=2097152
sysctl -w net.netfilter.nf_conntrack_tcp_timeout_established=84600
sysctl -w fs.file-max=2097152
sysctl -w fs.nr_open=2097152

ulimit -n 2000000
