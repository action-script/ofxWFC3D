#pragma once

#include "ofMain.h"
#include <regex>
#include "arrays.h"

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

struct InstancedTile {
    size_t t;
    size_t x;
    size_t y;
    size_t z;
};

class ofxWFC3D {
public:
    ofxWFC3D() {}
    void SetUp(
            std::string name, std::string subset_name,
            size_t max_x, size_t max_y, size_t max_z,
            bool periodic=false,
            std::string ground_name = "",
            std::string surround_name = ""
    );
    bool Run(int seed);
    bool SetTile(std::string tile_name, size_t x, size_t y, size_t z);
    bool SetTile(std::string tile_name, glm::vec3 position) {
        return this->SetTile(tile_name, position.x, position.y, position.z);
    };

    // getters
    std::vector<ofNode> getNodes(
            ofNode& parent_node, glm::vec3 grid_size = glm::vec3(1.f,1.f,1.f),
            std::vector<std::string> ignore = {""});
    std::vector<size_t> getIndices(std::vector<std::string> ignore = {""});
    std::vector<std::string> getTileNames(std::vector<std::string> ignore = {""});

    // output
    std::string TextOutput();
    std::vector< std::vector< std::vector< std::unordered_map<std::string, size_t >> > > TileOutput();
    std::vector< std::pair<std::string, ofNode> > NodeTileOutput(
            ofNode& parent_node, glm::vec3 grid_size = glm::vec3(1.f,1.f,1.f),
            std::vector<std::string> ignore = {""});

protected:
    Status Observe();
    bool Propagate();
    void Clear();

private:
    size_t max_x, max_y, max_z;
    size_t num_patterns;
    bool periodic;

    int ground_id, surround_id;

    Array4D<Bool> wave;
    Array3D<Bool> changes;
    Array3D<int> observed;

    Array3D<Bool> propagator;
    std::vector<double> pattern_weight;
    std::vector< std::pair<size_t, size_t> > height_range;
    std::vector<std::string> tile_data;

    std::vector<InstancedTile> instanced_tiles;

    std::vector<double> log_prob;
	double log_T;

    ofXml xml;
};

