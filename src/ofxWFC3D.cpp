#include "ofxWFC3D.h"

size_t weightedRandom(const std::vector<double>& a, double between_zero_and_one);

// ------------------

void ofxWFC3D::SetUp(std::string config_file, std::string subset_name, size_t max_x, size_t max_y, size_t max_z, bool periodic, std::string ground_name, std::string surround_name)
{
    //initialize members
    this->max_x = max_x;
    this->max_y = max_y;
    this->max_z = max_z;
    this->periodic = periodic;

    ground_id = -1;
    surround_id = -1;

    this->pattern_weight.clear();
    this->height_range.clear();
    this->tile_data.clear();
    this->instanced_tiles.clear();

    if ( !xml.load(config_file) ) {
        ofLogError() << "Couldn't load WFC configuration file";
    }
    auto xmln_set = xml.getChild("set");

    std::vector<std::string> subset;

    if (subset_name != "" && subset_name != "default") {
        auto xmln_subsets = xmln_set.getChild("subsets");
        for (auto& xmln_subset : xmln_subsets.getChildren()) {
            if (subset_name == xmln_subset.getAttribute("name").getValue()) {
                for (auto & xmln_stiles : xmln_subset.getChildren()) {
                    subset.push_back(xmln_stiles.getAttribute("name").getValue());
                }
            } 
        }
    }


	std::vector<std::array<int, 8>> action;
	std::unordered_map<std::string, size_t> first_occurrence;
	std::vector<int> no_symmetry;

    auto xmln_tiles = xmln_set.getChild("tiles");

    for (auto& xmln_tile : xmln_tiles.getChildren()) {
        const std::string tile_name = xmln_tile.getAttribute("name").getValue();

        // find tile_name on subset
        //auto it = std::find(subset.begin(), subset.end(), tile_name);
        //if (subset.size() > 0 && it == subset.end()) continue;
        if (subset.size() > 0 && !ofContains(subset, tile_name)) continue;

        std::function<int(int)> a, b;
		int cardinality;

        const std::string sym = xmln_tile.getAttribute("symmetry").getValue();
        if (sym == "L") {
            cardinality = 4;
            a = [](int i){ return (i + 1) % 4; };
            b = [](int i){ return i % 2 == 0 ? i + 1 : i - 1; };
        } else if (sym == "T") {
            cardinality = 4;
            a = [](int i){ return (i + 1) % 4; };
            b = [](int i){ return i % 2 == 0 ? i : 4 - i; };
        } else if (sym == "+") {
            cardinality = 4;
            a = [](int i){ return (i + 1) % 4; };
            //b = [](int i){ return (i + 2) % 4; };
            b = [](int i){ return i; };
            no_symmetry.push_back(action.size());
        } else if (sym == "I") {
            cardinality = 2;
            a = [](int i){ return 1 - i; };
            b = [](int i){ return i; };
        } else if (sym == "\\") {
            cardinality = 2;
            a = [](int i){ return 1 - i; };
            b = [](int i){ return 1 - i; };
        } else if (sym == "X") {
            cardinality = 1;
            a = [](int i){ return i; };
            b = [](int i){ return i; };
        } else {
            ofLogError() << "Unknown symmetry " << sym;
        }

        num_patterns = action.size();
		first_occurrence[tile_name] = num_patterns;

        if (tile_name == ground_name) ground_id = num_patterns;
        if (tile_name == surround_name) surround_id = num_patterns;

        for (int t = 0; t < cardinality; ++t) {
            std::array<int, 8> map;

            // Rotation
			map[0] = t;
			map[1] = a(t);
			map[2] = a(a(t));
			map[3] = a(a(a(t)));

            // Mirror of Rotations
			map[4] = b(t);
			map[5] = b(a(t));
			map[6] = b(a(a(t)));
			map[7] = b(a(a(a(t))));

			for (int s = 0; s < 8; ++s) {
				map[s] += num_patterns;
			}

			action.push_back(map);
		}

        tile_data.push_back(tile_name+" 0");
        for (int t = 1; t < cardinality; t++) {
            tile_data.push_back(tile_name + (" " + ofToString(t)));
		}

        // weights
        auto xmla_tile_weight = xmln_tile.getAttribute("weight");
        float tile_weight = xmla_tile_weight.getValue() == "" ? 1.0f : xmla_tile_weight.getFloatValue();
        for (int t = 0; t < cardinality; t++) {
		    pattern_weight.push_back(tile_weight);
		}

        // height range
        auto xmla_minheight = xmln_tile.getAttribute("min-height"); 
        auto xmla_maxheight = xmln_tile.getAttribute("max-height");
        size_t tile_minheight = xmla_minheight.getValue() == "" ? 0 : xmla_minheight.getIntValue();
        size_t tile_maxheight = xmla_maxheight.getValue() == "" ? this->max_z : xmla_maxheight.getIntValue();
        for(int t = 0; t < cardinality; t++) {
            height_range.push_back({tile_minheight, tile_maxheight});
        }

    } // end tiles


    num_patterns = action.size();

    propagator = Array3D<Bool> (6,num_patterns,num_patterns, false);

    wave = Array4D<Bool> (max_x, max_y, max_z, num_patterns, false);
    changes = Array3D<Bool> (max_x, max_y, max_z, false);
    observed = Array3D<int> (max_x, max_y, max_z, -1);


    auto xmln_neighbors = xmln_set.getChild("neighbors");
    for (auto& xmln_neighbor : xmln_neighbors.getChildren()) {
        const std::string neighbor_type = xmln_neighbor.getName();
        auto left = ofSplitString( xmln_neighbor.getAttribute("left").getValue(), " ", true);
        auto right = ofSplitString( xmln_neighbor.getAttribute("right").getValue(), " ", true);

        if (left.size() == 1) left.push_back("0");
        if (right.size() == 1) right.push_back("0");

        if (subset.size() > 0 && (!ofContains(subset, left[0]) || !ofContains(subset, right[0]))) continue;


        int L = action[first_occurrence[left[0]]] [ofToInt(left[1])];
		int R = action[first_occurrence[right[0]]][ofToInt(right[1])];
		int D = action[L][1]; // turn +1 = 90 anticlockwise 
		int U = action[R][1];

        // allow [+] tiles to work without symmetry
        bool symmetry = true;
        for (auto& s : no_symmetry)
            if (s == first_occurrence[left[0]] || s == first_occurrence[right[0]]) symmetry = false;

        // coord and axis replacement for xyz | x+ yUp zForward
        // 0 -> 0
        // 1 -> 4
        // 2 -> 2
        // 3 -> 5
        // 4 -> 1
        // 5 -> 3

        if (neighbor_type == "horizontal") {
            propagator(0, R, L) = true;
            propagator(0, action[R][6], action[L][6]) = symmetry;
            propagator(0, action[L][4], action[R][4]) = symmetry;
            propagator(0, action[L][2], action[R][2]) = true;

            propagator(4, U, D) = true;
            propagator(4, action[D][6], action[U][6]) = symmetry;
            propagator(4, action[U][4], action[D][4]) = symmetry;
            propagator(4, action[D][2], action[U][2]) = true;
        } else {
            for (int g = 0; g < 8; g++) propagator(1, action[L][g], action[R][g]) = true;

        }

        for (size_t t1 = 0; t1 < num_patterns; ++t1) {
            for (size_t t2 = 0; t2 < num_patterns; ++t2) {
                propagator(2, t1, t2) = propagator(0, t2, t1);
                propagator(5 ,t1, t2) = propagator(4, t2, t1);
                propagator(3, t1, t2) = propagator(1, t2, t1);
            }
        }

    } // end neighbors


    //ofLog() << "Initialize Done";
}

Status ofxWFC3D::Observe()
{
    //ofLog() << " -- Observing --";
    double min = 1E+3, sum, main_sum, log_sum, noise, entropy;

    
    // Lowest Entropy Coordinate Selection
    int selected_x = -1, selected_y = -1, selected_z = -1, amount;

    for (size_t x = 0; x < max_x; x++) {
        for (size_t y = 0; y < max_y; y++) {
            for (size_t z = 0; z < max_z; z++) {
                amount = 0;
                sum = 0;

                for (size_t t = 0; t < num_patterns; t++) {
                    if (wave(x,y,z,t)) {
                        amount += 1;
                        sum += pattern_weight[t];
                    }
                }

                if (sum == 0) return Status::ObsFail;

                noise = 1E-6 * ofRandom(1);

                // Only 1 Tile Valid, So Entropy must be lowest
                if (amount == 1)
                    entropy = 0;
                else if (amount == num_patterns)
                    entropy = log_T;
                else {
                    main_sum = 0;
                    log_sum = log(sum);

                    for (size_t t = 0; t < num_patterns; t++) {
                        if (wave(x,y,z,t)) main_sum += pattern_weight[t] * log_prob[t];
                    }

                    entropy = log_sum - main_sum / sum;
                }

                if (entropy > 0 && entropy + noise < min) {
                    min = entropy + noise;
                    selected_x = x;
                    selected_y = y;
                    selected_z = z;
                }
                
            }
        }
    } // end of x,y,z

    // No Tile Got Selected
    if (selected_x == -1 && selected_y == -1 && selected_z == -1) {

        for (size_t x = 0; x < max_x; x++) {
            for (size_t y = 0; y < max_y; y++) {
                for (size_t z = 0; z < max_z; z++) {
                    
                    for (size_t t = 0; t < num_patterns; t++) {
                        if (wave(x, y, z, t)) {
                            observed(x, y, z) = t;
                            break;
                        }
                    }

                }
            }
        }

        return Status::ObsSuccess;

    }

    std::vector<double> distribution(num_patterns);

    for (size_t t = 0; t < num_patterns; t++) {
        distribution[t] = wave(selected_x, selected_y, selected_z, t) ? pattern_weight[t] : 0;
    }
    int r = weightedRandom(std::move(distribution), ofRandom(1));
    for (size_t t = 0; t < num_patterns; t++) {
        wave(selected_x, selected_y, selected_z, t) = t == r;
    }
    changes(selected_x, selected_y, selected_z) = true;


    return Status::ObsUnfinished;
}

bool ofxWFC3D::Propagate()
{
    //ofLog() << " -- Propagating --";
    bool change = false, can_prop;

    for (size_t x2 = 0; x2 < max_x; x2++) {
        for (size_t y2 = 0; y2 < max_y; y2++) {
            for (size_t z2 = 0; z2 < max_z; z2++) {
                for (size_t d = 0; d < 6; d++) {

                    int x1 = x2, y1 = y2, z1 = z2;

                     // Periodic Check
                    if (d == 0) { // <
                        if (x2 == 0) {
                            if (!periodic) continue;
                            else x1 = max_x - 1;
                        }
                        else x1 = x2 - 1;
                    }
                    else if (d == 1) { // ^
                        if (y2 == max_y - 1) {
                            if (!periodic) continue;
                            else y1 = 0;
                        }
                        else y1 = y2 + 1;
                    }
                    else if (d == 2) { // >
                        if (x2 == max_x - 1) {
                            if (!periodic) continue;
                            else x1 = 0;
                        }
                        else x1 = x2 + 1;
                    }
                    else if (d == 3) { // v
                        if (y2 == 0) {
                            if (!periodic) continue;
                            else y1 = max_y - 1;
                        }
                        else y1 = y2 - 1;
                    }
                    else if (d == 4) { // z>
                        if (z2 == max_z - 1) {
                            if (!periodic) continue;
                            else z1 = 0;
                        }
                        else z1 = z2 + 1;
                    }
                    else { // <z
                        if (z2 == 0) {
                            if (!periodic) continue;
                            else z1 = max_z - 1;
                        }
                        else z1 = z2 - 1;
                    } // end periodic check


                    // No Changes this Voxel
                    if (!changes(x1, y1, z1)) {
                        continue;
                    }

                    // x2 = Original Voxel
                    // x1 = Neighbor Depending on Direction (0 - 6) and Periodicity
                    
                    for (size_t t2 = 0; t2 < num_patterns; t2++)  {
                        if (wave(x2, y2, z2, t2)) {
                            //auto prop = propagator[d][t2];
                            can_prop = false;

                            for (size_t t1 = 0; t1 < num_patterns && !can_prop; t1++) {
                                if (wave(x1, y1, z1, t1)) {
                                    can_prop = propagator(d, t2, t1); // t2 -> t1 Propagate Check
                                }
                            }

                            if (!can_prop) {
                                wave(x2, y2, z2, t2) = false;
                                changes(x2, y2, z2) = true;
                                change = true;
                            }

                        }
                    } // end t2


                }
            }
        }
    } // end x2, y2, z2

    return change;
}

void ofxWFC3D::Clear()
{
    for (size_t x = 0 ; x < max_x; x++) {
        for (size_t y = 0 ; y < max_y; y++) {
            for (size_t z = 0 ; z < max_z; z++) {

                changes(x, y, z) = false;
                for (size_t t = 0; t < num_patterns; t++) {
                    int h_min = height_range[t].first;
                    int h_max = height_range[t].second;

                    if (h_min <= y && h_max >= y) {
                        wave(x, y, z, t) = true;
                        changes(x, y, z) = true;
                    } else {
                        wave(x, y, z, t) = false;
                    }
                }

            }
        }
    }

    if (ground_id >= 0) {

        for (size_t x = 0 ; x < max_x; x++) {
            for (size_t z = 0 ; z < max_z; z++) {

                for (size_t t = 0; t < num_patterns; t++) {
                    if (t != ground_id) wave(x, 0, z, t) = false;
                }
                changes(x, 0, z) = true;

                //for (size_t z = 0; z < max_z - 1; z++) {
                for (size_t y = 1; y < max_y; y++) {
                    wave(x, y, z, ground_id) = false;
                    changes(x, y, z) = true;
                }
            }
        }
    }

    if (surround_id >= 0) {
        for (size_t y = 0 ; y < max_y; y++) {
            for (size_t z = 0 ; z < max_z; z++) {

                for (size_t t = 0; t < num_patterns; t++) {
                    if (t != surround_id) {
                        wave(0, y, z, t) = false;
                        wave(max_x-1, y, z, t) = false;
                    }
                }
                changes(0, y, z) = true;
                changes(max_x-1, y, z) = true;
            }

            for (size_t x = 0 ; x < max_x; x++) {
                for (size_t t = 0; t < num_patterns; t++) {
                    if (t != surround_id) {
                        wave(x, y, 0, t) = false;
                        wave(x, y, max_z-1, t) = false;
                    }
                }
                changes(x, y, 0) = true;
                changes(x, y, max_z-1) = true;

            }
        }
    }

    for (auto& instanced : instanced_tiles) {
        for (size_t t = 0; t < num_patterns; t++) {
            if (t != instanced.t) wave(instanced.x, instanced.y, instanced.z, t) = false;
        }
        wave(instanced.x, instanced.y, instanced.z, instanced.t) = true;
        changes(instanced.x, instanced.y, instanced.z) = true;
    }
}

bool ofxWFC3D::SetTile(std::string tile_name, size_t x, size_t y, size_t z)
{
    if (std::regex_match (tile_name, std::regex("([_[:alnum:]]*)", std::regex::ECMAScript) ))
        tile_name += " 0";

    bool found = false;

    if (x < max_x && y < max_y && z < max_z && x >= 0 && y >= 0 && z >= 0) {
        for (int i = 0; i < tile_data.size(); i++) {
            if (tile_data[i] == tile_name) {
                instanced_tiles.push_back({static_cast<size_t>(i), x, y ,z});
                break;
            }
        }
    }

    return found;
}

bool ofxWFC3D::Run(int seed)
{
    log_T = log(num_patterns);

    for (size_t t = 0; t < num_patterns; t++)
        log_prob.push_back( log(pattern_weight[t]) );

    Clear();

    ofSeedRandom(seed);

    while(true) {

        auto result = Observe();
        if (result != Status::ObsUnfinished)
            return result == Status::ObsSuccess ? true : false;

        while(Propagate());
    }
    return false;
}

std::string ofxWFC3D::TextOutput()
{
    std::string result = "";
    for (size_t x = 0 ; x < max_x; x++) {
        for (size_t y = 0 ; y < max_y; y++) {
            for (size_t z = 0 ; z < max_z; z++) {
                result.append("[");
                result.append(tile_data[observed(x, y, z)]);
                result.append("], ");
            }
            result.append("\n");
        }
        result.append("\n");
    }
    
    return result;
}

std::vector< std::vector< std::vector< std::unordered_map<std::string, size_t >> > > ofxWFC3D::TileOutput()
{
    std::vector< std::vector< std::vector< std::unordered_map<std::string, size_t >> > > tiles;
    tiles.resize(max_x);
    for (size_t x = 0 ; x < max_x; x++) {
        tiles[x].resize(max_y);
        for (size_t y = 0 ; y < max_y; y++) {
            tiles[x][y].resize(max_z);
            for (size_t z = 0 ; z < max_z; z++) {

                auto tile_cardinality = ofSplitString(tile_data[observed(x, y, z)], " ", true);
                tiles[x][y][z][tile_cardinality[0]] = ofToInt(tile_cardinality[1]);
                 
            }
        }
    }

    return tiles;
}

std::vector< std::pair<std::string, ofNode> > ofxWFC3D::NodeTileOutput(ofNode& parent_node, ofVec3f grid_size, std::vector<std::string> ignore)
{
    ofVec3f axis_y = ofVec3f(0.0, 1.0, 0.0);
    std::vector< std::pair<std::string, ofNode> > tiles;
    for (size_t x = 0 ; x < max_x; x++) {
        for (size_t y = 0 ; y < max_y; y++) {
            for (size_t z = 0 ; z < max_z; z++) {

                auto tile_cardinality = ofSplitString(tile_data[observed(x, y, z)], " ", true);
                if ( ofContains(ignore, tile_cardinality[0]) ) continue;


                ofNode tile_node;
                tile_node.setParent(parent_node);
                tile_node.setPosition(x*grid_size.x, y*grid_size.y, z*grid_size.z);
                tile_node.rotateDeg(ofToInt( tile_cardinality[1])*90.0f, axis_y );

                std::pair<std::string, ofNode> tile = std::make_pair(tile_cardinality[0], tile_node);

                tiles.push_back(tile);

            }
        }
    }

    return tiles;
}

size_t weightedRandom(const std::vector<double>& a, double between_zero_and_one)
{
    double sum = 0;
    for (int i = 0; i < a.size(); ++i) sum += a[i];

	if (sum == 0.0) {
		return std::floor(between_zero_and_one * a.size());
	}

	double between_zero_and_sum = between_zero_and_one * sum;

	double accumulated = 0;
    
    for (int i = 0; i < a.size(); i++) {
		accumulated += a[i];
		if (between_zero_and_sum <= accumulated)
			return i;
	}

	return 0;
}
