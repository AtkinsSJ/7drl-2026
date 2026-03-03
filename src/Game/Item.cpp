/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Item.h"
#include "AppState.h"
#include <Game/ItemCatalogue.h>

Item::Item(ItemType type)
    : m_type(type)
{
    auto& def = ItemCatalogue::the().find(type);
    m_sprite = { def.sprite_name, AppState::the().cosmeticRandom->next() };
}
