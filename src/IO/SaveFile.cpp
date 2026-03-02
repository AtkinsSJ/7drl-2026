/*
 * Copyright (c) 2019-2025, Sam Atkins <sam@samatkins.co.uk>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "SaveFile.h"

#include "Util/Time.h"

#include <Gfx/Renderer.h>
#include <IO/BinaryFileReader.h>
#include <IO/BinaryFileWriter.h>

bool writeSaveFile(FileHandle* file)
{
    bool succeeded = file->isOpen;

    if (succeeded) {
        BinaryFileWriter writer = startWritingFile(SAV_FILE_ID, SAV_VERSION, &temp_arena());

        // Prepare the TOC
        writer.addTOCEntry(SAV_META_ID);

        // Meta
        {
            writer.startSection<SAVSection_Meta>(SAV_META_ID, SAV_META_VERSION);
            SAVSection_Meta metaSection = {};

            metaSection.saveTimestamp = get_current_unix_timestamp();

            writer.endSection(&metaSection);
        }

        succeeded = writer.outputToFile(file);
    }

    return succeeded;
}

bool loadSaveFile(FileHandle* file)
{
    // So... I'm not really sure how to signal success, honestly.
    // I suppose the process ouytside of this function is:
    // - User confirms to load a city.
    // - Existing city, if any, is discarded.
    // - This function is called.
    // - If it fails, discard the city, else it's in memory.
    // So, if loading fails, then no city will be in memory, regardless of whether one was
    // before the loading was attempted! I think that makes the most sense.
    // Another option would be to load into a second City struct, and then swap it if it
    // successfully loads... but that makes a bunch of memory-management more complicated.
    // This way, we only ever have one City in memory so we can clean up easily.

    // For now, reading the whole thing into memory and then processing it is simpler.
    // However, it's wasteful memory-wise, so if save files get big we might want to
    // read the file a bit at a time. @Size

    bool succeeded = false;

    BinaryFileReader reader = readBinaryFile(file, SAV_FILE_ID, &temp_arena());
    // This doesn't actually loop, we're just using a `while` so we can break out of it
    while (reader.isValidFile) {
        // META
        bool readMeta = reader.startSection(SAV_META_ID, SAV_META_VERSION);
        if (readMeta) {
            SAVSection_Meta* meta = reader.readStruct<SAVSection_Meta>(0);
            (void)meta;
            // TODO: Load!
        } else {
            break;
        }

        // And we're done!
        succeeded = true;
        break;
    }

    return succeeded;
}
