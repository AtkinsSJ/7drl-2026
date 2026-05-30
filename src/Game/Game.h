/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "AppStatus.h"
#include <Gfx/Camera.h>
#include <Util/ChunkedArray.h>
#include <Util/OwnPtr.h>
#include <flecs.h>

class Game {
public:
    static NonnullOwnPtr<Game> create();

    AppStatus update_and_render(float delta_time);

    ArrayChunkPool<flecs::entity>& entity_chunk_pool() { return m_entity_chunk_pool; }

    flecs::world world() { return m_world; }
    flecs::entity player() const;

private:
    explicit Game(u32 width, u32 height);
    MemoryArena m_arena;
    ArrayChunkPool<flecs::entity> m_entity_chunk_pool;
    flecs::world m_world;

    flecs::entity m_simulation_phase;
};
