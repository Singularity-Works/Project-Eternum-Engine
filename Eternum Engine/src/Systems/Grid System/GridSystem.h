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

    // Console Commands
    static void ClearConsole()
    {
        std::cout << "\033[2J\033[H"; // ANSI escape code to clear console
    }

    // -------------------------------------------------------------------
    // Singleton pattern to ensure only one instance of GridSystem exists
    // -------------------------------------------------------------------
    static std::shared_ptr<GridSystem> GetInstance()
    {
        static std::shared_ptr<GridSystem> instance(new GridSystem());
        return instance;
    }

private:
    std::unordered_map<std::string, Grid> m_maps;
    std::string m_activeMapName;
    bool m_needsRedraw = true;
};

REGISTER_SYSTEM(GridSystem)

#endif //GRIDSYSTEM_H
