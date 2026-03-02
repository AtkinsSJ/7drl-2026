/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Tile.h"
#include "AppState.h"

void Tile::set_terrain(Terrain terrain)
{
    if (m_terrain == terrain)
        return;
    m_terrain = terrain;
    m_terrain_sprite = SpriteRef { get_terrain_def(terrain).sprite_name, AppState::the().cosmeticRandom->next() };
}

void Tile::set_terrain_raw(Badge<Map>, Terrain terrain)
{
    m_terrain = terrain;
}

Sprite& Tile::terrain_sprite() const
{
    return m_terrain_sprite.get();
}

void Tile::fetch_sprite()
{
    m_terrain_sprite = SpriteRef { get_terrain_def(m_terrain).sprite_name, AppState::the().cosmeticRandom->next() };
}
