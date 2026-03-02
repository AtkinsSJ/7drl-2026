/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Terrain.h"
#include <Util/EnumMap.h>

static EnumMap<Terrain, TerrainDef>& get_terrain_defs()
{
    static EnumMap<Terrain, TerrainDef> terrain_defs {
        { "Water"_s, "t_water"_s },
        { "Grass"_s, "t_grass"_s },
        { "Sand"_s, "t_sand"_s },
    };

    return terrain_defs;
}

TerrainDef const& get_terrain_def(Terrain terrain)
{
    return get_terrain_defs()[terrain];
}
