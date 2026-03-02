/*
 * Copyright (c) 2025, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Settings/Setting.h>
#include <Util/ChunkedArray.h>
#include <Util/HashTable.h>
#include <Util/Locale.h>
#include <Util/Optional.h>
#include <Util/Ref.h>
#include <Util/String.h>

class BaseSettingsState {
public:
    virtual ~BaseSettingsState() = default;

    void restore_default_values();
    void copy_from(BaseSettingsState& other);

    Optional<Ref<Setting>> setting_by_name(String const& name);

    void load_from_file(String filename, Blob data);
    // FIXME: Should be const, but that's awkward right now.
    bool save_to_file(String filename);

    template<typename Callback>
    void for_each_setting(Callback callback)
    {
        for (auto it = m_settings_order.iterate();
            it.hasNext();
            it.next()) {
            String name = it.getValue();
            auto setting = *m_settings_by_name.find(name).value();
            callback(*setting);
        }
    }

protected:
    explicit BaseSettingsState(MemoryArena& arena);
    void register_setting(Setting&);

private:
    HashTable<Ref<Setting>> m_settings_by_name;
    ChunkedArray<String> m_settings_order;
};

class SettingsState final : public BaseSettingsState {
public:
    explicit SettingsState(MemoryArena& arena);
    virtual ~SettingsState() override = default;

    BoolSetting windowed { "windowed"_s, "setting_windowed"_s, true };
    V2ISetting resolution { "resolution"_s, "setting_resolution"_s, v2i(1024, 600) };
    PercentSetting music_volume { "music_volume"_s, "setting_music_volume"_s, 0.5f };
    PercentSetting sound_volume { "sound_volume"_s, "setting_sound_volume"_s, 0.5f };
};
