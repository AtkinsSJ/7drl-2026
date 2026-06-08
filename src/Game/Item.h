/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Forward.h>
#include <Util/Variant.h>
#include <flecs.h>

struct Item {
    ItemType type;
};
struct Quantity {
    u32 quantity;
};

struct HasInventory { };
struct InInventory { };

struct ActiveCraftingRecipe {
    RecipeID id;
};
using ItemData = Variant<Empty, ActiveCraftingRecipe>;

u32 item_quantity(flecs::entity item);
String describe_item(flecs::entity item);

void give_item_to_entity(flecs::world&, ItemType, u32 quantity, flecs::entity owner);
void give_item_to_entity(flecs::world&, flecs::entity item, flecs::entity new_owner);
