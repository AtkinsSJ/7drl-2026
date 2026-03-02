/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Game.h"
#include "AppState.h"
#include <Debug/Debug.h>
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
            m_map.tile_at(x, y).set_terrain(random->random_below(100) < 20 ? Terrain::Water : Terrain::Grass);
        }
    }

    the_renderer().world_camera().set_position(v2(width * 0.5f, height * 0.5f));
}

AppStatus Game::update_and_render(float delta_time)
{
    DEBUG_FUNCTION_T(DebugCodeDataTag::GameUpdate);

    // TODO: Take input
    // TODO: Update game state

    // Render
    m_map.render();

    return AppStatus::Game;
}
