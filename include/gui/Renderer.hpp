#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <utility>
#include "gui/GameState.hpp"

class Renderer {
public:
    Renderer(sf::RenderWindow& win, const std::string& spritesDir);

    // Call once after GameState is created.
    void buildVariants(const GameState& gs);

    // Set the ship at the origin without animation (call after buildVariants).
    void initShipPos(int x, int y);

    // Animate the ship along a full path. durationMs = total travel time.
    // isReturnPath changes the path overlay to the return colour (gold).
    void startPathAnimation(const std::vector<std::pair<int,int>>& path,
                            int durationMs, bool isReturnPath);

    // Main render call.
    void render(const GameState& gs,
                bool paused,
                const std::string& statusMsg);

    // Input handlers
    void handleScroll  (float delta, sf::Vector2i mousePixel);
    void handlePanBegin(sf::Vector2i mousePixel);
    void handlePanMove (sf::Vector2i mousePixel);
    void handlePanEnd  ();

    bool fontLoaded() const { return fontLoaded_; }

private:
    // Sprites are now exactly 8×8 px, matching one world tile unit.
    static constexpr int   TILE       = 8;
    static constexpr float SPRITE_SZ  = 8.f;
    static constexpr float SP_SCALE   = 1.f; // 1:1, no scaling
    static constexpr float HUD_FRAC   = 0.25f;
    static constexpr float MIN_ZOOM   = 1.f;
    static constexpr float MAX_ZOOM   = 24.f;

    sf::RenderWindow& win_;
    std::string       spritesDir_;

    sf::View     gridView_;
    float        zoom_    = 8.f;
    bool         panning_ = false;
    sf::Vector2i panOrigin_;
    sf::Vector2f panViewOrigin_;

    // Textures (8×8 px each)
    sf::Texture bgTex_[4];
    sf::Texture asteroidTex_[3];
    sf::Texture baseTex_, treasureTex_, shipTex_;
    sf::Texture wormholeTex_[2];
    sf::Texture nebulaTex_;
    sf::Texture pulsarCenterTex_, pulsarJetTex_;
    sf::Texture bhCoreTex_;
    sf::Texture bhR1Tex_[8];        // 0-7: tl,t,tr,l,r,bl,b,br
    sf::Texture bhR2CornerTex_[4];  // 0=tl 1=tr 2=bl 3=br
    sf::Texture bhR2OuterTex_;

    sf::Font hudFont_;
    bool     fontLoaded_ = false;

    // Per-cell hash for variant selection
    std::vector<std::vector<unsigned>> variantGrid_;

    // Ship path animation
    std::vector<sf::Vector2f> shipWaypoints_;   // world-space positions along path
    float        shipSegDur_   = 0.f;           // seconds per waypoint segment
    float        shipRotation_ = 0.f;
    bool         isReturnPath_ = false;
    sf::Clock    shipClock_;

    void loadTextures();
    void resetView(int gridSize);

    static int bhR1Index     (int di, int dj);
    static int bhR2CornerIndex(int di, int dj);

    sf::Texture& tileTexture(const GameState& gs, int x, int y);

    void drawGrid(const GameState& gs);
    void drawHUD (const GameState& gs,
                  bool paused,
                  const std::string& statusMsg);

    // Draw a scaled sprite at world tile position (x,y) with optional rotation.
    void drawTileSprite(sf::Sprite& spr, sf::Texture& tex,
                        float worldX, float worldY,
                        float rotation = 0.f, bool centerOrigin = false);
};

#endif
