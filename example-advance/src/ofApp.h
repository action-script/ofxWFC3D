#pragma once

#include "ofMain.h"
#include "ofxWFC3D.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
        

        // scene
        ofEasyCam cam;
        ofMaterial material;
        ofLight light;

        // models
        ofNode world_node;
        ofMesh m_line, m_end_l, m_end_r, m_turn, m_base_line, m_base_end_l, m_base_end_r;
        std::unordered_map<std::string, ofMesh*> tiles;
        std::vector< std::pair<std::string, ofNode> > nodes;
        float vs = 2.0; // model size (2m)

        // WFC
        ofxWFC3D wfc;

        bool show_tiles = false;
};
