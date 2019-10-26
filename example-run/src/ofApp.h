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
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        

        // scene
        ofEasyCam cam;
        ofBoxPrimitive container;
        ofMaterial material;
        ofLight light;

        // models
        ofMesh m_down, m_line, m_turn, m_up, m_vertical, m_cube, m_totem;
        ofNode worldNode;
        std::unordered_map<std::string, ofMesh*> tiles;
        std::vector< std::unordered_map<std::string, ofNode> > nodes;
        float vs = 3.0; // voxel size

        // WFC
        ofxWFC3D wfc;

        // gui
        ofxPanel gui;
        ofParameterGroup structure_group;
        ofParameter<float> bound_width, bound_height, bound_length;
};
