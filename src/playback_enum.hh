#ifndef INCLUDED_playback_enum_HH
#define INCLUDED_playback_enum_HH

enum PlaybackEventType {
	PlaybackEventType_Play = 0,
	PlaybackEventType_Stop,
	PlaybackEventType_Rewind,
	PlaybackEventType_Fwd,
	PlaybackEventType_RewindAll,
	PlaybackEventType_FwdAll,
};

#endif
