#include "ofApp.h"

void ofApp::setup(){
    //ofSetVerticalSync(true);

    // SCENE
    cam.setDistance(20);
    cam.setNearClip(0.1);

    material.setDiffuseColor( ofColor(220, 220, 220) );
    light.move(2,-5,-1);
    light.lookAt(ofVec3f(0.0,0.0,0.0));
    light.setDirectional();
	light.setDiffuseColor( ofColor(240, 240, 200) );
    light.setAmbientColor( ofColor(50,50,110) );
    light.setup();

    // MODELS
    // model made on blender and exported as ply 
    // Y Up
    // Z forward
    m_down.load("down.ply");
    m_line.load("line.ply");
    m_turn.load("turn.ply");
    m_up.load("up.ply");
    m_vertical.load("vertical.ply");
    m_cube.load("cube.ply");
    m_totem.load("totem.ply");

    // set tile-name mapping
    tiles["down"] = &m_down;
    tiles["empty"] = &m_down;
    tiles["line"] = &m_line;
    tiles["turn"] = &m_turn;
    tiles["up"] = &m_up;
    tiles["vertical"] = &m_vertical;

    worldNode.setOrientation(glm::angleAxis(ofDegToRad(0.f), glm::vec3{1.f, 0.f, 0.f}));


    // GUI
    structure_group.add( bound_width.set("bounding width", 5, 1, 20 ));
    structure_group.add( bound_height.set("bounding height", 3, 1, 20 ));
    structure_group.add( bound_length.set("bounding length", 5, 1, 20 ));
    gui.setup(structure_group);

}

//--------------------------------------------------------------
void ofApp::update(){
    int x = bound_width, y = bound_height, z = bound_length;
    container.set(x, y, z);
    container.setScale(1.0f);
    container.setPosition(0, y/2.0f, 0);
}

//--------------------------------------------------------------
void ofApp::draw(){
    //ofBackground(20);

    cam.begin();
    ofEnableDepthTest();

    ofDrawGrid(1.0, 10, false, false, true, false);
    ofDrawAxis(10);

    ofSetColor(50, 50, 200);
    container.drawWireframe();

    material.begin();
    light.enable();

    for (auto& node : nodes) {
        auto key = node.first;
        node.second.transformGL();
        tiles[key]->draw();
        node.second.restoreTransformGL();
    }

    light.disable();
    material.end();
    cam.end();

    ofDisableDepthTest();
    ofDisableLighting();
    gui.draw();
    ofDrawBitmapStringHighlight("FPS " + ofToString(ofGetFrameRate(),0), 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 32) {
        int x = bound_width, y = bound_height, z = bound_length;

        double micro_factot = 0.000001;

        uint64_t t1 = ofGetSystemTimeMicros();
        // config_name, subset, x, y, z, periodic, ground, surround
        wfc.SetUp("data.xml", "default", x+2, y, z+2, false, "", "empty");
        //wfc.SetUp("data.xml", "default", x, y, z, false, "vertical");
        //wfc.SetUp("data.xml", "only turns", x, y, z, false, "none");
        //wfc.SetUp("data.xml", "vertical", x, y, z, false, "none");
        int64_t t2 = ofGetSystemTimeMicros();
        ofLog() << "SetUp time in micros = " << (t2-t1);
        

        uint64_t t3 = ofGetSystemTimeMicros();
        int limit = 8, seed = (int)ofRandom(1000);
        for (int k = 0; k < limit; k++) {
            bool result = wfc.Run(seed++);

            if (result) {
                ofLog() << "WFC success";

                // process tiles and convert to ofNode tree
                nodes = wfc.NodeTileOutput(worldNode, ofVec3f(vs,vs,vs), {"empty"});
                break;
            } else {
                ofLog() << "WFC failure";
            }
        }
        int64_t t4 = ofGetSystemTimeMicros();
        ofLog() << "Run time in micros = " << (t4-t3);

        worldNode.setPosition(-x/2.0, 0, -z/2.0);
        worldNode.move(-0.5,0.5,-0.5);
        worldNode.setScale(1/vs);

    }
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
