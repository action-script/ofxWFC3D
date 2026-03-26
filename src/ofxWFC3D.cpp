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
	std::vector<size_t> no_symmetry;

    auto xmln_tiles = xmln_set.getChild("tiles");

    // =========================================================================
    // SYMMETRY & ROTATION SYSTEM
    // =========================================================================
    // Coordinate system: X-right, Y-up, Z-forward (right-handed)
    // All rotations are around the Y axis (vertical), anticlockwise when viewed from above.
    //
    // Each tile has a "symmetry" attribute that determines how many distinct
    // orientations it produces (cardinality) and how rotations/mirrors relate them.
    //
    // Two functions define each symmetry group:
    //   a(i) = rotate 90deg CCW around Y.  Maps rotation index i to the next rotation.
    //   b(i) = mirror/reflect the tile.    Maps rotation index i to its mirrored variant.
    //
    // Rotation index i:  0 = 0deg,  1 = 90deg CCW,  2 = 180deg,  3 = 270deg CCW
    //
    // Symmetry types (viewed from above, Y axis pointing at you):
    //
    //   "X" — Fully symmetric (e.g. cube, sphere). All rotations are identical.
    //         Cardinality: 1.  a(i)=i, b(i)=i
    //
    //   "I" — Line symmetry (e.g. straight pipe). 0deg and 180deg are the same.
    //         Cardinality: 2.  a(i)=1-i (flip between 0 and 1), b(i)=i (mirror = identity)
    //
    //   "\" — Diagonal symmetry (e.g. diagonal connector).
    //         Cardinality: 2.  a(i)=1-i, b(i)=1-i (mirror = rotation)
    //
    //   "L" — L-shape (e.g. corner piece). No rotational symmetry, mirror swaps adjacent.
    //         Cardinality: 4.  a(i)=(i+1)%4, b(i)= i even? i+1 : i-1
    //         Mirror swaps: 0<->1, 2<->3
    //
    //   "T" — T-shape (e.g. T-junction). Mirror preserves 0 and 2, swaps 1 and 3.
    //         Cardinality: 4.  a(i)=(i+1)%4, b(i)= i even? i : 4-i
    //         Mirror swaps: 1<->3, keeps 0 and 2
    //
    //   "+" — Cross/plus shape. Has 4 rotations but NO mirror symmetry.
    //         Cardinality: 4.  a(i)=(i+1)%4, b(i)=i (mirror = identity)
    //         Marked in no_symmetry[] to disable mirrored propagation rules.
    // =========================================================================

    for (auto& xmln_tile : xmln_tiles.getChildren()) {
        const std::string tile_name = xmln_tile.getAttribute("name").getValue();

        if (subset.size() > 0 && !ofContains(subset, tile_name)) continue;

        std::function<int(int)> a, b;
		int cardinality;

        const std::string sym = xmln_tile.getAttribute("symmetry").getValue();
        if (sym == "L") {
            cardinality = 4;
            a = [](int i){ return (i + 1) % 4; };       // rotate 90 CCW
            b = [](int i){ return i % 2 == 0 ? i + 1 : i - 1; }; // mirror: swap 0<->1, 2<->3
        } else if (sym == "T") {
            cardinality = 4;
            a = [](int i){ return (i + 1) % 4; };       // rotate 90 CCW
            b = [](int i){ return i % 2 == 0 ? i : 4 - i; };     // mirror: keep 0,2; swap 1<->3
        } else if (sym == "+") {
            cardinality = 4;
            a = [](int i){ return (i + 1) % 4; };       // rotate 90 CCW
            b = [](int i){ return i; };                  // mirror = identity (no reflection)
            no_symmetry.push_back(action.size());        // flag: disable mirrored propagation rules
        } else if (sym == "I") {
            cardinality = 2;
            a = [](int i){ return 1 - i; };             // rotate 90 = flip (0<->1)
            b = [](int i){ return i; };                  // mirror = identity
        } else if (sym == "\\") {
            cardinality = 2;
            a = [](int i){ return 1 - i; };             // rotate 90 = flip (0<->1)
            b = [](int i){ return 1 - i; };             // mirror = same as rotation
        } else if (sym == "X") {
            cardinality = 1;
            a = [](int i){ return i; };                  // rotate = identity (fully symmetric)
            b = [](int i){ return i; };                  // mirror  = identity
        } else {
            cardinality = 0;
            ofLogError() << "Unknown symmetry " << sym;
        }

        num_patterns = action.size();
		first_occurrence[tile_name] = num_patterns;

        if (tile_name == ground_name) ground_id = num_patterns;
        if (tile_name == surround_name) surround_id = num_patterns;

        // Build the action map for this tile.
        // action[pattern_id] is an 8-element array mapping the pattern to its
        // transformed variants (global pattern IDs):
        //   [0] = same orientation (0deg)
        //   [1] = rotated  90deg CCW around Y
        //   [2] = rotated 180deg around Y
        //   [3] = rotated 270deg CCW (= 90deg CW) around Y
        //   [4] = mirrored at 0deg
        //   [5] = mirrored at 90deg
        //   [6] = mirrored at 180deg  (used in propagation as "-90deg mirror")
        //   [7] = mirrored at 270deg
        for (int t = 0; t < cardinality; ++t) {
            std::array<int, 8> map;

            // Rotations (CCW around Y axis, viewed from above)
			map[0] = t;              //   0deg — original orientation
			map[1] = a(t);           //  90deg CCW
			map[2] = a(a(t));        // 180deg
			map[3] = a(a(a(t)));     // 270deg CCW (= 90deg CW)

            // Mirrored variants of each rotation
			map[4] = b(t);           // mirror of   0deg
			map[5] = b(a(t));        // mirror of  90deg
			map[6] = b(a(a(t)));     // mirror of 180deg
			map[7] = b(a(a(a(t)))); // mirror of 270deg

            // Offset by num_patterns to convert local rotation index to global pattern ID
			for (int s = 0; s < 8; ++s) {
				map[s] += num_patterns;
			}

			action.push_back(map);
		}

        // tile_data stores "tileName rotationIndex" strings, one per pattern.
        // rotationIndex: 0=0deg, 1=90deg, 2=180deg, 3=270deg (CCW around Y)
        // Used by NodeTileOutput() / getNodes() to apply the rotation when rendering.
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
        size_t tile_maxheight = xmla_maxheight.getValue() == "" ? this->max_y : xmla_maxheight.getIntValue();
        for(int t = 0; t < cardinality; t++) {
            height_range.push_back({tile_minheight, tile_maxheight});
        }

    } // end tiles


    num_patterns = action.size();

    // =========================================================================
    // PROPAGATOR — neighbor compatibility matrix
    // =========================================================================
    // propagator(d, patternA, patternB) = true means:
    //   "patternA at cell C can have patternB as its neighbor in direction d from C"
    //
    // Direction index convention (6 directions in 3D grid):
    //   d=0: -X  (left)
    //   d=1: +Y  (up)
    //   d=2: +X  (right)
    //   d=3: -Y  (down)
    //   d=4: +Z  (forward, into screen)
    //   d=5: -Z  (back, out of screen)
    //
    // Opposite directions are mirrors of each other:
    //   d=0 (-X) is the reverse of d=2 (+X)
    //   d=3 (-Y) is the reverse of d=1 (+Y)
    //   d=5 (-Z) is the reverse of d=4 (+Z)
    // =========================================================================
    propagator = Array3D<Bool> (6,num_patterns,num_patterns, false);

    wave = Array4D<Bool> (max_x, max_y, max_z, num_patterns, false);
    changes = Array3D<Bool> (max_x, max_y, max_z, false);
    observed = Array3D<int> (max_x, max_y, max_z, -1);
    in_queue = Array3D<Bool> (max_x, max_y, max_z, false);


    auto xmln_neighbors = xmln_set.getChild("neighbors");
    for (auto& xmln_neighbor : xmln_neighbors.getChildren()) {
        // XML neighbor format:
        //   <horizontal left="tileName rotationIndex" right="tileName rotationIndex"/>
        //   <vertical   left="tileName rotationIndex" right="tileName rotationIndex"/>
        // rotationIndex defaults to 0 if omitted.
        //
        // For "horizontal": left/right define an adjacency on the X axis (and Z via rotation).
        //   "left" tile is at -X, "right" tile is at +X.
        //   The XML says: "left" can appear to the LEFT of "right" (or equivalently,
        //   "right" can appear to the RIGHT of "left").
        //
        // For "vertical": left=bottom, right=top (Y axis adjacency).
        //   The XML says: "left" tile can appear BELOW "right" tile.
        const std::string neighbor_type = xmln_neighbor.getName();
        auto left = ofSplitString( xmln_neighbor.getAttribute("left").getValue(), " ", true);
        auto right = ofSplitString( xmln_neighbor.getAttribute("right").getValue(), " ", true);

        if (left.size() == 1) left.push_back("0");
        if (right.size() == 1) right.push_back("0");

        if (subset.size() > 0 && (!ofContains(subset, left[0]) || !ofContains(subset, right[0]))) continue;

        // L = global pattern ID for the "left" tile at its specified rotation
        // R = global pattern ID for the "right" tile at its specified rotation
        // The action table converts (tile_name, rotation_index) → global pattern ID
        int L = action[first_occurrence[left[0]]] [ofToInt(left[1])];
		int R = action[first_occurrence[right[0]]][ofToInt(right[1])];

        // D = L rotated 90deg CCW.  When the horizontal rule is rotated 90deg CCW,
        //     the +X axis becomes +Z, so L (which was the left/-X neighbor) becomes
        //     the forward/+Z neighbor. D is used for Z-axis propagation rules.
        // U = R rotated 90deg CCW.  Same rotation applied to the right tile.
		int D = action[L][1]; // L rotated 90deg CCW around Y
		int U = action[R][1]; // R rotated 90deg CCW around Y

        // Disable mirror-based propagation rules when either tile has "+" symmetry,
        // because "+" tiles have rotational variants but no meaningful mirror.
        bool symmetry = true;
        for (auto& s : no_symmetry)
            if (s == first_occurrence[left[0]] || s == first_occurrence[right[0]]) symmetry = false;

        if (neighbor_type == "horizontal") {
            // --- X-axis rules (direction 2 = +X) ---
            // Base rule: L can have R to its right (+X) at the original orientation
            propagator(2, L, R) = true;
            if (symmetry) {
                // Mirror rotated variants:
                // action[L][6] = L mirrored at 180deg, action[R][6] = R mirrored at 180deg
                propagator(2, action[L][6], action[R][6]) = true;   // mirrored -90deg variant
                // action[R][4] = R mirrored at 0deg, action[L][4] = L mirrored at 0deg
                propagator(2, action[R][4], action[L][4]) = true;   // mirrored +90deg variant
            }
            // 180deg rotation: R and L swap sides
            propagator(2, action[R][2], action[L][2]) = true;       // 180deg rotated variant

            // --- Z-axis rules (direction 4 = +Z) ---
            // The horizontal rule rotated 90deg CCW maps X-axis to Z-axis.
            // U (rotated R) can have D (rotated L) in the +Z direction.
            propagator(4, U, D) = true;
            if (symmetry) {
                propagator(4, action[D][6], action[U][6]) = true;   // mirrored -90deg variant
                propagator(4, action[U][4], action[D][4]) = true;   // mirrored +90deg variant
            }
            propagator(4, action[D][2], action[U][2]) = true;       // 180deg rotated variant

        } else {
            // --- Y-axis rules (direction 1 = +Y) ---
            // Vertical neighbors: all 8 rotation/mirror variants are valid.
            // "left" is below, "right" is above. Vertical adjacency is rotation-invariant
            // around Y, so every transformed pair is also valid.
            for (int g = 0; g < 8; g++) propagator(1, action[L][g], action[R][g]) = true;
        }

        // Derive opposite direction rules by swapping pattern order:
        // "A can have B to its right" implies "B can have A to its left"
        for (size_t t1 = 0; t1 < num_patterns; ++t1) {
            for (size_t t2 = 0; t2 < num_patterns; ++t2) {
                propagator(0, t1, t2) = propagator(2, t2, t1);  // -X from +X (left from right)
                propagator(5 ,t1, t2) = propagator(4, t2, t1);  // -Z from +Z (back from forward)
                propagator(3, t1, t2) = propagator(1, t2, t1);  // -Y from +Y (down from up)
            }
        }

    } // end neighbors

    // Precompute log probabilities once here, not on every Run() call
    log_T = log(num_patterns);
    log_prob.clear();
    log_prob.reserve(num_patterns);
    for (size_t t = 0; t < num_patterns; t++)
        log_prob.push_back(log(pattern_weight[t]));

    // Precompute bitset masks for pattern compatibility (if num_patterns <= 64)
    prop_mask.clear();
    if (num_patterns <= 64) {
        prop_mask.assign(6, std::vector<uint64_t>(num_patterns, 0ULL));
        for (size_t d = 0; d < 6; d++) {
            for (size_t t2 = 0; t2 < num_patterns; t2++) {
                for (size_t t1 = 0; t1 < num_patterns; t1++) {
                    if (propagator(d, t2, t1)) {
                        prop_mask[d][t2] |= (1ULL << t1);
                    }
                }
            }
        }
    }

    // Allocate entropy cache vectors once (reset in Clear())
    size_t grid_size = max_x * max_y * max_z;
    ec_entropy.resize(grid_size);
    ec_sum_weight.resize(grid_size);
    ec_sum_log_weight.resize(grid_size);
    ec_wave_count.resize(grid_size);

    //ofLog() << "Initialize Done";
}

Status ofxWFC3D::Observe()
{
    //ofLog() << " -- Observing --";
    double min = 1E+3, noise, entropy;


    // Lowest Entropy Coordinate Selection (using cached entropy)
    int selected_x = -1, selected_y = -1, selected_z = -1;

    for (size_t x = 0; x < max_x; x++) {
        for (size_t y = 0; y < max_y; y++) {
            for (size_t z = 0; z < max_z; z++) {
                size_t idx = x * max_y * max_z + y * max_z + z;

                if (ec_wave_count[idx] == 0) return Status::ObsFail;

                entropy = ec_entropy[idx];
                noise = 1E-6 * ofRandom(1);

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
    size_t r = weightedRandom(std::move(distribution), ofRandom(1));
    for (size_t t = 0; t < num_patterns; t++) {
        wave(selected_x, selected_y, selected_z, t) = t == r;
    }
    changes(selected_x, selected_y, selected_z) = true;

    // Update entropy cache for collapsed cell
    {
        size_t idx = (size_t)selected_x * max_y * max_z + (size_t)selected_y * max_z + (size_t)selected_z;
        ec_wave_count[idx] = 1;
        ec_sum_weight[idx] = pattern_weight[r];
        ec_sum_log_weight[idx] = pattern_weight[r] * log_prob[r];
        ec_entropy[idx] = 0.0;
    }

    return Status::ObsUnfinished;
}

bool ofxWFC3D::Propagate()
{
    //ofLog() << " -- Propagating --";

    // Seed the queue from changes[] array (cells that changed in Clear() or Observe())
    for (size_t x = 0; x < max_x; x++) {
        for (size_t y = 0; y < max_y; y++) {
            for (size_t z = 0; z < max_z; z++) {
                if (changes(x, y, z) && !in_queue(x, y, z)) {
                    prop_queue.push({x, y, z});
                    in_queue(x, y, z) = true;
                }
            }
        }
    }

    // Clear changes array — queue is now the sole worklist for this pass
    for (size_t x = 0; x < max_x; x++) {
        for (size_t y = 0; y < max_y; y++) {
            for (size_t z = 0; z < max_z; z++) {
                changes(x, y, z) = false;
            }
        }
    }

    // Direction offsets: x2 = x1+dx[d], y2 = y1+dy[d], z2 = z1+dz[d]
    // x1 is the changed cell (popped from queue), x2 is the neighbor to check.
    //
    // The offset moves FROM x1 TO x2, but the propagator direction d describes
    // the relationship from x2's perspective (where x1 is relative to x2):
    //
    // d=0: dx=+1 → x2 is to the RIGHT (+X) of x1.  From x2's view, x1 is LEFT  (-X)
    // d=1: dy=-1 → y2 is BELOW (-Y) x1.             From x2's view, x1 is UP    (+Y)
    // d=2: dx=-1 → x2 is to the LEFT (-X) of x1.    From x2's view, x1 is RIGHT (+X)
    // d=3: dy=+1 → y2 is ABOVE (+Y) x1.             From x2's view, x1 is DOWN  (-Y)
    // d=4: dz=-1 → z2 is BEHIND (-Z) x1.            From x2's view, x1 is FWD   (+Z)
    // d=5: dz=+1 → z2 is in FRONT (+Z) of x1.       From x2's view, x1 is BACK  (-Z)
    //
    // propagator(d, t2, t1) checks: "can pattern t2 at x2 have pattern t1 in direction d?"
    // This matches the propagator setup where d=2 is +X, d=0 is -X, etc.
    static const int dx[6] = {+1, 0, -1, 0, 0, 0};
    static const int dy[6] = {0, -1, 0, +1, 0, 0};
    static const int dz[6] = {0, 0, 0, 0, -1, +1};

    bool contradiction = false;

    // Drain the queue
    while (!prop_queue.empty()) {
        Cell c = prop_queue.front();
        prop_queue.pop();

        size_t x1 = c.x, y1 = c.y, z1 = c.z;
        in_queue(x1, y1, z1) = false;

        // Check all 6 neighbors
        for (int d = 0; d < 6; d++) {
            int x2_int = x1 + dx[d];
            int y2_int = y1 + dy[d];
            int z2_int = z1 + dz[d];

            // Boundary check with periodic wrapping
            if (x2_int < 0) {
                if (!periodic) continue;
                x2_int = max_x - 1;
            } else if (x2_int >= (int)max_x) {
                if (!periodic) continue;
                x2_int = 0;
            }

            if (y2_int < 0) {
                if (!periodic) continue;
                y2_int = max_y - 1;
            } else if (y2_int >= (int)max_y) {
                if (!periodic) continue;
                y2_int = 0;
            }

            if (z2_int < 0) {
                if (!periodic) continue;
                z2_int = max_z - 1;
            } else if (z2_int >= (int)max_z) {
                if (!periodic) continue;
                z2_int = 0;
            }

            size_t x2 = (size_t)x2_int;
            size_t y2 = (size_t)y2_int;
            size_t z2 = (size_t)z2_int;

            // Check compatibility and eliminate invalid patterns at (x2, y2, z2)
            bool cell_changed = false;
            size_t idx2 = x2 * max_y * max_z + y2 * max_z + z2;

            // Use bitset optimization if available, otherwise use original loop
            if (!prop_mask.empty()) {
                // Bitset version: build mask of compatible patterns at (x1,y1,z1)
                uint64_t compatible = 0ULL;
                for (size_t t1 = 0; t1 < num_patterns; t1++) {
                    if (wave(x1, y1, z1, t1)) {
                        compatible |= (1ULL << t1);
                    }
                }

                // Check each pattern at (x2,y2,z2)
                for (size_t t2 = 0; t2 < num_patterns; t2++) {
                    if (!wave(x2, y2, z2, t2)) continue;

                    // Use bitwise AND to check if any compatible pattern exists
                    bool can_prop = (compatible & prop_mask[d][t2]) != 0ULL;

                    if (!can_prop) {
                        wave(x2, y2, z2, t2) = false;
                        cell_changed = true;
                        // Incrementally update entropy cache
                        ec_wave_count[idx2]--;
                        ec_sum_weight[idx2] -= pattern_weight[t2];
                        ec_sum_log_weight[idx2] -= pattern_weight[t2] * log_prob[t2];
                    }
                }
            } else {
                // Original loop version (when num_patterns > 64)
                for (size_t t2 = 0; t2 < num_patterns; t2++) {
                    if (!wave(x2, y2, z2, t2)) continue;

                    bool can_prop = false;
                    for (size_t t1 = 0; t1 < num_patterns && !can_prop; t1++) {
                        if (wave(x1, y1, z1, t1)) {
                            can_prop = propagator(d, t2, t1);
                        }
                    }

                    if (!can_prop) {
                        wave(x2, y2, z2, t2) = false;
                        cell_changed = true;
                        // Incrementally update entropy cache
                        ec_wave_count[idx2]--;
                        ec_sum_weight[idx2] -= pattern_weight[t2];
                        ec_sum_log_weight[idx2] -= pattern_weight[t2] * log_prob[t2];
                    }
                }
            }

            if (cell_changed) {
                // Recompute cached entropy from updated sums
                if (ec_wave_count[idx2] <= 1) {
                    ec_entropy[idx2] = 0.0;
                } else if (ec_sum_weight[idx2] > 0.0) {
                    ec_entropy[idx2] = log(ec_sum_weight[idx2]) - ec_sum_log_weight[idx2] / ec_sum_weight[idx2];
                } else {
                    ec_entropy[idx2] = 0.0;
                }

                // Check for contradiction
                if (ec_wave_count[idx2] == 0) {
                    contradiction = true;
                }

                // Enqueue the neighbor if it's not already in the queue
                if (!in_queue(x2, y2, z2)) {
                    prop_queue.push({x2, y2, z2});
                    in_queue(x2, y2, z2) = true;
                }
            }
        }
    }

    return contradiction;
}

void ofxWFC3D::Clear()
{
    // Clear the propagation queue and in_queue state from previous runs
    while (!prop_queue.empty()) {
        prop_queue.pop();
    }
    for (size_t x = 0; x < max_x; x++) {
        for (size_t y = 0; y < max_y; y++) {
            for (size_t z = 0; z < max_z; z++) {
                in_queue(x, y, z) = false;
            }
        }
    }

    for (size_t x = 0 ; x < max_x; x++) {
        for (size_t y = 0 ; y < max_y; y++) {
            for (size_t z = 0 ; z < max_z; z++) {

                changes(x, y, z) = false;
                for (size_t t = 0; t < num_patterns; t++) {
                    size_t h_min = height_range[t].first;
                    size_t h_max = height_range[t].second;

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
                    if (t != (size_t)ground_id) wave(x, 0, z, t) = false;
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
                    if (t != (size_t)surround_id) {
                        wave(0, y, z, t) = false;
                        wave(max_x-1, y, z, t) = false;
                    }
                }
                changes(0, y, z) = true;
                changes(max_x-1, y, z) = true;
            }

            for (size_t x = 0 ; x < max_x; x++) {
                for (size_t t = 0; t < num_patterns; t++) {
                    if (t != (size_t)surround_id) {
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

    // Populate entropy cache from current wave state (no Array3D reassignment)
    std::fill(ec_entropy.begin(), ec_entropy.end(), 0.0);
    std::fill(ec_sum_weight.begin(), ec_sum_weight.end(), 0.0);
    std::fill(ec_sum_log_weight.begin(), ec_sum_log_weight.end(), 0.0);
    std::fill(ec_wave_count.begin(), ec_wave_count.end(), 0);

    for (size_t x = 0; x < max_x; x++) {
        for (size_t y = 0; y < max_y; y++) {
            for (size_t z = 0; z < max_z; z++) {
                size_t idx = x * max_y * max_z + y * max_z + z;
                int count = 0;
                double sw = 0.0;
                double slw = 0.0;

                for (size_t t = 0; t < num_patterns; t++) {
                    if (wave(x, y, z, t)) {
                        count++;
                        sw += pattern_weight[t];
                        slw += pattern_weight[t] * log_prob[t];
                    }
                }

                ec_wave_count[idx] = count;
                ec_sum_weight[idx] = sw;
                ec_sum_log_weight[idx] = slw;

                if (count <= 1) {
                    ec_entropy[idx] = 0.0;
                } else if (sw > 0.0) {
                    ec_entropy[idx] = log(sw) - slw / sw;
                } else {
                    ec_entropy[idx] = 0.0;
                }
            }
        }
    }
}

bool ofxWFC3D::SetTile(std::string tile_name, size_t x, size_t y, size_t z)
{
    if (std::regex_match (tile_name, std::regex("([_[:alnum:]]*)", std::regex::ECMAScript) ))
        tile_name += " 0";

    bool found = false;

    if (x < max_x && y < max_y && z < max_z && x >= 0 && y >= 0 && z >= 0) {
        for (size_t i = 0; i < tile_data.size(); i++) {
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
    Clear();

    ofSeedRandom(seed);

    while(true) {

        auto result = Observe();
        if (result != Status::ObsUnfinished)
            return result == Status::ObsSuccess ? true : false;

        // Propagate returns true if a contradiction is found
        if (Propagate()) return false;
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

// Outputs a 3D grid [x][y][z] where each cell is a map: { tileName → rotationIndex }
// rotationIndex: 0=0deg, 1=90deg, 2=180deg, 3=270deg (CCW around Y, viewed from above)
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

// Outputs a vector of (tileName, ofNode) pairs with position and rotation applied.
// Coordinate system: X-right, Y-up, Z-forward.
// Rotation: rotationIndex * 90 degrees CCW around Y axis (viewed from above).
//   rotationIndex 0 = 0deg (original), 1 = 90deg CCW, 2 = 180deg, 3 = 270deg CCW
// Position: grid cell (x,y,z) * grid_size — no additional offset.
std::vector< std::pair<std::string, ofNode> > ofxWFC3D::NodeTileOutput(ofNode& parent_node, glm::vec3 grid_size, std::vector<std::string> ignore)
{
    glm::vec3 axis_y = glm::vec3(0.0, 1.0, 0.0);
    std::vector< std::pair<std::string, ofNode> > tiles;
    for (size_t x = 0 ; x < max_x; x++) {
        for (size_t y = 0 ; y < max_y; y++) {
            for (size_t z = 0 ; z < max_z; z++) {

                // tile_cardinality[0] = tile name, tile_cardinality[1] = rotation index (0-3)
                auto tile_cardinality = ofSplitString(tile_data[observed(x, y, z)], " ", true);
                if ( ofContains(ignore, tile_cardinality[0]) ) continue;


                ofNode tile_node;
                tile_node.setParent(parent_node);
                tile_node.setPosition(x*grid_size.x, y*grid_size.y, z*grid_size.z);
                // Apply rotation: rotationIndex * 90deg CCW around Y
                tile_node.rotateDeg(ofToInt( tile_cardinality[1])*90.0f, axis_y );

                std::pair<std::string, ofNode> tile = std::make_pair(tile_cardinality[0], tile_node);

                tiles.push_back(tile);

            }
        }
    }

    return tiles;
}

// Same as NodeTileOutput but returns only the ofNode transformations (no tile names).
// Rotation: rotationIndex * 90 degrees CCW around Y axis (viewed from above).
// Position: grid cell (x,y,z) * grid_size.
std::vector<ofNode> ofxWFC3D::getNodes(ofNode& parent_node, glm::vec3 grid_size, std::vector<std::string> ignore) {
    std::vector<ofNode> transformations;

    glm::vec3 axis_y = glm::vec3(0.0, 1.0, 0.0);
    for (size_t x = 0 ; x < max_x; x++) {
        for (size_t y = 0 ; y < max_y; y++) {
            for (size_t z = 0 ; z < max_z; z++) {

                // tile_cardinality[0] = tile name, tile_cardinality[1] = rotation index (0-3)
                auto tile_cardinality = ofSplitString(tile_data[observed(x, y, z)], " ", true);
                if ( ofContains(ignore, tile_cardinality[0]) ) continue;


                ofNode tile_node;
                tile_node.setParent(parent_node);
                tile_node.setPosition(x*grid_size.x, y*grid_size.y, z*grid_size.z);
                // Apply rotation: rotationIndex * 90deg CCW around Y
                tile_node.rotateDeg(ofToInt( tile_cardinality[1])*90.0f, axis_y );

                transformations.push_back(tile_node);

            }
        }
    }

    return transformations;
}

// returns a list with the model index for each tile. Index based on xml configuration order.
std::vector<size_t> ofxWFC3D::getIndices(std::vector<std::string> ignore) {
    std::vector<size_t> indices;
    std::vector<std::string> tile_names = this->getTileNames(ignore);

    for (size_t x = 0 ; x < max_x; x++) {
        for (size_t y = 0 ; y < max_y; y++) {
            for (size_t z = 0 ; z < max_z; z++) {
                std::string data_name = ofSplitString(tile_data[observed(x, y, z)], " ", true)[0];
                if ( ofContains(ignore, data_name) ) continue;

                std::vector<std::string>::iterator it;
                it = std::find(tile_names.begin(), tile_names.end(), data_name);
                indices.push_back( (size_t)(it-tile_names.begin()) );
            }
        }
    }

    return indices;
}

// returns a list with the tile names based on xml configuration file.
std::vector<std::string> ofxWFC3D::getTileNames(std::vector<std::string> ignore) {
    std::vector<std::string> tileNames;
    for (auto& row : tile_data) {
        std::string tile_name = ofSplitString(row, " ", true)[0];
        if ( ofContains(ignore, tile_name) ) continue;
        tileNames.push_back(tile_name);
    }

    tileNames.erase( unique( tileNames.begin(), tileNames.end() ), tileNames.end() );
    return tileNames;
}


size_t weightedRandom(const std::vector<double>& a, double between_zero_and_one)
{
    double sum = 0;
    for (size_t i = 0; i < a.size(); ++i) sum += a[i];

	if (sum == 0.0) {
		return std::floor(between_zero_and_one * a.size());
	}

	double between_zero_and_sum = between_zero_and_one * sum;

	double accumulated = 0;
    
    for (size_t i = 0; i < a.size(); i++) {
		accumulated += a[i];
		if (between_zero_and_sum <= accumulated)
			return i;
	}

	return 0;
}
