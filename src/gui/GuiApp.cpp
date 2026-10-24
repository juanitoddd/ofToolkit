#include "GuiApp.h"

void GuiApp::setup() {
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	gui.setup();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::GetIO().MouseDrawCursor = false;
}

void GuiApp::update() {
	engine_->update();
}

void GuiApp::draw() {
	ofSetBackgroundColor(backgroundColor);

	// --- ImGui overlay on top ---
	gui.begin();

	if (ImGui::BeginMainMenuBar()) {
		ImGui::Text("| %s |", appState->toString(appState->mode).data());
		if (ImGui::BeginMenu("State")) {
			for (int i = 0; i <= static_cast<int>(State::STATE_MESH); ++i) {
				State state = static_cast<State>(i);
				if (ImGui::MenuItem(appState->toString(state).data())) {
					appState->setState(static_cast<int>(state));
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Debug")) {
			ImGui::Checkbox("Show Debug HUD", &appState->showDebugHud);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	// Debug HUD floating window
	if (appState->showDebugHud) {
		if (ImGui::Begin("Debug HUD", &appState->showDebugHud, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("FPS: %.1f", ofGetFrameRate());
			ImGui::Text("Window: %dx%d", ofGetWidth(), ofGetHeight());
			ImGui::Text("State: %s", appState->toString(appState->mode).data());
			ImGui::End();
		}
	}

	gui.end();
}

void GuiApp::keyPressed(int key) { }
void GuiApp::keyReleased(int key) { }
void GuiApp::mouseMoved(int x, int y) { }
void GuiApp::mouseDragged(int x, int y, int button) { }
void GuiApp::mousePressed(int x, int y, int button) { }
void GuiApp::mouseReleased(int x, int y, int button) { }
void GuiApp::mouseScrolled(int x, int y, float scrollX, float scrollY) { }
void GuiApp::mouseEntered(int x, int y) { }
void GuiApp::mouseExited(int x, int y) { }
void GuiApp::windowResized(int w, int h) { }
void GuiApp::gotMessage(ofMessage msg) { }
void GuiApp::dragEvent(ofDragInfo dragInfo) { }
void GuiApp::exit() { }
