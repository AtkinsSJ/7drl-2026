/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "RecipeCatalogue.h"
#include <Assets/Asset.h>
#include <Assets/AssetManager.h>
#include <Gfx/Sprite.h>
#include <Gfx/Texture.h>

class RecipesAsset final : public Asset {
public:
    void unload(AssetMetadata& metadata) override
    {
        // Nothing to do, defs are unloaded automatically in RecipeCatalogue::before_assets_unloaded().
    }
};

class RecipesAssetLoader final : public AssetLoader {
public:
    virtual void register_types(AssetManager& asset_manager) override
    {
        m_recipes_asset_type = asset_manager.register_asset_type("Recipes"_s, *this, { .file_extension = "recipes"_sv });
    }

    virtual void create_placeholder_assets(AssetManager&) override
    {
        // We don't need a placeholder
    }

    virtual ErrorOr<NonnullOwnPtr<Asset>> load_asset(AssetMetadata& metadata, Blob file_data) override
    {
        if (metadata.type != m_recipes_asset_type)
            return { "Wrong asset type for RecipesAssetLoader"_s };

        // NB: We directly load the recipes into here as we read them.
        auto& catalogue = RecipeCatalogue::the();

        LineReader reader { metadata.shortName, file_data };
        struct InternalRecipeDef {
            String name;
            Optional<RecipeMethod> method;
            Optional<String> in_progress_item_name;
            Optional<RecipeShape> shape;
            Array<RecipeDef::RecipeItem> ingredients;
            Array<RecipeDef::RecipeItem> outputs;
        };
        Optional<InternalRecipeDef> current_recipe;

        auto add_current_recipe_if_set = [&] {
            if (current_recipe.has_value()) {
                auto recipe = current_recipe.release_value();

                // Validate it
                if (!recipe.method.has_value() || !recipe.in_progress_item_name.has_value() || recipe.ingredients.is_empty() || recipe.outputs.is_empty()) {
                    reader.error(":Recipe needs a method, inProgress, ingredients, and outputs defined"_s);
                    return;
                }

                RecipeType recipe_type = catalogue.m_recipe_defs.count;
                RecipeDef def {
                    .type = recipe_type,
                    .name = recipe.name,
                    .method = recipe.method.release_value(),
                    .in_progress_item_name = recipe.in_progress_item_name.release_value(),
                    .shape = move(recipe.shape),
                    .ingredients = move(recipe.ingredients),
                    .outputs = move(recipe.outputs),
                };
                catalogue.m_recipe_name_to_type.put(recipe.name, recipe_type);
                catalogue.m_recipe_defs.append(move(def));
            }
        };

        auto read_recipe_items = [&catalogue](LineReader& reader) -> Optional<Array<RecipeDef::RecipeItem>> {
            auto rest_of_line = reader.remainder_of_current_line();
            auto item_count = 1;
            for (auto i = 0; i < rest_of_line.length(); ++i) {
                if (rest_of_line[i] == ',')
                    item_count++;
            }
            auto items_array = asset_manager().arena.allocate_array<RecipeDef::RecipeItem>(item_count);
            for (auto i = 0; i < item_count; ++i) {
                auto quantity = reader.read_int<u32>();
                auto name = reader.next_token(',');
                if (!quantity.has_value() || !name.has_value())
                    return {};
                auto trimmed_name = name.value().with_whitespace_trimmed();
                if (trimmed_name.is_empty())
                    return {};
                items_array.append({ catalogue.m_strings.intern(trimmed_name), quantity.value() });
            }

            ASSERT(items_array.count == item_count);
            return items_array;
        };

        while (reader.load_next_line()) {
            auto maybe_command = reader.next_token();
            if (!maybe_command.has_value())
                continue;
            auto command = maybe_command.release_value();

            if (command == ":Recipe"_sv) {
                add_current_recipe_if_set();

                auto name = reader.next_token();
                if (!name.has_value() || reader.peek_token().has_value())
                    return reader.make_error_message("Couldn't parse :Recipe. Expected: ':Recipe name'"_s);

                auto name_string = catalogue.m_strings.intern(name.value());
                current_recipe = InternalRecipeDef { .name = name_string };
                continue;
            }

            if (!current_recipe.has_value())
                return reader.make_error_message("Properties are only allowed inside :Recipe"_s);

            if (command == "method"_sv) {
                auto name = reader.next_token();
                Optional<RecipeMethod> method;
                if (name == "knapping"_sv)
                    method = RecipeMethod::Knapping;
                if (!method.has_value() || reader.peek_token().has_value())
                    return reader.make_error_message("Couldn't parse method. Expected: 'method knapping'"_s);

                current_recipe.value().method = method.release_value();
                continue;
            }

            if (command == "inProgress"_sv) {
                auto name = reader.next_token();
                if (!name.has_value() || reader.peek_token().has_value())
                    return reader.make_error_message("Couldn't parse inProgress. Expected: 'inProgress item_name'"_s);

                current_recipe.value().in_progress_item_name = catalogue.m_strings.intern(name.value());
                continue;
            }

            if (command == "ingredients"_sv) {
                auto ingredients = read_recipe_items(reader);
                if (!ingredients.has_value() || reader.peek_token().has_value())
                    return reader.make_error_message("Couldn't parse ingredients. Expected: 'ingredients (comma-separated list of `count item_name`)'"_s);
                current_recipe.value().ingredients = ingredients.release_value();
                continue;
            }

            if (command == "outputs"_sv) {
                auto outputs = read_recipe_items(reader);
                if (!outputs.has_value() || reader.peek_token().has_value())
                    return reader.make_error_message("Couldn't parse outputs. Expected: 'outputs (comma-separated list of `count item_name`)'"_s);
                current_recipe.value().outputs = outputs.release_value();
                continue;
            }

            if (command == "knappingShape"_sv) {
                if (reader.remainder_of_current_line() != "{"_sv)
                    return reader.make_error_message("Couldn't parse knappingShape. Expected: 'knappingShape {, then lines with the shape, then }'"_s);

                ChunkedArray<StringView> crafting_lines;
                initChunkedArray(&crafting_lines, &temp_arena(), 16);
                while (reader.load_next_line()) {
                    if (reader.remainder_of_current_line() == "}"_sv)
                        break;
                    crafting_lines.append(reader.remainder_of_current_line());
                }

                if (crafting_lines.is_empty())
                    return reader.make_error_message("Couldn't parse knappingShape. Expected: 'knappingShape {, then lines with the shape, then }'"_s);
                u32 width = crafting_lines[0].length();
                for (auto it = crafting_lines.iterate(); it.hasNext(); it.next()) {
                    if (it.get().length() != width)
                        return reader.make_error_message("Couldn't parse knappingShape. All lines should be the same length."_s);
                }
                auto shape = asset_manager().arena.allocate_array_2d<bool>(width, crafting_lines.count);
                for (auto it = crafting_lines.iterate(); it.hasNext(); it.next()) {
                    auto y = it.getIndex();
                    auto& line = it.get();
                    for (auto x = 0; x < line.length(); ++x)
                        shape.set(x, y, line[x] == 'X');
                }
                current_recipe.value().shape = move(shape);
                continue;
            }

            return reader.make_error_message("Unrecognized command."_s);
        }

        add_current_recipe_if_set();

        return { adopt_own(*new RecipesAsset) };
    }

private:
    AssetType m_recipes_asset_type { 0 };
};

RecipeCatalogue& RecipeCatalogue::the()
{
    static RecipeCatalogue s_recipe_catalogue {};
    return s_recipe_catalogue;
}

RecipeCatalogue::RecipeCatalogue()
    : m_recipe_name_to_type(1024)
{
    initChunkedArray(&m_recipe_defs, &asset_manager().arena, 1024);
}

NonnullOwnPtr<AssetLoader> RecipeCatalogue::make_loader()
{
    return adopt_own(*new RecipesAssetLoader);
}

Optional<RecipeType> RecipeCatalogue::find_name(String const& name) const
{
    return m_recipe_name_to_type.find_value(name);
}

RecipeDef const& RecipeCatalogue::find(RecipeType type) const
{
    return m_recipe_defs.get(type);
}

void RecipeCatalogue::before_assets_unloaded()
{
    m_recipe_defs.clear();
    m_recipe_name_to_type.clear();
}

void RecipeCatalogue::after_assets_loaded()
{
    logInfo("{} recipes loaded"_s, { formatInt(m_recipe_defs.count) });
}
