#pragma once
#include "ofMain.h"
#include <array>
#include <string_view>

enum State {
	STATE_IDLE,
	STATE_TEXT,
	STATE_IMAGE,
	STATE_VIDEO,
	STATE_CAMERA,
	STATE_SOUND,
	STATE_MESH
};

inline constexpr std::array<std::string_view, 8> StateNames = {
	"Idle",
	"Text",
	"Image",
	"Video",
	"Camera",
	"Sound",
	"Mesh",
};

class AppState {
public:
	// Global State
	State mode = State::STATE_IDLE;
	ofEvent<int> stateChanged;

	// Get State names
	inline std::string_view toString(State mode) {
		auto index = static_cast<size_t>(mode);
		if (index >= StateNames.size()) return "Unknown";
		return StateNames[index];
	}

	void setState(int index) {
		mode = static_cast<State>(index);
		ofLogNotice("AppState") << "State changed to " << toString(mode);
		ofNotifyEvent(stateChanged, index);
	}

	// Read the current mode
	State getState() const {
		return mode;
	}
};