#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
        // config_name, subset, x, y, z, periodic, ground
        //ofxWFC3D wfc("data.xml", "default", 5, 5, 5, false, "ground");
        wfc.SetUp("data.xml", "dense knots", 7, 8, 4, false, "ground");
        int limit = 3, seed = 101;
        for (int k = 0; k < limit; k++) {
            bool result = wfc.Run(seed);

            if (result) {
              ofLog() << "WFc success";
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
