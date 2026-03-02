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
    return adopt_own(*new Game(1024, 1024));
}

Game::Game(u32 width, u32 height)
    : m_arena("Game"_s)
    , m_map(width, height, m_arena)
{
    auto random = adopt_own(*Random::create());

    // Generate the most amazing map you've ever experienced.
    for (auto y = 0; y < height; ++y) {
        for (auto x = 0; x < width; ++x) {
            auto number = random->random_below(100);
            auto terrain = [number] {
                if (number < 20)
                    return Terrain::Water;
                if (number < 30)
                    return Terrain::Sand;
                return Terrain::Grass;
            }();
            m_map.tile_at(x, y).set_terrain(terrain);
        }
    }

    // Pop a player somewhere
    auto player = adopt_own(*new Player(width / 2, height / 2));
    m_player = player.ptr();
    m_map.add_actor(move(player));
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
            m_map.update();

        the_renderer().world_camera().set_position(v2(m_player->x(), m_player->y()));
    }

    // Render
    m_map.render(delta_time);

    return AppStatus::Game;
}
