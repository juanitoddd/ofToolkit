#include "Engine.h"
#include "ofFileUtils.h"
#include "ofUtils.h"

Engine::Engine(std::shared_ptr<AppState> state)
	: appState_(std::move(state)) { }

void Engine::setup() {
	ofClear(0, 0, 0, 255);
}

void Engine::update() {
}