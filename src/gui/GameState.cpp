#include "gui/GameState.hpp"
#include "algorithms/BFS.hpp"
#include "algorithms/DFS.hpp"
#include "algorithms/DLS.hpp"
#include "algorithms/IDS.hpp"
#include "algorithms/BDS.hpp"
#include "algorithms/UCS.hpp"
#include "algorithms/Greedy.hpp"
#include "algorithms/AStar.hpp"
#include "algorithms/AIAStar.hpp"
#include <algorithm>
#include <ctime>
#include <limits>

// ─── Internal helpers ─────────────────────────────────────────────────────────

static std::pair<int,int> pickCorner(int size, std::mt19937& rng) {
    int c = std::uniform_int_distribution<int>(0, 3)(rng);
    return { (c & 1) ? size - 1 : 0,
             (c & 2) ? size - 1 : 0 };
}

template<typename NodeT>
static std::vector<std::pair<int,int>> toCoords(const std::vector<NodeT*>& path) {
    std::vector<std::pair<int,int>> coords;
    coords.reserve(path.size());
    for (auto* n : path)
        coords.push_back({n->x, n->y});
    return coords;
}

// ─── Constructor ──────────────────────────────────────────────────────────────

GameState::GameState(int size, int wallProb, GraphMode mode, AlgoID algo,
                     int dlsLimit, int idsMaxLimit, int fuelSteps)
    : size_(size), wallProb_(wallProb), mode_(mode), algo_(algo),
      dlsLimit_(dlsLimit), idsMaxLimit_(idsMaxLimit), fuelLimit_(fuelSteps),
      rng_(static_cast<unsigned>(time(nullptr))),
      ox_(0), oy_(0), tx_(0), ty_(0)
{
    if (mode_ == GraphMode::UNWEIGHTED)
        buildUnweighted();
    else
        buildWeighted();
    cacheSpecialCells();
}

GameState::~GameState() = default;

// ─── Build helpers ────────────────────────────────────────────────────────────

void GameState::buildUnweighted() {
    ugraph_ = std::make_unique<Graph>(size_, size_);
    ugraph_->generateRandomMap(wallProb_);

    auto [ox, oy] = pickCorner(size_, rng_);
    ox_ = ox; oy_ = oy;
    ugraph_->grid[ox_][oy_].state = 0;

    // Pick a random passable cell as treasure (not the origin)
    std::vector<std::pair<int,int>> candidates;
    for (int i = 0; i < size_; i++)
        for (int j = 0; j < size_; j++)
            if (ugraph_->grid[i][j].state == 0 && !(i == ox_ && j == oy_))
                candidates.push_back({i, j});

    if (!candidates.empty()) {
        int idx = std::uniform_int_distribution<int>(0, (int)candidates.size()-1)(rng_);
        tx_ = candidates[idx].first;
        ty_ = candidates[idx].second;
    }

    // Run the algorithm once to get the expansion log and final path
    ugraph_->reset();
    std::vector<Node*> rawPath;
    switch (algo_) {
        case AlgoID::BFS:
            rawPath = BFS::findPathGrid(*ugraph_, ox_, oy_, tx_, ty_, &expansionLog_); break;
        case AlgoID::DFS:
            rawPath = DFS::findPathGrid(*ugraph_, ox_, oy_, tx_, ty_, &expansionLog_); break;
        case AlgoID::DLS:
            rawPath = DLS::findPathGrid(*ugraph_, ox_, oy_, tx_, ty_, dlsLimit_, &expansionLog_); break;
        case AlgoID::IDS:
            rawPath = IDS::findPathGrid(*ugraph_, ox_, oy_, tx_, ty_, idsMaxLimit_, &expansionLog_); break;
        case AlgoID::BDS:
            rawPath = BDS::findPathGrid(*ugraph_, ox_, oy_, tx_, ty_, &expansionLog_); break;
        default: break;
    }

    finalPath_ = toCoords(rawPath);
    pathHops_  = (int)finalPath_.size() > 1 ? (int)finalPath_.size() - 1 : 0;
    pathCost_  = 0.f;

    if (finalPath_.empty())
        phase_ = GamePhase::NO_PATH;
}

void GameState::buildWeighted() {
    wgraph_ = std::make_unique<W_Graph>(size_, size_);
    wgraph_->generateRandomMap(wallProb_);

    auto [ox, oy] = pickCorner(size_, rng_);
    ox_ = ox; oy_ = oy;
    wgraph_->grid[ox_][oy_].state  = CellType::OPEN;
    wgraph_->grid[ox_][oy_].weight = 1.0f;

    // Pick a random passable cell as treasure (not the origin)
    std::vector<std::pair<int,int>> candidates;
    for (int i = 0; i < size_; i++)
        for (int j = 0; j < size_; j++)
            if (wgraph_->grid[i][j].isPassable() && !(i == ox_ && j == oy_))
                candidates.push_back({i, j});

    if (!candidates.empty()) {
        int idx = std::uniform_int_distribution<int>(0, (int)candidates.size()-1)(rng_);
        tx_ = candidates[idx].first;
        ty_ = candidates[idx].second;
        wgraph_->grid[tx_][ty_].state  = CellType::OPEN;
        wgraph_->grid[tx_][ty_].weight = 1.0f;
    }

    // Run the algorithm once to get the expansion log and final path
    std::vector<W_Node*> rawPath;
    switch (algo_) {
        case AlgoID::UCS:
            rawPath = UCS::findPathGrid(*wgraph_, ox_, oy_, tx_, ty_, &expansionLog_); break;
        case AlgoID::GREEDY:
            rawPath = Greedy::findPathGrid(*wgraph_, ox_, oy_, tx_, ty_, &expansionLog_); break;
        case AlgoID::ASTAR:
            rawPath = AStar::findPathGrid(*wgraph_, ox_, oy_, tx_, ty_, &expansionLog_); break;
        case AlgoID::AIASTAR:
            rawPath = AIAStar::findPathGrid(*wgraph_, ox_, oy_, tx_, ty_, &expansionLog_); break;
        default: break;
    }

    finalPath_ = toCoords(rawPath);
    pathHops_  = (int)finalPath_.size() > 1 ? (int)finalPath_.size() - 1 : 0;
    pathCost_  = rawPath.empty() ? 0.f : rawPath.back()->g;

    if (finalPath_.empty())
        phase_ = GamePhase::NO_PATH;
}

void GameState::cacheSpecialCells() {
    if (wgraph_) {
        for (int i = 0; i < size_ && bh_cx_ == -1; i++)
            for (int j = 0; j < size_ && bh_cx_ == -1; j++)
                if (wgraph_->grid[i][j].state == CellType::BLACK_HOLE) {
                    bh_cx_ = i; bh_cy_ = j;
                }
        bool first = true;
        for (int i = 0; i < size_; i++)
            for (int j = 0; j < size_; j++)
                if (wgraph_->grid[i][j].state == CellType::WORMHOLE) {
                    if (first) { worm_a_ = {i, j}; first = false; }
                    else       { worm_b_ = {i, j}; goto done_worm; }
                }
        done_worm:;
    }
}

// ─── Advance ──────────────────────────────────────────────────────────────────

StepResult GameState::advance() {
    if (phase_ == GamePhase::DONE || phase_ == GamePhase::NO_PATH)
        return {phase_, revealIdx_};

    // Fuel check
    if (fuelLimit_ > 0 && revealIdx_ >= fuelLimit_) {
        phase_ = GamePhase::NO_PATH;
        return {phase_, revealIdx_};
    }

    if (revealIdx_ < (int)expansionLog_.size())
        ++revealIdx_;

    // Check if the entire expansion has been replayed
    if (revealIdx_ >= (int)expansionLog_.size()) {
        phase_ = finalPath_.empty() ? GamePhase::NO_PATH : GamePhase::DONE;
    }

    return {phase_, revealIdx_};
}

// ─── Grid queries ─────────────────────────────────────────────────────────────

int GameState::cellState(int x, int y) const {
    if (ugraph_) return ugraph_->grid[x][y].state;
    return 0;
}

int GameState::cellType(int x, int y) const {
    if (wgraph_) return wgraph_->grid[x][y].state;
    return CellType::OPEN;
}

float GameState::cellWeight(int x, int y) const {
    if (wgraph_) return wgraph_->grid[x][y].weight;
    return 1.0f;
}

std::pair<int,int> GameState::warpTarget(int x, int y) const {
    if (!wgraph_) return {-1, -1};
    W_Node* wt = wgraph_->grid[x][y].warp_target;
    if (!wt) return {-1, -1};
    return {wt->x, wt->y};
}
