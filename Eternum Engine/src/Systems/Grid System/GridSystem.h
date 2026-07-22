/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: GridSystem
* Description:
*              Handles creation, storage, and rendering of grid-based maps.
*
* Author:     Jax Clayton
* Created:    8/3/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef GRIDSYSTEM_H
#define GRIDSYSTEM_H

#include <Systems/system.h>
#include <Core/Camera/CameraShake.h>
#include <Core/Terminal/TerminalRenderer.h>

class GridSystem final : public System
{
public:
    struct Dimension
    {
        int m_Width;
        int m_Height;

        Dimension(const int x, const int y) : m_Width(x), m_Height(y) {}
        Dimension(const Dimension& other) : m_Width(other.m_Width), m_Height(other.m_Height) {}
    };

    struct Grid
    {
        Dimension m_Dimension;
        std::vector<char> cells;

        Grid() : m_Dimension(0, 0), cells() {} // Default: empty grid

        Grid(const int w, const int h, const char fill = '.')
            : m_Dimension(w,h), cells(w * h, fill) {}

        char GetCell(const int x, const int y) const {
            if (x < 0 || x >= m_Dimension.m_Width || y < 0 || y >= m_Dimension.m_Height) return ' ';
            return cells[y * m_Dimension.m_Width + x];
        }

        char GetCell(const Dimension& d) const {
            return GetCell(d.m_Width, d.m_Height);
        }

        void SetCell(const int x, const int y, const char value) {
            if (x < 0 || x >= m_Dimension.m_Width || y < 0 || y >= m_Dimension.m_Height) return;
            cells[y * m_Dimension.m_Width + x] = value;
        }

        void SetCell(const Dimension& d, const char value) {
            SetCell(d.m_Width, d.m_Height, value);
        }
    };

    GridSystem();

    void Init() override;
    void Shutdown() override;

    void Update(double deltaTime) override;
    void FixedUpdate() override;
    void Render() override;

    // Map management
    void CreateMap(const std::string& name, const Dimension& dimensions, char fill = '.');
    void AddMap(const std::string& name, const Grid& map);
    bool LoadMap(const std::string& name);
    void DeleteMap(const std::string& name);
    void ClearMaps();

    // whenever anything mutates the grid
    void MarkDirty() { m_needsRedraw = true; }

    // Active map controls
    int GetWidth() const;
    int GetHeight() const;
    char GetCell(int x, int y) const;
    void SetCell(int x, int y, char value);

    /// @brief  whether an Entity can stand on a cell
    /// @param  x   the column to test
    /// @param  y   the row to test
    /// @return false for walls and for anything off the map
    bool IsWalkable(int x, int y) const;

    /// @brief  knocks the view about
    /// @param  trauma  how hard, 1 is as shaken as it gets
    void AddShake(double trauma);

    /// @brief  stops any shake immediately
    void ClearShake();

    /// @brief  holds the shake on so it can be looked at
    /// @param  continuous  whether to keep it shaking
    void SetContinuousShake(bool continuous);

    /// @brief  whether the shake is being held on
    /// @return whether continuous mode is on
    bool IsContinuousShake() const { return m_Shake.IsContinuous(); }

    /// @brief  how far the view is currently pushed
    /// @return the offset in cells
    Vec2i const& GetShakeOffset() const { return m_Shake.GetOffset(); }

    /// @brief  whether the view is being pushed around right now
    /// @return whether anything is shaking
    bool IsShaking() const { return m_Shake.IsShaking(); }

    /// @brief  builds what should be on screen this frame
    /// @return the map shifted by the shake, with trails and Entities drawn over it
    Grid BuildFrame() const;

    /// @brief  sets the panel drawn beside the map
    /// @param  lines   the panel lines, they may carry their own escape codes
    void SetPanel(const std::vector<std::string>& lines);

    /// @brief  draws a trail of cells over the map, used to show a computed path
    /// @param  ownerId the component that owns this trail, so it can replace its own
    /// @param  cells   the cells to mark
    void SetOverlay(unsigned ownerId, const std::vector<Vec2i>& cells);

    /// @brief  removes one owner's trail
    /// @param  ownerId the component whose trail to remove
    void ClearOverlay(unsigned ownerId);

    /// @brief  removes every trail
    void ClearAllOverlays();

    /// @brief  sets whether trails are drawn at all
    /// @param  visible whether to draw them
    void SetOverlayVisible(bool visible);

    /// @brief  gets whether trails are drawn at all
    /// @return whether trails are drawn
    bool IsOverlayVisible() const { return m_OverlayVisible; }

    /// @brief  the character a trail is drawn with
    static constexpr char OVERLAY_SYMBOL = '*';

    /// @brief  repaints everything next frame instead of only what changed
    /// @note   needed after anything else writes to the console
    void ForceFullRedraw();


    /// @brief  characters an Entity cannot walk through
    inline static const std::string BLOCKING_TILES = "# ";

    // Console Commands
    static void ClearConsole()
    {
        std::cout << "\033[2J\033[H"; // ANSI escape code to clear console
    }

    inline static const std::unordered_map<char, const char*> TILE_COLORS = {
        { '#', "\x1b[90m" }, // Wall (gray)
        { '.', "\x1b[33m" }, // Floor (yellow)
        { ' ', "\x1b[30m" }, // Empty (black)
        { '@', "\x1b[31m" }, // Player (red)
        { 'E', "\x1b[35m" }, // Enemy (magenta)
        { 'e', "\x1b[95m" }, // Enemy roaming (bright magenta)
        { 'D', "\x1b[36m" }, // Door (cyan)
        { '~', "\x1b[34m" }, // Water (blue)
        { '*', "\x1b[32m" }, // Path trail (green)
    };


    // -------------------------------------------------------------------
    // Singleton pattern to ensure only one instance of GridSystem exists
    // -------------------------------------------------------------------
    static std::shared_ptr<GridSystem> GetInstance()
    {
        static std::shared_ptr<GridSystem> instance(new GridSystem());
        return instance;
    }

private:

    /// @brief  draws every trail over a copy of the map
    /// @param  frame   the copy to draw into
    /// @param  offset  how far the view is pushed
    void stampOverlays(Grid& frame, const Vec2i& offset) const;

    /// @brief  draws every visible Glyph over a copy of the map
    /// @param  frame   the copy to draw into
    /// @param  offset  how far the view is pushed
    static void stampEntities(Grid& frame, const Vec2i& offset);

    std::unordered_map<std::string, Grid> m_maps;
    std::string m_activeMapName;
    std::vector<std::string> m_Panel;

    /// @brief  how hard the view is being knocked about
    CameraShake m_Shake;

    /// @brief  the trails to draw, keyed by whichever component owns each one
    std::map<unsigned, std::vector<Vec2i>> m_Overlays;
    bool m_OverlayVisible = true;

    bool m_needsRedraw = true;
};

REGISTER_SYSTEM(GridSystem)

#endif //GRIDSYSTEM_H
