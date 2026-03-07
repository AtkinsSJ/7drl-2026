/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Player.h"

#include "Item.h"

#include <Game/GUI.h>
#include <Game/RecipeCatalogue.h>
#include <Gfx/Renderer.h>
#include <Input/Input.h>

void Player::try_act_from_user_input()
{
    // Opening windows is always allowed. Other keybinds are inactive while an input-consuming window is open.
    // eg, the "pick up items" window, which has keys for selecting the item, which collide with movement keys.
    if (keyJustPressed(SDLK_i))
        GUI::toggle_inventory();
    else if (keyJustPressed(SDLK_h))
        GUI::toggle_help();
    else if (GUI::any_input_consuming_windows_are_open())
        return;

    if (keyJustPressed(SDLK_p)) {
        GUI::show_pick_up_window();
    } else if (keyJustPressed(SDLK_d)) {
        GUI::show_drop_window();
    } else if (keyJustPressed(SDLK_k)) {
        // If we already have an in-progress knapping item, continue that instead of listing options.
        auto& recipe_catalogue = RecipeCatalogue::the();
        auto existing_knapping_item = inventory().find_first([&](auto& item) {
            if (auto* recipe_data = item->data().template try_get<ActiveCraftingRecipe>())
                return recipe_catalogue.find(recipe_data->id).method == RecipeMethod::Knapping;
            return false;
        });
        if (existing_knapping_item.has_value()) {
            GUI::show_knapping_window(existing_knapping_item.value().value()->data().get<ActiveCraftingRecipe>().id);
        } else {
            GUI::show_recipe_selection_window(RecipeMethod::Knapping);
        }
    } else if (keyJustPressed(SDLK_KP_1)) {
        m_has_acted = try_move(Direction::SW);
    } else if (keyJustPressed(SDLK_KP_2)) {
        m_has_acted = try_move(Direction::S);
    } else if (keyJustPressed(SDLK_KP_3)) {
        m_has_acted = try_move(Direction::SE);
    } else if (keyJustPressed(SDLK_KP_4)) {
        m_has_acted = try_move(Direction::W);
    } else if (keyJustPressed(SDLK_KP_6)) {
        m_has_acted = try_move(Direction::E);
    } else if (keyJustPressed(SDLK_KP_7)) {
        m_has_acted = try_move(Direction::NW);
    } else if (keyJustPressed(SDLK_KP_8)) {
        m_has_acted = try_move(Direction::N);
    } else if (keyJustPressed(SDLK_KP_9)) {
        m_has_acted = try_move(Direction::NE);
    }
}

void Player::update()
{
}

void Player::render(float delta_time)
{
    auto& renderer = the_renderer();
    drawSingleSprite(&renderer.world_buffer(), &m_sprite.get(), { x(), y(), 1, 1 }, renderer.shaderIds.pixelArt, Colour::white());
}
