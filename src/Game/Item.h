/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Forward.h>
#include <Gfx/Sprite.h>

class Item {
public:
    explicit Item(ItemType);

    ItemType type() const { return m_type; }
    Sprite& sprite() const { return m_sprite.get(); }

private:
    ItemType m_type;
    SpriteRef m_sprite;
};
