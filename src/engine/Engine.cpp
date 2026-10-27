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
	scanImages();
	if (appState_->imagePaths.empty()) {
		ofLogWarning("Engine") << "No images found in bin/data/images/";
	}
	scanVideos();
	if (appState_->videoPaths.empty()) {
		ofLogWarning("Engine") << "No videos found in bin/data/videos/";
	}
	scanCameraDevices();
	scanSounds();
	if (appState_->soundPaths.empty()) {
		ofLogWarning("Engine") << "No sounds found in bin/data/sounds/";
	}
	scanMeshes();
	fft_.assign(128, 0.0f);

	// NDI setup — receiver is opened lazily by ReceiveImage
	scanNDISources();

	// 3D lighting setup
	light1_ = std::make_unique<ofLight>();
	light1_->setPosition(200, 200, 200);
	light1_->setDiffuseColor(ofColor(255, 255, 255));
	light1_->enable();
	light2_ = std::make_unique<ofLight>();
	light2_->setPosition(-200, -200, 200);
	light2_->setDiffuseColor(ofColor(200, 200, 220));
	light2_->enable();
}

void Engine::update() {
	State current = appState_->getState();

	// Pause video when switching away from STATE_VIDEO
	if (previousState_ == State::STATE_VIDEO && current != State::STATE_VIDEO) {
		if (videoPlayer_.isLoaded()) {
			videoPlayer_.setPaused(true);
		}
	}

	// Stop camera when switching away from camera-using states
	bool wasUsingCamera = (previousState_ == State::STATE_CAMERA || previousState_ == State::STATE_NDI_SEND);
	bool isUsingCamera = (current == State::STATE_CAMERA || current == State::STATE_NDI_SEND);
	if (wasUsingCamera && !isUsingCamera) {
		closeCamera();
	}

	// Stop sound when switching away from STATE_SOUND
	if (previousState_ == State::STATE_SOUND && current != State::STATE_SOUND) {
		if (soundPlayer_.isLoaded()) {
			soundPlayer_.stop();
		}
	}

	// Init/start camera when entering camera-using states
	if (isUsingCamera) {
		if (!cameraInitialized_) {
			initCamera();
		}
		if (cameraInitialized_) {
			grabber_.update();
		}
	}

	// Update video player
	if (current == State::STATE_VIDEO && videoPlayer_.isLoaded()) {
		videoPlayer_.update();
	}

	// Update sound FFT when in STATE_SOUND
	if (current == State::STATE_SOUND && soundPlayer_.isPlaying()) {
		float * spectrum = ofSoundGetSpectrum(128);
		for (size_t i = 0; i < 128; ++i) {
			fft_[i] = spectrum[i];
		}
	} else {
		// Decay FFT bars when not playing
		for (auto & v : fft_) {
			v *= 0.95f;
		}
	}

	// NDI state transitions
	if (previousState_ == State::STATE_NDI_RECV && current != State::STATE_NDI_RECV) {
		disconnectNDISource();
	}
	if (previousState_ == State::STATE_NDI_SEND && current != State::STATE_NDI_SEND) {
		stopNDISender();
	}
	if (current == State::STATE_NDI_RECV) {
		// Refresh NDI sources every 2 seconds
		uint64_t now = ofGetElapsedTimeMillis();
		if (now - lastNdiRefreshTime_ > 2000) {
			lastNdiRefreshTime_ = now;
			scanNDISources();
		}
		// ReceiveImage lazily creates/opens the receiver internally
		ndiReceiver_.ReceiveImage(ndiRecvFbo_);
		appState_->ndiRecvConnected = ndiReceiver_.ReceiverConnected();
	}

	previousState_ = current;
}

void Engine::scanFonts() {
	ofDirectory dir("fonts");
	dir.allowExt("ttf");
	dir.allowExt("otf");
	dir.listDir();
	appState_->fontPaths.clear();
	for (size_t i = 0; i < dir.size(); ++i) {
		appState_->fontPaths.push_back(dir.getPath(i));
	}
	appState_->fontIndex = 0;
	fonts_.clear();
	fonts_.resize(appState_->fontPaths.size());
}

void Engine::scanImages() {
	ofDirectory dir("images");
	dir.allowExt("jpg");
	dir.allowExt("jpeg");
	dir.allowExt("png");
	dir.allowExt("gif");
	dir.allowExt("bmp");
	dir.allowExt("webp");
	dir.listDir();
	appState_->imagePaths.clear();
	for (size_t i = 0; i < dir.size(); ++i) {
		appState_->imagePaths.push_back(dir.getPath(i));
	}
	appState_->imageIndex = 0;
}

void Engine::scanVideos() {
	ofDirectory dir("videos");
	dir.allowExt("mov");
	dir.allowExt("mp4");
	dir.allowExt("avi");
	dir.allowExt("mkv");
	dir.allowExt("m4v");
	dir.allowExt("ogv");
	dir.allowExt("mpg");
	dir.allowExt("mpeg");
	dir.allowExt("wmv");
	dir.allowExt("flv");
	dir.listDir();
	appState_->videoPaths.clear();
	for (size_t i = 0; i < dir.size(); ++i) {
		appState_->videoPaths.push_back(dir.getPath(i));
	}
	appState_->videoIndex = 0;
}

void Engine::scanCameraDevices() {
	auto devices = grabber_.listDevices();
	appState_->cameraDeviceNames.clear();
	appState_->cameraDeviceIDs.clear();
	for (const auto & dev : devices) {
		appState_->cameraDeviceNames.push_back(dev.deviceName);
		appState_->cameraDeviceIDs.push_back(dev.id);
	}
	appState_->cameraDeviceIndex = 0;
	if (!appState_->cameraDeviceNames.empty()) {
		ofLogNotice("Engine") << "Found " << appState_->cameraDeviceNames.size() << " camera device(s)";
	} else {
		ofLogWarning("Engine") << "No camera devices found";
	}
}

void Engine::scanSounds() {
	ofDirectory dir("sounds");
	dir.allowExt("mp3");
	dir.allowExt("wav");
	dir.allowExt("aiff");
	dir.allowExt("aif");
	dir.allowExt("flac");
	dir.allowExt("ogg");
	dir.allowExt("oga");
	dir.listDir();
	appState_->soundPaths.clear();
	for (size_t i = 0; i < dir.size(); ++i) {
		appState_->soundPaths.push_back(dir.getPath(i));
	}
	appState_->soundIndex = 0;
}

void Engine::scanMeshes() {
	ofDirectory dir("meshes");
	dir.allowExt("ply");
	dir.allowExt("obj");
	dir.allowExt("stl");
	dir.listDir();
	appState_->meshPaths.clear();
	for (size_t i = 0; i < dir.size(); ++i) {
		appState_->meshPaths.push_back(dir.getPath(i));
	}
	// Always add built-in primitives at the end
	meshPrimitiveNames_ = { "Box", "Sphere", "Cone", "Cylinder", "Plane" };
	for (const auto & name : meshPrimitiveNames_) {
		appState_->meshPaths.push_back("primitive:" + name);
	}
	appState_->meshIndex = 0;
}

void Engine::scanNDISources() {
	ndiReceiver_.FindSenders();
	appState_->ndiSourceNames = ndiReceiver_.GetSenderList();
	if (appState_->ndiSourceIndex >= static_cast<int>(appState_->ndiSourceNames.size())) {
		appState_->ndiSourceIndex = 0;
	}
}

void Engine::selectNDISource(int index) {
	if (appState_->ndiSourceNames.empty()) {
		ofLogWarning("Engine") << "No NDI sources to connect to";
		return;
	}
	if (index < 0 || index >= static_cast<int>(appState_->ndiSourceNames.size())) {
		index = 0;
	}
	if (ndiReceiver_.SetSenderIndex(index)) {
		appState_->ndiSourceIndex = index;
		appState_->ndiRecvConnected = true;
		ofLogNotice("Engine") << "NDI receiver selected: " << appState_->ndiSourceNames[index];
	} else {
		ofLogWarning("Engine") << "Failed to select NDI source " << index;
	}
}

void Engine::disconnectNDISource() {
	ndiReceiver_.ReleaseReceiver();
	appState_->ndiRecvConnected = false;
}

GLuint Engine::getNDIRecvTextureID() const {
	if (ndiRecvFbo_.isAllocated()) {
		return ndiRecvFbo_.getTexture().getTextureData().textureID;
	}
	return 0;
}

void Engine::startNDISender() {
	if (ndiSenderCreated_) return;
	int w = 1920;
	int h = 1080;
	if (!ndiSendFbo_.isAllocated()) {
		ndiSendFbo_.allocate(w, h, GL_RGBA);
	}
	if (ndiSender_.CreateSender(appState_->ndiSendName.c_str(), w, h)) {
		ndiSenderCreated_ = true;
		appState_->ndiSendActive = true;
		ofLogNotice("Engine") << "NDI sender created: " << appState_->ndiSendName;
	} else {
		ofLogWarning("Engine") << "Failed to create NDI sender";
	}
}

void Engine::stopNDISender() {
	if (ndiSenderCreated_) {
		ndiSender_.ReleaseSender();
		ndiSenderCreated_ = false;
		appState_->ndiSendActive = false;
		appState_->ndiSendConnections = 0;
	}
}

// Camera controls
void Engine::initCamera() {
	if (appState_->cameraDeviceNames.empty()) {
		ofLogWarning("Engine") << "Cannot init camera: no devices available";
		return;
	}
	if (cameraInitialized_) {
		closeCamera();
	}
	int idx = appState_->cameraDeviceIndex;
	if (idx < 0 || idx >= static_cast<int>(appState_->cameraDeviceIDs.size())) {
		idx = 0;
		appState_->cameraDeviceIndex = 0;
	}
	grabber_.setDeviceID(appState_->cameraDeviceIDs[idx]);

	static const std::pair<int, int> resolutions[] = {
		{ 320, 240 },
		{ 640, 480 },
		{ 1280, 720 },
		{ 1920, 1080 },
	};
	int resIdx = ofClamp(appState_->cameraResolutionIndex, 0, 3);
	int w = resolutions[resIdx].first;
	int h = resolutions[resIdx].second;

	grabber_.setup(w, h);
	cameraInitialized_ = grabber_.isInitialized();
	if (cameraInitialized_) {
		ofLogNotice("Engine") << "Camera initialized: " << w << "x" << h;
	} else {
		ofLogWarning("Engine") << "Failed to initialize camera";
	}
}

void Engine::closeCamera() {
	if (cameraInitialized_) {
		grabber_.close();
		cameraInitialized_ = false;
	}
}

GLuint Engine::getCameraTextureID() const {
	if (cameraInitialized_) {
		return grabber_.getTexture().getTextureData().textureID;
	}
	return 0;
}

// Sound controls
void Engine::playSound() {
	int idx = appState_->soundIndex;
	if (appState_->soundPaths.empty() || idx < 0 || idx >= static_cast<int>(appState_->soundPaths.size())) {
		return;
	}
	const std::string & path = appState_->soundPaths[idx];
	if (lastSoundPath_ != path || !soundPlayer_.isLoaded()) {
		soundPlayer_.load(path);
		lastSoundPath_ = path;
	}
	if (soundPlayer_.isLoaded()) {
		soundPlayer_.play();
	}
}

void Engine::stopSound() {
	if (soundPlayer_.isLoaded()) {
		soundPlayer_.stop();
	}
}

void Engine::setSoundVolume(float volume) {
	if (soundPlayer_.isLoaded()) {
		soundPlayer_.setVolume(ofClamp(volume, 0.0f, 1.0f));
	}
}

const std::vector<float> & Engine::getSoundFFT() {
	return fft_;
}

// Video controls
void Engine::playVideo() {
	if (!videoPlayer_.isLoaded()) {
		loadCurrentVideo();
	}
	if (videoPlayer_.isLoaded()) {
		videoPlayer_.setPaused(false);
	}
}

void Engine::pauseVideo() {
	if (videoPlayer_.isLoaded()) {
		videoPlayer_.setPaused(true);
	}
}

void Engine::stopVideo() {
	if (videoPlayer_.isLoaded()) {
		videoPlayer_.stop();
		videoPlayer_.setFrame(0);
	}
}

void Engine::setVideoVolume(float volume) {
	if (videoPlayer_.isLoaded()) {
		videoPlayer_.setVolume(ofClamp(volume, 0.0f, 1.0f));
	}
}

void Engine::seekVideo(float position) {
	if (videoPlayer_.isLoaded()) {
		videoPlayer_.setPosition(ofClamp(position, 0.0f, 1.0f));
	}
}

float Engine::getVideoPosition() const {
	if (videoPlayer_.isLoaded()) {
		return videoPlayer_.getPosition();
	}
	return 0.0f;
}

float Engine::getVideoDuration() const {
	if (videoPlayer_.isLoaded()) {
		return videoPlayer_.getDuration();
	}
	return 0.0f;
}

bool Engine::isVideoPlaying() const {
	return videoPlayer_.isLoaded() && !videoPlayer_.isPaused();
}

GLuint Engine::getVideoTextureID() const {
	if (videoPlayer_.isLoaded()) {
		return videoPlayer_.getTexture().getTextureData().textureID;
	}
	return 0;
}

void Engine::loadCurrentVideo() {
	int idx = appState_->videoIndex;
	if (appState_->videoPaths.empty() || idx < 0 || idx >= static_cast<int>(appState_->videoPaths.size())) {
		return;
	}
	const std::string & path = appState_->videoPaths[idx];
	if (lastVideoPath_ != path) {
		videoPlayer_.load(path);
		videoPlayer_.setLoopState(OF_LOOP_NORMAL);
		lastVideoPath_ = path;
	}
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

	const std::string & text = appState_->textContent;
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

void Engine::renderImage() {
	ofSetBackgroundColor(ofColor(0, 0, 0));
	ofSetColor(ofColor(255, 255, 255));

	int idx = appState_->imageIndex;
	if (!appState_->imagePaths.empty() && idx >= 0 && idx < static_cast<int>(appState_->imagePaths.size())) {
		const std::string & path = appState_->imagePaths[idx];
		// Lazy load
		if (!projectorImage_.isAllocated() || lastImagePath_ != path) {
			projectorImage_.load(path);
			lastImagePath_ = path;
		}
		if (projectorImage_.isAllocated()) {
			float w = projectorImage_.getWidth();
			float h = projectorImage_.getHeight();
			float winW = ofGetWidth();
			float winH = ofGetHeight();
			float scale = std::min(winW / w, winH / h);
			float drawW = w * scale;
			float drawH = h * scale;
			float x = (winW - drawW) / 2.0f;
			float y = (winH - drawH) / 2.0f;
			projectorImage_.draw(x, y, drawW, drawH);
			return;
		}
	}

	// Empty state
	ofSetColor(ofColor(128, 128, 128));
	std::string label = "No images found";
	float x = ofGetWidth() / 2.0f - label.length() * 4.0f;
	float y = ofGetHeight() / 2.0f;
	ofDrawBitmapString(label, x, y);
}

void Engine::renderVideo() {
	ofSetBackgroundColor(ofColor(0, 0, 0));
	ofSetColor(ofColor(255, 255, 255));

	int idx = appState_->videoIndex;
	if (!appState_->videoPaths.empty() && idx >= 0 && idx < static_cast<int>(appState_->videoPaths.size())) {
		// Lazy load
		if (lastVideoPath_ != appState_->videoPaths[idx]) {
			videoPlayer_.load(appState_->videoPaths[idx]);
			videoPlayer_.setLoopState(OF_LOOP_NORMAL);
			lastVideoPath_ = appState_->videoPaths[idx];
		}
		if (videoPlayer_.isLoaded()) {
			float w = videoPlayer_.getWidth();
			float h = videoPlayer_.getHeight();
			float winW = ofGetWidth();
			float winH = ofGetHeight();
			if (w > 0 && h > 0) {
				float scale = std::min(winW / w, winH / h);
				float drawW = w * scale;
				float drawH = h * scale;
				float x = (winW - drawW) / 2.0f;
				float y = (winH - drawH) / 2.0f;
				videoPlayer_.draw(x, y, drawW, drawH);
			} else {
				videoPlayer_.draw(0, 0, winW, winH);
			}
			return;
		}
	}

	// Empty state
	ofSetColor(ofColor(128, 128, 128));
	std::string label = "No videos found";
	float x = ofGetWidth() / 2.0f - label.length() * 4.0f;
	float y = ofGetHeight() / 2.0f;
	ofDrawBitmapString(label, x, y);
}

void Engine::renderCamera() {
	ofSetBackgroundColor(ofColor(0, 0, 0));
	ofSetColor(ofColor(255, 255, 255));

	if (cameraInitialized_) {
		float w = grabber_.getWidth();
		float h = grabber_.getHeight();
		float winW = ofGetWidth();
		float winH = ofGetHeight();
		if (w > 0 && h > 0) {
			float scale = std::min(winW / w, winH / h);
			float drawW = w * scale;
			float drawH = h * scale;
			float x = (winW - drawW) / 2.0f;
			float y = (winH - drawH) / 2.0f;
			grabber_.draw(x, y, drawW, drawH);
		} else {
			grabber_.draw(0, 0, winW, winH);
		}
		return;
	}

	// Empty state
	ofSetColor(ofColor(128, 128, 128));
	std::string label = appState_->cameraDeviceNames.empty() ? "No camera found" : "Camera not initialized";
	float x = ofGetWidth() / 2.0f - label.length() * 4.0f;
	float y = ofGetHeight() / 2.0f;
	ofDrawBitmapString(label, x, y);
}

void Engine::renderSound() {
	ofSetBackgroundColor(ofColor(46, 52, 64));

	int idx = appState_->soundIndex;
	if (!appState_->soundPaths.empty() && idx >= 0 && idx < static_cast<int>(appState_->soundPaths.size())) {
		// Draw FFT bars
		int nBins = static_cast<int>(fft_.size());
		if (nBins > 0) {
			ofSetColor(ofColor(129, 199, 132)); // green
			float winW = ofGetWidth();
			float winH = ofGetHeight();
			float barW = winW / nBins * 0.8f;
			float spacing = winW / nBins;
			for (int i = 0; i < nBins; ++i) {
				float barH = fft_[i] * winH * 2.5f; // scale for visibility
				float x = 10.0f + i * spacing;
				float y = winH - barH;
				ofDrawRectangle(x, y, barW, barH);
			}
		}

		// Draw label
		ofSetColor(ofColor(216, 222, 233));
		std::string name = ofFilePath::getFileName(appState_->soundPaths[idx]);
		ofDrawBitmapString(name, 10, 20);
		return;
	}

	// Empty state
	ofSetColor(ofColor(128, 128, 128));
	std::string label = "No sounds found";
	float x = ofGetWidth() / 2.0f - label.length() * 4.0f;
	float y = ofGetHeight() / 2.0f;
	ofDrawBitmapString(label, x, y);
}

void Engine::drawPrimitiveMesh(const std::string & name) {
	if (name == "Box") {
		ofDrawBox(0, 0, 0, 200);
	} else if (name == "Sphere") {
		ofDrawSphere(0, 0, 0, 120);
	} else if (name == "Cone") {
		ofDrawCone(0, 0, 0, 100, 200);
	} else if (name == "Cylinder") {
		ofDrawCylinder(0, 0, 0, 80, 200);
	} else if (name == "Plane") {
		ofDrawPlane(0, 0, 0, 300, 300);
	}
}

void Engine::renderMesh() {
	ofSetBackgroundColor(ofColor(40, 44, 52));
	ofEnableDepthTest();

	// Camera
	float winW = ofGetWidth();
	float winH = ofGetHeight();
	ofPushView();
	ofViewport(0, 0, winW, winH);
	ofSetupScreenPerspective(winW, winH, 60, 0.1f, 10000.0f);

	// Apply orbit transformations
	float zoom = appState_->meshZoom;
	ofTranslate(winW / 2.0f, winH / 2.0f, 0);
	ofScale(zoom, zoom, zoom);
	ofRotateDeg(appState_->meshRotX, 1, 0, 0);
	ofRotateDeg(appState_->meshRotY, 0, 1, 0);
	ofRotateDeg(appState_->meshRotZ, 0, 0, 1);

	// Lighting
	ofEnableLighting();
	light1_->enable();
	light2_->enable();

	int idx = appState_->meshIndex;
	if (!appState_->meshPaths.empty() && idx >= 0 && idx < static_cast<int>(appState_->meshPaths.size())) {
		const std::string & path = appState_->meshPaths[idx];
		ofSetColor(ofColor(200, 200, 210));
		ofFill();

		bool isPrimitive = (path.find("primitive:") == 0);
		if (isPrimitive) {
			// Built-in primitive
			if (appState_->meshWireframe) {
				ofNoFill();
				ofSetColor(ofColor(129, 199, 132)); // green wireframe
			}
			drawPrimitiveMesh(path.substr(10));
		} else {
			// File-based mesh
			if (lastMeshPath_ != path) {
				loadedMesh_.load(path);
				lastMeshPath_ = path;
				meshLoaded_ = loadedMesh_.getVertices().size() > 0;
			}
			if (meshLoaded_) {
				if (appState_->meshWireframe) {
					ofSetColor(ofColor(129, 199, 132));
					loadedMesh_.drawWireframe();
				} else {
					loadedMesh_.draw();
				}
			} else {
				// Fallback to box if load failed
				if (appState_->meshWireframe) {
					ofNoFill();
					ofSetColor(ofColor(129, 199, 132));
				}
				ofDrawBox(0, 0, 0, 200);
			}
		}

		// Draw label
		ofPopView();
		ofDisableDepthTest();
		ofDisableLighting();
		ofSetColor(ofColor(216, 222, 233));
		std::string displayName = isPrimitive ? path.substr(10) : ofFilePath::getFileName(path);
		ofDrawBitmapString(displayName, 10, 20);
		return;
	}

	ofPopView();
	ofDisableDepthTest();
	ofDisableLighting();

	// Empty state
	ofSetColor(ofColor(128, 128, 128));
	std::string label = "No meshes found";
	float x = ofGetWidth() / 2.0f - label.length() * 4.0f;
	float y = ofGetHeight() / 2.0f;
	ofDrawBitmapString(label, x, y);
}

void Engine::renderNDIRecv() {
	ofSetBackgroundColor(ofColor(0, 0, 0));
	ofSetColor(ofColor(255, 255, 255));

	if (ndiReceiver_.ReceiverCreated() && ndiReceiver_.ReceiverConnected() && ndiRecvFbo_.isAllocated()) {
		float w = ndiRecvFbo_.getWidth();
		float h = ndiRecvFbo_.getHeight();
		float winW = ofGetWidth();
		float winH = ofGetHeight();
		if (w > 0 && h > 0) {
			float scale = std::min(winW / w, winH / h);
			float drawW = w * scale;
			float drawH = h * scale;
			float x = (winW - drawW) / 2.0f;
			float y = (winH - drawH) / 2.0f;
			ndiRecvFbo_.draw(x, y, drawW, drawH);
		} else {
			ndiRecvFbo_.draw(0, 0, winW, winH);
		}
		return;
	}

	// Not connected / no frame
	ofSetColor(ofColor(128, 128, 128));
	std::string label = appState_->ndiSourceNames.empty() ? "No NDI sources found" : "Select an NDI source";
	float x = ofGetWidth() / 2.0f - label.length() * 4.0f;
	float y = ofGetHeight() / 2.0f;
	ofDrawBitmapString(label, x, y);
}

void Engine::renderNDISend() {
	ofSetBackgroundColor(ofColor(0, 0, 0));
	ofSetColor(ofColor(255, 255, 255));

	if (cameraInitialized_) {
		float w = grabber_.getWidth();
		float h = grabber_.getHeight();
		float winW = ofGetWidth();
		float winH = ofGetHeight();
		if (w > 0 && h > 0) {
			float scale = std::min(winW / w, winH / h);
			float drawW = w * scale;
			float drawH = h * scale;
			float x = (winW - drawW) / 2.0f;
			float y = (winH - drawH) / 2.0f;
			grabber_.draw(x, y, drawW, drawH);
		} else {
			grabber_.draw(0, 0, winW, winH);
		}

		// Send camera texture via NDI if sender is active
		if (ndiSenderCreated_) {
			ndiSender_.SendImage(grabber_.getTexture());
			appState_->ndiSendConnections = ndiSender_.GetConnections(0);
		}
		return;
	}

	// No camera fallback
	ofSetColor(ofColor(128, 128, 128));
	std::string label = appState_->cameraDeviceNames.empty() ? "No camera found" : "Camera not initialized";
	float x = ofGetWidth() / 2.0f - label.length() * 4.0f;
	float y = ofGetHeight() / 2.0f;
	ofDrawBitmapString(label, x, y);
}

void Engine::renderProjector(State state) {
	switch (state) {
	case State::STATE_IDLE:
		renderIdle();
		break;
	case State::STATE_TEXT:
		renderText();
		break;
	case State::STATE_IMAGE:
		renderImage();
		break;
	case State::STATE_VIDEO:
		renderVideo();
		break;
	case State::STATE_CAMERA:
		renderCamera();
		break;
	case State::STATE_SOUND:
		renderSound();
		break;
	case State::STATE_MESH:
		renderMesh();
		break;
	case State::STATE_NDI_RECV:
		renderNDIRecv();
		break;
	case State::STATE_NDI_SEND:
		renderNDISend();
		break;
	default:
		ofSetBackgroundColor(ofColor(46, 52, 64));
		break;
	}
}
