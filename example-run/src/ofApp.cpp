#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);
    cam.setDistance(20);

    int x = 8, y = 3, z = 8;

    container.set(x, y, z);
    container.setScale(1.0f);
    container.setPosition(0, y/2.0f, 0);

    // config_name, subset, x, y, z, periodic, ground
    wfc.SetUp("data.xml", "default", x, y, z, false, "ground");
    //wfc.SetUp("data.xml", "dense knots", 7, 8, 4, false, "ground");

    int limit = 3, seed = 101;
    for (int k = 0; k < limit; k++) {
        bool result = wfc.Run(seed);

        if (result) {
            ofLog() << "WFC success";
            std::string map_text = wfc.TextOutput();
            ofLog() << map_text;
            break;
        } else {
            ofLog() << "WFc failure";
        }
    }
}

//--------------------------------------------------------------
void ofApp::update(){
}

//--------------------------------------------------------------
void ofApp::draw(){
    //ofBackground(20);

    cam.begin();

    ofEnableDepthTest();
    ofDrawGrid(10.0, 10, false, false, true, false);
    ofDrawAxis(10);

    //container.draw();
    container.drawWireframe();

    cam.end();

    ofSetColor(255);
    ofDrawBitmapString("FPS " + ofToString(ofGetFrameRate(),0), 20, 20);
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
