#include "GuiApp.h"

void GuiApp::setup() {
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	gui.setup();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::GetIO().MouseDrawCursor = false;

	// Initialize text buffer from shared state
	strncpy(textBuffer_, appState->textContent.c_str(), sizeof(textBuffer_) - 1);
	textBuffer_[sizeof(textBuffer_) - 1] = '\0';
}

void GuiApp::update() {
	engine_->update();
}

void GuiApp::drawTextPanel() {
	ImGui::SetNextWindowSize(ofVec2f(400, 300), ImGuiCond_FirstUseEver);
	ImGui::Begin("Text Editor");

	// Text input
	ImGui::Text("Content:");
	if (ImGui::InputTextMultiline("##text", textBuffer_, sizeof(textBuffer_), ImVec2(-1, 80))) {
		appState->textContent = textBuffer_;
	}

	// Color picker
	ImGui::Text("Color:");
	float color[4] = {
		appState->textColor.r / 255.0f,
		appState->textColor.g / 255.0f,
		appState->textColor.b / 255.0f,
		appState->textColor.a / 255.0f
	};
	if (ImGui::ColorEdit4("##color", color)) {
		appState->textColor = ofColor(
			color[0] * 255.0f,
			color[1] * 255.0f,
			color[2] * 255.0f,
			color[3] * 255.0f
		);
	}

	// Font dropdown
	ImGui::Text("Font:");
	if (appState->fontPaths.empty()) {
		ImGui::TextDisabled("No fonts found. Drop .ttf/.otf files into bin/data/fonts/");
	} else {
		std::string preview = appState->fontPaths.empty() ? "None" : ofFilePath::getFileName(appState->fontPaths[appState->fontIndex]);
		if (ImGui::BeginCombo("##font", preview.c_str())) {
			for (int i = 0; i < static_cast<int>(appState->fontPaths.size()); ++i) {
				bool selected = (appState->fontIndex == i);
				std::string label = ofFilePath::getFileName(appState->fontPaths[i]);
				if (ImGui::Selectable(label.c_str(), selected)) {
					appState->fontIndex = i;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}

	// Live preview
	ImGui::Separator();
	ImGui::Text("Preview:");
	ImVec4 previewColor(
		appState->textColor.r / 255.0f,
		appState->textColor.g / 255.0f,
		appState->textColor.b / 255.0f,
		appState->textColor.a / 255.0f
	);
	ImGui::PushStyleColor(ImGuiCol_Text, previewColor);
	ImGui::TextWrapped("%s", appState->textContent.c_str());
	ImGui::PopStyleColor();

	ImGui::End();
}

void GuiApp::drawImagePanel() {
	ImGui::SetNextWindowSize(ofVec2f(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Image Viewer");

	if (appState->imagePaths.empty()) {
		ImGui::TextDisabled("No images found. Drop files into bin/data/images/");
	} else {
		// File list
		ImGui::Text("Files (%lu):", appState->imagePaths.size());
		if (ImGui::BeginListBox("##images", ImVec2(-1, 120))) {
			for (int i = 0; i < static_cast<int>(appState->imagePaths.size()); ++i) {
				bool selected = (appState->imageIndex == i);
				std::string label = ofFilePath::getFileName(appState->imagePaths[i]);
				if (ImGui::Selectable(label.c_str(), selected)) {
					appState->imageIndex = i;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndListBox();
		}

		// Reload thumbnail if selection changed
		if (appState->imageIndex != lastImageIndex_) {
			lastImageIndex_ = appState->imageIndex;
			if (appState->imageIndex >= 0 && appState->imageIndex < static_cast<int>(appState->imagePaths.size())) {
				thumbnailImage_.load(appState->imagePaths[appState->imageIndex]);
			}
		}

		// Thumbnail + info
		if (thumbnailImage_.isAllocated()) {
			ImGui::Separator();
			ImGui::Text("Dimensions: %dx%d", thumbnailImage_.getWidth(), thumbnailImage_.getHeight());

			float maxW = 350.0f;
			float w = thumbnailImage_.getWidth();
			float h = thumbnailImage_.getHeight();
			float scale = std::min(maxW / w, 300.0f / h);
			float drawW = w * scale;
			float drawH = h * scale;
			ImGui::Image((ImTextureID)(uintptr_t)thumbnailImage_.getTexture().getTextureData().textureID, ImVec2(drawW, drawH));
		}
	}

	ImGui::End();
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
		ImGui::SetNextWindowSize(ofVec2f(200, 100), ImGuiCond_FirstUseEver);
		ImGui::Begin("Debug HUD", &appState->showDebugHud);
		ImGui::Text("FPS: %.1f", ofGetFrameRate());
		ImGui::Text("Window: %dx%d", ofGetWidth(), ofGetHeight());
		ImGui::Text("State: %s", appState->toString(appState->mode).data());
		ImGui::End();
	}

	// STATE_TEXT panel
	if (appState->mode == State::STATE_TEXT) {
		drawTextPanel();
	}

	// STATE_IMAGE panel
	if (appState->mode == State::STATE_IMAGE) {
		drawImagePanel();
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
