# Server File
import sys
import socket

server_sockets = None
bbdd_clients = []
serverConfigure = None
debug = False

class Socket:
    def __init__(self):
        self.udp_socket = None

        self.tcp_socket = None

class Client:
    def __init__(self):
        self.id = None
        self.state = "DISCONNECTED"
        self.udp_port = None
        self.tcp_port = None
        self.address = None

class ConfigFile:
    def __init__(self, ID, udpPort, tcpPort) -> None:
        # Diferent Config statments
        self.id = ID
        self.UDP_port = udpPort
        self.TCP_port = tcpPort


def readFromFile(serverFile):
    config_file = open(serverFile, "r")
    id_server = udp_port = tcp_port = ""
    # Read operation from file
    read_line = config_file.readlines()
    for line in read_line:
        try:
            operator = line.split()
            if operator[0] == "Id":
                id_server = operator[2]
            elif operator[0] == "UDP-port":
                udp_port = operator[2]
            elif operator[0] == "TCP-port":
                tcp_port = operator[2]
        except:
            pass
    if debug:
        print("Configuration added correclty")
    return ConfigFile(id_server, udp_port, tcp_port)

def parseArg():
    file = "server.cfg"
    if len(sys.argv) > 1:
        for arg in range(len(sys.argv)):
            if sys.argv[arg] == "-d":
                global debug
                debug = True
            elif sys.argv[arg] == "-c":
                file = sys.argv[arg+1]
    global serverConfigure
    serverConfigure = readFromFile(file)

def readClients():
    global bbdd_clients
    file_clients = open("bbdd_dev.dat","r")
    for line in file_clients:
        if line != '\n':
            valid_client = Client()
            valid_client.id = line
            bbdd_clients.append(valid_client)
    file_clients.close()
    if debug:
        print("Read clients correctly")

def openUdpTcpSocket():
    global server_sockets
    server_sockets = Socket()
    server_sockets.udp_socket = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    server_sockets.udp_socket.bind(("",int(serverConfigure.UDP_port)))

    server_sockets.tcp_socket = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    server_sockets.tcp_socket.bind(("",int(serverConfigure.TCP_port)))
    server_sockets.tcp_socket.listen(5)

def udpService():
    global server_sockets
    server_sockets: Socket
    if debug:
        print("UDP socket waiting")
    while True:
        (data , (ip , port)) = server_sockets.udp_socket.recvfrom(84)
        pass


if __name__ == "__main__":
    parseArg()
    readClients()
    openUdpTcpSocket()
    udpService()
