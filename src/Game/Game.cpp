/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Game.h"
#include "AppState.h"
#include <Debug/Debug.h>
#include <Game/GUI.h>
#include <Game/Item.h>
#include <Game/Player.h>
#include <Gfx/Renderer.h>
#include <Input/Input.h>

NonnullOwnPtr<Game> Game::create()
{
    return adopt_own(*new Game(128, 128));
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
}

AppStatus Game::update_and_render(float delta_time)
{
    DEBUG_FUNCTION_T(DebugCodeDataTag::GameUpdate);

    // UI!
    if (!isInputCaptured()) {
        if (keyJustPressed(SDLK_i)) {
            GUI::toggle_inventory();
        }

        // Try and update the player
        // FIXME: Don't do this if there's a menu open or something. Figure that out.
        if (m_player) {
            auto player_moved = m_player->try_act_from_user_input();

            // If they did something, update everyone else
            if (player_moved)
                m_map->update();

            the_renderer().world_camera().set_position(v2(m_player->x() + 0.5f, m_player->y() + 0.5f));
            the_renderer().world_camera().snap_to_rectangle({ 0, 0, static_cast<s32>(m_map->width()), static_cast<s32>(m_map->height()) });
        }
    }

    // Render
    m_map->render(delta_time);

    return AppStatus::Game;
}
