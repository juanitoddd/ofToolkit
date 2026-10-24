#pragma once
#include "../engine/Engine.h"
#include "../state/AppState.h"
#include "ofMain.h"
#include <memory>

class ProjectorApp : public ofBaseApp {
public:
	ProjectorApp(std::shared_ptr<AppState> state, std::shared_ptr<Engine> engine)
		: appState(std::move(state))
		, engine_(std::move(engine)) { }
	void setup() override;
	void draw() override;
	void mouseDragged(int x, int y, int button) override;
	void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
	void onStateChanged(int & index);

	ofColor backgroundColor = ofColor(46, 52, 64);

private:
	std::shared_ptr<AppState> appState;
	std::shared_ptr<Engine> engine_;
	ofEventListener stateListener;
};
