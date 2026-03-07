/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Forward.h>
#include <Gfx/Sprite.h>

struct ActiveCraftingRecipe {
    RecipeID id;
};
using ItemData = Variant<Empty, ActiveCraftingRecipe>;

class Item {
public:
    explicit Item(ItemType, u32 quantity = 1);

    ItemType type() const { return m_type; }
    Sprite& sprite() const { return m_sprite.get(); }
    u32 quantity() const { return m_quantity; }
    void increase_quantity(u32 amount);
    void decrease_quantity(u32 amount);

    String name() const;
    String describe() const;

    ItemData const& data() const { return m_data; }
    ItemData& data() { return m_data; }
    void set_data(ItemData&& data) { m_data = move(data); }

    // Try to combine source into this item. If it can't be done, or won't all fit, returns the leftover item.
    OwnPtr<Item> try_add_to_stack(NonnullOwnPtr<Item> source);

private:
    ItemType m_type;
    SpriteRef m_sprite;
    u32 m_quantity { 1 };
    ItemData m_data {};
};
