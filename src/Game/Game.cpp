/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Game.h"
#include "AppState.h"
#include <Debug/Debug.h>
#include <Game/GUI.h>
#include <Game/ItemCatalogue.h>
#include <Game/Player.h>
#include <Gfx/Renderer.h>
#include <Input/Input.h>
#include <UI/Toast.h>

NonnullOwnPtr<Game> Game::create()
{
    UI::Toast::show("Press 'h' to show help"_sv);
    return adopt_own(*new Game(128, 128));
}

static void focus_camera(Map& map, s32 x, s32 y)
{
    the_renderer().world_camera().set_position(v2(x + 0.5f, y + 0.5f));
    the_renderer().world_camera().snap_to_rectangle({ 0, 0, static_cast<s32>(map.width()), static_cast<s32>(map.height()) });
}

Game::Game(u32 width, u32 height)
    : m_arena("Game"_s)
{
    // 32 chosen because we're unlikely to have many items in a single tile, but want to avoid chunking too.
    initChunkPool(&m_item_chunk_pool, &m_arena, 32);

    auto random = adopt_own(*Random::create());
    m_map = Map::generate_surface(width, height, *random, m_arena, m_item_chunk_pool);

    // Pop a player somewhere
    auto player = adopt_own(*new Player(width / 2, height / 2, m_item_chunk_pool));
    m_player = player.ptr();
    m_map->add_actor(move(player));
    focus_camera(*m_map, m_player->x(), m_player->y());

    // DEBUG: Dump a bunch of items on the player
    auto& start_tile = m_map->tile_at(m_player->x(), m_player->y());
    auto& item_catalogue = ItemCatalogue::the();
    auto stick_type = item_catalogue.find_name("stick"_s).value();
    auto rock_type = item_catalogue.find_name("rock"_s).value();
    for (auto i = 0; i < 5; ++i)
        start_tile.add_item(stick_type);
    for (auto i = 0; i < 7; ++i)
        start_tile.add_item(rock_type);
}

AppStatus Game::update_and_render(float delta_time)
{
    DEBUG_FUNCTION_T(DebugCodeDataTag::GameUpdate);

    // UI!
    if (!isInputCaptured()) {
        if (keyJustPressed(SDLK_i)) {
            GUI::toggle_inventory();
        }
        if (keyJustPressed(SDLK_h)) {
            GUI::toggle_help();
        }
        if (keyJustPressed(SDLK_p)) {
            GUI::show_pick_up_window();
        }

        // Try and update the player
        bool pass_time = false;
        if (m_player && !GUI::any_input_consuming_windows_are_open())
            pass_time = m_player->try_act_from_user_input();

        // If they did something, update everyone else
        if (pass_time) {
            m_map->update();
            focus_camera(*m_map, m_player->x(), m_player->y());
        }
    }

    // Render
    m_map->render(delta_time);

    return AppStatus::Game;
}
