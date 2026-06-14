#include "ofApp.h"
#include <future>

//--------------------------------------------------------------
void ofApp::setup(){
//    dancer1.all_OFF();
//    dancer2.all_OFF();
    // Async http request:
    ofRegisterURLNotification(this);
    timeline.setup();
    ofBackground(0);
//    timeline.addVideoTrack("Video","telao.mp4");
    videoTrack = timeline.addVideoTrack("Video", "telao.mov");
    updateVideoFile();
    
    // Hard coded due to problem saving timeline
    for(Dancer &dancer : dancers)
    {
        timeline.addSwitches(dancer.name);
        if (dancer.name == "Márcio")
            // Extra switcher
            timeline.addSwitches("Márcio-CORAÇÃO");
    }

    flags = timeline.addFlags("flags");
    timeline.addColors("COR");
    gui.setup();
    gui.setPosition(150,10);
//    ------
    device.setup();
    deviceGroup.setName("device group");
    deviceGroup.add(device.params);
    
//    ------
    playGroup.setName("play group");
    playGroup.add(button.set("RESET"));
    playGroup.add(button2.set("FADE ESCURO"));
    playGroup.add(button4.set("LIGANDO HemoSphere"));
    playGroup.add(button3.set("ENTRADA MARCIO"));
    playGroup.add(btnSender.set("TESTE"));
//    ------
    gui.add(deviceGroup);
    gui.add(playGroup);
    
    gui.loadFromFile("settings.xml");
    ofLogError() << device.tcpClient.isConnected();
    ofLogError() << device.tcpClient.getIP();
//    // OSC Listener for the start:
    receiver.setup(PORT);
//    sender.setup(HOST, PORT);
    button.addListener(this,&ofApp::resetClicked);
    btnSender.addListener(this,&ofApp::btnTest);
    device.tcpSetup();
}

void ofApp::urlResponse(ofHttpResponse & response) {
   // do whatever you want to respond to "response"
}

void ofApp::updateVideoFile(){
    if(videoTrack != NULL){ //in the case the video load failed check against null
        timeline.setFrameRate(videoTrack->getPlayer()->getTotalNumFrames()/videoTrack->getPlayer()->getDuration());
        timeline.setDurationInFrames(videoTrack->getPlayer()->getTotalNumFrames());
        timeline.setTimecontrolTrack(videoTrack); //video playback will control the time
    }
}
//--------------------------------------------------------------
void ofApp::update(){
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m;
        receiver.getNextMessage(&m);
        if (m.getAddress() == "/mr_start")
        {
            start_osc = m.getArgAsBool(0);
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    timeline.draw();
    // If OSC message to the address designed to start was received.
    if(start_osc){
        // reset setting
        videoTrack->stop();
        timeline.setCurrentTimeMillis(0);
        timeline.setCurrentFrame(0);
        videoTrack->selectFrame(0);
        // Start and unflag it.
        videoTrack->play();
        start_osc = false;
    }
    // OUTSIDE OF THE LOOP, only one that calls relay1.
    if(timeline.isSwitchOn("Márcio-CORAÇÃO")){
        std::future<void> fut =  std::async(std::launch::async, &Dancer::relay1_ON, &dancer1);
//        dancer1.relay1_ON();
    }
    if(!timeline.isSwitchOn("Márcio-CORAÇÃO")){
        std::future<void> fut =  std::async(std::launch::async, &Dancer::relay1_OFF, &dancer1);
//        dancer1.relay1_OFF();
    }

    
    for(Dancer &dancer : dancers)
    {
        if(timeline.isSwitchOn(dancer.name)){
            std::future<void> fut =  std::async(std::launch::async, &Dancer::relay0_ON, &dancer);
//            dancer.relay0_ON();
        }
        if(!timeline.isSwitchOn(dancer.name)){
            std::future<void> fut =  std::async(std::launch::async, &Dancer::relay0_OFF, &dancer);
//            dancer.relay0_OFF();
        }
    }
    
//    if(button){
////        timeline.stop();
////        timeline.setPercentComplete(0);
////        timeline.setShowTimeControls(1);
//        ofLogNotice() << timeline.getCurrentTime();
//        videoTrack->stop();
//        timeline.setCurrentTimeMillis(0);
//        timeline.setCurrentFrame(0);
//
//        videoTrack->selectFrame(0);
//
////        timeline.setShowInoutControl(1);
//        ofLogNotice() << timeline.getCurrentTime();
//
//    }
//
//    if(button2){
//        videoTrack->selectFrame(2486);
//        timeline.setCurrentFrame(2486);
//        timeline.flagUserChangedValue();
//        videoTrack->update();
//    }
//    if(button3){
//        videoTrack->selectFrame(3968);
//        timeline.setCurrentFrame(3968);
//        timeline.flagUserChangedValue();
//        videoTrack->update();
//    }
//    if(button4){
//        videoTrack->selectFrame(3330);
//        timeline.setCurrentFrame(3330);
//        timeline.flagUserChangedValue();
//        videoTrack->update();
//    }
//
//
//    if (btnSender){
//        keyframes = flags->getKeyframes();
//        for (int i = 0; i < keyframes.size(); i++){
//            ofxTLFlag* flag = (ofxTLFlag*)keyframes[i];
//            ofLogError() << i << ": " << flag->textField.text << " [" << flag->time << "]";
//        }
//
//    }
    
//    if (btnSender){
//
//       //Open the Open File Dialog
//        ofFileDialogResult mainMovie = ofSystemLoadDialog("Select a avi or other");
//
//        //Check if the user opened a file
//        if (mainMovie.bSuccess){
//
//            ofLogVerbose("User selected a file");
//
//            //We have a file, check it and process it
//            processOpenFileSelection(mainMovie);
//        }else {
//            ofLogVerbose("User hit cancel");
//        }
//    }
    
//    if (btnSender){
//        ofxOscMessage m;
//        m.setAddress( "/led" );
//        m.addIntArg( 1 );
//        sender.sendMessage(m);
//        ofLogNotice() << sender.getHost() << std::endl;
//    } else {
//        ofxOscMessage m;
//        m.setAddress( "/led" );
//        m.addIntArg( 0 );
//        sender.sendMessage(m);
//    }
    gui.draw();
    
}

void ofApp::btnTest(){
    deviceGroup.remove(device.params);
    
    
}
void ofApp::resetClicked(){
//        if(button){
    //        timeline.stop();
    //        timeline.setPercentComplete(0);
    //        timeline.setShowTimeControls(1);
            ofLogNotice() << timeline.getCurrentTime();
            videoTrack->stop();
            timeline.setCurrentTimeMillis(0);
            timeline.setCurrentFrame(0);
    
            videoTrack->selectFrame(0);
    
    //        timeline.setShowInoutControl(1);
            ofLogNotice() << timeline.getCurrentTime();
    
//        }
}

void ofApp::processOpenFileSelection(ofFileDialogResult mainMovie){

      
    ofLogError("getName(): "  + mainMovie.getName());
    ofLogError("getPath(): "  + mainMovie.getPath());
      
//    ofFile mainMov (mainMovie.getPath());
    videoTrack->load(mainMovie.getPath());
    updateVideoFile();
    //nom du film: mainMov'
  
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

