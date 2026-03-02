/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Util/Basic.h>
#include <Util/String.h>

enum class Terrain : u8 {
    Water,
    Grass,
    Sand,
    COUNT,
};

struct TerrainDef {
    String name;
    String sprite_name;
};
TerrainDef const& get_terrain_def(Terrain);
