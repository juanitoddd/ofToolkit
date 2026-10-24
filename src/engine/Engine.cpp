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

void Engine::renderProjector(State state) {
	switch (state) {
	case State::STATE_IDLE: {
		ofSetBackgroundColor(ofColor(46, 52, 64));
		ofSetColor(ofColor(216, 222, 233));
		std::string label = "Idle";
		float x = ofGetWidth() / 2.0f - label.length() * 4.0f;
		float y = ofGetHeight() / 2.0f;
		ofDrawBitmapString(label, x, y);
		break;
	}
	default:
		ofSetBackgroundColor(ofColor(46, 52, 64));
		break;
	}
}
