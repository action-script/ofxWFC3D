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
        ofNode worldNode;
        ofMesh m_down, m_line, m_turn, m_up, m_vertical;
        std::unordered_map<std::string, ofMesh*> tiles;
        std::vector< std::pair<std::string, ofNode> > nodes;
        float vs = 3.0; // model size (3 blender units)

        // WFC
        ofxWFC3D wfc;

        // gui
        ofxPanel gui;
        ofParameterGroup structure_group;
        ofParameter<float> bound_width, bound_height, bound_length;
};
