/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Actor.h>
#include <Game/Terrain.h>
#include <Gfx/Sprite.h>

class Tile {
public:
    Tile() = default;
    explicit Tile(Terrain terrain)
    {
        set_terrain(terrain);
    }

    void set_terrain(Terrain);
    Terrain terrain() const { return m_terrain; }
    Sprite& terrain_sprite() const;

    Actor* actor() const { return m_actor; }
    void set_actor(Actor* actor)
    {
        ASSERT((actor == nullptr) != (m_actor == nullptr));
        m_actor = actor;
    }

private:
    Terrain m_terrain;
    SpriteRef m_terrain_sprite;
    Actor* m_actor;
};
