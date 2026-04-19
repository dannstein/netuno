#ifndef W_GRAPH_HPP
#define W_GRAPH_HPP

#include <vector>
#include <algorithm>
#include <random>
#include "W_Node.hpp"

class W_Graph {
public:
    int nx, ny;
    std::vector<std::vector<W_Node>> grid;

    W_Graph(int x, int y);

    void reset();

    std::vector<W_Node*> sucessors_grid(W_Node* current);

    float getEdgeCost(W_Node* from, W_Node* to) const;

    void generateRandomMap(int wallProbability);

    void drawConsole();

    std::vector<W_Node*> showPath(W_Node* lastNode);
    void drawPath(const std::vector<W_Node*>& path);

    float heuristic(W_Node* from, W_Node* goal) const;

private:
    W_Node* wormhole_a = nullptr;

    void placeBlackHole(int cx, int cy);
    void placePulsar(int cx, int cy, std::mt19937& rng);
    void placeNebula(int cx, int cy, int size, std::mt19937& rng);
    void placeWormhole(int ax, int ay, int bx, int by);
};

#endif
