//
//  myDancer.cpp
//  ESP8266_timeline_controller
//
//  Created by Felipe Maion on 21/07/22.
//
#include "ofApp.h"
#include "myDancer.h"

void Dancer::relay0(int value){
    ofxOscMessage m;
    m.setAddress( "/relay0" );
    m.addIntArg( value );
//    ofLogNotice() << "DEVICE: " + this->ip + " (" + this->name + ") - SENDING MESSAGE:" << m.getAddress() + " args:" << m.getNumArgs() + " sender info:" << this->sender.getHost() + ":" << this->sender.getPort() << std::endl;
    this->sender.sendMessage( m );
}

void Dancer::relay1(int value){
    ofxOscMessage m;
    m.setAddress( "/relay1" );
    m.addIntArg( value );
//    ofLogNotice() << "DEVICE: " + this->ip + " (" + this->name + ") - SENDING MESSAGE:" << m.getAddress() + " args:" << m.getNumArgs() + " sender info:" << this->sender.getHost() + ":" << this->sender.getPort() << std::endl;
    this->sender.sendMessage( m );
}



bool Dancer::connect(string url){
    try {
       
        
//         OLD WORKING VERSION USING HTTP SERVER @PORT 80
    ofHttpRequest request;

    request.method = ofHttpRequest::GET;
    request.url = "http://" + this->ip + url;
    request.body = "content";
    request.saveTo = false;
    request.timeoutSeconds = 0;

    ofURLFileLoader http;
        auto response = http.handleRequest(request);
                ofLogNotice() << "DEVICE: " + this->ip + " (" + this->name + ") - (request = " + url + "). Response=" << response.status << std::endl;
        if (response.status == 200) {
            return true;
        }
        else {
            ofLogNotice() << "Erro response.status:" << response.status << " " << std::endl;
            return false;
        }

    }
    catch (int errNum){
        ofLogNotice() << "Erro conexão:" << errNum << " " << std::endl;
        return false;
    }
}
void Dancer::relay1_ON(){
    this->relay1(1);
//    string url =  "/5/on";
//    this->connect(url);
  
        }
void Dancer::relay1_OFF(){
//    string url = "/5/off";
//    this->connect(url);
    this->relay1(0);
        }
    
void Dancer::relay0_ON(){
//    string url =  "/4/on";
//    this->connect(url);
    this->relay0(1);
        }
void Dancer::relay0_OFF(){
//    string url =  "/4/off";
//    this->connect(url);
    this->relay0(0);
        }
    
void Dancer::all_OFF(){
        this->relay0_OFF();
        this->relay1_OFF();
    }

void Dancer::all_ON(){
        this->relay0_ON();
        this->relay1_ON();
    }

