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
        for (auto& k : node) {
            auto key = k.first;
            node[key].transformGL();
            tiles[key]->draw();
            node[key].restoreTransformGL();
        }
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

        // config_name, subset, x, y, z, periodic, ground, surround
        wfc.SetUp("data.xml", "default", x+2, y, z+2, false, "", "empty");
        //wfc.SetUp("data.xml", "default", x, y, z, false, "vertical");
        //wfc.SetUp("data.xml", "only turns", x, y, z, false, "none");
        //wfc.SetUp("data.xml", "vertical", x, y, z, false, "none");


        std::vector< std::vector< std::vector< std::unordered_map<std::string, size_t >> > > tiles_wfc;

        int limit = 8, seed = (int)ofRandom(1000);
        for (int k = 0; k < limit; k++) {
            bool result = wfc.Run(seed++);

            if (result) {
                ofLog() << "WFC success";
                //std::string map_text = wfc.TextOutput();
                //ofLog() << map_text;

                tiles_wfc = wfc.TileOutput();
                break;
            } else {
                ofLog() << "WFC failure";
            }
        }

        // process tiles and convert to ofNode tree
        int ix = 0, iy = 0, iz = 0;
        nodes.clear();
        for (auto& tx : tiles_wfc) {
            ix++;
            for (auto& ty : tx) {
                iy++;
                for (auto& tz : ty) {
                    iz++;
                    for (auto& key: tz) {
                        if (key.first != "empty") {
                            std::unordered_map<std::string, ofNode> nmap;
                            ofNode cardinality;

                            cardinality.setParent(worldNode);
                            cardinality.setPosition(ix*vs, iy*vs, iz*vs);
                            cardinality.rotateDeg(tz[key.first]*90.0f, ofVec3f(0.0, 1.0, 0.0));
                            nmap[key.first] = cardinality;
                            nodes.push_back(nmap);
                        }
                    }
                }
                iz = 0;
            }
            iy = 0;
        }

        worldNode.setPosition(-x*vs/2.0/3.0, 0, -z*vs/2.0/3.0);
        worldNode.move(-1.0,0,-1.0); // surround offset
        worldNode.move(-0.5,-0.5,-0.5);
        worldNode.setScale(1/3.0);

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
