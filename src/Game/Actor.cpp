/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Actor.h"
#include <Game/Map.h>

Actor::Actor(s32 x, s32 y)
    : m_x(x)
    , m_y(y)
{
}

void Actor::set_map(Map* map)
{
    if (m_map) {
        // FIXME: Remove from old map
        VERIFY_NOT_REACHED();
    }

    m_map = map;
    if (map)
        map->tile_at(x(), y()).set_actor(this);
}

bool Actor::try_move(Direction direction)
{
    ASSERT(m_map);

    auto try_move_to = [&](s32 new_x, s32 new_y) {
        if (new_x < 0 || new_x >= m_map->width() || new_y < 0 || new_y >= m_map->height())
            return false;

        auto& old_tile = m_map->tile_at(x(), y());
        auto& new_tile = m_map->tile_at(new_x, new_y);

        // TODO: Checks if we can actually move into the tile
        if (new_tile.actor())
            return false;

        // Move!
        old_tile.set_actor(nullptr);
        new_tile.set_actor(this);
        m_x = new_x;
        m_y = new_y;
        return true;
    };

    switch (direction) {
    case Direction::N:
        return try_move_to(x(), y() - 1);
    case Direction::NE:
        return try_move_to(x() + 1, y() - 1);
    case Direction::E:
        return try_move_to(x() + 1, y());
    case Direction::SE:
        return try_move_to(x() + 1, y() + 1);
    case Direction::S:
        return try_move_to(x(), y() + 1);
    case Direction::SW:
        return try_move_to(x() - 1, y() + 1);
    case Direction::W:
        return try_move_to(x() - 1, y());
    case Direction::NW:
        return try_move_to(x() - 1, y() - 1);
    }

    VERIFY_NOT_REACHED();
}
