#pragma once

namespace admplug {
enum class ImportStatus {
    INIT,
    STARTING,
    PARSING_METADATA,
    METADATA_PARSED,
    EXTRACTING_AUDIO,
    AUDIO_READY,
    CREATING_REAPER_ELEMENTS,
    COMPLETE,
    CANCELLED,
    ERROR_OCCURRED
};
}
