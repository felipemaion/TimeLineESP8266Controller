//
//  myDevice.h
//  ESP8266_timeline_controller
//
//  Created by Felipe Maion on 12/08/22.
//

#ifndef myDevice_h
#define myDevice_h

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxTCPClient.h"

class Device {
private:
    ofEventListener listener;
    
public:
    void setup();
    void update();
    void relay_0(bool& b);
    void relay_1(bool& b);
    void tcpSetup();
//    ofXml xml;

    
    ofParameterGroup params;
    ofParameter<string> name;
    ofParameter<string> ip;
    ofParameter<int> port;
    
    ofParameterGroup actions;
    ofParameter<void> reconnect;
    ofParameter<bool> relay0;
    ofParameter<bool> relay1;
    
    ofParameterGroup timelineGroup;
    ofParameter<bool> relay0_timeline;
    ofParameter<bool> relay1_timeline;
    
    ofxTCPClient tcpClient;
    
    
//    Device() {
//
//    listener = ofEvents().mousePressed.newListener([this](ofMouseEventArgs&mouse){
//        std::cout << mouse << std::endl;
//        ofLogError() << "PRESSED";
//    });
//    void draw();
//    };

};
#endif /* myDevice_h */
