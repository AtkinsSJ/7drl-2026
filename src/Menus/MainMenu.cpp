/*
 * Copyright (c) 2016-2025, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "MainMenu.h"
#include "AppState.h"
#include <Debug/Debug.h>
#include <Gfx/Renderer.h>
#include <IO/SavedGames.h>
#include <Menus/About.h>
#include <Settings/Settings.h>
#include <UI/Panel.h>

AppStatus updateAndRenderMainMenu(float /*deltaTime*/)
{
    DEBUG_FUNCTION();

    AppStatus result = AppStatus::MainMenu;
    auto& renderer = the_renderer();

    auto window_size = renderer.window_size();
    UI::Panel panel = UI::Panel({ 0, window_size.y / 4, window_size.x, window_size.y }, "mainMenu"_s);

    panel.addLabel(getText("game_title"_s));
    panel.addLabel(getText("game_subtitle"_s));

    String newGameText = getText("button_new_game"_s);
    String loadText = getText("button_load"_s);
    String creditsText = getText("button_credits"_s);
    String settingsText = getText("button_settings"_s);
    String aboutText = getText("button_about"_s);
    String exitText = getText("button_exit"_s);

    if (panel.addTextButton(newGameText)) {
        AppState::the().game = Game::create();
        result = AppStatus::Game;
    }
    if (panel.addTextButton(loadText)) {
        showLoadGameWindow();
    }
    if (panel.addTextButton(creditsText)) {
        result = AppStatus::Credits;
    }
    if (panel.addTextButton(settingsText)) {
        showSettingsWindow();
    }
    if (panel.addTextButton(aboutText)) {
        showAboutWindow();
    }
    if (panel.addTextButton(exitText)) {
        result = AppStatus::Quit;
    }

    panel.end(true);

    return result;
}
