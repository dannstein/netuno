#ifndef GRAPH_HPP 
#define GRAPH_HPP  

#include <vector>
#include <algorithm>
#include "Node.hpp"

class Graph {
public:
    int nx, ny; // Dimensões do grid
    std::vector<std::vector<Node>> grid; // O seu 'mapa'

    Graph(int x, int y);

    void reset(); 
    
    std::vector<Node*> sucessors_grid(Node* current);

    void generateRandomMap(int wallProbability);

    void drawConsole();

    std::vector<Node*> showPath(Node* lastNode);

    void drawPath(const std::vector<Node*>& path);

    // Game render: overlays path (X), base (S) and treasure (T) on the map.
    void drawGame(const std::vector<Node*>& path, int ox, int oy, int tx, int ty);
};

#endif