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

	ofEvent<State> onRequestState;

private:
	std::shared_ptr<AppState> appState_;
};
