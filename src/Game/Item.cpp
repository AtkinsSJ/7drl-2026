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

static u32 merge_into_existing_inventory(flecs::world& world, ItemType item_type, u32 initial_quantity, flecs::entity new_owner)
{
    auto quantity_remaining = initial_quantity;
    auto& item_def = ItemCatalogue::the().find(item_type);

    // Merge with existing stacks if possible
    if (item_def.stack_size > 1) {
        auto inventory = world.query_builder<Item const, Quantity>()
                             .with(world.component<InInventory>(), new_owner)
                             .build();

        inventory.run([&](flecs::iter& it) {
            while (it.next()) {
                auto item = it.field<Item const>(0);
                auto quantity = it.field<Quantity>(1);

                // Inner loop
                for (auto i : it) {
                    if (item[i].type != item_type)
                        continue;

                    if (quantity[i].quantity < item_def.stack_size) {
                        auto amount_to_add = min(item_def.stack_size - quantity[i].quantity, quantity_remaining);
                        it.entity(i).set(Quantity { quantity[i].quantity + amount_to_add });
                        quantity_remaining -= amount_to_add;

                        if (quantity_remaining == 0)
                            break;
                    }
                }
                if (quantity_remaining == 0) {
                    it.fini();
                    break;
                }
            }
        });
    }

    return quantity_remaining;
}

void give_item_to_entity(flecs::world& world, ItemType item_type, u32 quantity, flecs::entity new_owner)
{
    auto quantity_remaining = merge_into_existing_inventory(world, item_type, quantity, new_owner);
    if (quantity_remaining) {
        ItemCatalogue::the()
            .instantiate(world, item_type)
            .set(Quantity { quantity_remaining })
            .add<InInventory>(new_owner);
    }
}

void give_item_to_entity(flecs::world& world, flecs::entity source_item, flecs::entity new_owner)
{
    auto item_type = source_item.get<Item>().type;
    auto initial_quantity = item_quantity(source_item);
    auto quantity_remaining = merge_into_existing_inventory(world, item_type, initial_quantity, new_owner);

    // Finally, if we still have an item, move it into the inventory
    if (quantity_remaining) {
        source_item
            .set(Quantity { quantity_remaining })
            .remove<Position>()
            .remove<InInventory>()
            .add<InInventory>(new_owner);
    } else {
        source_item.destruct();
    }
}
