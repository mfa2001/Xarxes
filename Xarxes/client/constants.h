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

#define ALIVE 0xb0
#define ALIVE_NACK 0xb1
#define ALIVE_REJ 0xb2

#define SEND_DATA 0xc0
#define DATA_ACK 0xc1
#define DATA_NACK 0xc2
#define DATA_REJ 0xc3
#define SET_DATA 0xc4
#define GET_DATA 0xc5

#define t 1
#define u 2
#define n 8
#define o 3
#define p 2
#define q 4
#define v 2
#define r 2
#define s 3
#define m 3