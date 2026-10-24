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
}

void Engine::update() {
	// Pause video when switching away from STATE_VIDEO
	State current = appState_->getState();
	if (previousState_ == State::STATE_VIDEO && current != State::STATE_VIDEO) {
		if (videoPlayer_.isLoaded()) {
			videoPlayer_.setPaused(true);
		}
	}
	previousState_ = current;

	// Update video player
	if (current == State::STATE_VIDEO && videoPlayer_.isLoaded()) {
		videoPlayer_.update();
	}
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
	for (int i = 0; i < dir.size(); ++i) {
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
	for (int i = 0; i < dir.size(); ++i) {
		appState_->videoPaths.push_back(dir.getPath(i));
	}
	appState_->videoIndex = 0;
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
	const std::string& path = appState_->videoPaths[idx];
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

void Engine::renderImage() {
	ofSetBackgroundColor(ofColor(0, 0, 0));
	ofSetColor(ofColor(255, 255, 255));

	int idx = appState_->imageIndex;
	if (!appState_->imagePaths.empty() && idx >= 0 && idx < static_cast<int>(appState_->imagePaths.size())) {
		const std::string& path = appState_->imagePaths[idx];
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
	default:
		ofSetBackgroundColor(ofColor(46, 52, 64));
		break;
	}
}
