#include "ofApp.h"

void ofApp::setup(){
    //ofSetVerticalSync(true);

    // SCENE
    cam.setDistance(20);
    cam.setNearClip(0.1);

    material.setDiffuseColor( ofColor(220, 220, 220) );
    light.move(2,-5,-1);
    light.lookAt(glm::vec3(0.0,0.0,0.0));
    light.setDirectional();
	light.setDiffuseColor( ofColor(240, 240, 210) );
    light.setAmbientColor( ofColor(60,50,90) );
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

    // set tile-name mapping
    tiles["down"] = &m_down;
    tiles["line"] = &m_line;
    tiles["turn"] = &m_turn;
    tiles["up"] = &m_up;
    tiles["vertical"] = &m_vertical;

    world_node.setOrientation(glm::angleAxis(ofDegToRad(0.f), glm::vec3{1.f, 0.f, 0.f}));


    // GUI
    structure_group.add( bound_width.set("bounding width", 8, 1, 20 ));
    structure_group.add( bound_height.set("bounding height", 4, 1, 20 ));
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
    cam.begin();
    ofEnableDepthTest();

    ofDrawGrid(1.0, 10, false, false, true, false);
    ofDrawAxis(10);

    ofSetColor(50, 50, 200);
    container.drawWireframe();

    // draw tiles
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

    ofSetColor(255);
    ofDrawBitmapString("PRESS SPACEBAR TO GENERATE A NEW STRUCTURE", 20, ofGetHeight() - 40);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 32) {
        int x = bound_width, y = bound_height, z = bound_length;


        // config_name, subset, x, y, z, periodic=false, ground="", surround=false
        wfc.SetUp("data.xml", "default", x, y, z);
        

        int limit = 8, seed = (int)ofRandom(1000);
        for (int k = 0; k < limit; k++) {
            bool result = wfc.Run(seed++);

            if (result) {
                ofLog() << "WFC success";

                // process tiles and convert to ofNode tree
                nodes = wfc.NodeTileOutput(world_node, glm::vec3(vs,vs,vs), {"empty"});
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

