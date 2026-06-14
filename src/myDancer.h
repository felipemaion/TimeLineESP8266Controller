//
//  myDancer.hpp
//  ESP8266_timeline_controller
//
//  Created by Felipe Maion on 21/07/22.
//

#ifndef myDancer_h
#define myDancer_h

//#include <stdio.h>
#include "ofApp.h"
#include "ofxOsc.h"

class Dancer{
    public:
    ofxOscSender sender;
    string ip;
    string name;
    int port = 5555;
    size_t timeout=0.2;
    Dancer(string ip, string name){
        this->ip = ip;
        this->name = name;
        ofLogNotice() << "Setting HOST IP: " + ip + ", and PORT: " << port << std::endl;
        this->sender.setup(ip, port);
    }
    void relay0(int value);
    void relay1(int value);
    void relay0_ON();
        
    void relay0_OFF();
    
    void relay1_ON();
    void relay1_OFF();
    
    void all_OFF();
    void all_ON();
    
    bool connect(string url);
};

#endif /* myDancer_hpp */

