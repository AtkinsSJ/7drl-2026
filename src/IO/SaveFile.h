/*
 * Copyright (c) 2019-2025, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <IO/BinaryFile.h>
#include <IO/Forward.h>
#include <Util/Basic.h>

//
// A crazy, completely-unnecessary idea: we could implement a thumbnail handler so that
// saved games show a little picture of the city. Probably we'd embed a small rendered
// image of the city in the save file (which would be useful for the in-game load menu)
// and then just use that.
// See https://docs.microsoft.com/en-us/windows/win32/shell/thumbnail-providers
// This is very clearly not a good use of time but it would be SUPER COOL.
// - Sam, 23/10/2019
//

#pragma pack(push, 1)

u8 const SAV_VERSION = 1;
FileIdentifier const SAV_FILE_ID = "CITY"_id;

u8 const SAV_META_VERSION = 1;
FileIdentifier const SAV_META_ID = "META"_id;
struct SAVSection_Meta {
    leU64 saveTimestamp; // Unix timestamp
    leU16 cityWidth;
    leU16 cityHeight;
};

struct SAVSection_Mods {
    // List which mods are enabled in this save, so that we know if they're present or not!
    // Also, probably turn individual mods on/off to match the save game, that'd be useful!

    // Hmmm... I guess we may also want to allow mods to add their own chunks. That's a bit
    // trickier! Well, that's only if mods can have their own code somehow - I know we'll
    // want to support custom content that's just data, but scripting is a whole other thing.
    // Just-data mods won't need extra sections.

    // Another thought: we could just pack mod-chunk data inside the MODS data maybe. Just
    // have sub-chunks within it. Again, this is very far away, so just throwing ideas out,
    // but if we store enabled mods as:
    // (name, data size, data offset)
    // then they can have any amount of data they like, they just have to save/load to a
    // binary blob.
};

#pragma pack(pop)

bool writeSaveFile(FileHandle* file);
bool loadSaveFile(FileHandle* file);
