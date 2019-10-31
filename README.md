# Wave Function Collapse

A C++ implementation of the [Wave Function Collapse](https://github.com/mxgmn/WaveFunctionCollapse) **Tile-Model** algorithm by @[mxgmn](https://github.com/mxgmn) ready for OpenFrameworks.

![wfc_git_01](./wfc_git_01.png)

![wfc_git_02](./wfc_git_02.png)



### About

**ofxWFC3D** works in 3 dimensions, uses XML configuration files and outputs ofNode elements.

It supports the original symmetry, rotations and subsets. It also implements new functionalities.

**ofxWFC3D** does not require any specific hardware or external libraries and work in every OS.

![symmetry](./symmetry.png)



### Usage

```c++
// instance the WFC3D
ofxWFC3D wfc;

// config_name, subset, x, y, z, periodic=false, ground_name="", surround_name=""
wfc.SetUp("data.xml", "default", width, height, length);

// run the WFC3D
// (the algorithm may runs into contradictions)
wfc.Run(seed);

// get the list of ofNodes, using 'world_node' as parent
ofNode world_node;
auto nodes = wfc.NodeTileOutput(world_node, ofVec3f(size_x,size_y,size_z));

```



#### Specific tiles instantiation

It reduces entropy and limits randomness creating predictable results.

```c++
// instanciate an specific tile on the WFC
wfc.SetTile("tile", x, y, z); 
```



### Special features

#### + symmetry

This version of the WFC, includes tiles without symmetry.
For complex structures and systems that require a very concrete tile union, use the symmetry type "**+**".

```xml
<tile name="corner" symmetry="+"/>
```



#### Height range

```xml
<tile name="roof" symmetry="I" min-height="2" max-height="4"/>
```



## License

ofxWFC3D is distributed under the MIT License.