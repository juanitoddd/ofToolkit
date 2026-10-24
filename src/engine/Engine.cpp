#include "Engine.h"
#include "ofFileUtils.h"
#include "ofUtils.h"

Engine::Engine(std::shared_ptr<AppState> state)
	: appState_(std::move(state)) { }

void Engine::setup() {
	ofClear(0, 0, 0, 255);
	scanFonts();
	if (appState_->fontPaths.empty()) {
		ofLogWarning("Engine") << "No fonts found in bin/data/fonts/";
	}
}

void Engine::update() {
}

void Engine::scanFonts() {
	ofDirectory dir("fonts");
	dir.allowExt("ttf");
	dir.allowExt("otf");
	dir.listDir();
	appState_->fontPaths.clear();
	for (int i = 0; i < dir.size(); ++i) {
		appState_->fontPaths.push_back(dir.getPath(i));
	}
	appState_->fontIndex = 0;
	fonts_.clear();
	fonts_.resize(appState_->fontPaths.size());
}

void Engine::renderIdle() {
	ofSetBackgroundColor(ofColor(46, 52, 64));
	ofSetColor(ofColor(216, 222, 233));
	std::string label = "Idle";
	float x = ofGetWidth() / 2.0f - label.length() * 4.0f;
	float y = ofGetHeight() / 2.0f;
	ofDrawBitmapString(label, x, y);
}

void Engine::renderText() {
	ofSetBackgroundColor(ofColor(46, 52, 64));
	ofSetColor(appState_->textColor);

	const std::string& text = appState_->textContent;
	float x = ofGetWidth() / 2.0f;
	float y = ofGetHeight() / 2.0f;

	int idx = appState_->fontIndex;
	if (!appState_->fontPaths.empty() && idx >= 0 && idx < static_cast<int>(appState_->fontPaths.size())) {
		if (!fonts_[idx].isLoaded()) {
			fonts_[idx].load(appState_->fontPaths[idx], 48);
		}
		if (fonts_[idx].isLoaded()) {
			ofRectangle bounds = fonts_[idx].getStringBoundingBox(text, 0, 0);
			x -= bounds.width / 2.0f;
			y -= bounds.height / 2.0f;
			fonts_[idx].drawString(text, x, y);
			return;
		}
	}

	// Fallback to bitmap string
	x -= text.length() * 4.0f;
	ofDrawBitmapString(text, x, y);
}

void Engine::renderProjector(State state) {
	switch (state) {
	case State::STATE_IDLE:
		renderIdle();
		break;
	case State::STATE_TEXT:
		renderText();
		break;
	default:
		ofSetBackgroundColor(ofColor(46, 52, 64));
		break;
	}
}
