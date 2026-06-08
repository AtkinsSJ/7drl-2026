/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Gfx/Colour.h>
#include <Gfx/Sprite.h>
#include <Util/Basic.h>

struct Position {
    s32 x;
    s32 y;
};

struct HasSprite {
    SpriteRef ref;
    Colour colour = Colour::white();
};

enum class DrawLayer : u8 {
    Item,
    Player,
};

struct Name {
    String name;
};
