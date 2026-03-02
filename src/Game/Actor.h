/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Game/Forward.h>
#include <Util/Basic.h>

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
    Actor(s32 x, s32 y);
    virtual ~Actor() = default;
    virtual void update() = 0;
    virtual void render(float delta_time) = 0;

    s32 x() const { return m_x; }
    s32 y() const { return m_y; }

    void set_map(Map*);

    bool try_move(Direction);

private:
    Map* m_map { nullptr };
    s32 m_x;
    s32 m_y;
};
