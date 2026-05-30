/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Tile.h"
#include "AppState.h"
#include <Game/Item.h>
#include <Game/ItemCatalogue.h>

Tile::Tile(Terrain terrain)
    : m_terrain(terrain)
    , m_terrain_sprite(get_terrain_def(terrain).sprite_name, AppState::the().cosmeticRandom->next())
{
}

void Tile::set_terrain(Terrain terrain)
{
    if (m_terrain == terrain)
        return;
    m_terrain = terrain;
    m_terrain_sprite = SpriteRef { get_terrain_def(terrain).sprite_name, AppState::the().cosmeticRandom->next() };
}

void Tile::set_terrain_raw(Badge<Map>, Terrain terrain)
{
    m_terrain = terrain;
}

Sprite& Tile::terrain_sprite() const
{
    return m_terrain_sprite.get();
}

void Tile::fetch_sprite()
{
    m_terrain_sprite = SpriteRef { get_terrain_def(m_terrain).sprite_name, AppState::the().cosmeticRandom->next() };
}

TileItemsCache::TileItemsCache(MemoryArena& arena, u32 width, u32 height)
    : m_tiles(arena.allocate_array_2d<ChunkedArray<flecs::entity>>(width, height))
{
    initChunkPool(&m_pool, &arena, 32);

    // FIXME: Flat Array2 iteration
    for (int y = 0; y < m_tiles.h; ++y) {
        for (int x = 0; x < m_tiles.w; ++x) {
            initChunkedArray(&m_tiles.get(x, y), &m_pool);
        }
    }
}

void TileItemsCache::add(s32 x, s32 y, flecs::entity item)
{
    m_tiles.get(x, y).append(item);
}

void TileItemsCache::clear()
{
    // FIXME: Flat Array2 iteration
    for (int y = 0; y < m_tiles.h; ++y) {
        for (int x = 0; x < m_tiles.w; ++x) {
            m_tiles.get(x, y).clear();
        }
    }
}

void TileItemsCache::sort_and_compact(flecs::world)
{
    // TODO: Implement this!
}

ChunkedArray<flecs::entity>& TileItemsCache::items_in_tile(s32 x, s32 y)
{
    return m_tiles.get(x, y);
}
