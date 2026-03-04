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

struct ItemDef {
    ItemType type;
    String name;
    String sprite_name;
    u32 stack_size { 1 };
};

class ItemCatalogue final
    : public AssetManagerListener {
    friend class ItemsAssetLoader;

public:
    static ItemCatalogue& the();

    static NonnullOwnPtr<AssetLoader> make_loader();

    Optional<ItemType> find_name(String const& name) const;
    ItemDef const& find(ItemType) const;

    ChunkedArray<ItemDef> const& defs() const { return m_item_defs; }

    // ^ AssetManagerListener
    virtual void before_assets_unloaded() override;
    virtual void after_assets_loaded() override;

private:
    ItemCatalogue();

    StringTable m_strings;
    ChunkedArray<ItemDef> m_item_defs;
    HashTable<ItemType> m_item_name_to_type;
};
