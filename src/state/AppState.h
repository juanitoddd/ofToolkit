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

	// Debug HUD toggle (cross-cutting)
	bool showDebugHud = true;

	// STATE_TEXT properties
	std::string textContent = "Hello, World!";
	ofColor textColor = ofColor(255, 255, 255);
	std::vector<std::string> fontPaths;
	int fontIndex = 0;

	// STATE_IMAGE properties
	std::vector<std::string> imagePaths;
	int imageIndex = 0;

	// STATE_VIDEO properties
	std::vector<std::string> videoPaths;
	int videoIndex = 0;

	// STATE_CAMERA properties
	std::vector<std::string> cameraDeviceNames;
	std::vector<int> cameraDeviceIDs;
	int cameraDeviceIndex = 0;
	int cameraResolutionIndex = 1; // default 640x480

	// STATE_SOUND properties
	std::vector<std::string> soundPaths;
	int soundIndex = 0;

	// STATE_MESH properties
	std::vector<std::string> meshPaths;
	int meshIndex = 0;
	float meshRotX = 0.0f;
	float meshRotY = 0.0f;
	float meshRotZ = 0.0f;
	float meshZoom = 1.0f;
	bool meshWireframe = false;

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