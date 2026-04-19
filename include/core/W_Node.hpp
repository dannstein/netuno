#ifndef W_NODE_HPP
#define W_NODE_HPP

namespace CellType {
    constexpr int OPEN           = 0;  // weight 1-49 (random)
    constexpr int ASTEROID       = 1;  // impassable wall
    constexpr int BLACK_HOLE     = 2;  // gravity singularity   – weight 100
    constexpr int BLACK_HOLE_R1  = 3;  // 8 cells around center – weight 80
    constexpr int BLACK_HOLE_R2  = 4;  // 16 cells outer ring   – weight 50
    constexpr int WORMHOLE       = 5;  // teleports ship to paired cell
    constexpr int NEBULA         = 6;  // interstellar gas cloud – weight 15-25
    constexpr int PULSAR_CENTER  = 7;  // neutron star core      – weight 75
    constexpr int PULSAR_JET     = 8;  // radiation jet (cross)  – weight 55
}

struct W_Node {
    W_Node* parent;
    int     state;
    float   g;           
    float   h;           
    int     x, y;
    float   weight;  
    bool    visited;
    W_Node* warp_target;

    W_Node(int _x = 0, int _y = 0, int _state = 0, W_Node* _parent = nullptr)
        : x(_x), y(_y), state(_state), parent(_parent),
          g(0.0f), h(0.0f), weight(1.0f), visited(false), warp_target(nullptr) {}

    float getF()       const { return g + h; }
    bool  isPassable() const { return state != CellType::ASTEROID; }
};

#endif
