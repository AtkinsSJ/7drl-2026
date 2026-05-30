/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Player.h"
#include <Game/Components.h>
#include <Game/Map.h>
#include <Input/Input.h>

bool try_move_player(flecs::world world, flecs::entity player, Direction direction)
{
    auto& map = world.get<Map>();
    auto& position = player.get<Position>();

    auto try_move_to = [&](s32 new_x, s32 new_y) {
        if (new_x < 0 || new_x >= map.width() || new_y < 0 || new_y >= map.height())
            return false;

        // FIXME: Reimplement cache for entities on a tile
        // auto& old_tile = map.tile_at(position.x, position.y);
        // auto& new_tile = map.tile_at(new_x, new_y);
        //
        // // TODO: Checks if we can actually move into the tile
        // if (new_tile.actor())
        //    return false;
        //
        // // Move!
        // old_tile.set_actor(nullptr);
        // new_tile.set_actor(this);
        player.set<Position>({ new_x, new_y });
        return true;
    };

    switch (direction) {
    case Direction::N:
        return try_move_to(position.x, position.y - 1);
    case Direction::NE:
        return try_move_to(position.x + 1, position.y - 1);
    case Direction::E:
        return try_move_to(position.x + 1, position.y);
    case Direction::SE:
        return try_move_to(position.x + 1, position.y + 1);
    case Direction::S:
        return try_move_to(position.x, position.y + 1);
    case Direction::SW:
        return try_move_to(position.x - 1, position.y + 1);
    case Direction::W:
        return try_move_to(position.x - 1, position.y);
    case Direction::NW:
        return try_move_to(position.x - 1, position.y - 1);
    }

    VERIFY_NOT_REACHED();
}
