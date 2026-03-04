/*
 * Copyright (c) 2017-2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "../AppState.h"
#include <Debug/Console.h>
#include <Game/ItemCatalogue.h>
#include <Gfx/Renderer.h>
#include <Settings/Settings.h>
#include <UI/Toast.h>
#include <UI/Window.h>
#include <Util/TokenReader.h>

#pragma warning(push)
#pragma warning(disable : 4100) // Disable unused-arg warnings for commands, as they all have to take the same args.

#define ConsoleCommand(name) static void cmd_##name([[maybe_unused]] Console* console, [[maybe_unused]] s32 argumentsCount, [[maybe_unused]] StringView arguments)

ConsoleCommand(exit)
{
    consoleWriteLine("Quitting game..."_s, ConsoleLineStyle::Success);
    AppState::the().appStatus = AppStatus::Quit;
}

ConsoleCommand(hello)
{
    consoleWriteLine("Hello human!"_s);
    consoleWriteLine(myprintf("Testing formatInt bases: 10:{0}, 16:{1}, 36:{2}, 8:{3}, 2:{4}"_s, { formatInt(123456, 10), formatInt(123456, 16), formatInt(123456, 36), formatInt(123456, 8), formatInt(123456, 2) }));
}

ConsoleCommand(help)
{
    consoleWriteLine("Available commands are:"_s);

    for (auto it = globalConsole->commands.iterate();
        it.hasNext();
        it.next()) {
        Command* command = it.get();
        consoleWriteLine(myprintf(" - {0}"_s, { command->name }));
    }
}

ConsoleCommand(items)
{
    consoleWriteLine("Items:"_s);

    for (auto it = ItemCatalogue::the().defs().iterate();
        it.hasNext();
        it.next()) {
        auto& item_def = it.get();
        consoleWriteLine(myprintf(" - #{}: {} sprite({}) stack({})"_s, { formatInt(item_def.type), item_def.name, item_def.sprite_name, formatInt(item_def.stack_size) }));
    }
}

ConsoleCommand(reload_assets)
{
    asset_manager().reload();
}

ConsoleCommand(reload_settings)
{
    Settings::the().load();
    Settings::the().apply();
}

ConsoleCommand(setting)
{
    auto& settings = *Settings::the().settings;

    if (argumentsCount == 0) {
        consoleWriteLine("Available settings:"_s, ConsoleLineStyle::Success);
        settings.for_each_setting([](auto& setting) {
            consoleWriteLine(setting.name(), ConsoleLineStyle::Success);
        });
        return;
    }

    // FIXME: This is hacky, surely we can come up with a nice API for this.
    LineReader reader { "console"_s, Blob { static_cast<smm>(arguments.length()), const_cast<u8*>(reinterpret_cast<u8 const*>(arguments.raw_pointer_to_characters())) } };
    reader.load_next_line();

    auto maybe_setting_name = reader.next_token();
    if (!maybe_setting_name.has_value()) {
        consoleWriteLine("Missing setting name."_s, ConsoleLineStyle::Error);
        return;
    }
    auto setting_name = maybe_setting_name.release_value();
    auto maybe_setting = settings.setting_by_name(setting_name.deprecated_to_string());
    if (!maybe_setting.has_value()) {
        consoleWriteLine(myprintf("Unrecognized setting name '{}'."_s, { setting_name }), ConsoleLineStyle::Error);
        return;
    }
    auto& setting = *maybe_setting.release_value();

    if (argumentsCount == 1) {
        consoleWriteLine(myprintf("{} is {}"_s, { setting_name, setting.serialize_value() }), ConsoleLineStyle::Success);
        return;
    }

    if (setting.set_from_file(reader)) {
        Settings::the().apply();
        consoleWriteLine(myprintf("Set {} to {}"_s, { setting_name, setting.serialize_value() }), ConsoleLineStyle::Success);
    } else {
        consoleWriteLine(myprintf("Unable to set {}: invalid format"_s, { setting_name }), ConsoleLineStyle::Error);
    }
}

ConsoleCommand(speed)
{
    auto& app_state = AppState::the();

    if (argumentsCount == 0) {
        consoleWriteLine(myprintf("Current game speed: {0}"_s, { formatFloat(app_state.speedMultiplier, 3) }), ConsoleLineStyle::Success);
    } else {
        TokenReader tokens { arguments };
        if (auto speed_multiplier = tokens.next_token().value().to_float(); speed_multiplier.has_value()) {
            float multiplier = speed_multiplier.value();
            app_state.setSpeedMultiplier(multiplier);
            consoleWriteLine(myprintf("Set speed to {0}"_s, { formatFloat(multiplier, 3) }), ConsoleLineStyle::Success);
            return;
        }
        consoleWriteLine("Usage: speed (multiplier), where multiplier is a float, or with no argument to list the current speed"_s, ConsoleLineStyle::Error);
    }
}

ConsoleCommand(toast)
{
    UI::Toast::show(arguments);
}

ConsoleCommand(zoom)
{
    auto& renderer = the_renderer();

    if (argumentsCount == 0) {
        // list the zoom
        float zoom = renderer.world_camera().zoom();
        consoleWriteLine(myprintf("Current zoom is {0}"_s, { formatFloat(zoom, 3) }), ConsoleLineStyle::Success);
    } else if (argumentsCount == 1) {
        // set the zoom
        TokenReader tokens { arguments };
        if (auto requested_zoom = tokens.next_token().value().to_float(); requested_zoom.has_value()) {
            float newZoom = requested_zoom.release_value();
            renderer.world_camera().set_zoom(newZoom);
            consoleWriteLine(myprintf("Set zoom to {0}"_s, { formatFloat(newZoom, 3) }), ConsoleLineStyle::Success);
            return;
        }
        consoleWriteLine("Usage: zoom (scale), where scale is a float, or with no argument to list the current zoom"_s, ConsoleLineStyle::Error);
    }
}

#undef ConsoleCommand

void initCommands(Console* console)
{
    // NB: Increase the count before we reach it - hash tables like lots of extra space!
    s32 commandCapacity = 128;
    console->commands = HashTable<Command>::allocate_fixed_size(AppState::the().systemArena, commandCapacity);

    // NB: a max-arguments value of -1 means "no maximum"
#define AddCommand(name, minArgs, maxArgs) console->commands.put(#name##_h, Command(#name##_h, &cmd_##name, minArgs, maxArgs))
    AddCommand(help, 0, 0);
    AddCommand(exit, 0, 0);
    AddCommand(hello, 0, 1);
    AddCommand(items, 0, 0);
    AddCommand(reload_assets, 0, 0);
    AddCommand(reload_settings, 0, 0);
    AddCommand(setting, 0, -1);
    AddCommand(speed, 0, 1);
    AddCommand(toast, 1, -1);
    AddCommand(zoom, 0, 1);
#undef AddCommand
}

#pragma warning(pop)
