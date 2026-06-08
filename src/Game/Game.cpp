/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Game.h"
#include "AppState.h"
#include <Debug/Debug.h>
#include <Game/Components.h>
#include <Game/GUI.h>
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

    m_simulation_phase = m_world.entity().add(flecs::Phase).depends_on(flecs::OnUpdate);
    auto post_simulation_phase = m_world.entity().add(flecs::Phase).depends_on(m_simulation_phase);

    // Take player input
    m_world.system("PlayerInput")
        .with<Player>()
        .kind(flecs::PreUpdate)
        .immediate()
        .each([&](flecs::entity player) {
            m_simulation_phase.disable();

            bool has_acted = false;

            // Opening windows is always allowed. Other keybinds are inactive while an input-consuming window is open.
            // eg, the "pick up items" window, which has keys for selecting the item, which collide with movement keys.
            if (keyJustPressed(SDLK_i)) {
                GUI::toggle_inventory();
            } else if (keyJustPressed(SDLK_h)) {
                GUI::toggle_help();
            } else if (GUI::any_input_consuming_windows_are_open()) {
                return;
            }

            if (keyJustPressed(SDLK_m)) {
                GUI::toggle_pause_menu();
            } else if (keyJustPressed(SDLK_p)) {
                GUI::show_pick_up_window();
            } else if (keyJustPressed(SDLK_d)) {
                GUI::show_drop_window();
            } else if (keyJustPressed(SDLK_k)) {
                // If we already have an in-progress knapping item, continue that instead of listing options.
                auto& recipe_catalogue = RecipeCatalogue::the();
                auto active_crafts = m_world.query_builder<ActiveCraftingRecipe const>()
                                         .with<Item>()
                                         .with(m_world.component<InInventory>(), player)
                                         .build();
                auto in_progress_knapping = active_crafts.find([&recipe_catalogue](ActiveCraftingRecipe const& active_crafting_recipe) {
                    return recipe_catalogue.find(active_crafting_recipe.id).method == RecipeMethod::Knapping;
                });
                if (in_progress_knapping.is_valid()) {
                    GUI::show_knapping_window(in_progress_knapping.get<ActiveCraftingRecipe>().id, false);
                } else {
                    GUI::show_recipe_selection_window(RecipeMethod::Knapping);
                }
            } else if (keyJustPressed(SDLK_KP_1)) {
                has_acted = try_move_player(m_world, player, Direction::SW);
            } else if (keyJustPressed(SDLK_KP_2)) {
                has_acted = try_move_player(m_world, player, Direction::S);
            } else if (keyJustPressed(SDLK_KP_3)) {
                has_acted = try_move_player(m_world, player, Direction::SE);
            } else if (keyJustPressed(SDLK_KP_4)) {
                has_acted = try_move_player(m_world, player, Direction::W);
            } else if (keyJustPressed(SDLK_KP_6)) {
                has_acted = try_move_player(m_world, player, Direction::E);
            } else if (keyJustPressed(SDLK_KP_7)) {
                has_acted = try_move_player(m_world, player, Direction::NW);
            } else if (keyJustPressed(SDLK_KP_8)) {
                has_acted = try_move_player(m_world, player, Direction::N);
            } else if (keyJustPressed(SDLK_KP_9)) {
                has_acted = try_move_player(m_world, player, Direction::NE);
            }

            if (has_acted)
                m_simulation_phase.enable();
        });

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
