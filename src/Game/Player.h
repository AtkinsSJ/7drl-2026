/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Direction.h>
#include <flecs.h>

struct Player { };

flecs::entity create_player(flecs::world&, s32 x, s32 y);

bool try_move_player(flecs::world&, flecs::entity player, Direction);
