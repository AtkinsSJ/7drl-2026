/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Actor.h>
#include <Gfx/Sprite.h>

class Player final : public Actor {
public:
    Player(u32 x, u32 y)
        : Actor(x, y)
        , m_sprite("player"_sv, 0)
    {
    }

    bool try_act_from_user_input();

    void update() override;
    void render(float delta_time) override;

private:
    SpriteRef m_sprite;
};
