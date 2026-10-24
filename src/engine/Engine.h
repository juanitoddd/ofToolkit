#pragma once
#include "../state/AppState.h"
#include "ofMain.h"
#include <memory>
#include <string>

class Engine {
public:
	Engine(std::shared_ptr<AppState> state);

	void setup();
	void update();

	void renderProjector(State state);
	void scanFonts();
	void scanImages();
	void scanVideos();

	// Video controls
	void playVideo();
	void pauseVideo();
	void stopVideo();
	void setVideoVolume(float volume);
	void seekVideo(float position);
	float getVideoPosition() const;
	float getVideoDuration() const;
	bool isVideoPlaying() const;
	GLuint getVideoTextureID() const;

	ofEvent<State> onRequestState;

private:
	std::shared_ptr<AppState> appState_;
	std::vector<ofTrueTypeFont> fonts_;
	ofImage projectorImage_;
	std::string lastImagePath_;
	ofVideoPlayer videoPlayer_;
	std::string lastVideoPath_;
	State previousState_ = State::STATE_IDLE;

	void loadCurrentVideo();

	void renderIdle();
	void renderText();
	void renderImage();
	void renderVideo();
};
