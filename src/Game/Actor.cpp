/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Actor.h"
#include <Game/Item.h>
#include <Game/ItemCatalogue.h>
#include <Game/Map.h>

Actor::Actor(s32 x, s32 y, ArrayChunkPool<NonnullOwnPtr<Item>>& item_chunk_pool)
    : m_x(x)
    , m_y(y)
{
    initChunkedArray(&m_inventory, &item_chunk_pool);
}

void Actor::give_item(NonnullOwnPtr<Item> item)
{
    // Try to add it to existing item stacks
    for (auto it = m_inventory.iterate(); it.hasNext(); it.next()) {
        auto& existing_item = *it.get();
        auto leftover = existing_item.try_add_to_stack(move(item));
        if (leftover == nullptr)
            return;
        item = leftover.release_nonnull();
    }
    // Append any remainder
    m_inventory.append(move(item));
}

bool Actor::has_item(ItemType type, u32 quantity) const
{
    auto remaining_quantity = quantity;
    for (auto it = m_inventory.iterate(); it.hasNext(); it.next()) {
        auto& existing_item = *it.get();
        if (existing_item.type() == type) {
            remaining_quantity -= min(remaining_quantity, existing_item.quantity());
            if (remaining_quantity == 0)
                return true;
        }
    }
    return false;
}

void Actor::remove_item(ItemType type, u32 quantity)
{
    auto remaining_quantity = quantity;
    // We iterate backwards so that we remove from partial stacks first.
    while (remaining_quantity > 0) {
        auto maybe_item = m_inventory.find_last([type](auto& item) {
            return item->type() == type;
        });
        if (!maybe_item.has_value())
            break;
        auto& existing_item = *maybe_item.value().value();
        if (existing_item.quantity() <= remaining_quantity) {
            remaining_quantity -= existing_item.quantity();
            m_inventory.take_index(maybe_item.value().index());
        } else {
            existing_item.decrease_quantity(remaining_quantity);
            remaining_quantity = 0;
        }
    }
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
