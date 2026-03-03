/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Map.h"
#include <Game/Item.h>
#include <Game/ItemCatalogue.h>
#include <Gfx/Renderer.h>
#include <Util/MemoryArena.h>
#include <Util/Random.h>

NonnullOwnPtr<Map> Map::generate_surface(u32 width, u32 height, Random& random, MemoryArena& arena, ArrayChunkPool<NonnullOwnPtr<Item>>& item_chunk_pool)
{
    auto map = adopt_own(*new Map(width, height, arena, item_chunk_pool));

    // General idea:
    // Sea to the south, a sandy beach, and then grass/woodland.

    // Grass default
    for (auto y = 0; y < height; ++y) {
        for (auto x = 0; x < width; ++x) {
            map->tile_at(x, y).set_terrain_raw({}, Terrain::Grass);
        }
    }

    // Coast
    auto coastline_water = temp_arena().allocate_array<float>(width, true);
    auto coastline_sand = temp_arena().allocate_array<float>(width, true);
    random.fill_with_noise(coastline_water, 10, false);
    random.fill_with_noise(coastline_sand, 12, false);

    for (auto x = 0; x < width; ++x) {
        auto column_coastline_level = static_cast<int>(10 + (coastline_water[x] * 12));
        auto column_sand_level = static_cast<int>(column_coastline_level + (coastline_sand[x] * 8));

        for (auto y = height - column_sand_level; y < height - column_coastline_level; ++y)
            map->tile_at(x, y).set_terrain_raw({}, Terrain::Sand);

        for (auto y = height - column_coastline_level; y < height; ++y)
            map->tile_at(x, y).set_terrain_raw({}, Terrain::Water);
    }

    auto& item_catalogue = ItemCatalogue::the();
    ItemType stick = item_catalogue.find_name("stick"_s).release_value();
    auto stick_count = random.random_between(width * height / 300, width * height / 200);
    for (auto i = 0; i < stick_count; ++i) {
        auto x = random.random_below(width);
        auto y = random.random_below(height);
        map->tile_at(x, y).add_item(stick);
    }

    ItemType rock = item_catalogue.find_name("rock"_s).release_value();
    auto rock_count = random.random_between(width * height / 300, width * height / 200);
    for (auto i = 0; i < rock_count; ++i) {
        auto x = random.random_below(width);
        auto y = random.random_below(height);
        map->tile_at(x, y).add_item(rock);
    }

    // Initialize all sprites
    for (auto y = 0; y < height; ++y) {
        for (auto x = 0; x < width; ++x) {
            map->tile_at(x, y).fetch_sprite();
        }
    }

    return map;
}

Map::Map(u32 width, u32 height, MemoryArena& arena, ArrayChunkPool<NonnullOwnPtr<Item>>& item_chunk_pool)
    : m_tiles(arena.allocate_array_2d<Tile>(width, height))
{
    initChunkedArray(&m_actors, &arena, 1024);

    // Properly init tiles
    for (auto y = 0; y < height; ++y) {
        for (auto x = 0; x < width; ++x) {
            m_tiles.set(x, y, Tile(Terrain::Grass, item_chunk_pool));
        }
    }
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

    // Draw actors and items
    for (s32 y = visible_tile_bounds.y();
        y < visible_tile_bounds.y() + visible_tile_bounds.height();
        y++) {
        for (s32 x = visible_tile_bounds.x();
            x < visible_tile_bounds.x() + visible_tile_bounds.width();
            x++) {
            auto& tile = tile_at(x, y);

            if (!tile.items().is_empty()) {
                drawSingleSprite(&renderer.world_buffer(), &tile.items().get(0)->sprite(), { x, y, 1, 1 }, renderer.shaderIds.pixelArt, Colour::white());
            }

            if (auto* actor = tile.actor())
                actor->render(delta_time);
        }
    }
}
