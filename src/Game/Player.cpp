/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Player.h"
#include <Gfx/Renderer.h>
#include <Input/Input.h>

bool Player::try_act_from_user_input()
{
    if (keyJustPressed(SDLK_KP_1))
        return try_move(Direction::SW);
    if (keyJustPressed(SDLK_KP_2))
        return try_move(Direction::S);
    if (keyJustPressed(SDLK_KP_3))
        return try_move(Direction::SE);
    if (keyJustPressed(SDLK_KP_4))
        return try_move(Direction::W);
    if (keyJustPressed(SDLK_KP_6))
        return try_move(Direction::E);
    if (keyJustPressed(SDLK_KP_7))
        return try_move(Direction::NW);
    if (keyJustPressed(SDLK_KP_8))
        return try_move(Direction::N);
    if (keyJustPressed(SDLK_KP_9))
        return try_move(Direction::NE);
    return false;
}

void Player::update()
{
}

void Player::render(float delta_time)
{
    auto& renderer = the_renderer();
    drawSingleSprite(&renderer.world_buffer(), &m_sprite.get(), { x(), y(), 1, 1 }, renderer.shaderIds.pixelArt, Colour::white());
}
