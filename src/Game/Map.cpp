/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Map.h"
#include <Game/Components.h>
#include <Game/Item.h>
#include <Game/ItemCatalogue.h>
#include <Game/Player.h>
#include <Gfx/Renderer.h>
#include <Util/MemoryArena.h>
#include <Util/Random.h>

Map::Map() = default;

Map::Map(u32 width, u32 height, MemoryArena& arena)
    : m_tiles(arena.allocate_array_2d<Tile>(width, height))
{
}

Map::~Map() = default;

Tile& Map::tile_at(u32 x, u32 y)
{
    return m_tiles.get(x, y);
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
}

void Map::generate(flecs::world world, u32 width, u32 height, Random& random, MemoryArena& arena)
{
    Map map { width, height, arena };

    // General idea:
    // Sea to the south, a sandy beach, and then grass/woodland.

    // Grass default
    for (auto y = 0; y < height; ++y) {
        for (auto x = 0; x < width; ++x) {
            map.tile_at(x, y).set_terrain_raw({ }, Terrain::Grass);
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
            map.tile_at(x, y).set_terrain_raw({ }, Terrain::Sand);

        for (auto y = height - column_coastline_level; y < height; ++y)
            map.tile_at(x, y).set_terrain_raw({ }, Terrain::Water);
    }

    auto& item_catalogue = ItemCatalogue::the();
    ItemType stick = item_catalogue.find_name("stick"_s).release_value();
    auto stick_count = random.random_between(width * height / 300, width * height / 200);
    for (auto i = 0; i < stick_count; ++i) {
        auto x = random.random_below(width);
        auto y = random.random_below(height);
        item_catalogue.instantiate(world, stick)
            .set(Position { .x = x, .y = y });
    }

    ItemType stone = item_catalogue.find_name("stone"_s).release_value();
    auto stone_count = random.random_between(width * height / 300, width * height / 200);
    for (auto i = 0; i < stone_count; ++i) {
        auto x = random.random_below(width);
        auto y = random.random_below(height);
        item_catalogue.instantiate(world, stone)
            .set(Position { .x = x, .y = y });
    }

    // Generate some trees
    // FIXME: Define this properly!
    auto make_tree = [&](Position position) {
        return world.entity()
            .add<BlocksMovement>()
            .set<Name>({ "tree"_s })
            .set(move(position))
            .set<HasSprite>({ .ref = { "tree"_sv, 0 } })
            .set(DrawLayer::Plant);
    };
    auto tree_count = random.random_between(width * height / 100, width * height / 50);
    for (auto i = 0; i < tree_count; ++i) {
        auto x = random.random_below(width);
        auto y = random.random_below(height);
        make_tree({ x, y });
    }

    // Pop a player somewhere
    auto player = create_player(world, static_cast<s32>(width / 2), static_cast<s32>(height * 0.8f));

    // Give the player a few things
    give_item_to_entity(world, stick, 11, player);
    give_item_to_entity(world, stone, 19, player);

    // Initialize all sprites
    for (auto y = 0; y < height; ++y) {
        for (auto x = 0; x < width; ++x) {
            map.tile_at(x, y).fetch_sprite();
        }
    }

    world.set<Map>(move(map));
}

mod_map::mod_map(flecs::world& world)
{
    world.component<Map>().add(flecs::Singleton);

    world.component<TileItemsCache>().add(flecs::Singleton);

    world.system()
        .kind(flecs::OnStart)
        .run([](flecs::iter& it) {
            // Might be good to construct things here... but how? We don't know the map size.
            // ...unless we stick the map size in somehow before this runs.
        });

    auto post_simulation = world.lookup("PostSimulationPhase");
    world.system<Position const>()
        .kind(post_simulation)
        .with<Item const>()
        .run([](flecs::iter& it) {
            auto& tile_items = it.world().get_mut<TileItemsCache>();

            // Clear all tile item caches
            tile_items.clear();

            // Create new caches
            while (it.next()) {
                auto position = it.field<Position const>(0);
                for (auto i : it) {
                    auto entity = it.entity(i);
                    tile_items.add(position[i].x, position[i].y, entity);
                }
            }

            // Sort and compact items
            tile_items.sort_and_compact(it.world());
        });

    world.system<Map>("Draw Map")
        .kind(flecs::OnStore)
        .each([](flecs::iter& it, size_t, Map const& map) {
            map.render(it.delta_time());
        });
}
