/*
 * Copyright (c) 2026, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "ItemCatalogue.h"
#include <Assets/Asset.h>
#include <Assets/AssetManager.h>
#include <Gfx/Sprite.h>
#include <Gfx/Texture.h>

class ItemsAsset final : public Asset {
public:
    void unload(AssetMetadata& metadata) override
    {
        // Nothing to do, defs are unloaded automatically in ItemCatalogue::before_assets_unloaded().
    }
};

class ItemsAssetLoader final : public AssetLoader {
public:
    virtual void register_types(AssetManager& asset_manager) override
    {
        m_items_asset_type = asset_manager.register_asset_type("Items"_s, *this, { .file_extension = "items"_sv });
    }

    virtual void create_placeholder_assets(AssetManager&) override
    {
        // We don't need a placeholder
    }

    virtual ErrorOr<NonnullOwnPtr<Asset>> load_asset(AssetMetadata& metadata, Blob file_data) override
    {
        if (metadata.type != m_items_asset_type)
            return { "Wrong asset type for ItemsAssetLoader"_s };

        // NB: We directly load the items into here as we read them.
        auto& catalogue = ItemCatalogue::the();

        LineReader reader { metadata.shortName, file_data };
        Optional<ItemDef> current_item;

        auto add_current_item_if_set = [&] {
            if (current_item.has_value()) {
                auto item = current_item.release_value();

                // Validate it
                if (item.sprite_name.is_empty()) {
                    reader.error(":Item needs a sprite name defined"_s);
                    return;
                }

                ItemType item_type = catalogue.m_item_defs.count;
                item.type = item_type;
                catalogue.m_item_name_to_type.put(item.name, item_type);
                catalogue.m_item_defs.append(move(item));
            }
        };

        while (reader.load_next_line()) {
            auto maybe_command = reader.next_token();
            if (!maybe_command.has_value())
                continue;
            auto command = maybe_command.release_value();

            if (command == ":Item"_sv) {
                add_current_item_if_set();

                auto name = reader.next_token();
                if (!name.has_value() || reader.peek_token().has_value())
                    return reader.make_error_message("Couldn't parse :Item. Expected: ':Item name'"_s);

                auto name_string = catalogue.m_strings.intern(name.value());
                current_item = ItemDef { .name = name_string };
            } else if (command == "sprite"_sv) {
                if (!current_item.has_value())
                    return reader.make_error_message("'sprite' is only valid inside :Item"_s);

                auto name = reader.next_token();
                if (!name.has_value() || reader.peek_token().has_value())
                    return reader.make_error_message("Couldn't parse sprite. Expected: 'sprite name'"_s);

                asset_manager().add_asset(SpriteGroup::asset_type(), name.value());

                current_item.value().sprite_name = catalogue.m_strings.intern(name.value());
            } else if (command == "stackSize"_sv) {
                if (!current_item.has_value())
                    return reader.make_error_message("'stackSize' is only valid inside :Item"_s);

                auto stack_size = reader.read_int<u32>();
                if (!stack_size.has_value() || reader.peek_token().has_value())
                    return reader.make_error_message("Couldn't parse stackSize. Expected: 'stackSize count'"_s);

                current_item.value().stack_size = stack_size.release_value();
            } else {
                return reader.make_error_message("Unrecognized command."_s);
            }
        }

        add_current_item_if_set();

        return { adopt_own(*new ItemsAsset) };
    }

private:
    AssetType m_items_asset_type { 0 };
};

ItemCatalogue& ItemCatalogue::the()
{
    static ItemCatalogue s_item_catalogue {};
    return s_item_catalogue;
}

ItemCatalogue::ItemCatalogue()
    : m_item_name_to_type(1024)
{
    initChunkedArray(&m_item_defs, &asset_manager().arena, 1024);
}

NonnullOwnPtr<AssetLoader> ItemCatalogue::make_loader()
{
    return adopt_own(*new ItemsAssetLoader);
}

Optional<ItemType> ItemCatalogue::find_name(String const& name) const
{
    return m_item_name_to_type.find_value(name);
}

ItemDef const& ItemCatalogue::find(ItemType type) const
{
    return m_item_defs.get(type);
}

void ItemCatalogue::before_assets_unloaded()
{
    m_item_defs.clear();
    m_item_name_to_type.clear();
}

void ItemCatalogue::after_assets_loaded()
{
    logInfo("{} items loaded"_s, { formatInt(m_item_defs.count) });
}
