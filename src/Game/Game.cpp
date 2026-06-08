/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Game.h"
#include "AppState.h"
#include <Debug/Debug.h>
#include <Game/Components.h>
#include <Game/Item.h>
#include <Game/ItemCatalogue.h>
#include <Game/Map.h>
#include <Game/Player.h>
#include <Game/RecipeCatalogue.h>
#include <Gfx/Renderer.h>
#include <Input/Input.h>
#include <UI/Toast.h>

NonnullOwnPtr<Game> Game::create()
{
    UI::Toast::show("Press 'h' to show help"_sv);
    return adopt_own(*new Game(128, 128));
}

Game::Game(u32 width, u32 height)
    : m_arena("Game"_s)
{
    auto random = adopt_own(*Random::create());
    m_world.component<Map>().add(flecs::Singleton);
    Map::generate(m_world, width, height, *random, m_arena);

    m_world.component<TileItemsCache>().add(flecs::Singleton);
    m_world.emplace<TileItemsCache>(m_arena, width, height);

    the_renderer().world_camera().set_zoom(2);

    m_simulation_phase = m_world.entity("SimulationPhase").add(flecs::Phase).depends_on(flecs::OnUpdate);
    auto post_simulation_phase = m_world.entity().add(flecs::Phase).depends_on(m_simulation_phase);

    m_world.import<mod_player>();

    // Take a turn
    m_world.system()
        .kind(m_simulation_phase)
        .run([](flecs::iter& it) {
            logInfo("Taking a turn"_s);
        });

    m_world.system<Position const>()
        .kind(post_simulation_phase)
        .with<Item const>()
        .run([](flecs::iter& it) {
            logInfo("TileItemsCache"_s);
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

    // Centre the camera on the player
    // TODO: Use a special tag for this so we can follow something else?
    m_world.system<Position const, Player const, Map const>("Move Camera")
        .kind(flecs::PreStore)
        .each([](Position const& position, Player const&, Map const& map) {
            auto& world_camera = the_renderer().world_camera();
            world_camera.set_position(v2(position.x + 0.5f, position.y + 0.5f));
            world_camera.snap_to_rectangle({ 0, 0, static_cast<s32>(map.width()), static_cast<s32>(map.height()) });
        });

    // Draw
    m_world.system<Map>("Draw Map")
        .kind(flecs::OnStore)
        .each([](flecs::iter& it, size_t, Map const& map) {
            map.render(it.delta_time());
        });
    m_world.system<Position const, HasSprite const>("Draw Entities")
        .kind(flecs::OnStore)
        .with<DrawLayer const>()
        .order_by<DrawLayer>([](flecs::entity_t, DrawLayer const* d1, flecs::entity_t, DrawLayer const* d2) {
            return to_underlying(*d1) - to_underlying(*d2);
        })
        .each([](Position const& position, HasSprite const& sprite) {
            auto& renderer = the_renderer();
            drawSingleSprite(&renderer.world_buffer(), &sprite.ref.get(), { position.x, position.y, 1, 1 },
                renderer.shaderIds.pixelArt, sprite.colour);
        });
}

AppStatus Game::update_and_render(float delta_time)
{
    DEBUG_FUNCTION_T(DebugCodeDataTag::GameUpdate);

    if (!m_world.progress(delta_time))
        return AppStatus::Quit;

    return AppStatus::Game;
}

flecs::entity Game::player() const
{
    return m_world.query<Player>().first();
}
