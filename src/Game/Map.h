/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Actor.h>
#include <Game/Tile.h>
#include <Util/Array2.h>

class Map {
public:
    static NonnullOwnPtr<Map> generate_surface(u32 width, u32 height, Random&, MemoryArena&, ArrayChunkPool<NonnullOwnPtr<Item>>&);
    ~Map();

    u32 width() const { return m_tiles.w; }
    u32 height() const { return m_tiles.h; }

    Tile& tile_at(u32 x, u32 y);
    Tile const& tile_at(u32 x, u32 y) const { return const_cast<Map*>(this)->tile_at(x, y); }

    void add_actor(NonnullOwnPtr<Actor>);

    void update();
    void render(float delta_time) const;

private:
    Map(u32 width, u32 height, MemoryArena&, ArrayChunkPool<NonnullOwnPtr<Item>>&);
    Array2<Tile> m_tiles;
    ChunkedArray<NonnullOwnPtr<Actor>> m_actors;
};
