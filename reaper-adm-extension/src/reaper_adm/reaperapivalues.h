#pragma once

enum I_SENDMODE {
    PostFaderPostFx = 0,
    PreFaderPreFx = 1,
    PreFaderPostFx = 3
};

enum TrackRouteTarget {
    onReceive = -1,
    onSend = 0,
    onHwOutput = 1
};

enum TrackFXAddMode {
    CreateNew = -1,
    QueryPresence = 0,
    CreateIfMissing = 1
};

enum TrackMoveMode {
    Normal = 0,
    AsChildTrack = 1,
    InToFolder = 2
};

enum EnvelopeShape {
    Linear = 0,
    Square = 1,
    SlowStartEnd = 2,
    FastStart = 3,
    FastEnd = 4,
    Bezier = 5
};
