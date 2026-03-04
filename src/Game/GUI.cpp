/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "GUI.h"
#include <AppState.h>
#include <Game/Item.h>
#include <UI/Window.h>

namespace GUI {

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

}
