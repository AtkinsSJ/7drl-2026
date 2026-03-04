/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Forward.h>
#include <Util/Basic.h>
#include <Util/ChunkedArray.h>

enum class Direction : u8 {
    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    NW,
};

class Actor {
public:
    Actor(s32 x, s32 y, ArrayChunkPool<NonnullOwnPtr<Item>>&);
    virtual ~Actor() = default;
    virtual void update() = 0;
    virtual void render(float delta_time) = 0;

    s32 x() const { return m_x; }
    s32 y() const { return m_y; }

    ChunkedArray<NonnullOwnPtr<Item>> const& inventory() const { return m_inventory; }
    void give_item(NonnullOwnPtr<Item>);

    void set_map(Map*);

    bool try_move(Direction);

private:
    Map* m_map { nullptr };
    s32 m_x;
    s32 m_y;
    ChunkedArray<NonnullOwnPtr<Item>> m_inventory;
};
