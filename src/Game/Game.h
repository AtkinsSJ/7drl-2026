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

    ArrayChunkPool<NonnullOwnPtr<Item>>& item_chunk_pool() { return m_item_chunk_pool; }

private:
    explicit Game(u32 width, u32 height);
    MemoryArena m_arena;
    ArrayChunkPool<NonnullOwnPtr<Item>> m_item_chunk_pool;

    OwnPtr<Map> m_map;
    Player* m_player;
};
