/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Game.h"
#include "AppState.h"
#include <Debug/Debug.h>
#include <Game/Player.h>
#include <Gfx/Renderer.h>

NonnullOwnPtr<Game> Game::create()
{
    return adopt_own(*new Game(128, 128));
}

Game::Game(u32 width, u32 height)
    : m_arena("Game"_s)
{
    auto random = adopt_own(*Random::create());
    m_map = Map::generate_surface(width, height, *random, m_arena);

    // Pop a player somewhere
    auto player = adopt_own(*new Player(width / 2, height / 2));
    m_player = player.ptr();
    m_map->add_actor(move(player));
}

AppStatus Game::update_and_render(float delta_time)
{
    DEBUG_FUNCTION_T(DebugCodeDataTag::GameUpdate);

    // Try and update the player
    // FIXME: Don't do this if there's a menu open or something. Figure that out.
    if (m_player) {
        auto player_moved = m_player->try_act_from_user_input();

        // If they did something, update everyone else
        if (player_moved)
            m_map->update();

        the_renderer().world_camera().set_position(v2(m_player->x(), m_player->y()));
    }

    // Render
    m_map->render(delta_time);

    return AppStatus::Game;
}
