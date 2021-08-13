#pragma once

namespace ear {
namespace plugin {
namespace detail {
#ifdef WIN32
static const char* SCENE_MASTER_CONTROL_ENDPOINT =
    "ipc://tmp/ear-production-suite-sm";

static const char* SCENE_MASTER_METADATA_ENDPOINT =
    "ipc://tmp/ear-production-suite-sm-meta";

static const char* SCENE_MASTER_SCENE_STREAM_ENDPOINT =
    "ipc://tmp/ear-production-suite-sm-scene";

#else
static const char* SCENE_MASTER_CONTROL_ENDPOINT =
    "ipc:///tmp/ear-production-suite-sm";

static const char* SCENE_MASTER_METADATA_ENDPOINT =
    "ipc:///tmp/ear-production-suite-sm-meta";

static const char* SCENE_MASTER_SCENE_STREAM_ENDPOINT =
    "ipc:///tmp/ear-production-suite-sm-scene";
#endif

static const char* MORE_INFO_URL =
    "https://ear-production-suite.ebu.io/";
}  // namespace detail
}  // namespace plugin
}  // namespace ear
