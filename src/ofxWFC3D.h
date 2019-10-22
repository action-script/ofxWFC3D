#pragma once

#include <iostream>
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
    ofxWFC3D(std::string name, std::string subset_name, size_t max_x, size_t max_y, size_t max_z, bool periodic, std::string ground_name);
    bool Run(int seed);
    std::string TextOutput();

protected:
    Status Observe();
    bool Propagate();
    void Clean();

    size_t max_x, max_y, max_z;
    size_t num_patterns;
    bool periodic;

    std::vector<double> pattern_weight;
};

