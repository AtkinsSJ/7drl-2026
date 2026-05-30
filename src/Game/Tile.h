/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Forward.h>
#include <Game/Terrain.h>
#include <Gfx/Sprite.h>
#include <flecs.h>

class Tile {
public:
    Tile() = default;
    explicit Tile(Terrain);

    void set_terrain(Terrain);
    void set_terrain_raw(Badge<Map>, Terrain);
    Terrain terrain() const { return m_terrain; }
    Sprite& terrain_sprite() const;
    void fetch_sprite();

private:
    Terrain m_terrain;
    SpriteRef m_terrain_sprite;
};

class TileItemsCache {
public:
    explicit TileItemsCache(MemoryArena& arena, u32 width, u32 height);

    void add(s32 x, s32 y, flecs::entity item);
    void clear();
    void sort_and_compact(flecs::world);

    ChunkedArray<flecs::entity>& items_in_tile(s32 x, s32 y);

private:
    ArrayChunkPool<flecs::entity> m_pool;
    Array2<ChunkedArray<flecs::entity>> m_tiles;
};
