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
    initChunkedArray(&m_actors, &arena, 1024);
}

Map::~Map()
{
    // FIXME: We have to deallocate the actors manually because ChunkedArray is dumb.
    for (auto it = m_actors.iterate(); it.hasNext(); it.next()) {
        it.get()->~Actor();
    }
    m_actors.clear();
}

Tile& Map::tile_at(u32 x, u32 y)
{
    return m_tiles.get(x, y);
}

void Map::add_actor(NonnullOwnPtr<Actor> actor)
{
    actor->set_map(this);
    m_actors.append(move(actor));
}

void Map::update()
{
    for (auto it = m_actors.iterate(); it.hasNext(); it.next()) {
        it.get()->update();
    }
}

void Map::render(float delta_time) const
{
    auto& renderer = the_renderer();

    // Calculate the tile area that's visible to the player.
    // We err on the side of drawing too much, rather than risking having holes in the world.
    auto& world_camera = renderer.world_camera();
    Rect2I visible_tile_bounds = Rect2I::create_centre_size(
        v2i(world_camera.position()), v2i(world_camera.size() / world_camera.zoom()) + v2i(3, 3));
    visible_tile_bounds = visible_tile_bounds.intersected({ 0, 0, static_cast<s32>(width()), static_cast<s32>(height()) });

    // Draw tiles
    for (s32 y = visible_tile_bounds.y();
        y < visible_tile_bounds.y() + visible_tile_bounds.height();
        y++) {
        for (s32 x = visible_tile_bounds.x();
            x < visible_tile_bounds.x() + visible_tile_bounds.width();
            x++) {
            auto& terrain_sprite = tile_at(x, y).terrain_sprite();
            drawSingleSprite(&renderer.world_buffer(), &terrain_sprite, { x, y, 1, 1 }, renderer.shaderIds.pixelArt, Colour::white());
        }
    }

    // Draw actors
    for (s32 y = visible_tile_bounds.y();
        y < visible_tile_bounds.y() + visible_tile_bounds.height();
        y++) {
        for (s32 x = visible_tile_bounds.x();
            x < visible_tile_bounds.x() + visible_tile_bounds.width();
            x++) {
            if (auto* actor = tile_at(x, y).actor())
                actor->render(delta_time);
        }
    }
}
