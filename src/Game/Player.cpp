/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Player.h"
#include <Game/Components.h>
#include <Game/GUI.h>
#include <Game/Item.h>
#include <Game/Map.h>
#include <Game/RecipeCatalogue.h>
#include <Input/Input.h>

flecs::entity create_player(flecs::world& world, s32 x, s32 y)
{
    return world.entity("Player")
        .set(Position { .x = x, .y = y })
        .add<Player>()
        .add<HasInventory>()
        .add<BlocksMovement>()
        .set<Name>({ "player"_s })
        .set(HasSprite { .ref = { "player"_sv, 0 } })
        .set(DrawLayer::Player);
}

bool try_move_player(flecs::world& world, flecs::entity player, Direction direction)
{
    auto& map = world.get<Map>();
    auto& position = player.get<Position>();

    auto try_move_to = [&](s32 new_x, s32 new_y) {
        if (new_x < 0 || new_x >= map.width() || new_y < 0 || new_y >= map.height())
            return false;

        // TODO: Cache query!
        auto query = world.query_builder<Position const>()
                         .with<BlocksMovement>()
                         .build();
        auto entity_in_the_way = query.find([new_x, new_y](auto const& position) {
            return position.x == new_x && position.y == new_y;
        });
        if (entity_in_the_way) {
            logDebug("There is something in the way!"_s);
            return false;
        }
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

mod_player::mod_player(flecs::world& world)
{
    world.component<Player>();

    world.system("PlayerInput")
        .with<Player>()
        .kind(flecs::PreUpdate)
        .immediate()
        .each([&world](flecs::entity player) {
            auto simulation_phase = world.lookup("SimulationPhase");
            simulation_phase.disable();

            bool has_acted = false;

            // Opening windows is always allowed. Other keybinds are inactive while an input-consuming window is open.
            // eg, the "pick up items" window, which has keys for selecting the item, which collide with movement keys.
            if (keyJustPressed(SDLK_i)) {
                GUI::toggle_inventory();
            } else if (keyJustPressed(SDLK_h)) {
                GUI::toggle_help();
            } else if (GUI::any_input_consuming_windows_are_open()) {
                return;
            }

            if (keyJustPressed(SDLK_m)) {
                GUI::toggle_pause_menu();
            } else if (keyJustPressed(SDLK_p)) {
                GUI::show_pick_up_window();
            } else if (keyJustPressed(SDLK_d)) {
                GUI::show_drop_window();
            } else if (keyJustPressed(SDLK_k)) {
                // If we already have an in-progress knapping item, continue that instead of listing options.
                auto& recipe_catalogue = RecipeCatalogue::the();
                auto active_crafts = world.query_builder<ActiveCraftingRecipe const>()
                                         .with<Item>()
                                         .with(world.component<InInventory>(), player)
                                         .build();
                auto in_progress_knapping = active_crafts.find([&recipe_catalogue](ActiveCraftingRecipe const& active_crafting_recipe) {
                    return recipe_catalogue.find(active_crafting_recipe.id).method == RecipeMethod::Knapping;
                });
                if (in_progress_knapping.is_valid()) {
                    GUI::show_knapping_window(in_progress_knapping.get<ActiveCraftingRecipe>().id, false);
                } else {
                    GUI::show_recipe_selection_window(RecipeMethod::Knapping);
                }
            } else if (keyJustPressed(SDLK_KP_1)) {
                has_acted = try_move_player(world, player, Direction::SW);
            } else if (keyJustPressed(SDLK_KP_2)) {
                has_acted = try_move_player(world, player, Direction::S);
            } else if (keyJustPressed(SDLK_KP_3)) {
                has_acted = try_move_player(world, player, Direction::SE);
            } else if (keyJustPressed(SDLK_KP_4)) {
                has_acted = try_move_player(world, player, Direction::W);
            } else if (keyJustPressed(SDLK_KP_6)) {
                has_acted = try_move_player(world, player, Direction::E);
            } else if (keyJustPressed(SDLK_KP_7)) {
                has_acted = try_move_player(world, player, Direction::NW);
            } else if (keyJustPressed(SDLK_KP_8)) {
                has_acted = try_move_player(world, player, Direction::N);
            } else if (keyJustPressed(SDLK_KP_9)) {
                has_acted = try_move_player(world, player, Direction::NE);
            }

            if (has_acted)
                simulation_phase.enable();
        });
}
