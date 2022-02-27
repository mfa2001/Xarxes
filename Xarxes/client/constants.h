//
// Created by mfarr on 23/02/2022.
//

#define REG_REQ 0xa0
#define REG_ACK 0Xa1
#define REG_NACK 0Xa2
#define REG_REJ 0Xa3
#define REG_INFO 0Xa4
#define INFO_ACK 0Xa5
#define INFO_NACK 0Xa6
#define INFO_REJ 0Xa7
#define ERROR 0Xa8

#define DISCONNECTED 0xf0
#define NOT_REGISTERED 0xf1
#define WAIT_ACK_REG 0xf2
#define WAIT_INFO 0xf3
#define WAIT_ACK_INFO 0xf4
#define REGISTERED 0xf5
#define SEND_ALIVE 0xf6