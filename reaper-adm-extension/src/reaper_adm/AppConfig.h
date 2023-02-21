#pragma once

#define JUCE_WEB_BROWSER 0
#define JUCE_ALSA 0
#define JUCE_JACK 0
#define JUCE_USE_CURL 0

#ifndef JUCE_DISPLAY_SPLASH_SCREEN
#define JUCE_DISPLAY_SPLASH_SCREEN 0
#endif

#ifndef JUCE_REPORT_APP_USAGE
#define JUCE_REPORT_APP_USAGE 0
#endif

#define JUCE_USE_DARK_SPLASH_SCREEN 1

#define JUCE_MODULE_AVAILABLE_juce_audio_basics          1
#define JUCE_MODULE_AVAILABLE_juce_audio_devices         1
#define JUCE_MODULE_AVAILABLE_juce_audio_formats         1
#define JUCE_MODULE_AVAILABLE_juce_audio_processors      1
#define JUCE_MODULE_AVAILABLE_juce_core                  1
#define JUCE_MODULE_AVAILABLE_juce_cryptography          1
#define JUCE_MODULE_AVAILABLE_juce_data_structures       1
#define JUCE_MODULE_AVAILABLE_juce_events                1
#define JUCE_MODULE_AVAILABLE_juce_graphics              1
#define JUCE_MODULE_AVAILABLE_juce_gui_basics            1
#define JUCE_MODULE_AVAILABLE_juce_gui_extra             1
#define JUCE_MODULE_AVAILABLE_juce_opengl                1

#define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 1

#ifndef    JUCE_STRICT_REFCOUNTEDPOINTER
#define   JUCE_STRICT_REFCOUNTEDPOINTER 1
#endif

#ifndef    JUCE_STANDALONE_APPLICATION
#define  JUCE_STANDALONE_APPLICATION 1
#endif
