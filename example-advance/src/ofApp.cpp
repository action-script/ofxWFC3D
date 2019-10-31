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



    // GUI
    structure_group.add( bound_width.set("bounding width", 8, 1, 20 ));
    structure_group.add( bound_height.set("bounding height", 1, 1, 20 ));
    structure_group.add( bound_length.set("bounding length", 8, 1, 20 ));
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
    auto render = ofGetCurrentRenderer();
    cam.begin();
    ofEnableDepthTest();

    ofDrawGrid(1.0, 10, false, false, true, false);
    ofDrawAxis(10);


    ofSetColor(50, 50, 200);
    container.drawWireframe();

    // draw tiles
    light.enable();

    for (auto& node : nodes) {
        auto key = node.first;
        if (key == "line" || key == "base_line") material.setDiffuseColor( ofColor(220, 100, 100) );
        if (key == "end_l" || key == "base_end_l") material.setDiffuseColor( ofColor(100, 220, 100) );
        if (key == "end_r" || key == "base_end_r") material.setDiffuseColor( ofColor(100, 100, 220) );
        if (key == "turn") material.setDiffuseColor( ofColor(100, 220, 220) );
        material.begin();
        node.second.transformGL();
        tiles[key]->draw();
        material.end();
        ofSetColor(255);
        int ri = int(node.second.getOrientationEulerRad().y+2.0);
        if (ri == 0) ri = 3;
        else if (ri == 1) ri = 2;
        else if (ri == 2) ri = 0;
        else if (ri == 3) ri = 1;
        render->drawString(ofToString(ri), 0, 0.5, 0);
        node.second.restoreTransformGL();
    }

    light.disable();
    cam.end();

    ofDisableDepthTest();
    ofDisableLighting();
    gui.draw();
    ofDrawBitmapStringHighlight("FPS " + ofToString(ofGetFrameRate(),0), 20, 20);

    ofSetColor(255);
    //ofDrawBitmapString("PRESS SPACEBAR TO GENERATE A NEW STRUCTURE", 20, ofGetHeight() - 40);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 32) {
        int x = bound_width, y = bound_height, z = bound_length;


        // config_name, subset, x, y, z, periodic=false, ground="", surround=false
        wfc.SetUp("data.xml", "default", x, y, z, false, "", "empty");

        // instanciate an specific tile on the WFC
        //for (int ix = 1; ix < x-1; ix++)
            //for (int iz = 1; iz < z-1; iz++)
                //wfc.SetTile("empty", ix,y-1,iz);

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

