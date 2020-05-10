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

    //for (auto& node : nodes) {
    for (int i = 0; i < tileNodes.size(); i++) { 
        // transform the model
        tileNodes[i].transformGL();
        // draw instance of the model
        tileMeshes[ modelIndices[i] ]->drawInstanced(OF_MESH_FILL, 1);

        tileNodes[i].restoreTransformGL();
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

        // config_name, subset, x, y, z, periodic=false, ground="", surround=""
        wfc.SetUp("data.xml", "default", x, y, z);
        

        int limit = 8, seed = (int)ofRandom(1000);
        for (int k = 0; k < limit; k++) {
            bool result = wfc.Run(seed++);

            if (result) {
                ofLog() << "WFC success";

                // process tiles and convert to ofNode tree
                //nodes = wfc.NodeTileOutput(world_node, glm::vec3(vs,vs,vs), {"empty"});

                tileNodes = wfc.getNodes(world_node, glm::vec3(vs,vs,vs), {"empty"});
                modelIndices = wfc.getIndices({"empty"}); // 'empty' ignored on getNodes()

                // mapping tileMeshes as the xml config file order.
                // this can be done once on the setup manually, by reading the xml file or using getTileName()
                auto tileNames = wfc.getTileNames({"empty"}); // also ignoring the same tile
                tileMeshes.clear();
                for (auto& name : tileNames) {
                    if (name == "down") tileMeshes.push_back(&m_down);
                    if (name == "line") tileMeshes.push_back(&m_line);
                    if (name == "turn") tileMeshes.push_back(&m_turn);
                    if (name == "up") tileMeshes.push_back(&m_up);
                    if (name == "vertical") tileMeshes.push_back(&m_vertical);
                }

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

