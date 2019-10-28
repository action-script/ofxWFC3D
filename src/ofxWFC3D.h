#pragma once

#include "ofMain.h"

/*
 * Implementation based on 
 * https://github.com/mxgmn/WaveFunctionCollapse
 * https://bitbucket.org/mxgmn/basic3dwfc/src/master/
 * https://github.com/emilk/wfc/
 */

enum class Status {
    ObsSuccess,
    ObsFail,
    ObsUnfinished,
};

class ofxWFC3D {
public:
    ofxWFC3D() {}
    void SetUp(std::string name, std::string subset_name, size_t max_x, size_t max_y, size_t max_z, bool periodic=false, std::string ground_name = "", std::string surround_name = "");
    bool Run(int seed);
    std::string TextOutput();
    std::vector< std::vector< std::vector< std::unordered_map<std::string, size_t >> > > TileOutput();
    std::vector< std::pair<std::string, ofNode> > NodeTileOutput(ofNode& parent_node, ofVec3f grid_size, std::vector<std::string> ignore = {""});

protected:
    Status Observe();
    bool Propagate();
    void Clear();

private:
    size_t max_x, max_y, max_z;
    size_t num_patterns;
    bool periodic;

    int ground_id, surround_id;

    std::vector< std::vector< std::vector< std::vector<bool> > > > wave;    // bool [][][][]
    std::vector< std::vector< std::vector<bool> > > changes;                // bool [][][]
    std::vector< std::vector< std::vector<int> > > observed;                // int  [][][]

    std::vector< std::vector< std::vector<bool> > > propagator;             // bool [][][]
    std::vector<double> pattern_weight;
    std::vector<std::string> tile_data;

    std::vector<double> log_prob;
	double log_T;

    ofXml xml;
};

