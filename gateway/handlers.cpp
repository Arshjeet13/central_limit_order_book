#include "session.hpp"
#include "boe/protocol.hpp"
#include "database.hpp"
#include <cstdint>
#include <string>
#include <utility>
#include <sys/socket.h> 

void send_message(int fd, const void* data, int len){

}

boe::login_response make_login_response(char status){
    auto login_response = boe::make_message<boe::login_response>();

    std::string login_response_text;
    login_response.login_response_status = status;

    if(status == 'A'){
        login_response_text = "Successful login";
    }
    else if(status == 'N'){
        login_response_text = "Incorrect username/password/session_sub_id";
    }
    else if(status == 'M'){
        login_response_text = "Invalid login_request structure";        
    }
    else if(status == 'B'){
        login_response_text = "Already logged in";
    }

    // Would need to add a check to ensure login response text 
    // is <= 60 bytes, if different messages are implemented
    memcpy(login_response.login_response_text, login_response_text.c_str(), login_response_text.size());

    return login_response;
}

boe::replay_complete make_replay_complete(){
    auto replay_complete = boe::make_message<boe::replay_complete>();
    return replay_complete;
}

void handle_login_request(Session& session, const boe::login_request& login_request)
{
    if(login_request.matching_unit != 0 || login_request.sequence_number != 0){
        auto login_response = make_login_response('M');
        send_message(session.fd, &login_response, sizeof(login_response));
        return;
    }
    if(session.logged_in){
        auto login_response = make_login_response('B');
        send_message(session.fd, &login_response, sizeof(login_response));
        return;
    }

    // Version 1 : Since we will only have 1 client with no concurrency,
    // we just add the details in the session without checking for the existence
    // of a session with the same username and session_sub_id
    if(!db::validate(login_request.username, login_request.password, login_request.session_sub_id)){
        auto login_response = make_login_response('N');
        send_message(session.fd, &login_response, sizeof(login_response));
        return;
    }

    memcpy(session.session_sub_id, login_request.session_sub_id, 4);
    memcpy(session.username,       login_request.username,       4);
    session.logged_in = true;

    auto login_response =  make_login_response('A');
    auto replay_complete = make_replay_complete();

    send_message(session.fd, &login_response, sizeof(login_response));
    send_message(session.fd, &replay_complete, sizeof(replay_complete));
}
