#include "gui/Renderer.hpp"
#include "core/W_Node.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <set>

static constexpr float PI = 3.14159265f;

// ─── BH direction helpers ────────────────────────────────────────────────────

int Renderer::bhR1Index(int di, int dj) {
    if (di == -1 && dj == -1) return 0;
    if (di ==  0 && dj == -1) return 1;
    if (di == +1 && dj == -1) return 2;
    if (di == -1 && dj ==  0) return 3;
    if (di == +1 && dj ==  0) return 4;
    if (di == -1 && dj == +1) return 5;
    if (di ==  0 && dj == +1) return 6;
    return 7; // (+1,+1)
}

int Renderer::bhR2CornerIndex(int di, int dj) {
    if (std::abs(di) == 2 && std::abs(dj) == 2) {
        if (di < 0 && dj < 0) return 0;
        if (di > 0 && dj < 0) return 1;
        if (di < 0 && dj > 0) return 2;
        return 3;
    }
    return -1;
}

// ─── Constructor / texture loading ──────────────────────────────────────────

Renderer::Renderer(sf::RenderWindow& win, const std::string& spritesDir)
    : win_(win), spritesDir_(spritesDir)
{
    std::cerr << "Renderer: sprites dir = " << spritesDir_ << "\n";
    loadTextures();

    const std::vector<std::string> fontPaths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
    };
    for (auto& p : fontPaths)
        if (hudFont_.loadFromFile(p)) { fontLoaded_ = true; break; }
}

void Renderer::loadTextures() {
    auto load = [&](sf::Texture& tex, const std::string& rel) {
        if (!tex.loadFromFile(spritesDir_ + rel))
            std::cerr << "Failed: " << spritesDir_ + rel << "\n";
        tex.setSmooth(false); // keep crisp pixel art
    };

    for (int i = 0; i < 4; i++)
        load(bgTex_[i], "background/space_0" + std::to_string(i+1) + ".png");

    load(asteroidTex_[0], "asteroid/a_01.png");
    load(asteroidTex_[1], "asteroid/a_02.png");
    load(asteroidTex_[2], "asteroid/a_03.png");
    load(baseTex_,         "base/b_01.png");
    load(treasureTex_,     "treasure/t_01.png");
    load(shipTex_,         "spaceship/ss_01.png");
    load(wormholeTex_[0],  "wormhole/wh_01.png");
    load(wormholeTex_[1],  "wormhole/wh_02.png");
    load(nebulaTex_,       "nebula/n_01.png");
    load(pulsarCenterTex_, "pulsar/p_center.png");
    load(pulsarJetTex_,    "pulsar/p_sides.png");
    load(bhCoreTex_,       "black_hole/bh_center.png");

    const std::string r1[8] = {
        "black_hole/bh_top_left.png", "black_hole/bh_top.png",
        "black_hole/bh_top_right.png", "black_hole/bh_left.png",
        "black_hole/bh_right.png", "black_hole/bh_bottom_left.png",
        "black_hole/bh_bottom.png", "black_hole/bh_bottom_right.png",
    };
    for (int i = 0; i < 8; i++) load(bhR1Tex_[i], r1[i]);

    const std::string r2c[4] = {
        "black_hole/bh_outer/bh_top_left.png",
        "black_hole/bh_outer/bh_top_right.png",
        "black_hole/bh_outer/bh_bottom_left.png",
        "black_hole/bh_outer/bh_bottom_right.png",
    };
    for (int i = 0; i < 4; i++) load(bhR2CornerTex_[i], r2c[i]);
    load(bhR2OuterTex_, "black_hole/bh_outer/bh_outer.png");
}

// ─── buildVariants / resetView ───────────────────────────────────────────────

void Renderer::buildVariants(const GameState& gs) {
    int n = gs.gridSize();
    variantGrid_.assign(n, std::vector<unsigned>(n, 0u));
    for (int x = 0; x < n; x++) {
        for (int y = 0; y < n; y++) {
            // Wang-style hash: independent per (x,y), no axis correlation
            unsigned h = (unsigned)(x * 73856093u) ^ (unsigned)(y * 19349663u);
            h ^= h >> 16; h *= 0x45d9f3bu; h ^= h >> 16;
            variantGrid_[x][y] = h;
        }
    }
    resetView(n);
}

void Renderer::resetView(int gridSize) {
    float gridW = static_cast<float>(gridSize * TILE);
    float viewW = static_cast<float>(win_.getSize().x) * (1.f - HUD_FRAC);
    float viewH = static_cast<float>(win_.getSize().y);
    // Fit the full grid into the viewport
    zoom_ = std::min(viewW / gridW, viewH / gridW);
    zoom_ = std::max(zoom_, MIN_ZOOM);
    gridView_.setViewport({0.f, 0.f, 1.f - HUD_FRAC, 1.f});
    gridView_.setSize(viewW / zoom_, viewH / zoom_);
    gridView_.setCenter(gridW * 0.5f, gridW * 0.5f);
}

// ─── Ship animation ──────────────────────────────────────────────────────────

void Renderer::initShipPos(int x, int y) {
    shipWaypoints_ = { sf::Vector2f(x * (float)TILE, y * (float)TILE) };
    shipSegDur_    = 0.f;
    shipRotation_  = 0.f;
    isReturnPath_  = false;
    shipClock_.restart();
}

void Renderer::startPathAnimation(const std::vector<std::pair<int,int>>& path,
                                   int durationMs, bool isReturnPath) {
    isReturnPath_ = isReturnPath;
    shipWaypoints_.clear();
    for (auto& [x, y] : path)
        shipWaypoints_.push_back(sf::Vector2f(x * (float)TILE, y * (float)TILE));
    if (shipWaypoints_.empty()) return;
    int segs = (int)shipWaypoints_.size() - 1;
    float totalSecs = durationMs / 1000.f;
    shipSegDur_ = (segs > 0) ? (totalSecs / segs) : totalSecs;
    // Point ship toward first segment on start
    if (segs > 0) {
        sf::Vector2f d = shipWaypoints_[1] - shipWaypoints_[0];
        if (d.x != 0.f || d.y != 0.f)
            shipRotation_ = std::atan2(d.y, d.x) * 180.f / PI + 90.f;
    }
    shipClock_.restart();
}

// ─── Tile texture selection ──────────────────────────────────────────────────

// Background: space_01 = 80% (16/20), space_02–04 share remaining 20% (≈2 slots each).
static const int BG_TABLE[20] = {
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,  // 16 × space_01
    1,1,2,3                             // ~2× space_02, 1× space_03, 1× space_04
};

// Asteroid: equal chance among a_01, a_02, a_03 (hash % 3).
// No extra table needed — just use hash % 3 directly.

sf::Texture& Renderer::tileTexture(const GameState& gs, int x, int y) {
    unsigned hash = variantGrid_.empty() ? 0u : variantGrid_[x][y];

    if (gs.isWeighted()) {
        int ct = gs.cellType(x, y);
        switch (ct) {
            case CellType::ASTEROID:
                return asteroidTex_[hash % 3];
            case CellType::BLACK_HOLE:
                return bhCoreTex_;
            case CellType::BLACK_HOLE_R1: {
                auto [bhx, bhy] = gs.bhCenter();
                if (bhx >= 0) return bhR1Tex_[bhR1Index(x-bhx, y-bhy)];
                return bhR1Tex_[0];
            }
            case CellType::BLACK_HOLE_R2: {
                auto [bhx, bhy] = gs.bhCenter();
                if (bhx >= 0) {
                    int ci = bhR2CornerIndex(x-bhx, y-bhy);
                    if (ci >= 0) return bhR2CornerTex_[ci];
                }
                return bhR2OuterTex_;
            }
            case CellType::WORMHOLE: {
                auto [ax, ay] = gs.wormholeA();
                return (x == ax && y == ay) ? wormholeTex_[0] : wormholeTex_[1];
            }
            case CellType::NEBULA:        return nebulaTex_;
            case CellType::PULSAR_CENTER: return pulsarCenterTex_;
            case CellType::PULSAR_JET:    return pulsarJetTex_;
            default:
                return bgTex_[BG_TABLE[hash % 20]];
        }
    } else {
        if (gs.cellState(x, y) == 1)
            return asteroidTex_[hash % 3];
        return bgTex_[BG_TABLE[hash % 20]];
    }
}

// ─── Sprite drawing helper ───────────────────────────────────────────────────

void Renderer::drawTileSprite(sf::Sprite& spr, sf::Texture& tex,
                               float worldX, float worldY,
                               float rotation, bool centerOrigin) {
    spr.setTexture(tex, true); // true = reset texture rect
    spr.setScale(SP_SCALE, SP_SCALE);
    if (centerOrigin) {
        spr.setOrigin(SPRITE_SZ * 0.5f, SPRITE_SZ * 0.5f); // 4,4 for 8x8
        spr.setPosition(worldX + TILE * 0.5f, worldY + TILE * 0.5f);
    } else {
        spr.setOrigin(0.f, 0.f);
        spr.setPosition(worldX, worldY);
    }
    spr.setRotation(rotation);
    win_.draw(spr);
    // Reset to safe defaults for next call
    spr.setOrigin(0.f, 0.f);
    spr.setRotation(0.f);
}

// ─── drawGrid ────────────────────────────────────────────────────────────────

void Renderer::drawGrid(const GameState& gs,
                        const std::vector<std::pair<int,int>>& activePath) {
    int n = gs.gridSize();
    std::set<std::pair<int,int>> pathSet(activePath.begin(), activePath.end());

    auto [ox, oy] = gs.origin();
    auto [tx, ty] = gs.treasure();

    sf::Sprite spr;
    sf::RectangleShape pathOverlay(sf::Vector2f(TILE, TILE));
    pathOverlay.setFillColor(sf::Color(100, 200, 255, 130));

    // ── Terrain layer ─────────────────────────────────────────────────────────
    for (int y = 0; y < n; y++) {
        for (int x = 0; x < n; x++) {
            float px = x * (float)TILE;
            float py = y * (float)TILE;
            drawTileSprite(spr, tileTexture(gs, x, y), px, py);
        }
    }

    // ── Path overlay — blue for exploration, gold for return path ────────────
    sf::Color overlayCol = isReturnPath_
        ? sf::Color(255, 200,  50, 160)   // gold
        : sf::Color(100, 200, 255, 130);  // blue
    pathOverlay.setFillColor(overlayCol);
    for (auto& [px, py] : pathSet) {
        pathOverlay.setPosition(px * (float)TILE, py * (float)TILE);
        win_.draw(pathOverlay);
    }

    // ── Base (S) and Treasure (T) — drawn on top of terrain & path ───────────
    drawTileSprite(spr, baseTex_,     ox * (float)TILE, oy * (float)TILE);
    drawTileSprite(spr, treasureTex_, tx * (float)TILE, ty * (float)TILE);

    // ── Ship — traverse waypoints, rotate to face current segment ─────────────
    sf::Vector2f shipWorld;
    if (shipWaypoints_.empty()) {
        shipWorld = {0.f, 0.f};
    } else if (shipWaypoints_.size() == 1 || shipSegDur_ <= 0.f) {
        shipWorld = shipWaypoints_.back();
    } else {
        float elapsed  = shipClock_.getElapsedTime().asSeconds();
        int   totalSeg = (int)shipWaypoints_.size() - 1;
        float totalDur = shipSegDur_ * totalSeg;
        float tf = std::clamp(elapsed / totalDur, 0.f, 1.f);
        float segF = tf * totalSeg;
        int   seg  = std::min((int)segF, totalSeg - 1);
        float segT = segF - seg;
        segT = segT * segT * (3.f - 2.f * segT); // smoothstep per segment
        shipWorld = shipWaypoints_[seg] + segT * (shipWaypoints_[seg + 1] - shipWaypoints_[seg]);
        // Rotate toward current segment
        sf::Vector2f dir = shipWaypoints_[seg + 1] - shipWaypoints_[seg];
        if (dir.x != 0.f || dir.y != 0.f)
            shipRotation_ = std::atan2(dir.y, dir.x) * 180.f / PI + 90.f;
    }
    drawTileSprite(spr, shipTex_, shipWorld.x, shipWorld.y, shipRotation_, true);
}

// ─── drawHUD ─────────────────────────────────────────────────────────────────

void Renderer::drawHUD(const GameState& gs, bool paused, float totalCost,
                       const std::string& statusMsg) {
    float winW = static_cast<float>(win_.getSize().x);
    float winH = static_cast<float>(win_.getSize().y);
    float hudX = winW * (1.f - HUD_FRAC);
    float hudW = winW * HUD_FRAC;

    sf::RectangleShape panel({hudW, winH});
    panel.setPosition(hudX, 0.f);
    panel.setFillColor(sf::Color(15, 15, 30, 230));
    win_.draw(panel);

    sf::RectangleShape sep({2.f, winH});
    sep.setPosition(hudX, 0.f);
    sep.setFillColor(sf::Color(60, 80, 120, 200));
    win_.draw(sep);

    if (!fontLoaded_) return;

    const unsigned FS = 14u;
    const float    LH = 20.f;
    float cx = hudX + 10.f, row = 14.f;

    auto txt = [&](const std::string& s, sf::Color col = sf::Color(200, 220, 255)) {
        sf::Text t;
        t.setFont(hudFont_); t.setCharacterSize(FS);
        t.setFillColor(col); t.setString(s);
        t.setPosition(cx, row); win_.draw(t); row += LH;
    };

    txt("  N E T U N O", sf::Color(80, 180, 255));
    row += 6.f;

    std::string modeName = gs.isWeighted() ? "Weighted" : "Unweighted";
    std::string algoName;
    switch (gs.algoID()) {
        case AlgoID::BFS:     algoName="BFS";     break;
        case AlgoID::DFS:     algoName="DFS";     break;
        case AlgoID::DLS:     algoName="DLS";     break;
        case AlgoID::IDS:     algoName="IDS";     break;
        case AlgoID::BDS:     algoName="BDS";     break;
        case AlgoID::UCS:     algoName="UCS";     break;
        case AlgoID::GREEDY:  algoName="Greedy";  break;
        case AlgoID::ASTAR:   algoName="A*";      break;
        case AlgoID::AIASTAR: algoName="AIA*";    break;
    }
    txt("Mode: " + modeName,  sf::Color(160,200,160));
    txt("Algo: " + algoName,  sf::Color(160,200,160));
    row += 6.f;

    auto [ox,oy] = gs.origin();
    auto [txp,ty] = gs.treasure();
    auto [sx,sy] = gs.shipPos();
    txt("Base:     (" + std::to_string(ox) + "," + std::to_string(oy) + ")");
    txt("Treasure: (" + std::to_string(txp)+ "," + std::to_string(ty) + ")");
    txt("Ship:     (" + std::to_string(sx) + "," + std::to_string(sy) + ")");
    row += 6.f;

    if (gs.isWeighted() && totalCost > 0.f)
        txt("Total cost: " + std::to_string(static_cast<int>(totalCost)));

    int fuelRem = gs.fuelRemaining();
    if (fuelRem >= 0) {
        int fuelTot = gs.fuelTotal();
        int pct     = fuelTot > 0 ? (fuelRem * 100 / fuelTot) : 0;
        sf::Color fuelCol = (pct > 30)
            ? sf::Color(80, 200, 80)
            : sf::Color(255, 80, 80);
        txt("Fuel: " + std::to_string(fuelRem) + "/" + std::to_string(fuelTot)
            + " (" + std::to_string(pct) + "%)", fuelCol);
    }

    std::string phaseStr;
    switch (gs.phase()) {
        case GamePhase::EXPLORING:      phaseStr="Exploring...";    break;
        case GamePhase::TREASURE_FOUND: phaseStr="Treasure found!"; break;
        case GamePhase::DONE:           phaseStr="Mission done!";   break;
        case GamePhase::NO_PATH:        phaseStr="No path found.";  break;
    }
    txt(phaseStr, sf::Color(255,220,80));
    row += 6.f;

    if (!statusMsg.empty()) {
        // Simple 28-char wrap
        std::string s = statusMsg;
        while (!s.empty()) {
            size_t cut = std::min(s.size(), (size_t)28);
            txt(s.substr(0, cut), sf::Color(180,180,180));
            s = (s.size() > 28) ? s.substr(28) : "";
        }
    }

    if (paused) { row += 6.f; txt("[ PAUSED ]", sf::Color(255,100,100)); }

    // Controls section — pinned to bottom, tighter line height
    const float LH_C = 17.f;
    bool done = (gs.phase() == GamePhase::DONE || gs.phase() == GamePhase::NO_PATH);
    int  ctrlLines = done ? 5 : 5; // same count either way
    row = winH - 10.f - ctrlLines * LH_C - 20.f; // 20 for header

    auto ctxt = [&](const std::string& s) {
        sf::Text t;
        t.setFont(hudFont_); t.setCharacterSize(12u);
        t.setFillColor(sf::Color(120,120,160)); t.setString(s);
        t.setPosition(cx, row); win_.draw(t); row += LH_C;
    };

    ctxt("Controls:");
    ctxt("SPACE    pause / resume");
    if (!done)
        ctxt("RIGHT    step forward");
    else
        ctxt("ENTER    see results");
    ctxt("SCROLL   zoom");
    ctxt("DRAG     pan");
    ctxt("ESC      main menu");
}

// ─── render ──────────────────────────────────────────────────────────────────

void Renderer::render(const GameState& gs,
                      const std::vector<std::pair<int,int>>& activePath,
                      bool paused, float totalCost,
                      const std::string& statusMsg) {
    win_.clear(sf::Color(5, 5, 15));
    win_.setView(gridView_);
    drawGrid(gs, activePath);
    win_.setView(win_.getDefaultView());
    drawHUD(gs, paused, totalCost, statusMsg);
}

// ─── Zoom / pan ──────────────────────────────────────────────────────────────

void Renderer::handleScroll(float delta, sf::Vector2i mousePixel) {
    float factor  = (delta > 0.f) ? 1.15f : (1.f / 1.15f);
    float newZoom = std::clamp(zoom_ * factor, MIN_ZOOM, MAX_ZOOM);
    if (newZoom == zoom_) return;
    sf::Vector2f before = win_.mapPixelToCoords(mousePixel, gridView_);
    zoom_ = newZoom;
    float viewW = static_cast<float>(win_.getSize().x) * (1.f - HUD_FRAC);
    float viewH = static_cast<float>(win_.getSize().y);
    gridView_.setSize(viewW / zoom_, viewH / zoom_);
    sf::Vector2f after = win_.mapPixelToCoords(mousePixel, gridView_);
    gridView_.move(before - after);
}

void Renderer::handlePanBegin(sf::Vector2i mousePixel) {
    panning_       = true;
    panOrigin_     = mousePixel;
    panViewOrigin_ = gridView_.getCenter();
}

void Renderer::handlePanMove(sf::Vector2i mousePixel) {
    if (!panning_) return;
    sf::Vector2i delta = panOrigin_ - mousePixel;
    float viewW = static_cast<float>(win_.getSize().x) * (1.f - HUD_FRAC);
    float viewH = static_cast<float>(win_.getSize().y);
    gridView_.setCenter(panViewOrigin_ +
        sf::Vector2f(delta.x * gridView_.getSize().x / viewW,
                     delta.y * gridView_.getSize().y / viewH));
}

void Renderer::handlePanEnd() { panning_ = false; }
