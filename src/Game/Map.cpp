/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Map.h"
#include <Gfx/Renderer.h>
#include <Util/MemoryArena.h>

Map::Map(u32 width, u32 height, MemoryArena& arena)
    : m_tiles(arena.allocate_array_2d<Tile>(width, height))
{
}

Map::~Map()
{
}

Tile& Map::tile_at(u32 x, u32 y)
{
    return m_tiles.get(x, y);
}

void Map::render() const
{
    auto& renderer = the_renderer();

    for (auto y = 0; y < m_tiles.h; ++y) {
        for (auto x = 0; x < m_tiles.w; ++x) {
            auto& terrain_sprite = tile_at(x, y).terrain_sprite();
            drawSingleSprite(&renderer.world_buffer(), &terrain_sprite, { x, y, 1, 1 }, renderer.shaderIds.pixelArt, Colour::white());
        }
    }

    // FIXME: Draw the actors
}
