#ifndef DLS_HPP
#define DLS_HPP

#include <vector>
#include <stack>
#include <map>
#include <utility>
#include "../core/Node.hpp"
#include "../core/Graph.hpp"

class DLS {
public:
    static std::vector<Node*> findPathGrid(Graph& graph, int startX, int startY, int endX, int endY, int limit,
                                           std::vector<std::pair<int,int>>* log = nullptr);
};

#endif
