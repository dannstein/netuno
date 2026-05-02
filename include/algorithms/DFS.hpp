#ifndef DFS_HPP
#define DFS_HPP

#include <vector>
#include <stack>
#include <utility>
#include "../core/Node.hpp"
#include "../core/Graph.hpp"

class DFS {
public:
    static std::vector<Node*> findPathGrid(Graph& graph, int startX, int startY, int endX, int endY,
                                           std::vector<std::pair<int,int>>* log = nullptr);
};

#endif
