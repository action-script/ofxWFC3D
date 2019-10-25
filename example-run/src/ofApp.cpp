#include "ofApp.h"
// Y Up
// Z forward

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);
    cam.setDistance(20);
    cam.setNearClip(0.1);

    material.setDiffuseColor(ofFloatColor::green);
    light.move(2,-5,-1);
    light.lookAt(ofVec3f(0.0,0.0,0.0));
    light.setDirectional();
	light.setDiffuseColor(ofFloatColor::yellow);
    light.setup();

    light2.move(-2,4,1);
    light2.lookAt(ofVec3f(0.0,0.0,0.0));
    light2.setDirectional();
    ofFloatColor l2(0.2,0.2,0.5);
	light2.setDiffuseColor(l2);
    light2.setup();


    //int x = 7, y = 1, z = 7;

    container.set(x, y, z);
    container.setScale(1.0f);
    container.setPosition(0, y/2.0f, 0);

    // models
    float m_scale = 2/1000.0;
    float m_s_scale = m_scale * (1-1/3.0);
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

    //tiles["down"] = &m_up;
    //tiles["up"] = &m_down;

    worldNode.setOrientation(glm::angleAxis(ofDegToRad(0.f), glm::vec3{1.f, 0.f, 0.f}));


    // config_name, subset, x, y, z, periodic, ground
    //wfc.SetUp("data.xml", "default", x, z, y, false, "none");
    //wfc.SetUp("data.xml", "default", x, z, y, false, "none");
    //wfc.SetUp("data.xml", "default", x, z, y, false, "empty");
    //wfc.SetUp("data.xml", "default", x, z, y, false, "vertical");
    wfc.SetUp("data.xml", "only turns", x, z, y, false, "none");
    //wfc.SetUp("data.xml", "vertical", x, z, y, false, "none");


    std::vector< std::vector< std::vector< std::unordered_map<std::string, size_t >> > > tiles_wfc;

    int limit = 5, seed = 101;
    for (int k = 0; k < limit; k++) {
        bool result = wfc.Run(seed++);

        if (result) {
            ofLog() << "WFC success";
            std::string map_text = wfc.TextOutput();
            ofLog() << map_text;

            tiles_wfc = wfc.TileOutput();
            break;
        } else {
            ofLog() << "WFc failure";
        }
    }

    // process tiles and convert to ofNode tree
    int ix = 0, iy = 0, iz = 0;
    for (auto& tx : tiles_wfc) {
        ix++;
        for (auto& ty : tx) {
            iy++;
            for (auto& tz : ty) {
                iz++;
                //ofNode gridpos;
                //gridpos.setParent(worldNode);
                //gridpos.setPosition(ix, iy, iz);
                //trash.push_back(gridpos);
                //ofLog() << "node pos: " << ix << ", " << iy << ", " << iz;
                for (auto& key: tz) {
                    if (key.first != "empty") {
                        std::unordered_map<std::string, ofNode> nmap;
                        ofNode cardinality;

                        cardinality.setParent(worldNode);
                        //cardinality.setPosition(ix-x/2, iy, iz-z/2);
                        cardinality.setPosition(ix*vs, iz*vs, iy*vs);
                        cardinality.rotateDeg(tz[key.first]*90.0f, ofVec3f(0.0, 1.0, 0.0));
                        nmap[key.first] = cardinality;
                        //nmap[key.first] = gridpos;
                        nodes.push_back(nmap);
                    }
                }
            }
            iz = 0;
        }
        iy = 0;
    }

    gui.setup();
    gui.add(slider_1.setup("up", 0, 0, 360));
    gui.add(slider_2.setup("down", 0, 0, 360));

}

//--------------------------------------------------------------
void ofApp::update(){
}

//--------------------------------------------------------------
void ofApp::draw(){
    //ofBackground(20);

    cam.begin();

    ofEnableDepthTest();
    ofDrawGrid(1.0, 10, false, false, true, false);
    ofDrawAxis(10);
    //light.draw();

    //ofDrawBox(ofVec3f(-2.5, 0.5, -2.5), 1.0);

    //container.drawWireframe();

    //m_cube.drawFaces();

    material.begin();
    light.enable();
    //light2.enable();


    //worldNode.rotateDeg(4.0, ofVec3f(0.0, 1.0, 0.0));
    //worldNode.move(0.0, 0.0, 0.004);
    worldNode.transformGL();
    m_totem.draw();
    worldNode.restoreTransformGL();

    for (auto& node : nodes) {
        for (auto& k : node) {
            auto key = k.first;
            node[key].transformGL();
            tiles[key]->draw();
            node[key].restoreTransformGL();
        }
    }


    //light2.disable();
    light.disable();
    material.end();

    cam.end();

    ofSetColor(255);
    ofDrawBitmapString("FPS " + ofToString(ofGetFrameRate(),0), 20, 20);
    gui.draw();


    //m_turn.setRotation(1, slider_1, 0.0, 1.0, 0.0);
    //m_line.setRotation(1, slider_2, 0.0, 1.0, 0.0);
    //m_up.setRotation(1, slider_1, 0.0, 1.0, 0.0);
    //m_down.setRotation(1, slider_2, 0.0, 1.0, 0.0);
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
