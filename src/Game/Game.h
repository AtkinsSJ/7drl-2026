/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "AppStatus.h"
#include <Game/Map.h>
#include <Game/Player.h>
#include <Gfx/Camera.h>
#include <Util/OwnPtr.h>

class Game {
public:
    static NonnullOwnPtr<Game> create();

    AppStatus update_and_render(float delta_time);

private:
    explicit Game(u32 width, u32 height);
    MemoryArena m_arena;
    OwnPtr<Map> m_map;
    Player* m_player;
};
