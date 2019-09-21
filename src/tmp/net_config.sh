# enable IP_FORWARD
ip link add {simpbr0} type bridge\n
ip link add {vethabc} type veth peer name {vethdef}\n
ip link set {vethabc} master {simpbr0}\n
ip link set {vethdef} netns {mypid}\n
ifconfig {simpbr0} 192.168.9.1/24 up\n
ifconfig {vethabc} up\n
iptables -t nat -A POSTROUTING -s 192.168.9.0/24 ! -d 192.168.9.0/24 -j MASQUERADE \n

# ifconfig {vethdef} 192.168.9.100/24 up \n
# route add default gw 192.168.9.1 \n
