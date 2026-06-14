#pragma once

#include "ofMain.h"
#include "ofxTimeline.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "myDancer.h"
#include "myDevice.h"
//#define OF_ADDON_USING_OFXOSC
// LISTENER OSC AT PORT 8888
#define PORT 8888
//#define HOST "localhost"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        
    
    ofxOscSender sender;
    ofxOscReceiver receiver;
    bool start_osc = false;
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void urlResponse(ofHttpResponse & response);
        void processOpenFileSelection(ofFileDialogResult mainMovie);
        void updateVideoFile();
        void resetClicked();
        void btnTest();
    ofURLFileLoader UFL;
    ofHttpRequest request;
    ofxTimeline timeline;
    ofxTLVideoTrack* videoTrack;
    ofxTLFlags* flags;
    vector<ofxTLKeyframe*> keyframes;
    // ----- TO BE MOVED
    // SETING IP ADDRESS
    Dancer dancer1 = Dancer("192.168.15.14", "Márcio");
    Dancer dancer2 = Dancer("192.168.15.17", "Tuanny");
    Dancer dancer3 = Dancer("192.168.0.102", "Amanda");
    Dancer dancer4 = Dancer("192.168.0.103", "Karyne");
    Dancer dancers[4] = {dancer1,dancer2,dancer3,dancer4};
    // --------
    
    ofxPanel gui;
    ofParameter<void> button;
    ofParameter<void> button2;
    ofParameter<void> button3;
    ofParameter<void> button4;
    ofxToggle tgSender;
    ofParameter<void> btnSender;
    
    
    Device device;
    ofParameterGroup deviceGroup;
    ofParameterGroup playGroup;
    
   
};


