//
//  myDevice.cpp
//  ESP8266_timeline_controller
//
//  Created by Felipe Maion on 12/08/22.
//

#include "myDevice.h"


void Device::tcpSetup() {
    bool connected = this->tcpClient.setup(this->ip.get(), this->port.get());
    if(this->tcpClient.isConnected()) {
        this->tcpClient.send("0");
        this->tcpClient.send("2");
        ofLogError() << "ALL: OFF";
    }
}

// Knows: Name, IP, Port, Status
// Does: Connect, Disconnect, relay0_on, relay0_off, relay1_on/off, sendStatus?

void Device::setup(){
    params.setName("Device");
    params.add(name.set("name","name"));
    params.add(ip.set("ip",""));
    params.add(port.set("port", 585,0,65535));
    
    actions.setName("Actions");
    actions.add(reconnect.set("reconnect"));
    actions.add(relay0.set("relay0", false));
    actions.add(relay1.set("relay1", false));
    
    params.add(actions);
    
    timelineGroup.setName("Set Timelines");
    timelineGroup.add(relay0_timeline.set("relay0", true));
    timelineGroup.add(relay1_timeline.set("relay1", true));
    
    params.add(timelineGroup);
    
    
    tcpSetup();
    
    relay0.addListener(this, &Device::relay_0);
    relay1.addListener(this, &Device::relay_1);
    reconnect.addListener(this,&Device::tcpSetup);
}

void Device::update(){
    
}
void Device::relay_0(bool& b){

    if(tcpClient.isConnected()) {
        if (b){
            tcpClient.send("1");
            ofLogError() << "relay0: ON";
        } else {
            tcpClient.send("0");
            ofLogError() << "relay0: OFF";
        }
        
        
    } else {
        ofLogError() << "NOT CONNECTED";
    }
    
}

void Device::relay_1(bool& b){
    if(tcpClient.isConnected()) {
        if (b){
            tcpClient.send("3"); // 11
            ofLogError() << "relay1: ON";
        } else {
            tcpClient.send("2"); // 10
            ofLogError() << "relay1: OFF";
        }
        
        
    } else {
        ofLogError() << "NOT CONNECTED";
    }
    
}
