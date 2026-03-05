/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Assets/AssetLoader.h>
#include <Assets/AssetManagerListener.h>
#include <Game/Forward.h>
#include <Util/ChunkedArray.h>
#include <Util/HashTable.h>
#include <Util/String.h>
#include <Util/StringTable.h>

enum class RecipeMethod : u8 {
    Knapping,
};

using RecipeShape = Array2<bool>;

struct RecipeDef {
    RecipeType type;
    String name;
    RecipeMethod method;
    String in_progress_item_name;

    Optional<RecipeShape> shape;

    struct RecipeItem {
        String item_name;
        u32 quantity;
    };
    Array<RecipeItem> ingredients;
    Array<RecipeItem> outputs;
};

class RecipeCatalogue final
    : public AssetManagerListener {
    friend class RecipesAssetLoader;

public:
    static RecipeCatalogue& the();

    static NonnullOwnPtr<AssetLoader> make_loader();

    Optional<RecipeType> find_name(String const& name) const;
    RecipeDef const& find(RecipeType) const;

    ChunkedArray<RecipeDef> const& defs() const { return m_recipe_defs; }

    // ^ AssetManagerListener
    virtual void before_assets_unloaded() override;
    virtual void after_assets_loaded() override;

private:
    RecipeCatalogue();

    StringTable m_strings;
    ChunkedArray<RecipeDef> m_recipe_defs;
    HashTable<RecipeType> m_recipe_name_to_type;
};
