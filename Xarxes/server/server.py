# Server File
import random
import struct
import sys
import socket
import threading

end_threads = False


class Socket:
    def __init__(self):
        self.udp_socket = None
        self.tcp_socket = None

class Client:
    def __init__(self):
        self.id = None
        self.state = "DISCONNECTED"
        self.id_communication = random.randint(1000000000,9999999999)
        self.elements = ''
        self.udp_port = None
        self.tcp_port = None
        self.address_port = None
        self.alive_not_consecutive = 0

class ConfigFile:
    def __init__(self, ID, udpPort, tcpPort) -> None:
        # Diferent Config statments
        self.id = ID
        self.UDP_port = udpPort
        self.TCP_port = tcpPort

def main():
    try:
        global formatoPdu, end_threads
        formatoPdu = 'B11s11s61s'
        parseArg()
        readClients()
        setupSockets()
        service()
    except KeyboardInterrupt:
        end_threads = True


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
    global serverConfigure
    file = "server.cfg"
    if len(sys.argv) > 1:
        for arg in range(len(sys.argv)):
            if sys.argv[arg] == "-d":
                global debug
                debug = True
            elif sys.argv[arg] == "-c":
                file = sys.argv[arg+1]
    serverConfigure = readFromFile(file)

def readClients():
    global bbdd_clients
    bbdd_clients = []
    file_clients = open("bbdd_dev.dat","r")
    for line in file_clients:
        if line != '\n':
            valid_client = Client()
            valid_client.id = line.strip()
            bbdd_clients.append(valid_client)
    file_clients.close()
    if debug:
        print("Read clients correctly")


def setupSockets():
    global server_sockets
    server_sockets = Socket()
    server_sockets.udp_socket = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    server_sockets.udp_socket.bind(('',int(serverConfigure.UDP_port)))

    server_sockets.tcp_socket = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    server_sockets.tcp_socket.bind(('',int(serverConfigure.TCP_port)))
    server_sockets.tcp_socket.listen(5)


def service():
    thread_tcp = threading.Thread(target=tcp_loop)
    thread_tcp.daemon = True
    thread_tcp.start()

    thread_command = threading.Thread(target=read_stdin)
    thread_command.daemon = True
    thread_command.start()



    while True:
        if end_threads:
            server_sockets.udp_socket.close()
        data, addres = server_sockets.udp_socket.recvfrom(84)
        unpacked_data = struct.unpack(formatoPdu,data)
        thread_reg = threading.Thread(target=udpTypeConnection, args=(unpacked_data,addres))
        thread_reg.daemon = True
        thread_reg.start()


def udpTypeConnection(unpacked_data,adress):
    for client in bbdd_clients:
        if client.id == unpacked_data[1].decode().split('\x00')[0]:
            type = unpacked_data[0]
            if getStringByType(type) == "REG_REQ" and client.state == "DISCONNECTED":
                udpService(unpacked_data,adress)
            elif getStringByType(type)== "ALIVE" and (client.state == "REGISTERED" or client.state == "SEND_ALIVE"):
                send_alive()
            else:
                regPacket = mountPDU(client,"ERROR Tipo en registro no esperado",0xa2)
                server_sockets.udp_socket.sendto(regPacket,adress)
                changeClientState(client,"DISCONNECTED")



def udpService(data_unpacket,packet_adrress):
    packet = checkCorrectPackage(data_unpacket) #Comoroba rerrors
    for client in bbdd_clients:
        if client.id == packet['Identificador']:
            if client.state == "DISCONNECTED" and packet['id_communication'] == '0000000000':
                client.address_port = packet_adrress
                changeClientState(client,"WAIT_INFO")
                get_info_packet(client)
                '''
                info_data,addres_info) = client.udp_socket.recvfrom(84)
                data_unpacked = checkCorrectPackage(info_data,"REG_INFO",str(client.id_communication))
                if data_unpacked == 0:
                    print("INFO PACKET ERROR\n")
                    sys.exit(0)
                client.elements = data_unpacked.get('datos')
                packeg_send_info = mountPDU(client,serverConfigure.TCP_port,0xa5)
                client.udp_socket.sendto(packeg_send_info,addres_info)
                changeClientState(client,"REGISTERED")
                createThreadAlive(client)
                '''
            #mountPDU()
            #server_sockets.udp_socket.sendto()


def get_info_packet(client):
    global ClientInfoSocket
    if end_threads == False:
        client.udp_port = random.randint(1000,50000)
        ClientInfoSocket = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        ClientInfoSocket.bind(('',client.udp_port))
        packeg_send = mountPDU(client, client.udp_port, 0xa1)
        server_sockets.udp_socket.sendto(packeg_send, client.address_port)
        data, adress = ClientInfoSocket.recvfrom(84)
        unpacked_data = struct.unpack(formatoPdu,data)
        info = checkCorrectPackage(unpacked_data)
        info['datos'] = info['datos'].decode()
        #Control errores del paquete info
        if "REG_INFO" != getStringByType(info['Tipo']):
            #error reg info
            pass
        elif info['id_communication'] != str(client.id_communication):
            #error id_communication
            pass
        elif info['Identificador'] != client.id:
            #error id cliente
            pass
        else:
            changeClientState(client,"REGISTERED")
            client.tcp_port = info['datos'].split(',')[0]
            client.elements = info['datos'].split(',')[1]
            infoAckPaquet = mountPDU(client,client.tcp_port,0xa5)
            ClientInfoSocket.sendto(infoAckPaquet,adress)
        ClientInfoSocket.close()
        exit()


def changeClientState(client,newState):
    client.state = newState


def send_alive():
    if not end_threads:
        for client in bbdd_clients:
            if end_threads:
                break
            if client.state == "SEND_ALIVE" and client.alive_not_consecutive < 3:
                packet = mountPDU(client,client.id,0xb0)
                server_sockets.udp_socket.sendto(packet,client.address_port)
                packet_lost = 0
            elif client.state == "REGISTERED" and client.alive_not_consecutive < 3:
                changeClientState(client,"SEND_ALIVE")
                packet = mountPDU(client,client.id, 0xb0)
                server_sockets.udp_socket.sendto(packet, client.address_port)
                packet_lost = 0
            elif client.alive_not_consecutive >= 3:
                changeClientState(client,"DISCONNECTED")
        '''
        (data_alive,addres_alive) = client.udp_socket.recvfrom(84)
        unpacked_send = checkCorrectPackage(data_alive,"SEND_ALIVE",client.id_communication)
        if unpacked_send == 0:
            pass
            #error checking and send packet with error
        elif client.state == "REGISTERED":
            changeClientState(client,"SEND_ALIVE")
            packet_alive = mountPDU(client,client.id,0xb0)
            client.udp_socket.sendto(packet_alive,addres_alive)
        elif client.state == "SEND_ALIVE":
            packet_alive = mountPDU(client, client.id, 0xb0)
            client.udp_socket.sendto(packet_alive, addres_alive)
            '''


def mountPDU(client,datos,type):
    global serverConfigure, formatoPdu
    id_server = serverConfigure.id.encode()
    id_communicon = str(client.id_communication).encode()
    return struct.pack(formatoPdu,type ,id_server,id_communicon,str(datos).encode())


def checkCorrectPackage(data):
    package = {'Tipo': "",'Identificador': "",'id_communication': "",'datos':"" }

    type_package = data[0]
    id = data[1].decode().split('\x00')[0]
    id_communication = data[2].decode().split('\x00')[0]
    packet_datos = data[3]
    package['Tipo'] = type_package
    package['Identificador'] = id
    package['id_communication'] = id_communication
    package['datos'] = packet_datos
    return package


def getStringByType(type):
    if type == 0xa0:
        return "REG_REQ"
    elif type == 0xa1:
        return "REG_ACK"
    elif type == 0xa2:
        return  "REG_NACK"
    elif type == 0xa3:
        return "REG_REJ"
    elif type == 0xa4:
        return "REG_INFO"
    elif type == 0xa5:
        return "INFO_ACK"
    elif type == 0xa6:
        return "INFO_NACK"
    elif type == 0xa7:
        return "INFO_REJ"
    elif type == 0xf0:
        return "DISCONNECTED"
    elif type == 0xf1:
        return "NOT_REGISTERED"
    elif type == 0xf6:
        return "SEND_ALIVE"
    elif type == 0xb0:
        return "ALIVE"
    #ACABAR AMB TOTS


def tcp_loop():
    while True:
        new_sock, ip_port = server_sockets.tcp_socket.accept()
        recived_tcp_unpacked = new_sock.recv(84)
        packet = checkCorrectPackage(recived_tcp_unpacked)
        thread_to_serve_tcp = threading.Thread(target=serve_tcp,args=(packet,new_sock,ip_port))
        thread_to_serve_tcp.daemon = True
        thread_to_serve_tcp.start()

def mountPDUTCP(type,id_communication,element,info,valor):
    global serverConfigure, formatoPdu
    id_server = serverConfigure.id.encode()
    id_communicon = id_communication.encode()
    elemen = element.encode()
    valor.encode()
    return struct.pack('B11s11s8s16s80s', type, id_server, id_communicon, elemen,valor,str(info).encode())

def serve_tcp(packet,new_socket,ip_port):
    type = packet[0]

    if getStringByType(type) == "SEND_DATA":
        server_send_file(packet,new_socket,ip_port)
    else:
        for client in bbdd_clients:
            if client.id == packet[1].decode().split('\x00')[0]:
                sendPacket = mountPDUTCP(0xc3,"0000000000","","ERROR: Tipo de paquete no esperado","")
                new_socket.sendall(sendPacket)

def server_send_file(packet,new_socket,ip_port):
    pass

def read_stdin():
    while True:
        command = input()
        if command == "list":
            print("   Id     |     State  |  Id_comm  |    Adress    |   Elements   ")
            print("----------|------------|-----------|--------------|--------------")
            for client in bbdd_clients:
                print(client.id +" "+ client.state +" "+ str(client.id_communication) +" "+ str(client.address_port) +" "+ str(client.elements))

        elif command == "set":
            print("NOT IMPLEMENTED\n")
        elif command == "get":
            print("NOT IMPLEMENTED\n")
        elif command == "quit":
            end_threads = True
            exit(0)


if __name__ == "__main__":
    main()
