ifconfig {vethdef} 192.168.9.100/24 up\n
ip route add default via 192.168.9.1 dev {vethdef}