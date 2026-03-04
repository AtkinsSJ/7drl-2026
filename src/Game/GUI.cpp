/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "GUI.h"
#include <AppState.h>
#include <Game/Item.h>
#include <UI/Toast.h>
#include <UI/Window.h>

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
    ui.addLabel("Up/Down to select an item.\n+/- to adjust quantity.\nEnter to pick it up.\nEscape to close this."_sv, "small-instructions"_sv);
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
    ui.addLabel("Up/Down to select an item.\n+/- to adjust quantity.\nEnter to drop it.\nEscape to close this."_sv, "small-instructions"_sv);
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

bool any_input_consuming_windows_are_open()
{
    return UI::isWindowOpen(pick_up_window_proc) || UI::isWindowOpen(drop_window_proc);
}

}
