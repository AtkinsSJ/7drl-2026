/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "GUI.h"
#include <AppState.h>
#include <Game/Item.h>
#include <Game/ItemCatalogue.h>
#include <Game/RecipeCatalogue.h>
#include <Menus/About.h>
#include <UI/Toast.h>
#include <UI/Window.h>
#include <Util/BitArray.h>

namespace GUI {

// Ew, globals. But it's the easiest way to make persistence work in windows until I rework that.
static u32 s_selected_item_index = 0;
static u32 s_item_quantity = 1;
static void select_item_index(u32 index, ChunkedArray<NonnullOwnPtr<Item>>& items)
{
    s_selected_item_index = index;
    // Default to the whole item stack.
    s_item_quantity = items[index]->quantity();
}

static void pause_menu_window_proc(UI::WindowContext* context, void*)
{
    DEBUG_FUNCTION();
    UI::Panel* ui = &context->windowPanel;
    ui->alignWidgets(HAlign::Fill);

    if (ui->addTextButton(getText("button_resume"_s))) {
        context->closeRequested = true;
    }

    if (ui->addTextButton(getText("button_about"_s))) {
        showAboutWindow();
    }

    if (ui->addTextButton(getText("button_exit"_s))) {
        AppState::the().appStatus = AppStatus::MainMenu;
        UI::closeAllWindows();
    }
}

void toggle_pause_menu()
{
    if (UI::isWindowOpen(pause_menu_window_proc)) {
        UI::closeWindow(pause_menu_window_proc);
        return;
    }
    UI::showWindow(UI::WindowTitle::fromTextAsset("title_menu"_s), 300, 200, v2i(0, 0), "default"_s,
        WindowFlags::Unique | WindowFlags::Modal | WindowFlags::AutomaticHeight | WindowFlags::Pause,
        pause_menu_window_proc);
}

static void inventory_window_proc(UI::WindowContext* context, void*)
{
    auto& game = AppState::the().game;
    if (!game || !game->player()) {
        logWarn("Closing inventory window because no game or no player found."_s);
        UI::closeWindow(inventory_window_proc);
        return;
    }
    auto& player = *game->player();
    auto& inventory = player.inventory();
    UI::Panel& ui = context->windowPanel;

    if (inventory.is_empty()) {
        ui.startNewLine(HAlign::Left);
        ui.addLabel("No items"_s);
    } else {
        for (auto it = inventory.iterate(); it.hasNext(); it.next()) {
            auto& item = *it.get();
            ui.startNewLine(HAlign::Left);
            ui.addLabel(item.describe());
        }
    }
}

void toggle_inventory()
{
    if (UI::isWindowOpen(inventory_window_proc)) {
        UI::closeWindow(inventory_window_proc);
        return;
    }
    UI::showWindow(UI::WindowTitle::fromTextAsset("title_inventory"_s), 200, 200, {}, "default"_s, WindowFlags::AutomaticHeight | WindowFlags::UniqueKeepPosition, inventory_window_proc);
}

static void help_window_proc(UI::WindowContext* context, void*)
{
    UI::Panel& ui = context->windowPanel;
    ui.startNewLine(HAlign::Left);
    ui.addLabel("Controls"_s);
    ui.startNewLine(HAlign::Left);
    ui.addLabel("Numpad: Move"_s);
    ui.startNewLine(HAlign::Left);
    ui.addLabel("h: Help (this window)"_s);
    ui.startNewLine(HAlign::Left);
    ui.addLabel("i: Inventory"_s);
    ui.startNewLine(HAlign::Left);
    ui.addLabel("k: Knap stones"_s);
    ui.startNewLine(HAlign::Left);
    ui.addLabel("p: Pick up item"_s);
    ui.startNewLine(HAlign::Left);
    ui.addLabel("d: Drop item"_s);
}

void toggle_help()
{
    if (UI::isWindowOpen(help_window_proc)) {
        UI::closeWindow(help_window_proc);
        return;
    }
    UI::showWindow(UI::WindowTitle::fromTextAsset("title_help"_s), 200, 200, {}, "default"_s, WindowFlags::AutomaticHeight | WindowFlags::UniqueKeepPosition, help_window_proc);
}

static void pick_up_window_proc(UI::WindowContext* context, void*)
{
    auto& game = AppState::the().game;
    if (!game || !game->player() || !game->map()) {
        logWarn("Closing pick-up items window because no game/player/map found."_s);
        UI::closeWindow(pick_up_window_proc);
        return;
    }
    auto& player = *game->player();
    auto& map = *game->map();
    auto& tile = map.tile_at(player.x(), player.y());

    if (tile.items().is_empty()) {
        UI::closeWindow(pick_up_window_proc);
        return;
    }

    // We're treating this list of labels like a menu. [Up] and [Down] select the item, and [Enter] selects it.
    if (keyJustPressed(SDLK_ESCAPE)) {
        UI::closeWindow(pick_up_window_proc);
        return;
    }

    if ((keyJustPressed(SDLK_UP) || keyJustPressed(SDLK_KP_8)) && s_selected_item_index > 0)
        select_item_index(s_selected_item_index - 1, tile.items());
    if ((keyJustPressed(SDLK_DOWN) || keyJustPressed(SDLK_KP_2)) && s_selected_item_index < tile.items().count - 1)
        select_item_index(s_selected_item_index + 1, tile.items());
    if (keyJustPressed(SDLK_PLUS) || keyJustPressed(SDLK_KP_PLUS)) {
        // TODO: Shortcut to select all?
        s_item_quantity = min(s_item_quantity + 1, tile.items()[s_selected_item_index]->quantity());
    }
    if (keyJustPressed(SDLK_MINUS) || keyJustPressed(SDLK_KP_MINUS)) {
        // TODO: Shortcut to select 1?
        s_item_quantity = max(s_item_quantity - 1, 1);
    }
    if (keyJustPressed(SDLK_RETURN) || keyJustPressed(SDLK_KP_ENTER)) {
        // If we're taking the whole stack, transfer it directly.
        if (s_item_quantity == tile.items()[s_selected_item_index]->quantity()) {
            auto item = tile.items().take_index(s_selected_item_index, true);
            UI::Toast::show(getText("msg_picked_up_item"_s, { item->describe() }));
            player.give_item(move(item));
        }
        // Otherwise, create a new item of the type and quantity, and give that.
        else {
            auto& source_item = tile.items()[s_selected_item_index];
            source_item->decrease_quantity(s_item_quantity);
            auto new_item = adopt_own(*new Item(source_item->type(), s_item_quantity));
            UI::Toast::show(getText("msg_picked_up_item"_s, { new_item->describe() }));
            player.give_item(move(new_item));
        }
        player.set_has_acted(true);

        if (tile.items().is_empty()) {
            UI::closeWindow(pick_up_window_proc);
            return;
        }

        if (s_selected_item_index >= tile.items().count)
            select_item_index(tile.items().count - 1, tile.items());
        else
            select_item_index(s_selected_item_index, tile.items());
    }

    UI::Panel& ui = context->windowPanel;
    for (auto it = tile.items().iterate(); it.hasNext(); it.next()) {
        ui.startNewLine(HAlign::Left);
        if (it.getIndex() == s_selected_item_index) {
            auto& item = *it.get();
            String quantity_string = ""_s;
            if (item.quantity() > 1)
                quantity_string = myprintf("{}/{} "_s, { formatInt(s_item_quantity), formatInt(item.quantity()) });
            ui.addLabel(myprintf("> Take {}{} <"_s, { quantity_string, item.name() }), "small-selected"_sv);
        } else {
            ui.addLabel(it.get()->describe());
        }
    }
    ui.startNewLine(HAlign::Left);
    ui.addLabel("Up/Down to highlight an item.\n+/- to adjust quantity.\nEnter to select it.\nEscape to close this."_sv, "small-instructions"_sv);
}

void show_pick_up_window()
{
    auto& game = *AppState::the().game;
    auto& player = *game.player();
    auto& map = *game.map();
    auto& tile = map.tile_at(player.x(), player.y());

    if (tile.items().is_empty()) {
        UI::Toast::show(getText("msg_no_items_in_location"_s));
        return;
    }

    select_item_index(0, tile.items());

    UI::showWindow(UI::WindowTitle::fromTextAsset("title_pick_up_items"_s), 200, 200, {}, "default"_s, WindowFlags::AutomaticHeight | WindowFlags::UniqueKeepPosition, pick_up_window_proc);
}

static void drop_window_proc(UI::WindowContext* context, void*)
{
    auto& game = AppState::the().game;
    if (!game || !game->player() || !game->map()) {
        logWarn("Closing drop items window because no game/player/map found."_s);
        UI::closeWindow(drop_window_proc);
        return;
    }
    auto& player = *game->player();
    auto& map = *game->map();
    auto& tile = map.tile_at(player.x(), player.y());

    if (player.inventory().is_empty()) {
        UI::closeWindow(drop_window_proc);
        return;
    }

    // We're treating this list of labels like a menu. [Up] and [Down] select the item, and [Enter] selects it.
    if (keyJustPressed(SDLK_ESCAPE)) {
        UI::closeWindow(drop_window_proc);
        return;
    }

    if ((keyJustPressed(SDLK_UP) || keyJustPressed(SDLK_KP_8)) && s_selected_item_index > 0)
        select_item_index(s_selected_item_index - 1, player.inventory());
    if ((keyJustPressed(SDLK_DOWN) || keyJustPressed(SDLK_KP_2)) && s_selected_item_index < tile.items().count - 1)
        select_item_index(s_selected_item_index + 1, player.inventory());
    if (keyJustPressed(SDLK_PLUS) || keyJustPressed(SDLK_KP_PLUS)) {
        // TODO: Shortcut to select all?
        s_item_quantity = min(s_item_quantity + 1, player.inventory()[s_selected_item_index]->quantity());
    }
    if (keyJustPressed(SDLK_MINUS) || keyJustPressed(SDLK_KP_MINUS)) {
        // TODO: Shortcut to select 1?
        s_item_quantity = max(s_item_quantity - 1, 1);
    }
    if (keyJustPressed(SDLK_RETURN) || keyJustPressed(SDLK_KP_ENTER)) {
        // If we're taking the whole stack, transfer it directly.
        if (s_item_quantity == player.inventory()[s_selected_item_index]->quantity()) {
            auto item = player.inventory().take_index(s_selected_item_index, true);
            UI::Toast::show(getText("msg_dropped_item"_s, { item->describe() }));
            map.tile_at(player.x(), player.y()).add_item(move(item));
        }
        // Otherwise, create a new item of the type and quantity, and give that.
        else {
            auto& source_item = player.inventory()[s_selected_item_index];
            source_item->decrease_quantity(s_item_quantity);
            auto new_item = adopt_own(*new Item(source_item->type(), s_item_quantity));
            UI::Toast::show(getText("msg_dropped_item"_s, { new_item->describe() }));
            map.tile_at(player.x(), player.y()).add_item(move(new_item));
        }

        player.set_has_acted(true);

        if (player.inventory().is_empty()) {
            UI::closeWindow(drop_window_proc);
            return;
        }

        if (s_selected_item_index >= player.inventory().count)
            select_item_index(player.inventory().count - 1, player.inventory());
        else
            select_item_index(s_selected_item_index, player.inventory());
    }

    UI::Panel& ui = context->windowPanel;
    for (auto it = player.inventory().iterate(); it.hasNext(); it.next()) {
        ui.startNewLine(HAlign::Left);
        if (it.getIndex() == s_selected_item_index) {
            auto& item = *it.get();
            String quantity_string = ""_s;
            if (item.quantity() > 1)
                quantity_string = myprintf("{}/{} "_s, { formatInt(s_item_quantity), formatInt(item.quantity()) });
            ui.addLabel(myprintf("> Drop {}{} <"_s, { quantity_string, item.name() }), "small-selected"_sv);
        } else {
            ui.addLabel(it.get()->describe());
        }
    }
    ui.startNewLine(HAlign::Left);
    ui.addLabel("Up/Down to highlight an item.\n+/- to adjust quantity.\nEnter to select it.\nEscape to close this."_sv, "small-instructions"_sv);
}

void show_drop_window()
{
    auto& game = *AppState::the().game;
    auto& player = *game.player();

    if (player.inventory().is_empty()) {
        UI::Toast::show(getText("msg_no_items_in_inventory"_s));
        return;
    }

    select_item_index(0, player.inventory());
    UI::showWindow(UI::WindowTitle::fromTextAsset("title_drop_items"_s), 200, 200, {}, "default"_s, WindowFlags::AutomaticHeight | WindowFlags::UniqueKeepPosition, drop_window_proc);
}

static void recipe_selection_window_proc(UI::WindowContext* context, void* recipe_method_as_void_pointer)
{
    auto& game = AppState::the().game;
    if (!game || !game->player() || !game->map()) {
        logWarn("Closing recipe selection window because no game/player/map found."_s);
        UI::closeWindow(recipe_selection_window_proc);
        return;
    }
    auto& player = *game->player();

    // We're treating this list of labels like a menu. [Up] and [Down] select the item, and [Enter] selects it.
    if (keyJustPressed(SDLK_ESCAPE)) {
        UI::closeWindow(recipe_selection_window_proc);
        return;
    }

    auto recipe_method_int = reinterpret_cast<u64>(recipe_method_as_void_pointer);
    ASSERT(recipe_method_int < to_underlying(RecipeMethod::COUNT));
    auto recipe_method = static_cast<RecipeMethod>(recipe_method_int);
    auto& recipe_catalogue = RecipeCatalogue::the();
    auto& recipes = recipe_catalogue.all_recipes_with_method(recipe_method);

    auto player_can_craft = [&](RecipeDef const& recipe) {
        return recipe.ingredients.span().all_are([&player](RecipeDef::RecipeItem const& item) {
            return player.has_item(item.item_type, item.quantity);
        });
    };

    if ((keyJustPressed(SDLK_UP) || keyJustPressed(SDLK_KP_8)) && s_selected_item_index > 0)
        s_selected_item_index--;
    if ((keyJustPressed(SDLK_DOWN) || keyJustPressed(SDLK_KP_2)) && s_selected_item_index < recipes.count - 1)
        s_selected_item_index++;
    if (keyJustPressed(SDLK_RETURN) || keyJustPressed(SDLK_KP_ENTER)) {
        auto& recipe = recipe_catalogue.find(recipes.get(s_selected_item_index));
        if (player_can_craft(recipe)) {
            // Remove the items.
            for (auto const& ingredient : recipe.ingredients)
                player.remove_item(ingredient.item_type, ingredient.quantity);

            // Insert an item representing the in-progress craft.
            auto in_progress_item_type = ItemCatalogue::the().find_name(recipe.in_progress_item_name).release_value();
            auto in_progress_item = adopt_own(*new Item(in_progress_item_type));
            in_progress_item->set_data(ActiveCraftingRecipe { .id = recipe.id });
            player.give_item(move(in_progress_item));

            // Show the crafting window and close this one.
            switch (recipe_method) {
            case RecipeMethod::Knapping:
                show_knapping_window(recipe.id, true);
                break;
            case RecipeMethod::COUNT:
                VERIFY_NOT_REACHED();
            }
            context->closeRequested = true;
            return;
        }

        UI::Toast::show(myprintf("Missing some ingredients. Need: {}"_s, { describe_recipe_item_list(recipe.ingredients) }));
    }

    UI::Panel& ui = context->windowPanel;
    for (auto it = recipes.iterate(); it.hasNext(); it.next()) {
        auto recipe_id = it.get();
        auto& recipe = recipe_catalogue.find(recipe_id);
        ui.startNewLine(HAlign::Left);
        if (it.getIndex() == s_selected_item_index) {
            ui.addLabel(myprintf("> {} {} ({}) <"_s, { recipe_method_data[recipe_method].imperative, recipe.description, describe_recipe_item_list(recipe.ingredients) }), "small-selected"_sv);
        } else {
            ui.addLabel(myprintf("{} {} ({})"_s, { recipe_method_data[recipe_method].imperative, recipe.description, describe_recipe_item_list(recipe.ingredients) }), "small-selected"_sv);
        }
    }
    ui.startNewLine(HAlign::Left);
    ui.addLabel("Up/Down to highlight a recipe.\nEnter to select it.\nEscape to close this."_sv, "small-instructions"_sv);
}

void show_recipe_selection_window(RecipeMethod recipe_method)
{
    s_selected_item_index = 0;
    UI::showWindow(UI::WindowTitle::fromTextAsset(recipe_method_data[recipe_method].selection_window_title), 300, 200, {}, "default"_s, WindowFlags::AutomaticHeight | WindowFlags::UniqueKeepPosition, recipe_selection_window_proc, reinterpret_cast<void*>(recipe_method));
}

// Enough for 32x32
constexpr int max_knapping_size = 32 * 32;
constexpr int knapping_data_u64_count = BitArray::calculate_u64_count(max_knapping_size);
static u64 s_knapping_progress_data[knapping_data_u64_count];
static BitArray s_knapping_progress {};
static void knapping_window_proc(UI::WindowContext* context, void* recipe_id_as_void_pointer)
{
    auto& game = AppState::the().game;
    if (!game || !game->player() || !game->map()) {
        logWarn("Closing knapping window because no game/player/map found."_s);
        UI::closeWindow(knapping_window_proc);
        return;
    }
    auto& player = *game->player();

    if (keyJustPressed(SDLK_ESCAPE)) {
        context->closeRequested = true;
        return;
    }

    auto recipe_id = static_cast<RecipeID>(reinterpret_cast<u64>(recipe_id_as_void_pointer));
    auto& recipe = RecipeCatalogue::the().find(recipe_id);

    UI::Panel& ui = context->windowPanel;
    ui.startNewLine(HAlign::Left);
    ui.addLabel(myprintf("Knapping {}"_s, { recipe.description }));

    // Grid of buttons for knapping
    auto& shape = recipe.shape.value();
    bool changed = false;
    for (auto y = 0, bit_index = 0; y < shape.h; ++y) {
        ui.startNewLine(HAlign::Left);
        for (auto x = 0; x < shape.w; ++x, ++bit_index) {
            bool should_be_present = shape.get(x, y);
            bool is_present = s_knapping_progress[bit_index];
            // FIXME: Oh boy I don't like that we have to do the wrapping manually!!!
            auto& sprite_group = SpriteGroup::get(is_present ? "btn_knapping_solid"_sv : "btn_knapping_removed"_sv);
            auto& sprite = sprite_group.sprites[bit_index % sprite_group.count];
            auto style = (is_present && !should_be_present) ? "knapping-to-remove"_sv : "knapping"_sv;
            if (ui.addImageButton(&sprite, is_present ? ButtonState::Normal : ButtonState::Disabled, style)) {
                s_knapping_progress.unset_bit(bit_index);
                changed = true;
            }
        }
    }

    auto replace_in_progress_item_with_outputs = [&](auto& outputs) {
        auto in_progress_item_type = ItemCatalogue::the().find_name(recipe.in_progress_item_name).release_value();
        player.remove_item(in_progress_item_type, 1);

        for (auto const& output : outputs)
            player.give_item(adopt_own(*new Item(output.item_type, output.quantity)));
    };

    // If we now match the target pattern, complete the craft
    if (changed) {
        bool matches = [&] {
            for (auto y = 0, bit_index = 0; y < shape.h; ++y) {
                for (auto x = 0; x < shape.w; ++x, ++bit_index) {
                    if (shape.get(x, y) != s_knapping_progress[bit_index])
                        return false;
                }
            }
            return true;
        }();
        if (matches) {
            UI::Toast::show(myprintf("Created {}"_s, { recipe.description }));
            replace_in_progress_item_with_outputs(recipe.outputs);
            context->closeRequested = true;
        }
    }

    // Instructions
    ui.startNewLine(HAlign::Left);
    ui.addLabel("Click on highlighted sections to remove them, until you have the right shape. If you stop part-way, you can resume with [k]."_s, "small-instructions"_sv);

    // Control buttons
    ui.startNewLine(HAlign::Right);
    if (ui.addTextButton("Discard"_sv)) {
        replace_in_progress_item_with_outputs(recipe.cancelled_outputs);
        context->closeRequested = true;
    }
    if (ui.addTextButton("Stop for now"_sv)) {
        context->closeRequested = true;
    }
}

void show_knapping_window(RecipeID recipe_id, bool new_craft)
{
    if (s_knapping_progress.size == 0) {
        initBitArray(&s_knapping_progress, max_knapping_size, { knapping_data_u64_count, knapping_data_u64_count, s_knapping_progress_data });
    }
    if (new_craft)
        s_knapping_progress.set_all();

    // FIXME: Might be nice to put the recipe description in the title somehow.
    UI::showWindow(UI::WindowTitle::fromTextAsset("title_knapping"_s), 300, 200, {}, "default"_s, WindowFlags::AutomaticHeight | WindowFlags::UniqueKeepPosition, knapping_window_proc, reinterpret_cast<void*>(recipe_id));
}

bool any_input_consuming_windows_are_open()
{
    return UI::isWindowOpen(pick_up_window_proc)
        || UI::isWindowOpen(drop_window_proc)
        || UI::isWindowOpen(recipe_selection_window_proc)
        || UI::isWindowOpen(knapping_window_proc);
}

}
