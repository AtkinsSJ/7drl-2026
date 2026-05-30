/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Direction.h>
#include <flecs.h>

struct Player { };

bool try_move_player(flecs::world, flecs::entity player, Direction);
