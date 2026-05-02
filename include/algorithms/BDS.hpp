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
    static std::vector<Node*> findPathGrid(Graph& graph, int startX, int startY, int endX, int endY,
                                           std::vector<std::pair<int,int>>* log = nullptr);
};

#endif
