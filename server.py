# Server File
class ConfigFile:
    def __init__(self, ID, udpPort, tcpPort) -> None:
        # Diferent Config statments
        self.id = ID
        self.UDP_port = udpPort
        self.TCP_port = tcpPort


def read(serverFile):
    config_file = open(serverFile, "r")
    # Read operation from file
    for line in config_file:
        try:
            operator = line.split()
            if operator[0] == "Id":
                ID = operator[2]
            if operator[0] == "UDP-port":
                UDP_port = operator[2]
            if operator[0] == "TCP-port":
                TCP_port = operator[2]
        except:
            pass

    return ConfigFile(ID, UDP_port, TCP_port)


if __name__ == "__main__":
    file = "server.cfg"
    configServer = read(file)
