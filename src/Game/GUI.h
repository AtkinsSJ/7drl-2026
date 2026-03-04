/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

namespace GUI {

void toggle_inventory();
void toggle_help();
void show_pick_up_window();
void show_drop_window();

bool any_input_consuming_windows_are_open();

}
