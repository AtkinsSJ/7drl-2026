/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Tile.h"
#include "AppState.h"

void Tile::set_terrain(Terrain terrain)
{
    m_terrain = terrain;
    m_terrain_sprite = SpriteRef { get_terrain_def(terrain).sprite_name, AppState::the().cosmeticRandom->next() };
}

Sprite& Tile::terrain_sprite() const
{
    return m_terrain_sprite.get();
}
