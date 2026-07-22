/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: GridSystem.cpp
* Description:
*     Handles creation, storage, and rendering of grid-based maps.
*
* Author:     Jax Clayton
* Created:    8/3/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "GridSystem.h"
#include <Core/ECS/Component/Glyph/Glyph.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Core/ECS/Entity/Entity.h>


// --------------------------------------------------------
// Constructor / Destructor
// --------------------------------------------------------
GridSystem::GridSystem()
    :  System("Grid System") , m_activeMapName("")
{
}

// --------------------------------------------------------
// Lifecycle Methods
// --------------------------------------------------------
void GridSystem::Init()
{
    System::Init();

    // from here the game draws to a screen of its own
    Terminal().BeginSession();
}

void GridSystem::Shutdown()
{
    // hand the terminal back before anything else prints
    Terminal().EndSession();

    System::Shutdown();
}

void GridSystem::ForceFullRedraw()
{
    Terminal().ForceFullRedraw();
    MarkDirty();
}

void GridSystem::SetOverlay(const unsigned ownerId, const std::vector<Vec2i>& cells)
{
    if (cells.empty())
    {
        ClearOverlay(ownerId);
        return;
    }

    m_Overlays[ownerId] = cells;
    MarkDirty();
}

void GridSystem::ClearOverlay(const unsigned ownerId)
{
    if (m_Overlays.erase(ownerId) > 0)
        MarkDirty();
}

void GridSystem::ClearAllOverlays()
{
    if (m_Overlays.empty())
        return;

    m_Overlays.clear();
    MarkDirty();
}

void GridSystem::SetOverlayVisible(const bool visible)
{
    if (m_OverlayVisible == visible)
        return;

    m_OverlayVisible = visible;
    MarkDirty();
}

void GridSystem::Update(double deltaTime)
{
    // the grid only stores and draws maps, the DungeonSystem decides what goes in them.
    // the one thing it owns is the view, and the view is what gets shaken
    const Vec2i before = m_Shake.GetOffset();
    m_Shake.Update(deltaTime);

    if (!(m_Shake.GetOffset() == before))
        MarkDirty();
}

void GridSystem::AddShake(const double trauma)
{
    m_Shake.AddTrauma(trauma);
}

void GridSystem::ClearShake()
{
    if (!m_Shake.IsShaking() && !m_Shake.IsContinuous())
        return;

    m_Shake.Clear();
    MarkDirty();
}

void GridSystem::SetContinuousShake(const bool continuous)
{
    if (m_Shake.IsContinuous() == continuous)
        return;

    m_Shake.SetContinuous(continuous);

    if (!continuous)
        m_Shake.Clear();

    MarkDirty();
}

void GridSystem::FixedUpdate()
{

}

void GridSystem::Render()
{
    // nothing to draw or map invalid? skip
    if (m_activeMapName.empty() ||
        m_maps.find(m_activeMapName) == m_maps.end() ||
        !m_needsRedraw)
    {
        return;
    }

    // draw into a copy, the stored map stays exactly as the generator made it
    const Grid frame = BuildFrame();

    Terminal().Draw(frame.cells, frame.m_Dimension.m_Width, frame.m_Dimension.m_Height,
                    m_Panel, TILE_COLORS);

    // we are now up to date
    m_needsRedraw = false;
}

GridSystem::Grid GridSystem::BuildFrame() const
{
    const Grid& source = m_maps.at(m_activeMapName);
    const Vec2i offset = m_Shake.GetOffset();

    const int width = source.m_Dimension.m_Width;
    const int height = source.m_Dimension.m_Height;

    // nothing shaking is the usual case, so do not pay for the copy loop
    Grid frame = source;

    if (!(offset == Vec2i{ 0, 0 }))
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const int sourceX = x + offset.x();
                const int sourceY = y + offset.y();

                const bool inside = sourceX >= 0 && sourceX < width
                                 && sourceY >= 0 && sourceY < height;

                frame.SetCell(x, y, inside ? source.GetCell(sourceX, sourceY) : '#');
            }
        }

        // the outer ring is the frame the picture sits in, so it is held still while the
        // inside slides. shifting it too puts floor against the screen edge, which reads
        // as the dungeon leaking rather than the view being knocked
        for (int x = 0; x < width; ++x)
        {
            frame.SetCell(x, 0, '#');
            frame.SetCell(x, height - 1, '#');
        }

        for (int y = 0; y < height; ++y)
        {
            frame.SetCell(0, y, '#');
            frame.SetCell(width - 1, y, '#');
        }
    }

    stampOverlays(frame, offset);
    stampEntities(frame, offset);

    return frame;
}

void GridSystem::stampOverlays(Grid& frame, const Vec2i& offset) const
{
    if (!m_OverlayVisible)
        return;

    for (const auto& [ownerId, cells] : m_Overlays)
    {
        for (const Vec2i& cell : cells)
        {
            // the world moved under the view, so everything in it draws shifted the other way
            const int x = cell.x() - offset.x();
            const int y = cell.y() - offset.y();

            // only mark open floor, a trail should never paint over a wall
            if (frame.GetCell(x, y) == '.')
                frame.SetCell(x, y, OVERLAY_SYMBOL);
        }
    }
}

void GridSystem::stampEntities(Grid& frame, const Vec2i& offset)
{
    // copy so the draw order sort does not reorder the System's own list
    std::vector<Glyph*> glyphs = Components<Glyph>()->GetComponents();

    std::stable_sort(glyphs.begin(), glyphs.end(),
        [](const Glyph* a, const Glyph* b) { return a->GetDrawOrder() < b->GetDrawOrder(); });

    for (const Glyph* glyph : glyphs)
    {
        if (!glyph->IsVisible())
            continue;

        Entity* entity = glyph->GetEntity();
        if (entity == nullptr)
            continue;

        const Transform* transform = entity->GetComponent<Transform>();
        if (transform == nullptr)
            continue;

        const Vec2f& position = transform->GetTranslation();
        frame.SetCell(static_cast<int>(position.x()) - offset.x(),
                      static_cast<int>(position.y()) - offset.y(),
                      glyph->GetSymbol());
    }
}


// --------------------------------------------------------
// Map Management
// --------------------------------------------------------
void GridSystem::CreateMap(const std::string& name, const Dimension& dimensions, const char fill)
{
    m_maps[name] = Grid(dimensions.m_Width, dimensions.m_Height, fill);
    MarkDirty();
}

void GridSystem::AddMap(const std::string& name, const Grid& map)
{
    m_maps[name] = map;
}

bool GridSystem::LoadMap(const std::string& name)
{
    if (m_maps.find(name) == m_maps.end())
        return false;

    m_activeMapName = name;
    return true;
}

void GridSystem::DeleteMap(const std::string& name)
{
    m_maps.erase(name);
    if (m_activeMapName == name)
        m_activeMapName.clear();
    MarkDirty();
}

void GridSystem::ClearMaps()
{
    m_maps.clear();
    m_activeMapName.clear();
    MarkDirty();
}

// --------------------------------------------------------
// Cell Access
// --------------------------------------------------------
int GridSystem::GetWidth() const
{
    if (m_activeMapName.empty()) return 0;
    return m_maps.at(m_activeMapName).m_Dimension.m_Width;
}

int GridSystem::GetHeight() const
{
    if (m_activeMapName.empty()) return 0;
    return m_maps.at(m_activeMapName).m_Dimension.m_Height;
}

char GridSystem::GetCell(const int x, const int y) const
{
    if (m_activeMapName.empty()) return ' ';
    return m_maps.at(m_activeMapName).GetCell(x, y);
}

void GridSystem::SetCell(const int x, const int y, const char value)
{
    if (m_activeMapName.empty()) return;
    m_maps.at(m_activeMapName).SetCell(x, y, value);
    MarkDirty();
}

bool GridSystem::IsWalkable(const int x, const int y) const
{
    if (m_activeMapName.empty()) return false;

    const Grid& grid = m_maps.at(m_activeMapName);
    if (x < 0 || x >= grid.m_Dimension.m_Width || y < 0 || y >= grid.m_Dimension.m_Height)
        return false;

    return BLOCKING_TILES.find(grid.GetCell(x, y)) == std::string::npos;
}

void GridSystem::SetPanel(const std::vector<std::string>& lines)
{
    if (m_Panel == lines)
        return;

    m_Panel = lines;
    MarkDirty();
}

