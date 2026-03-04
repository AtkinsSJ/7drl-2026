/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Tile.h"
#include "AppState.h"
#include <Game/Item.h>
#include <Game/ItemCatalogue.h>

Tile::Tile(Terrain terrain, ArrayChunkPool<NonnullOwnPtr<Item>>& item_chunk_pool)
    : m_terrain(terrain)
    , m_terrain_sprite(get_terrain_def(terrain).sprite_name, AppState::the().cosmeticRandom->next())
    , m_actor(nullptr)
{
    initChunkedArray(&m_items, &item_chunk_pool);
}

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

Item& Tile::add_item(ItemType item_type)
{
    auto& def = ItemCatalogue::the().find(item_type);
    if (def.stack_size > 1 && !m_items.is_empty()) {
        // For stackable items, try to combine it with an existing stack.
        auto existing_item = m_items.find_first([&](auto& item) {
            return item->type() == item_type && item->quantity() < def.stack_size;
        });
        if (existing_item.has_value()) {
            existing_item.value().value()->increase_quantity(1);
            return *existing_item.value().value();
        }
    }

    return **m_items.append(adopt_own(*new Item(item_type)));
}

void Tile::add_item(NonnullOwnPtr<Item> item)
{
    // FIXME: Copy-paste from Actor::give_item! Make some kind of class for this.
    // Try to add it to existing item stacks
    for (auto it = m_items.iterate(); it.hasNext(); it.next()) {
        auto& existing_item = *it.get();
        auto leftover = existing_item.try_add_to_stack(move(item));
        if (leftover == nullptr)
            return;
        item = leftover.release_nonnull();
    }
    // Append any remainder
    m_items.append(move(item));
}
