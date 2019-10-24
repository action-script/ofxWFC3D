#include "ofApp.h"

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


    int x = 8, y = 4, z = 10;

    container.set(x, y, z);
    container.setScale(1.0f);
    container.setPosition(0, y/2.0f, 0);

    // models
    float m_scale = 2/1000.0;
    float m_s_scale = m_scale * (1-1/3.0);
    m_down.loadModel("down.ply");
    m_line.loadModel("line.ply");
    m_turn.loadModel("turn.ply");
    m_up.loadModel("up.ply");
    m_vertical.loadModel("vertical.ply");
    m_cube.loadModel("cube.ply");

    // set tile-name mapping
    tiles["down"] = &m_down;
    tiles["empty"] = &m_down;
    tiles["line"] = &m_line;
    tiles["turn"] = &m_turn;
    tiles["up"] = &m_up;
    tiles["vertical"] = &m_vertical;

    m_down.setScale(m_s_scale,m_s_scale,m_s_scale);
    //m_down.setRotation(1, 180.0, 0.0, 1.0, 0.0);
    //m_down.setPosition(0.5, 0.5, 0.5);
    m_line.setScale(m_scale,m_scale,m_scale);
    //m_line.setPosition(0.5, 0.5, 0.5);
    m_turn.setScale(m_s_scale,m_s_scale,m_s_scale);
    //m_turn.setRotation(1, 270.0, 0.0, 1.0, 0.0);
    //m_turn.setPosition(0.5, 0.5, 0.5);
    m_up.setScale(m_s_scale,m_s_scale,m_s_scale);
    //m_up.setRotation(1, 180.0, 0.0, 1.0, 0.0);
    //m_up.setPosition(0.5, 0.5, 0.5);
    m_vertical.setScale(m_scale,m_scale,m_scale);
    //m_vertical.setPosition(0.5, 0.5, 0.5);
    m_cube.setScale(m_scale, m_scale, m_scale);
    //m_cube.setPosition(0.5, 0.5, 0.5);

/*
 *    m_down.disableMaterials();
 *    m_line.disableMaterials();
 *    m_turn.disableMaterials();
 *    m_up.disableMaterials();
 *    m_vertical.disableMaterials();
 *
 *    m_down.disableTextures();
 *    m_line.disableTextures();
 *    m_turn.disableTextures();
 *    m_up.disableTextures();
 *    m_vertical.disableTextures();
 */

    worldNode.setOrientation(glm::angleAxis(ofDegToRad(0.f), glm::vec3{1.f, 0.f, 0.f}));
    meshNode.setParent(worldNode);
    meshNode.setPosition(5.0, 0.0, 0.0);


    // config_name, subset, x, y, z, periodic, ground
    //wfc.SetUp("data.xml", "vertical", x, z, y, false, "none");
    //wfc.SetUp("data.xml", "default", x, z, y, false, "none");
    wfc.SetUp("data.xml", "default", x, z, y, false, "none");
    //wfc.SetUp("data.xml", "default", x, z, y, false, "empty");
    //wfc.SetUp("data.xml", "default", x, z, y, false, "vertical");
    //wfc.SetUp("data.xml", "dense knots", 7, 8, 4, false, "ground");


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
                        cardinality.setPosition(ix-x/2, y-iz, iy-z/2);
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

    //worldNode.rotateDeg(4.0, ofVec3f(0.0, 1.0, 0.0));
    //meshNode.transformGL();
    //m_line.drawFaces();
    //m_turn.drawFaces();
    //meshNode.restoreTransformGL();

    //material.begin();
    light.enable();
    //light2.enable();

    for (auto& node : nodes) {
        for (auto& k : node) {
            auto key = k.first;
            node[key].transformGL();
            tiles[key]->drawFaces();
            node[key].restoreTransformGL();
        }
    }

    //light2.disable();
    light.disable();
    //material.end();

    cam.end();

    ofSetColor(255);
    ofDrawBitmapString("FPS " + ofToString(ofGetFrameRate(),0), 20, 20);
    gui.draw();


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
