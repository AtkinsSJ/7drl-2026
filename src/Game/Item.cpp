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
    if (auto* quantity = item.try_get<Quantity>())
        return quantity->quantity;
    return 1;
}

String describe_item(flecs::entity item)
{
    auto& name = item.get<Name>().name;
    if (auto* quantity = item.try_get<Quantity>())
        return myprintf("{} x {}"_s, { name, formatInt(quantity->quantity) });
    return name;
}
