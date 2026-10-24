#include "GuiApp.h"

void GuiApp::setup() {
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	gui.setup();
	ofLogNotice() << "Dear ImGui version: " << ImGui::GetVersion();
	ofLogNotice() << "OpenGL version: " << (const char *)glGetString(GL_VERSION);
	ofLogNotice() << "OpenGL renderer: " << (const char *)glGetString(GL_RENDERER);
	ImGuiIO & io = ImGui::GetIO();
	ofLogNotice() << "ImGui renderer backend: " << (io.BackendRendererName ? io.BackendRendererName : "Not initialized");
	ofLogNotice() << "ImGui platform backend: " << (io.BackendPlatformName ? io.BackendPlatformName : "Not initialized");
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::GetIO().MouseDrawCursor = false;
	ofDisableArbTex(); // Before loading the image
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
			color[3] * 255.0f);
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
		appState->textColor.a / 255.0f);
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
				ofLogNotice("GuiApp") << "imagePaths: " << appState->imagePaths[appState->imageIndex];
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
			GLuint textureID = thumbnailImage_.getTexture().getTextureData().textureID;
			ImTextureID id = (ImTextureID)(intptr_t)textureID;
			ImTextureRef ref(id);
			ImGui::Image(ref, ImVec2(drawW, drawH));
		}
	}

	ImGui::End();
}

void GuiApp::drawVideoPanel() {
	ImGui::SetNextWindowSize(ofVec2f(400, 350), ImGuiCond_FirstUseEver);
	ImGui::Begin("Video Player");

	if (appState->videoPaths.empty()) {
		ImGui::TextDisabled("No videos found. Drop files into bin/data/videos/");
	} else {
		// File list
		ImGui::Text("Files (%lu):", appState->videoPaths.size());
		if (ImGui::BeginListBox("##videos", ImVec2(-1, 100))) {
			for (int i = 0; i < static_cast<int>(appState->videoPaths.size()); ++i) {
				bool selected = (appState->videoIndex == i);
				std::string label = ofFilePath::getFileName(appState->videoPaths[i]);
				if (ImGui::Selectable(label.c_str(), selected)) {
					appState->videoIndex = i;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndListBox();
		}

		// Stop current video when selection changes
		if (appState->videoIndex != lastVideoIndex_) {
			lastVideoIndex_ = appState->videoIndex;
			engine_->stopVideo();
			videoSeekDragging_ = false;
		}

		// Playback controls
		ImGui::Separator();
		if (ImGui::Button("Play")) {
			engine_->playVideo();
		}
		ImGui::SameLine();
		if (ImGui::Button("Pause")) {
			engine_->pauseVideo();
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop")) {
			engine_->stopVideo();
		}

		// Volume
		static float volume = 1.0f;
		if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f, "%.2f")) {
			engine_->setVideoVolume(volume);
		}

		// Seek bar
		static float draggedSeek = 0.0f;
		if (!videoSeekDragging_) {
			draggedSeek = engine_->getVideoPosition();
		}
		if (ImGui::SliderFloat("Seek", &draggedSeek, 0.0f, 1.0f, "")) {
			videoSeekDragging_ = true;
		}
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			engine_->seekVideo(draggedSeek);
			videoSeekDragging_ = false;
		}

		// Time labels
		float pos = engine_->getVideoPosition();
		float dur = engine_->getVideoDuration();
		int posMins = static_cast<int>(pos * dur) / 60;
		int posSecs = static_cast<int>(pos * dur) % 60;
		int durMins = static_cast<int>(dur) / 60;
		int durSecs = static_cast<int>(dur) % 60;
		ImGui::Text("%d:%02d / %d:%02d", posMins, posSecs, durMins, durSecs);
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

	// STATE_VIDEO panel
	if (appState->mode == State::STATE_VIDEO) {
		drawVideoPanel();
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
