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
#include <Util/EnumMap.h>
#include <Util/HashTable.h>
#include <Util/String.h>
#include <Util/StringTable.h>

enum class RecipeMethod : u8 {
    Knapping,
    COUNT,
};

struct RecipeMethodData {
    String imperative;
    String selection_window_title;
};
static EnumMap<RecipeMethod, RecipeMethodData> const recipe_method_data {
    { .imperative = "Knap"_s, .selection_window_title = "title_select_knap_recipe"_s }
};

using RecipeShape = Array2<bool>;

struct RecipeDef {
    RecipeID id;
    String name;
    String description;
    RecipeMethod method;
    String in_progress_item_name;

    Optional<RecipeShape> shape;

    struct RecipeItem {
        String item_name;
        u32 quantity;
        ItemType item_type;
    };
    Array<RecipeItem> ingredients;
    Array<RecipeItem> outputs;
    Array<RecipeItem> cancelled_outputs;
};

StringView describe_recipe_item_list(ReadonlySpan<RecipeDef::RecipeItem>);

class RecipeCatalogue final
    : public AssetManagerListener {
    friend class RecipesAssetLoader;

public:
    static RecipeCatalogue& the();

    static NonnullOwnPtr<AssetLoader> make_loader();

    Optional<RecipeID> find_name(String const& name) const;
    RecipeDef const& find(RecipeID) const;

    ChunkedArray<RecipeDef> const& defs() const { return m_recipe_defs; }

    ChunkedArray<RecipeID> const& all_recipes_with_method(RecipeMethod method) const
    {
        return m_all_recipes_for_method[method];
    }

    // ^ AssetManagerListener
    virtual void before_assets_unloaded() override;
    virtual void after_assets_loaded() override;

private:
    RecipeCatalogue();

    StringTable m_strings;
    ChunkedArray<RecipeDef> m_recipe_defs;
    HashTable<RecipeID> m_recipe_name_to_type;
    EnumMap<RecipeMethod, ChunkedArray<RecipeID>> m_all_recipes_for_method;
};
