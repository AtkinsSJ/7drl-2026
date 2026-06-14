/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Tile.h>
#include <Util/Array2.h>
#include <flecs.h>

class Map {
public:
    Map();
    Map(u32 width, u32 height, MemoryArena&);
    ~Map();

    static void generate(flecs::world, u32 width, u32 height, Random&, MemoryArena&);

    u32 width() const { return m_tiles.w; }
    u32 height() const { return m_tiles.h; }

    Tile& tile_at(u32 x, u32 y);
    Tile const& tile_at(u32 x, u32 y) const { return const_cast<Map*>(this)->tile_at(x, y); }

    void render(float delta_time) const;

private:
    Array2<Tile> m_tiles;
};

struct mod_map {
    explicit mod_map(flecs::world&);
};
