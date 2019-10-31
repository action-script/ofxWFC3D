#include "ofApp.h"

void ofApp::setup(){
    // SCENE
    cam.setDistance(10);
    cam.setNearClip(0.1);

    material.setDiffuseColor( ofColor(220, 220, 220) );
    light.move(2,-5,-1);
    light.lookAt(ofVec3f(0.0,0.0,0.0));
    light.setDirectional();
	light.setDiffuseColor( ofColor(240, 240, 210) );
    light.setAmbientColor( ofColor(60,50,90) );
    light.setup();

    // MODELS
    // model made on blender and exported as ply 
    // Y Up
    // Z forward
    m_line.load("line.ply");
    m_end_l.load("end_l.ply");
    m_end_r.load("end_r.ply");
    m_turn.load("turn.ply");
    m_base_line.load("base_line.ply");
    m_base_end_l.load("base_end_l.ply");
    m_base_end_r.load("base_end_r.ply");

    // set tile-name mapping
    tiles["line"] = &m_line;
    tiles["end_l"] = &m_end_l;
    tiles["end_r"] = &m_end_r;
    tiles["turn"] = &m_turn;
    tiles["base_line"] = &m_base_line;
    tiles["base_end_l"] = &m_base_end_l;
    tiles["base_end_r"] = &m_base_end_r;

    world_node.setOrientation(glm::angleAxis(ofDegToRad(0.f), glm::vec3{1.f, 0.f, 0.f}));
}

//--------------------------------------------------------------
void ofApp::update(){
}

//--------------------------------------------------------------
void ofApp::draw(){
    auto render = ofGetCurrentRenderer();
    cam.begin();
    ofEnableDepthTest();

    if (show_tiles)
        ofDrawGrid(1.0, 10, false, false, true, false);

    // draw tiles
    light.enable();

    for (auto& node : nodes) {
        auto key = node.first;
        node.second.transformGL();
        if (show_tiles) {
            // show rotation
            ofSetColor(255);
            int ri = int(node.second.getOrientationEulerRad().y+2.0);
            if (ri == 0) ri = 3;
            else if (ri == 1) ri = 2;
            else if (ri == 2) ri = 0;
            else if (ri == 3) ri = 1;
            render->drawString(ofToString(ri), 0, 0.5, 0);

            if (key == "line" || key == "base_line") material.setDiffuseColor( ofColor(220, 100, 100) );
            if (key == "end_l" || key == "base_end_l") material.setDiffuseColor( ofColor(100, 220, 100) );
            if (key == "end_r" || key == "base_end_r") material.setDiffuseColor( ofColor(100, 100, 220) );
            if (key == "turn") material.setDiffuseColor( ofColor(100, 220, 220) );
        } else  material.setDiffuseColor( ofColor(220, 220, 220) );

        material.begin();
        tiles[key]->draw();
        material.end();
        node.second.restoreTransformGL();
    }

    light.disable();
    cam.end();

    ofDisableDepthTest();
    ofDisableLighting();
    ofDrawBitmapStringHighlight("FPS " + ofToString(ofGetFrameRate(),0), 20, 20);

    ofDrawBitmapString("PRESS SPACEBAR TO GENERATE A NEW STRUCTURE", 20, ofGetHeight() - 40);
    ofDrawBitmapString("PRESS S TO VISUALIZE THE TILES", 20, ofGetHeight() - 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 115) show_tiles = !show_tiles;
    if (key == 32) {
        int x = 9, y = 5, z = 9;

        // config_name, subset, x, y, z, periodic=false, ground="", surround=false
        wfc.SetUp("data.xml", "default", x, y, z, false, "", "empty");

        // instanciate an specific tile on the WFC
        /*
         *for (int ix = 1; ix < x-1; ix++)
         *    for (int iz = 1; iz < z-1; iz++)
         *        wfc.SetTile("empty", ix,y-1,iz);
         */

        int limit = 8, seed = (int)ofRandom(1000);
        for (int k = 0; k < limit; k++) {
            bool result = wfc.Run(seed++);

            if (result) {
                ofLog() << "WFC success";

                // process tiles and convert to ofNode tree
                nodes = wfc.NodeTileOutput(world_node, ofVec3f(vs,vs,vs), {"empty"});
                break;
            } else {
                ofLog() << "WFC failure";
            }
        }

        world_node.setPosition(-x/2.0, 0, -z/2.0);
        world_node.move(0.5,0.5,0.5);
        world_node.setScale(1/vs);

    }
}

