#include <SFML/Graphics.hpp>
#include "gui/GameState.hpp"
#include "gui/Renderer.hpp"
#include <string>
#include <memory>
#include <functional>
#include <filesystem>
#include <unistd.h>

// Finds the sprites/ directory using the absolute exe path so it works
// regardless of the working directory (e.g. running from build/).
static std::string findSpritesDir() {
    namespace fs = std::filesystem;
    char buf[4096] = {};
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) {
        fs::path exeDir = fs::path(std::string(buf, len)).parent_path();
        // build/netuno_pathfinding → project root is one level up
        for (auto& rel : std::vector<fs::path>{"sprites", fs::path("..") / "sprites"}) {
            auto candidate = (exeDir / rel).lexically_normal();
            if (fs::exists(candidate / "background"))
                return candidate.string() + "/";
        }
    }
    return "sprites/"; // last-resort fallback
}

// ─── App state machine ───────────────────────────────────────────────────────

enum class AppState {
    MAIN_MENU,
    TUTORIAL,
    CONFIG_SIZE,
    CONFIG_PROB,
    CONFIG_FUEL,
    ALGO_SELECT,
    PARAMS_DLS,
    PARAMS_IDS,
    PLAYING,
    RESULT
};

// ─── UI helpers ──────────────────────────────────────────────────────────────

static void drawCenteredText(sf::RenderWindow& win, sf::Font& font,
                              const std::string& str, unsigned size,
                              float y, sf::Color col = sf::Color::White) {
    sf::Text t;
    t.setFont(font);
    t.setCharacterSize(size);
    t.setFillColor(col);
    t.setString(str);
    sf::FloatRect bounds = t.getLocalBounds();
    t.setPosition((win.getSize().x - bounds.width) * 0.5f - bounds.left, y);
    win.draw(t);
}

static void drawText(sf::RenderWindow& win, sf::Font& font,
                     const std::string& str, unsigned size,
                     float x, float y, sf::Color col = sf::Color::White) {
    sf::Text t;
    t.setFont(font);
    t.setCharacterSize(size);
    t.setFillColor(col);
    t.setString(str);
    t.setPosition(x, y);
    win.draw(t);
}

static std::string algoName(AlgoID id) {
    switch (id) {
        case AlgoID::BFS:     return "BFS";
        case AlgoID::DFS:     return "DFS";
        case AlgoID::DLS:     return "DLS";
        case AlgoID::IDS:     return "IDS";
        case AlgoID::BDS:     return "BDS";
        case AlgoID::UCS:     return "UCS";
        case AlgoID::GREEDY:  return "Greedy";
        case AlgoID::ASTAR:   return "A*";
        case AlgoID::AIASTAR: return "AIA*";
    }
    return "?";
}

// ─── Main ────────────────────────────────────────────────────────────────────

int main() {
    sf::RenderWindow win(sf::VideoMode(1280, 800), "NETUNO",
                         sf::Style::Default);
    win.setFramerateLimit(60);

    // Load font (same search as Renderer)
    sf::Font font;
    bool fontOk = false;
    for (auto& p : std::vector<std::string>{
            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
            "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf",
            "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
            "/usr/share/fonts/truetype/freefont/FreeMono.ttf"}) {
        if (font.loadFromFile(p)) { fontOk = true; break; }
    }
    if (!fontOk) {
        // Fallback: cannot render text properly, but keep window open
    }

    // ── Shared game parameters ───────────────────────────────────────────────
    int       gridSize   = 15;
    int       wallProb   = 25;
    int       fuelSteps  = 0;
    GraphMode graphMode  = GraphMode::UNWEIGHTED;
    AlgoID    algo       = AlgoID::BFS;
    int       dlsLimit   = 5;
    int       idsMaxLim  = 10;

    // ── State machine variables ──────────────────────────────────────────────
    AppState  appState   = AppState::MAIN_MENU;
    int       tutPage    = 0;
    std::string inputBuf;
    std::string configPrompt;

    std::unique_ptr<GameState> gs;
    std::unique_ptr<Renderer>  renderer;

    std::vector<std::pair<int,int>> lastPath;
    std::string statusMsg;
    bool   paused        = false;
    float  totalCost     = 0.f;
    float  returnCost    = 0.f;
    int    lastStep      = 0;
    sf::Clock stepClock;
    const int STEP_MS    = 600;

    // ── Event / draw loop ────────────────────────────────────────────────────
    bool suppressNextText = false; // blocks TextEntered caused by a menu keypress
    while (win.isOpen()) {
        sf::Event ev;
        while (win.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) win.close();

            // ── Zoom / pan (only during PLAYING) ────────────────────────────
            if (appState == AppState::PLAYING && renderer) {
                if (ev.type == sf::Event::MouseWheelScrolled)
                    renderer->handleScroll(ev.mouseWheelScroll.delta,
                                           {ev.mouseWheelScroll.x,
                                            ev.mouseWheelScroll.y});
                // Pan with left OR middle mouse button
                bool panPress = ev.type == sf::Event::MouseButtonPressed &&
                    (ev.mouseButton.button == sf::Mouse::Left ||
                     ev.mouseButton.button == sf::Mouse::Middle);
                bool panRelease = ev.type == sf::Event::MouseButtonReleased &&
                    (ev.mouseButton.button == sf::Mouse::Left ||
                     ev.mouseButton.button == sf::Mouse::Middle);
                bool panHeld = sf::Mouse::isButtonPressed(sf::Mouse::Left) ||
                               sf::Mouse::isButtonPressed(sf::Mouse::Middle);

                if (panPress)
                    renderer->handlePanBegin({ev.mouseButton.x, ev.mouseButton.y});
                if (ev.type == sf::Event::MouseMoved && panHeld)
                    renderer->handlePanMove({ev.mouseMove.x, ev.mouseMove.y});
                if (panRelease)
                    renderer->handlePanEnd();
            }

            // ── Keyboard ────────────────────────────────────────────────────
            if (ev.type == sf::Event::KeyPressed) {
                auto key = ev.key.code;

                if (appState == AppState::MAIN_MENU) {
                    if (key == sf::Keyboard::Num1 || key == sf::Keyboard::Numpad1) {
                        graphMode = GraphMode::UNWEIGHTED;
                        inputBuf.clear(); suppressNextText = true;
                        configPrompt = "Grid size (e.g. 15):";
                        appState = AppState::CONFIG_SIZE;
                    } else if (key == sf::Keyboard::Num2 || key == sf::Keyboard::Numpad2) {
                        graphMode = GraphMode::WEIGHTED;
                        inputBuf.clear(); suppressNextText = true;
                        configPrompt = "Grid size (e.g. 15):";
                        appState = AppState::CONFIG_SIZE;
                    } else if (key == sf::Keyboard::Num3 || key == sf::Keyboard::Numpad3) {
                        tutPage  = 0;
                        appState = AppState::TUTORIAL;
                    } else if (key == sf::Keyboard::Num9 || key == sf::Keyboard::Escape) {
                        win.close();
                    }
                } else if (appState == AppState::TUTORIAL) {
                    if (key == sf::Keyboard::Right || key == sf::Keyboard::Return) {
                        if (tutPage < 1) ++tutPage;
                        else             appState = AppState::MAIN_MENU;
                    } else if (key == sf::Keyboard::Left) {
                        if (tutPage > 0) --tutPage;
                    } else if (key == sf::Keyboard::Escape) {
                        appState = AppState::MAIN_MENU;
                    }
                } else if (appState == AppState::CONFIG_SIZE ||
                           appState == AppState::CONFIG_PROB ||
                           appState == AppState::CONFIG_FUEL ||
                           appState == AppState::PARAMS_DLS  ||
                           appState == AppState::PARAMS_IDS) {
                    if (key == sf::Keyboard::BackSpace && !inputBuf.empty())
                        inputBuf.pop_back();
                    if (key == sf::Keyboard::Return && !inputBuf.empty()) {
                        int val = std::stoi(inputBuf);
                        inputBuf.clear();
                        if (appState == AppState::CONFIG_SIZE) {
                            gridSize = std::max(5, std::min(50, val));
                            configPrompt = "Wall probability 0-100 (e.g. 25):";
                            appState = AppState::CONFIG_PROB;
                        } else if (appState == AppState::CONFIG_PROB) {
                            wallProb = std::max(0, std::min(100, val));
                            inputBuf.clear();
                            configPrompt = "Fuel (max steps, 0=unlimited):";
                            appState = AppState::CONFIG_FUEL;
                        } else if (appState == AppState::CONFIG_FUEL) {
                            fuelSteps = std::max(0, val);
                            appState = AppState::ALGO_SELECT;
                        } else if (appState == AppState::PARAMS_DLS) {
                            dlsLimit = std::max(1, val);
                            goto launch_game;
                        } else if (appState == AppState::PARAMS_IDS) {
                            idsMaxLim = std::max(1, val);
                            goto launch_game;
                        }
                    }
                } else if (appState == AppState::ALGO_SELECT) {
                    AlgoID chosen = AlgoID::BFS;
                    bool picked = true;
                    if (graphMode == GraphMode::UNWEIGHTED) {
                        if      (key == sf::Keyboard::Num1) { chosen = AlgoID::BFS; }
                        else if (key == sf::Keyboard::Num2) { chosen = AlgoID::DFS; }
                        else if (key == sf::Keyboard::Num3) { chosen = AlgoID::DLS; }
                        else if (key == sf::Keyboard::Num4) { chosen = AlgoID::IDS; }
                        else if (key == sf::Keyboard::Num5) { chosen = AlgoID::BDS; }
                        else picked = false;
                    } else {
                        if      (key == sf::Keyboard::Num1) { chosen = AlgoID::UCS;     }
                        else if (key == sf::Keyboard::Num2) { chosen = AlgoID::GREEDY;  }
                        else if (key == sf::Keyboard::Num3) { chosen = AlgoID::ASTAR;   }
                        else if (key == sf::Keyboard::Num4) { chosen = AlgoID::AIASTAR; }
                        else picked = false;
                    }
                    if (picked) {
                        algo = chosen;
                        if (algo == AlgoID::DLS) {
                            inputBuf.clear();
                            configPrompt = "DLS depth limit:";
                            appState = AppState::PARAMS_DLS;
                        } else if (algo == AlgoID::IDS) {
                            inputBuf.clear();
                            configPrompt = "IDS max limit:";
                            appState = AppState::PARAMS_IDS;
                        } else {
                            goto launch_game;
                        }
                    }
                } else if (appState == AppState::PLAYING) {
                    if (key == sf::Keyboard::Space)
                        paused = !paused;
                    if (key == sf::Keyboard::Return && gs &&
                        (gs->phase() == GamePhase::DONE ||
                         gs->phase() == GamePhase::NO_PATH)) {
                        appState = AppState::RESULT;
                    }
                    if (key == sf::Keyboard::Right && gs &&
                        gs->phase() != GamePhase::DONE &&
                        gs->phase() != GamePhase::NO_PATH) {
                        auto res  = gs->advance();
                        lastPath  = res.path;
                        lastStep  = res.stepNum;
                        totalCost = res.totalCost;
                        if (res.isReturnPath) returnCost = res.stepCost;
                        if (renderer && !res.path.empty() && !res.noPathToTarget) {
                            int dur = res.isReturnPath ? 2500 : STEP_MS;
                            renderer->startPathAnimation(res.path, dur, res.isReturnPath);
                        }
                        if (res.noPathToTarget)
                            statusMsg = "No path to target. Skipping...";
                        else if (res.isReturnPath)
                            statusMsg = "Return path: " + std::to_string(res.path.size()) + " nodes";
                        else if (res.treasureOnRoute)
                            statusMsg = "Treasure found en route! Step " + std::to_string(res.stepNum);
                        else if (res.phase == GamePhase::TREASURE_FOUND)
                            statusMsg = "Treasure reached! Step " + std::to_string(res.stepNum);
                        else
                            statusMsg = "Step " + std::to_string(res.stepNum)
                                      + ": " + std::to_string(res.path.size()) + " nodes";
                        stepClock.restart();
                    }
                    if (key == sf::Keyboard::Escape) {
                        appState = AppState::MAIN_MENU;
                        gs.reset(); renderer.reset(); lastPath.clear();
                    }
                } else if (appState == AppState::RESULT) {
                    if (key == sf::Keyboard::Return || key == sf::Keyboard::Space)
                        appState = AppState::MAIN_MENU;
                    if (key == sf::Keyboard::Escape)
                        win.close();
                }
                goto skip_text;
            }

            // ── Text input for number fields ─────────────────────────────────
            if (ev.type == sf::Event::TextEntered) {
                if (!suppressNextText &&
                    (appState == AppState::CONFIG_SIZE ||
                     appState == AppState::CONFIG_PROB ||
                     appState == AppState::CONFIG_FUEL ||
                     appState == AppState::PARAMS_DLS  ||
                     appState == AppState::PARAMS_IDS)) {
                    char c = static_cast<char>(ev.text.unicode);
                    if (c >= '0' && c <= '9' && inputBuf.size() < 4)
                        inputBuf += c;
                }
                suppressNextText = false;
            }
            skip_text:;

            // ── goto target: launch game ─────────────────────────────────────
            if (false) {
                launch_game:
                lastPath.clear();
                statusMsg.clear();
                totalCost  = 0.f;
                returnCost = 0.f;
                lastStep   = 0;
                paused     = false;

                gs = std::make_unique<GameState>(
                    gridSize, wallProb, graphMode, algo, dlsLimit, idsMaxLim, fuelSteps);
                renderer = std::make_unique<Renderer>(win, findSpritesDir());
                renderer->buildVariants(*gs);
                renderer->initShipPos(gs->origin().first, gs->origin().second);
                stepClock.restart();
                appState = AppState::PLAYING;
            }
        }

        // ── Auto-advance ─────────────────────────────────────────────────────
        if (appState == AppState::PLAYING && !paused && gs &&
            gs->phase() != GamePhase::DONE &&
            gs->phase() != GamePhase::NO_PATH) {
            if (stepClock.getElapsedTime().asMilliseconds() >= STEP_MS) {
                auto res = gs->advance();
                lastPath  = res.path;
                lastStep  = res.stepNum;
                totalCost = res.totalCost;
                if (res.isReturnPath) returnCost = res.stepCost;
                if (renderer && !res.path.empty() && !res.noPathToTarget) {
                    int dur = res.isReturnPath ? 2500 : STEP_MS;
                    renderer->startPathAnimation(res.path, dur, res.isReturnPath);
                }
                if (res.noPathToTarget)
                    statusMsg = "No path to target. Skipping...";
                else if (res.isReturnPath)
                    statusMsg = "Return path: " + std::to_string(res.path.size()) + " nodes";
                else if (res.treasureOnRoute)
                    statusMsg = "Treasure found en route! Step " + std::to_string(res.stepNum);
                else if (res.phase == GamePhase::TREASURE_FOUND)
                    statusMsg = "Treasure reached! Step " + std::to_string(res.stepNum);
                else if (res.phase == GamePhase::DONE || res.phase == GamePhase::NO_PATH)
                    statusMsg = "Press ENTER to see results";
                else
                    statusMsg = "Step " + std::to_string(res.stepNum)
                              + ": " + std::to_string(res.path.size()) + " nodes";
                stepClock.restart();
            }
        }

        // ── Draw ─────────────────────────────────────────────────────────────
        win.clear(sf::Color(5, 5, 15));

        if (appState == AppState::MAIN_MENU) {
            if (fontOk) {
                float cy = 200.f;
                drawCenteredText(win, font, "N E T U N O", 48, cy,
                                 sf::Color(80, 180, 255));
                cy += 80.f;
                drawCenteredText(win, font, "1. Unweighted  (BFS / DFS / DLS / IDS / BDS)", 22, cy);
                cy += 40.f;
                drawCenteredText(win, font, "2. Weighted    (UCS / Greedy / A* / AIA*)", 22, cy);
                cy += 40.f;
                drawCenteredText(win, font, "3. Tutorial", 22, cy, sf::Color(100, 220, 160));
                cy += 60.f;
                drawCenteredText(win, font, "9. Quit", 20, cy, sf::Color(180, 100, 100));
            }
        } else if (appState == AppState::TUTORIAL) {
            if (fontOk) {
                const float WW = static_cast<float>(win.getSize().x);
                const float WH = static_cast<float>(win.getSize().y);
                const float LX = 60.f;
                const float RX = WW * 0.5f + 20.f;
                const float SQ = 16.f;
                constexpr unsigned FSIZ = 15u;
                const float LH = 22.f;

                auto sq = [&](float x, float y, sf::Color col) {
                    sf::RectangleShape r({SQ, SQ});
                    r.setPosition(x, y);
                    r.setFillColor(col);
                    r.setOutlineColor(sf::Color(120,120,160));
                    r.setOutlineThickness(1.f);
                    win.draw(r);
                };
                auto txt = [&](const std::string& s, float x, float y,
                               sf::Color col = sf::Color(200,220,255),
                               unsigned fsz  = 15u) {
                    sf::Text t;
                    t.setFont(font); t.setCharacterSize(fsz);
                    t.setFillColor(col); t.setString(s);
                    t.setPosition(x, y);
                    win.draw(t);
                };
                auto hline = [&](float y) {
                    sf::RectangleShape l({WW - 2*LX, 1.f});
                    l.setPosition(LX, y);
                    l.setFillColor(sf::Color(60,80,120));
                    win.draw(l);
                };
                auto entry = [&](float x, float& y,
                                 sf::Color col, const std::string& name,
                                 const std::string& weight, const std::string& desc) {
                    sq(x, y + 2.f, col);
                    txt(name,   x + SQ + 6.f,   y, sf::Color(240,240,180), FSIZ);
                    txt(weight, x + SQ + 135.f,  y, sf::Color(160,220,160), FSIZ);
                    txt(desc,   x + SQ + 235.f,  y, sf::Color(170,170,200), FSIZ - 1);
                    y += LH;
                };

                drawCenteredText(win, font, "N E T U N O  ::  T U T O R I A L",
                                 28, 14.f, sf::Color(80,180,255));
                std::string pageLabel = tutPage == 0
                    ? "Page 1/2  -  Game Overview & Unweighted Mode"
                    : "Page 2/2  -  Weighted Mode Terrain";
                drawCenteredText(win, font, pageLabel, 15, 52.f, sf::Color(120,130,160));
                hline(74.f);

                if (tutPage == 0) {
                    float y = 84.f;

                    txt("HOW THE GAME WORKS", LX, y, sf::Color(100,200,255), 17); y += 26.f;
                    txt("The ship starts at the Base (corner of the grid) and searches for the Treasure.", LX, y); y += LH;
                    txt("Each step, the algorithm picks a random open cell and finds a path to it.", LX, y); y += LH;
                    txt("If the Treasure is reached or lies along a path, the ship returns to Base.", LX, y); y += LH;
                    txt("Blue overlay = exploration route            Gold overlay = return to base",
                        LX, y, sf::Color(150,200,255)); y += LH + 4.f;

                    sq(LX,         y + 2.f, sf::Color(40,100,200));
                    txt("Base  (S)",  LX + SQ + 6.f, y, sf::Color(100,180,255));
                    sq(LX + 200.f, y + 2.f, sf::Color(220,180,30));
                    txt("Treasure  (T)", LX + 218.f, y, sf::Color(255,220,80));
                    sq(LX + 460.f, y + 2.f, sf::Color(100,200,255,200));
                    txt("Exploration path", LX + 478.f, y, sf::Color(100,200,255));
                    sq(LX + 700.f, y + 2.f, sf::Color(255,200,50,200));
                    txt("Return path",  LX + 718.f, y, sf::Color(255,200,80));
                    y += LH + 8.f;
                    hline(y); y += 10.f;

                    txt("UNWEIGHTED MODE  (BFS / DFS / DLS / IDS / BDS)",
                        LX, y, sf::Color(100,200,255), 17); y += 26.f;
                    txt("Cells are either passable or blocked. All moves cost the same.", LX, y); y += LH;
                    txt("Algorithms minimise the number of hops, not energy cost.", LX, y); y += LH + 6.f;
                    entry(LX, y, sf::Color(20,35,65),  "Open Space", "cost: 1",  "freely traversable");
                    entry(LX, y, sf::Color(80,72,64),  "Asteroid",   "BLOCKED",  "impassable wall");
                    y += 6.f;
                    hline(y); y += 10.f;

                    txt("ALGORITHMS", LX, y, sf::Color(100,200,255), 17); y += 26.f;
                    float ay = y, by = y;
                    txt("BFS  Breadth-First Search",             LX,  ay, sf::Color(200,230,255), FSIZ); ay += LH;
                    txt("     Level-by-level exploration.",      LX,  ay, sf::Color(160,170,190), FSIZ-1); ay += LH;
                    txt("     Guarantees shortest path (hops).", LX,  ay, sf::Color(160,170,190), FSIZ-1); ay += LH + 6.f;
                    txt("DFS  Depth-First Search",               LX,  ay, sf::Color(200,230,255), FSIZ); ay += LH;
                    txt("     Goes as deep as possible first.",  LX,  ay, sf::Color(160,170,190), FSIZ-1); ay += LH;
                    txt("     Not optimal.",                     LX,  ay, sf::Color(160,170,190), FSIZ-1); ay += LH + 6.f;
                    txt("DLS  Depth-Limited Search",             LX,  ay, sf::Color(200,230,255), FSIZ); ay += LH;
                    txt("     DFS with a hard depth cap.",       LX,  ay, sf::Color(160,170,190), FSIZ-1); ay += LH;
                    txt("     May miss path if limit too low.",  LX,  ay, sf::Color(160,170,190), FSIZ-1); ay += LH;

                    txt("IDS  Iterative Deepening",              RX, by, sf::Color(200,230,255), FSIZ); by += LH;
                    txt("     Repeats DLS with growing limit.",  RX, by, sf::Color(160,170,190), FSIZ-1); by += LH;
                    txt("     Optimal like BFS, low memory.",    RX, by, sf::Color(160,170,190), FSIZ-1); by += LH + 6.f;
                    txt("BDS  Bidirectional Search",             RX, by, sf::Color(200,230,255), FSIZ); by += LH;
                    txt("     Searches from start AND goal.",    RX, by, sf::Color(160,170,190), FSIZ-1); by += LH;
                    txt("     Meets in the middle, faster.",     RX, by, sf::Color(160,170,190), FSIZ-1); by += LH;

                    txt("RIGHT / ENTER  next page        ESC  back to menu",
                        LX, WH - 28.f, sf::Color(100,100,140), 14);

                } else {
                    float y = 84.f;
                    txt("WEIGHTED MODE  (UCS / Greedy / A* / AIA*)",
                        LX, y, sf::Color(100,200,255), 17); y += 26.f;
                    txt("Every cell has a movement cost (weight). Algorithms minimise total path cost.", LX, y); y += LH;
                    txt("Entering a cell costs its weight regardless of direction.", LX, y); y += LH + 6.f;
                    hline(y); y += 10.f;

                    txt("TERRAIN CELLS", LX, y, sf::Color(100,200,255), 17);
                    txt("Name",        LX + SQ + 6.f,   y, sf::Color(160,160,160), 13);
                    txt("Weight",      LX + SQ + 135.f,  y, sf::Color(160,160,160), 13);
                    txt("Description", LX + SQ + 235.f,  y, sf::Color(160,160,160), 13);
                    y += 24.f;
                    entry(LX, y, sf::Color(20,35,65),   "Open Space",    "1-49",    "random free-space, most common cell");
                    entry(LX, y, sf::Color(80,72,64),   "Asteroid",      "BLOCKED", "impassable wall");
                    entry(LX, y, sf::Color(90,25,130),  "Nebula",        "15-25",   "interstellar gas cloud, moderate slowdown");
                    entry(LX, y, sf::Color(0,180,200),  "Wormhole",      "1",       "teleports to its paired wormhole instantly");
                    entry(LX, y, sf::Color(255,145,20), "Pulsar Jet",    "55",      "radiation stream (cross pattern, 2 cells out)");
                    entry(LX, y, sf::Color(250,230,80), "Pulsar Center", "75",      "neutron star core");
                    entry(LX, y, sf::Color(55,25,80),   "BH Ring 2",     "50",      "outer gravitational field (16 cells)");
                    entry(LX, y, sf::Color(35,10,60),   "BH Ring 1",     "80",      "strong gravitational pull (8 cells)");
                    entry(LX, y, sf::Color(8,3,18),     "Black Hole",    "100",     "singularity core, highest movement cost");
                    y += 6.f;
                    hline(y); y += 10.f;

                    txt("ALGORITHMS", LX, y, sf::Color(100,200,255), 17); y += 26.f;
                    float ay = y, by = y;
                    txt("UCS    Uniform-Cost Search",              LX,  ay, sf::Color(200,230,255), FSIZ); ay += LH;
                    txt("       Expands cheapest-cost paths first.",LX, ay, sf::Color(160,170,190), FSIZ-1); ay += LH;
                    txt("       Guarantees optimal cost.",          LX, ay, sf::Color(160,170,190), FSIZ-1); ay += LH + 6.f;
                    txt("A*     A-Star",                            LX, ay, sf::Color(200,230,255), FSIZ); ay += LH;
                    txt("       UCS guided by a heuristic.",        LX, ay, sf::Color(160,170,190), FSIZ-1); ay += LH;
                    txt("       Optimal and faster than UCS.",      LX, ay, sf::Color(160,170,190), FSIZ-1); ay += LH;

                    txt("Greedy Best-First",                        RX, by, sf::Color(200,230,255), FSIZ); by += LH;
                    txt("       Follows heuristic only.",           RX, by, sf::Color(160,170,190), FSIZ-1); by += LH;
                    txt("       Fast but not always optimal.",      RX, by, sf::Color(160,170,190), FSIZ-1); by += LH + 6.f;
                    txt("AIA*   Anytime Incremental A*",            RX, by, sf::Color(200,230,255), FSIZ); by += LH;
                    txt("       Tightens cost bound each iteration.",RX,by, sf::Color(160,170,190), FSIZ-1); by += LH;
                    txt("       Finds optimal path progressively.", RX, by, sf::Color(160,170,190), FSIZ-1); by += LH;

                    y = std::max(ay, by) + 10.f;
                    hline(y); y += 8.f;
                    txt("HEURISTIC  (A* / Greedy / AIA*)", LX, y, sf::Color(100,200,255), 16); y += 22.f;
                    txt("Custom weighted Euclidean distance, direction-biased.", LX, y); y += LH;
                    txt("Wormhole shortcuts are evaluated: the heuristic picks the cheaper route.", LX, y);

                    txt("LEFT  previous page        ENTER / ESC  back to menu",
                        LX, WH - 28.f, sf::Color(100,100,140), 14);
                }
            }
        } else if (appState == AppState::CONFIG_SIZE ||
                   appState == AppState::CONFIG_PROB ||
                   appState == AppState::CONFIG_FUEL ||
                   appState == AppState::PARAMS_DLS  ||
                   appState == AppState::PARAMS_IDS) {
            if (fontOk) {
                drawCenteredText(win, font, configPrompt, 28, 300.f);
                drawCenteredText(win, font, inputBuf + "_", 36, 360.f,
                                 sf::Color(255, 220, 80));
                drawCenteredText(win, font, "[ENTER] confirm  [BACKSPACE] delete",
                                 18, 430.f, sf::Color(140, 140, 160));
            }
        } else if (appState == AppState::ALGO_SELECT) {
            if (fontOk) {
                drawCenteredText(win, font, "Choose algorithm:", 30, 200.f,
                                 sf::Color(80, 180, 255));
                float cy = 270.f;
                if (graphMode == GraphMode::UNWEIGHTED) {
                    for (auto& s : std::vector<std::string>{
                            "1. BFS", "2. DFS", "3. DLS (depth limit)",
                            "4. IDS (max limit)", "5. BDS"}) {
                        drawCenteredText(win, font, s, 22, cy);
                        cy += 36.f;
                    }
                } else {
                    for (auto& s : std::vector<std::string>{
                            "1. UCS", "2. Greedy", "3. A*", "4. AIA*"}) {
                        drawCenteredText(win, font, s, 22, cy);
                        cy += 36.f;
                    }
                }
            }
        } else if (appState == AppState::PLAYING && gs && renderer) {
            renderer->render(*gs, lastPath, paused, totalCost, statusMsg);
        } else if (appState == AppState::RESULT) {
            if (fontOk) {
                std::string title = (gs && gs->phase() == GamePhase::DONE)
                    ? "Mission Complete!" : "Treasure Not Found";
                drawCenteredText(win, font, title, 40, 220.f,
                                 sf::Color(255, 220, 80));
                float ry = 300.f;
                if (gs && gs->isWeighted()) {
                    float explCost = totalCost - returnCost;
                    if (explCost > 0.f) {
                        drawCenteredText(win, font,
                            "Exploration cost: " + std::to_string((int)explCost), 22, ry);
                        ry += 36.f;
                    }
                    if (returnCost > 0.f) {
                        drawCenteredText(win, font,
                            "Return path cost: " + std::to_string((int)returnCost), 22, ry,
                            sf::Color(255, 200, 80));
                        ry += 36.f;
                    }
                    if (totalCost > 0.f) {
                        drawCenteredText(win, font,
                            "Total cost: " + std::to_string((int)totalCost), 24, ry,
                            sf::Color(200, 255, 200));
                        ry += 40.f;
                    }
                }
                drawCenteredText(win, font,
                    "Steps: " + std::to_string(lastStep), 22, ry);
                ry += 60.f;
                drawCenteredText(win, font,
                    "[ENTER] Main Menu   [ESC] Quit", 20, ry,
                    sf::Color(140, 140, 160));
            }
        }

        win.display();
    }

    return 0;
}
