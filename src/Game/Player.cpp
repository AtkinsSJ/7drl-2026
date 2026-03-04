/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Player.h"
#include <Game/GUI.h>
#include <Gfx/Renderer.h>
#include <Input/Input.h>

void Player::try_act_from_user_input()
{
    // Opening windows is always allowed. Other keybinds are inactive while an input-consuming window is open.
    // eg, the "pick up items" window, which has keys for selecting the item, which collide with movement keys.
    if (keyJustPressed(SDLK_i))
        GUI::toggle_inventory();
    else if (keyJustPressed(SDLK_h))
        GUI::toggle_help();
    else if (GUI::any_input_consuming_windows_are_open())
        return;

    if (keyJustPressed(SDLK_p))
        GUI::show_pick_up_window();
    else if (keyJustPressed(SDLK_KP_1))
        m_has_acted = try_move(Direction::SW);
    else if (keyJustPressed(SDLK_KP_2))
        m_has_acted = try_move(Direction::S);
    else if (keyJustPressed(SDLK_KP_3))
        m_has_acted = try_move(Direction::SE);
    else if (keyJustPressed(SDLK_KP_4))
        m_has_acted = try_move(Direction::W);
    else if (keyJustPressed(SDLK_KP_6))
        m_has_acted = try_move(Direction::E);
    else if (keyJustPressed(SDLK_KP_7))
        m_has_acted = try_move(Direction::NW);
    else if (keyJustPressed(SDLK_KP_8))
        m_has_acted = try_move(Direction::N);
    else if (keyJustPressed(SDLK_KP_9))
        m_has_acted = try_move(Direction::NE);
}

void Player::update()
{
}

void Player::render(float delta_time)
{
    auto& renderer = the_renderer();
    drawSingleSprite(&renderer.world_buffer(), &m_sprite.get(), { x(), y(), 1, 1 }, renderer.shaderIds.pixelArt, Colour::white());
}
