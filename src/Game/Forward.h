/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Util/Basic.h>

class Actor;
class Game;
class Item;
class Map;
class Tile;
class Player;

using ItemType = u32;
using RecipeID = u32;

enum class RecipeMethod : u8;
