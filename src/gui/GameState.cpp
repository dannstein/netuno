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

// ─── Internal helpers ────────────────────────────────────────────────────────

static std::pair<int,int> pickCorner(int size, std::mt19937& rng) {
    int c = std::uniform_int_distribution<int>(0, 3)(rng);
    return { (c & 1) ? size - 1 : 0,
             (c & 2) ? size - 1 : 0 };
}

template<typename NodeT>
static bool isOnPath(const std::vector<NodeT*>& path, int x, int y) {
    for (auto* n : path)
        if (n->x == x && n->y == y) return true;
    return false;
}

template<typename NodeT>
static void removePathFromPool(std::vector<std::pair<int,int>>& pool,
                               const std::vector<NodeT*>& path) {
    for (auto* n : path) {
        auto it = std::find(pool.begin(), pool.end(), std::make_pair(n->x, n->y));
        if (it != pool.end())
            pool.erase(it);
    }
}

template<typename NodeT>
static std::vector<std::pair<int,int>> toCoords(const std::vector<NodeT*>& path) {
    std::vector<std::pair<int,int>> coords;
    coords.reserve(path.size());
    for (auto* n : path)
        coords.push_back({n->x, n->y});
    return coords;
}

// ─── Constructor ─────────────────────────────────────────────────────────────

GameState::GameState(int size, int wallProb, GraphMode mode, AlgoID algo,
                     int dlsLimit, int idsMaxLimit, int fuelSteps)
    : size_(size), wallProb_(wallProb), mode_(mode), algo_(algo),
      dlsLimit_(dlsLimit), idsMaxLimit_(idsMaxLimit), fuelLimit_(fuelSteps),
      rng_(static_cast<unsigned>(time(nullptr))),
      ox_(0), oy_(0), tx_(0), ty_(0), cx_(0), cy_(0)
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
    cx_ = ox_; cy_ = oy_;

    for (int i = 0; i < size_; i++)
        for (int j = 0; j < size_; j++)
            if (ugraph_->grid[i][j].state == 0 && !(i == ox_ && j == oy_))
                pool_.push_back({i, j});

    if (!pool_.empty()) {
        int idx = std::uniform_int_distribution<int>(0, (int)pool_.size()-1)(rng_);
        tx_ = pool_[idx].first;
        ty_ = pool_[idx].second;
    }

    switch (algo_) {
        case AlgoID::BFS:
            uAlgo_ = [&](int sx,int sy,int ex,int ey){
                ugraph_->reset(); return BFS::findPathGrid(*ugraph_,sx,sy,ex,ey); }; break;
        case AlgoID::DFS:
            uAlgo_ = [&](int sx,int sy,int ex,int ey){
                ugraph_->reset(); return DFS::findPathGrid(*ugraph_,sx,sy,ex,ey); }; break;
        case AlgoID::DLS:
            uAlgo_ = [&](int sx,int sy,int ex,int ey){
                ugraph_->reset(); return DLS::findPathGrid(*ugraph_,sx,sy,ex,ey,dlsLimit_); }; break;
        case AlgoID::IDS:
            uAlgo_ = [&](int sx,int sy,int ex,int ey){
                return IDS::findPathGrid(*ugraph_,sx,sy,ex,ey,idsMaxLimit_); }; break;
        case AlgoID::BDS:
            uAlgo_ = [&](int sx,int sy,int ex,int ey){
                ugraph_->reset(); return BDS::findPathGrid(*ugraph_,sx,sy,ex,ey); }; break;
        default: break;
    }
}

void GameState::buildWeighted() {
    wgraph_ = std::make_unique<W_Graph>(size_, size_);
    wgraph_->generateRandomMap(wallProb_);

    auto [ox, oy] = pickCorner(size_, rng_);
    ox_ = ox; oy_ = oy;
    wgraph_->grid[ox_][oy_].state  = CellType::OPEN;
    wgraph_->grid[ox_][oy_].weight = 1.0f;
    cx_ = ox_; cy_ = oy_;

    for (int i = 0; i < size_; i++)
        for (int j = 0; j < size_; j++)
            if (wgraph_->grid[i][j].isPassable() && !(i == ox_ && j == oy_))
                pool_.push_back({i, j});

    if (!pool_.empty()) {
        int idx = std::uniform_int_distribution<int>(0, (int)pool_.size()-1)(rng_);
        tx_ = pool_[idx].first;
        ty_ = pool_[idx].second;
        wgraph_->grid[tx_][ty_].state  = CellType::OPEN;
        wgraph_->grid[tx_][ty_].weight = 1.0f;
    }

    switch (algo_) {
        case AlgoID::UCS:
            wAlgo_ = [&](int sx,int sy,int ex,int ey){
                return UCS::findPathGrid(*wgraph_,sx,sy,ex,ey); }; break;
        case AlgoID::GREEDY:
            wAlgo_ = [&](int sx,int sy,int ex,int ey){
                return Greedy::findPathGrid(*wgraph_,sx,sy,ex,ey); }; break;
        case AlgoID::ASTAR:
            wAlgo_ = [&](int sx,int sy,int ex,int ey){
                return AStar::findPathGrid(*wgraph_,sx,sy,ex,ey); }; break;
        case AlgoID::AIASTAR:
            wAlgo_ = [&](int sx,int sy,int ex,int ey){
                return AIAStar::findPathGrid(*wgraph_,sx,sy,ex,ey); }; break;
        default: break;
    }
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
                    else        { worm_b_ = {i, j}; goto done_worm; }
                }
        done_worm:;
    }
}

// ─── Advance ─────────────────────────────────────────────────────────────────

StepResult GameState::advance() {
    StepResult result;
    result.stepNum   = step_;
    result.totalCost = totalCost_;

    // Already finished
    if (phase_ == GamePhase::DONE || phase_ == GamePhase::NO_PATH) {
        result.phase = phase_;
        return result;
    }

    // Run return path after treasure found
    if (treasureFoundFlag_) {
        treasureFoundFlag_ = false;
        result.isReturnPath = true;
        if (mode_ == GraphMode::UNWEIGHTED && uAlgo_) {
            auto raw = uAlgo_(tx_, ty_, ox_, oy_);
            result.path = toCoords(raw);
        } else if (mode_ == GraphMode::WEIGHTED && wAlgo_) {
            auto raw = wAlgo_(tx_, ty_, ox_, oy_);
            result.path = toCoords(raw);
            if (!raw.empty()) {
                result.stepCost  = raw.back()->g;
                result.totalCost = totalCost_ + result.stepCost;
            }
        }
        phase_       = GamePhase::DONE;
        result.phase = phase_;
        return result;
    }

    // Fuel exhausted
    if (fuelLimit_ > 0 && step_ >= fuelLimit_) {
        phase_       = GamePhase::NO_PATH;
        result.phase = phase_;
        return result;
    }

    // Pool exhausted without finding treasure
    if (pool_.empty()) {
        phase_       = GamePhase::NO_PATH;
        result.phase = phase_;
        return result;
    }

    ++step_;
    result.stepNum = step_;

    int idx = std::uniform_int_distribution<int>(0, (int)pool_.size()-1)(rng_);
    auto [targetX, targetY] = pool_[idx];

    if (mode_ == GraphMode::UNWEIGHTED) {
        auto raw = uAlgo_ ? uAlgo_(cx_, cy_, targetX, targetY)
                          : std::vector<Node*>{};
        if (raw.empty()) {
            pool_.erase(pool_.begin() + idx);
            result.noPathToTarget = true;
            result.phase = phase_;
            return result;
        }
        removePathFromPool(pool_, raw);
        result.path = toCoords(raw);

        if (targetX == tx_ && targetY == ty_) {
            phase_              = GamePhase::TREASURE_FOUND;
            treasureFoundFlag_  = true;
        } else if (isOnPath(raw, tx_, ty_)) {
            result.treasureOnRoute = true;
            phase_              = GamePhase::TREASURE_FOUND;
            treasureFoundFlag_  = true;
        } else {
            cx_ = targetX; cy_ = targetY;
        }
    } else {
        auto raw = wAlgo_ ? wAlgo_(cx_, cy_, targetX, targetY)
                          : std::vector<W_Node*>{};
        if (raw.empty()) {
            pool_.erase(pool_.begin() + idx);
            result.noPathToTarget = true;
            result.phase = phase_;
            return result;
        }
        removePathFromPool(pool_, raw);
        result.path     = toCoords(raw);
        result.stepCost = raw.back()->g;
        totalCost_     += result.stepCost;
        result.totalCost = totalCost_;

        if (targetX == tx_ && targetY == ty_) {
            phase_             = GamePhase::TREASURE_FOUND;
            treasureFoundFlag_ = true;
        } else if (isOnPath(raw, tx_, ty_)) {
            result.treasureOnRoute = true;
            phase_             = GamePhase::TREASURE_FOUND;
            treasureFoundFlag_ = true;
        } else {
            cx_ = targetX; cy_ = targetY;
        }
    }

    result.phase = phase_;
    return result;
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
