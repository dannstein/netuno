#ifndef BDS_HPP
#define BDS_HPP

#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <algorithm> 
#include "../core/Node.hpp"
#include "../core/Graph.hpp"

class BDS {
public:
    // Recebe o objeto graph (que contém o mapa/grid) e as coordenadas de início e fim
    static std::vector<Node*> findPathGrid(Graph& graph, int startX, int startY, int endX, int endY);
};

#endif
