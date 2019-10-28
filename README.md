# Wave Function Collapse

A C++ implementation of the [Wave Function Collapse](https://github.com/mxgmn/WaveFunctionCollapse) **Tile-Model** algorithm by @[mxgmn](https://github.com/mxgmn) ready for OpenFrameworks.

### About

ofxWFC3D works in 3 dimensions, uses XML configuration files and outputs ofNode elements.

It supports symmetry, subsets and custom tile instancing.

ofxWFC3D does not require any specific hardware or external libraries and work in every OS.



### Usage

```c++
// instance the WFC3D
ofxWFC3D wfc;

// config_name, subset, x, y, z, periodic=false, ground="", surround=false
wfc.SetUp("data.xml", "default", width, height, length);

// run the WFC3D
// (the algorithm may runs into contradictions)
wfc.Run(seed);

// get the list of ofNodes, using 'world_node' as parent
ofNode world_node;
auto nodes = wfc.NodeTileOutput(world_node, ofVec3f(size_x,size_y,size_z));

```





## License

ofxWFC3D is distributed under the MIT License.