#include "session.hpp"
#include "parser.hpp"

#include <sys/socket.h>   // recv, send
#include <unistd.h>       // close
#include <cstring>        // memcpy
#include <cstdint>        // uint8_t, uint16_t
#include <vector>         // message buffer

bool recv_exact(int fd, uint8_t* buffer, int n_bytes){
    while(n_bytes > 0){
        int received = recv(fd, buffer, n_bytes, 0);
        if(received <= 0){
            return false;
        }
        n_bytes -= received;
        buffer = buffer + received;
    }
    return true;
}

void run_session(int fd){

    while (true){
        // read 2 bytes get START_OF_MESSAGE    
        // read 2 bytes to get msg_len
        uint8_t header_buf[4];
        if(!recv_exact(fd, header_buf, 4)){
            break;  // client disconnected
        }

        // parse message length
        uint16_t msg_length;
        memcpy(&msg_length, header_buf + 2, 2);

        // read the actual message
        uint8_t buffer[1024];
        memcpy(buffer, header_buf, 4);
        int remaining = msg_length - 2;
        if(!recv_exact(fd, (uint8_t*)((char*)buffer + 4), remaining)){
            break;
        }

        Session session;
        session.fd = fd;

        parse_message(session, buffer);
                        
    }
}