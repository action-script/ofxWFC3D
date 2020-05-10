#pragma once

#include "ofMain.h"
#include "ofxWFC3D.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
        

        // scene
        ofEasyCam cam;
        ofBoxPrimitive container;
        ofMaterial material;
        ofLight light;

        // models
        ofVboMesh m_down, m_line, m_turn, m_up, m_vertical; // ofVboMesh allows instancing

        // tile models and positions
        ofNode world_node;
        std::vector<ofVboMesh*> tileMeshes; // each unique tile model
        std::vector<ofNode> tileNodes; // every tile position
        std::vector<size_t> modelIndices; // index of the model for each tile
        float vs = 3.0; // model size (3 blender units)

        // WFC
        ofxWFC3D wfc;

        // gui
        ofxPanel gui;
        ofParameterGroup structure_group;
        ofParameter<float> bound_width, bound_height, bound_length;
};
