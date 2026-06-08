/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Item.h"
#include <Game/Components.h>
#include <Game/ItemCatalogue.h>

u32 item_quantity(flecs::entity item)
{
    return item.get<Quantity>().quantity;
}

String describe_item(flecs::entity item)
{
    auto& name = item.get<Name>().name;
    if (auto& [quantity] = item.get<Quantity>(); quantity > 1)
        return myprintf("{} x {}"_s, { name, formatInt(quantity) });
    return name;
}
