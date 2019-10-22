#include "ofxWFC3D.h"

ofxWFC3D::ofxWFC3D(std::string config_name, std::string subset_name, size_t max_x, size_t max_y, size_t max_z, bool periodic, std::string ground_name) :
    max_x(max_x), max_y(max_y), max_z(max_z), periodic(periodic)
{
    std::cout << "Hello world" << std::endl;
}

