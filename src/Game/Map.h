/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Tile.h>
#include <Util/Array2.h>

class Map {
public:
    Map(u32 width, u32 height, MemoryArena&);
    ~Map();

    Tile& tile_at(u32 x, u32 y);
    Tile const& tile_at(u32 x, u32 y) const { return const_cast<Map*>(this)->tile_at(x, y); }

    void render() const;

private:
    Array2<Tile> m_tiles;
};
