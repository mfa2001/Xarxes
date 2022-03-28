# Server File
import sys
serverConfigure = None
debug = False

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

def start():
    pass


if __name__ == "__main__":
    parseArg()
    start()
