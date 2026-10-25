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

void ProjectorApp::mouseDragged(int x, int y, int button) {
	if (appState->getState() == State::STATE_MESH) {
		// Orbit: left button rotates Y/X, right button rotates Z
		float sensitivity = 0.5f;
		if (button == 0) {
			appState->meshRotY += (x - ofGetPreviousMouseX()) * sensitivity;
			appState->meshRotX += (y - ofGetPreviousMouseY()) * sensitivity;
		} else if (button == 2) {
			appState->meshRotZ += (x - ofGetPreviousMouseX()) * sensitivity;
		}
	}
}

void ProjectorApp::mouseScrolled(int x, int y, float scrollX, float scrollY) {
	if (appState->getState() == State::STATE_MESH) {
		float sensitivity = 0.05f;
		appState->meshZoom += scrollY * sensitivity;
		appState->meshZoom = ofClamp(appState->meshZoom, 0.1f, 5.0f);
	}
}
