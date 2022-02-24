# Server File
import sys


class ConfigFile:
    def __init__(self, ID, udpPort, tcpPort) -> None:
        # Diferent Config statments
        self.id = ID
        self.UDP_port = udpPort
        self.TCP_port = tcpPort


def read(serverFile, debuger):
    config_file = open(serverFile, "r")
    # Read operation from file
    for line in config_file:
        try:
            operator = line.split()
            if operator[0] == "Id":
                id = operator[2]
            if operator[0] == "UDP-port":
                udp_port = operator[2]
            if operator[0] == "TCP-port":
                tcp_port = operator[2]
        except:
            pass
    if debuger == 1:
        print("Configuration added correclty")
    return ConfigFile(id, udp_port, tcp_port)


def start():
    pass


if __name__ == "__main__":
    file = "server.cfg"
    debug = 0
    if len(sys.argv) > 1:
        for arg in range(len(sys.argv)):
            if arg == 0:
                pass
            elif sys.argv[arg] == "-d":
                debug = 1
            elif sys.argv[arg] == "-c":
                file = sys.argv[arg+1]
    configServer = read(file, debug)

    start()
