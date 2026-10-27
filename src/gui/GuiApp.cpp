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
			ImGui::Text("Dimensions: %fx%f", thumbnailImage_.getWidth(), thumbnailImage_.getHeight());

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

		// Video preview
		GLuint videoTexID = engine_->getVideoTextureID();
		if (videoTexID != 0) {
			ImGui::Separator();
			ImGui::Text("Preview:");
			float maxW = 350.0f;
			float maxH = 250.0f;
			// Default aspect ratio 16:9
			float drawW = maxW;
			float drawH = maxW * 9.0f / 16.0f;
			if (drawH > maxH) {
				float scale = maxH / drawH;
				drawW *= scale;
				drawH *= scale;
			}
			ImTextureID id = (ImTextureID)(intptr_t)videoTexID;
			ImTextureRef ref(id);
			ImGui::Image(ref, ImVec2(drawW, drawH));
		}
	}

	ImGui::End();
}

void GuiApp::drawCameraPanel() {
	ImGui::SetNextWindowSize(ofVec2f(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Camera Feed");

	if (appState->cameraDeviceNames.empty()) {
		ImGui::TextDisabled("No camera found.");
	} else {
		// Device dropdown
		ImGui::Text("Device:");
		std::string devicePreview = appState->cameraDeviceNames[appState->cameraDeviceIndex];
		if (ImGui::BeginCombo("##device", devicePreview.c_str())) {
			for (int i = 0; i < static_cast<int>(appState->cameraDeviceNames.size()); ++i) {
				bool selected = (appState->cameraDeviceIndex == i);
				if (ImGui::Selectable(appState->cameraDeviceNames[i].c_str(), selected)) {
					appState->cameraDeviceIndex = i;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		// Resolution dropdown
		static const char * resolutionLabels[] = { "320x240", "640x480", "1280x720", "1920x1080" };
		ImGui::Text("Resolution:");
		if (ImGui::BeginCombo("##resolution", resolutionLabels[appState->cameraResolutionIndex])) {
			for (int i = 0; i < 4; ++i) {
				bool selected = (appState->cameraResolutionIndex == i);
				if (ImGui::Selectable(resolutionLabels[i], selected)) {
					appState->cameraResolutionIndex = i;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		// Re-init camera when device or resolution changes
		if (appState->cameraDeviceIndex != lastCameraDeviceIndex_ || appState->cameraResolutionIndex != lastCameraResolutionIndex_) {
			lastCameraDeviceIndex_ = appState->cameraDeviceIndex;
			lastCameraResolutionIndex_ = appState->cameraResolutionIndex;
			engine_->closeCamera();
		}

		// Live preview
		GLuint camTexID = engine_->getCameraTextureID();
		if (camTexID != 0) {
			ImGui::Separator();
			ImGui::Text("Preview:");
			float maxW = 350.0f;
			float maxH = 250.0f;
			float drawW = maxW;
			float drawH = maxW * 3.0f / 4.0f; // camera typical 4:3
			if (drawH > maxH) {
				float scale = maxH / drawH;
				drawW *= scale;
				drawH *= scale;
			}
			ImTextureID id = (ImTextureID)(intptr_t)camTexID;
			ImTextureRef ref(id);
			ImGui::Image(ref, ImVec2(drawW, drawH));
		} else {
			ImGui::TextDisabled("Camera initializing...");
		}
	}

	ImGui::End();
}

void GuiApp::drawSoundPanel() {
	ImGui::SetNextWindowSize(ofVec2f(400, 450), ImGuiCond_FirstUseEver);
	ImGui::Begin("Sound Player");

	if (appState->soundPaths.empty()) {
		ImGui::TextDisabled("No sounds found. Drop files into bin/data/sounds/");
	} else {
		// File list
		ImGui::Text("Files (%lu):", appState->soundPaths.size());
		if (ImGui::BeginListBox("##sounds", ImVec2(-1, 100))) {
			for (int i = 0; i < static_cast<int>(appState->soundPaths.size()); ++i) {
				bool selected = (appState->soundIndex == i);
				std::string label = ofFilePath::getFileName(appState->soundPaths[i]);
				if (ImGui::Selectable(label.c_str(), selected)) {
					appState->soundIndex = i;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndListBox();
		}

		// Stop current sound when selection changes
		if (appState->soundIndex != lastSoundIndex_) {
			lastSoundIndex_ = appState->soundIndex;
			engine_->stopSound();
		}

		// Playback controls
		ImGui::Separator();
		if (ImGui::Button("Play")) {
			engine_->playSound();
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop")) {
			engine_->stopSound();
		}

		// Volume
		static float soundVolume = 1.0f;
		if (ImGui::SliderFloat("Volume", &soundVolume, 0.0f, 1.0f, "%.2f")) {
			engine_->setSoundVolume(soundVolume);
		}

		// FFT visualizer
		ImGui::Separator();
		ImGui::Text("Visualizer:");
		const std::vector<float> & fft = engine_->getSoundFFT();
		if (!fft.empty()) {
			float availW = ImGui::GetContentRegionAvail().x - 10.0f;
			float barW = availW / static_cast<float>(fft.size());
			float height = 120.0f;
			ImDrawList * drawList = ImGui::GetWindowDrawList();
			ImVec2 canvasPos = ImGui::GetCursorScreenPos();
			for (size_t i = 0; i < fft.size(); ++i) {
				float h = fft[i] * height * 3.0f;
				h = std::min(h, height);
				ImVec2 pMin(
					canvasPos.x + i * barW + 1.0f,
					canvasPos.y + height - h);
				ImVec2 pMax(
					canvasPos.x + (i + 1) * barW - 1.0f,
					canvasPos.y + height);
				ImU32 col = ImColor(129, 199, 132);
				drawList->AddRectFilled(pMin, pMax, col);
			}
			ImGui::Dummy(ImVec2(0, height + 5));
		}
	}

	ImGui::End();
}

void GuiApp::drawMeshPanel() {
	ImGui::SetNextWindowSize(ofVec2f(400, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("3D Mesh Viewer");

	if (appState->meshPaths.empty()) {
		ImGui::TextDisabled("No meshes found.");
	} else {
		// File/primitive list
		ImGui::Text("Meshes (%lu):", appState->meshPaths.size());
		if (ImGui::BeginListBox("##meshes", ImVec2(-1, 120))) {
			for (int i = 0; i < static_cast<int>(appState->meshPaths.size()); ++i) {
				bool selected = (appState->meshIndex == i);
				std::string label;
				if (appState->meshPaths[i].find("primitive:") == 0) {
					label = appState->meshPaths[i].substr(10);
				} else {
					label = ofFilePath::getFileName(appState->meshPaths[i]);
				}
				if (ImGui::Selectable(label.c_str(), selected)) {
					appState->meshIndex = i;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndListBox();
		}

		// Wireframe toggle
		ImGui::Separator();
		ImGui::Checkbox("Wireframe", &appState->meshWireframe);

		// Orbit controls
		ImGui::Text("Orbit:");
		ImGui::SliderFloat("Rotate X", &appState->meshRotX, -180.0f, 180.0f);
		ImGui::SliderFloat("Rotate Y", &appState->meshRotY, -180.0f, 180.0f);
		ImGui::SliderFloat("Rotate Z", &appState->meshRotZ, -180.0f, 180.0f);
		ImGui::SliderFloat("Zoom", &appState->meshZoom, 0.1f, 5.0f, "%.2f");
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
			for (int i = 0; i <= static_cast<int>(State::STATE_NDI_SEND); ++i) {
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

	// STATE_CAMERA panel
	if (appState->mode == State::STATE_CAMERA) {
		drawCameraPanel();
	}

	// STATE_SOUND panel
	if (appState->mode == State::STATE_SOUND) {
		drawSoundPanel();
	}

	// STATE_MESH panel
	if (appState->mode == State::STATE_MESH) {
		drawMeshPanel();
	}

	// STATE_NDI_RECV panel
	if (appState->mode == State::STATE_NDI_RECV) {
		drawNDIRecvPanel();
	}

	// STATE_NDI_SEND panel
	if (appState->mode == State::STATE_NDI_SEND) {
		drawNDISendPanel();
	}

	gui.end();
}

void GuiApp::drawNDIRecvPanel() {
	ImGui::SetNextWindowSize(ofVec2f(400, 450), ImGuiCond_FirstUseEver);
	ImGui::Begin("NDI Receiver");

	// Refresh sources button
	if (ImGui::Button("Refresh Sources")) {
		engine_->scanNDISources();
	}
	ImGui::SameLine();
	if (appState->ndiRecvConnected) {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
	} else {
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Disconnected");
	}

	if (appState->ndiSourceNames.empty()) {
		ImGui::TextDisabled("No NDI sources found on the network.");
	} else {
		ImGui::Text("Sources (%lu):", appState->ndiSourceNames.size());
		if (ImGui::BeginListBox("##ndiSources", ImVec2(-1, 120))) {
			for (int i = 0; i < static_cast<int>(appState->ndiSourceNames.size()); ++i) {
				bool selected = (appState->ndiSourceIndex == i);
				if (ImGui::Selectable(appState->ndiSourceNames[i].c_str(), selected)) {
					appState->ndiSourceIndex = i;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndListBox();
		}

		if (ImGui::Button("Connect")) {
			engine_->selectNDISource(appState->ndiSourceIndex);
		}
		ImGui::SameLine();
		if (ImGui::Button("Disconnect")) {
			engine_->disconnectNDISource();
		}
	}

	// Preview
	GLuint texID = engine_->getNDIRecvTextureID();
	if (texID != 0) {
		ImGui::Separator();
		ImGui::Text("Preview:");
		float maxW = 350.0f;
		float maxH = 220.0f;
		// Default 16:9
		float drawW = maxW;
		float drawH = maxW * 9.0f / 16.0f;
		if (drawH > maxH) {
			float scale = maxH / drawH;
			drawW *= scale;
			drawH *= scale;
		}
		ImTextureID id = (ImTextureID)(intptr_t)texID;
		ImTextureRef ref(id);
		ImGui::Image(ref, ImVec2(drawW, drawH));
	}

	ImGui::End();
}

void GuiApp::drawNDISendPanel() {
	ImGui::SetNextWindowSize(ofVec2f(400, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("NDI Sender");

	// Camera preview
	GLuint camTexID = engine_->getCameraTextureID();
	if (camTexID != 0) {
		ImGui::Text("Camera Preview:");
		float maxW = 350.0f;
		float maxH = 220.0f;
		float drawW = maxW;
		float drawH = maxW * 3.0f / 4.0f;
		if (drawH > maxH) {
			float scale = maxH / drawH;
			drawW *= scale;
			drawH *= scale;
		}
		ImTextureID id = (ImTextureID)(intptr_t)camTexID;
		ImTextureRef ref(id);
		ImGui::Image(ref, ImVec2(drawW, drawH));
		ImGui::Separator();
	}

	// Sender name
	char nameBuf[256];
	strncpy(nameBuf, appState->ndiSendName.c_str(), sizeof(nameBuf) - 1);
	nameBuf[sizeof(nameBuf) - 1] = '\0';
	ImGui::Text("Sender Name:");
	if (ImGui::InputText("##ndiName", nameBuf, sizeof(nameBuf))) {
		appState->ndiSendName = nameBuf;
	}

	// Controls
	ImGui::Separator();
	if (!appState->ndiSendActive) {
		if (ImGui::Button("Start Broadcasting")) {
			engine_->startNDISender();
		}
	} else {
		if (ImGui::Button("Stop Broadcasting")) {
			engine_->stopNDISender();
		}
	}

	// Status
	ImGui::Separator();
	if (appState->ndiSendActive) {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Broadcasting");
		ImGui::Text("Connections: %d", appState->ndiSendConnections);
	} else {
		ImGui::TextDisabled("Not broadcasting");
	}

	ImGui::End();
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
