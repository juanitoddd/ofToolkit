#include "ProjectorApp.h"

void ProjectorApp::setup() {
	stateListener = appState->stateChanged.newListener([this](int & index) {
		onStateChanged(index);
	});
}

void ProjectorApp::onStateChanged(int & index) {
	ofLogNotice("ProjectorApp") << "State changed to " << index;
}

void ProjectorApp::draw() {
	engine_->renderProjector(appState->getState());
}
