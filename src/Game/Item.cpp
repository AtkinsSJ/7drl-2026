/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Item.h"
#include "AppState.h"
#include <Game/ItemCatalogue.h>

Item::Item(ItemType type, u32 quantity)
    : m_type(type)
    , m_quantity(quantity)
{
    auto& def = ItemCatalogue::the().find(type);
    m_sprite = { def.sprite_name, AppState::the().cosmeticRandom->next() };
}

void Item::increase_quantity(u32 amount)
{
    m_quantity += amount;
}

void Item::decrease_quantity(u32 amount)
{
    ASSERT(m_quantity > amount);
    m_quantity -= amount;
}

OwnPtr<Item> Item::try_add_to_stack(NonnullOwnPtr<Item> source)
{
    // Different items can't stack together.
    if (m_type != source->m_type)
        return source;

    auto& def = ItemCatalogue::the().find(m_type);

    // If the stack is full, just do nothing.
    if (m_quantity >= def.stack_size)
        return source;

    // If it fits in one stack, consume the source.
    if (m_quantity + source->m_quantity <= def.stack_size) {
        m_quantity += source->m_quantity;
        return nullptr;
    }

    // Otherwise, move as much as possible into this item.
    auto remainder = (m_quantity + source->m_quantity) - def.stack_size;
    m_quantity = def.stack_size;
    source->m_quantity = remainder;
    return source;
}

String Item::name() const
{
    return ItemCatalogue::the().find(m_type).name;
}

String Item::describe() const
{
    return myprintf("{} x {}"_s, { name(), formatInt(m_quantity) });
}
