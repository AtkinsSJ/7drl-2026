/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Actor.h>
#include <Game/Terrain.h>
#include <Gfx/Sprite.h>

class Tile {
public:
    Tile() = default;
    Tile(Terrain, ArrayChunkPool<NonnullOwnPtr<Item>>&);

    void set_terrain(Terrain);
    void set_terrain_raw(Badge<Map>, Terrain);
    Terrain terrain() const { return m_terrain; }
    Sprite& terrain_sprite() const;
    void fetch_sprite();

    Actor* actor() const { return m_actor; }
    void set_actor(Actor* actor)
    {
        ASSERT((actor == nullptr) != (m_actor == nullptr));
        m_actor = actor;
    }

    ChunkedArray<NonnullOwnPtr<Item>> const& items() const { return m_items; }
    ChunkedArray<NonnullOwnPtr<Item>>& items() { return m_items; }
    Item& add_item(ItemType);

private:
    Terrain m_terrain;
    SpriteRef m_terrain_sprite;
    Actor* m_actor;
    ChunkedArray<NonnullOwnPtr<Item>> m_items;
};
