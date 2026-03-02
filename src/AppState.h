/*
 * Copyright (c) 2021-2025, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "AppStatus.h"
#include <Game/Game.h>
#include <Util/MemoryArena.h>
#include <Util/Random.h>

struct AppState {
    static AppState& the();

    AppStatus appStatus;
    MemoryArena systemArena;

    Random* cosmeticRandom; // Appropriate for when you need a random number and don't care if it's consistent!
    
    OwnPtr<Game> game;

    float rawDeltaTime;
    float speedMultiplier;
    float deltaTime;

    inline void setDeltaTimeFromLastFrame(float lastFrameTime)
    {
        rawDeltaTime = lastFrameTime;
        deltaTime = rawDeltaTime * speedMultiplier;
    }

    inline void setSpeedMultiplier(float multiplier)
    {
        speedMultiplier = multiplier;
        deltaTime = rawDeltaTime * speedMultiplier;
    }
};
