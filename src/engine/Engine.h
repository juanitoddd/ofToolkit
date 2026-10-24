#pragma once
#include "../state/AppState.h"
#include "ofMain.h"
#include <memory>
#include <string>

class Engine {
public:
	Engine(std::shared_ptr<AppState> state);

	void setup();
	void update();

	void renderProjector(State state);
	void scanFonts();

	ofEvent<State> onRequestState;

private:
	std::shared_ptr<AppState> appState_;
	std::vector<ofTrueTypeFont> fonts_;
	ofFbo textPreviewFbo_;

	void renderText();
	void renderIdle();
};
