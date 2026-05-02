#ifndef IDS_HPP
#define IDS_HPP

#include <vector>
#include <stack>
#include <map>
#include <utility>
#include "../core/Node.hpp"
#include "../core/Graph.hpp"

class IDS {
public:
    static std::vector<Node*> findPathGrid(Graph& graph, int startX, int startY, int endX, int endY, int max_limit,
                                           std::vector<std::pair<int,int>>* log = nullptr);
};

#endif
