#pragma once
#include "../state/AppState.h"
#include "ofMain.h"
#include "ofxNDI.h" // NDI classes
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

	// Camera controls
	void initCamera();
	void closeCamera();
	void scanCameraDevices();
	GLuint getCameraTextureID() const;

	// Sound controls
	void scanSounds();
	void playSound();
	void stopSound();
	void setSoundVolume(float volume);
	const std::vector<float> & getSoundFFT();

	// Mesh controls
	void scanMeshes();
	void renderMesh();

	// NDI controls
	void scanNDISources();
	void selectNDISource(int index);
	void disconnectNDISource();
	void startNDISender();
	void stopNDISender();

	ofEvent<State> onRequestState;

private:
	std::shared_ptr<AppState> appState_;
	std::vector<ofTrueTypeFont> fonts_;
	ofImage projectorImage_;
	std::string lastImagePath_;
	ofVideoPlayer videoPlayer_;
	std::string lastVideoPath_;
	State previousState_ = State::STATE_IDLE;
	ofVideoGrabber grabber_;
	bool cameraInitialized_ = false;
	ofSoundPlayer soundPlayer_;
	std::string lastSoundPath_;
	std::vector<float> fft_;
	std::unique_ptr<ofLight> light1_;
	std::unique_ptr<ofLight> light2_;
	ofMesh loadedMesh_;
	std::string lastMeshPath_;
	bool meshLoaded_ = false;
	std::vector<std::string> meshPrimitiveNames_;

	ofxNDIreceiver ndiReceiver_;
	ofxNDIsender ndiSender_;
	ofTexture ndiRecvTexture_;
	ofFbo ndiSendFbo_;
	bool ndiSenderCreated_ = false;
	uint64_t lastNdiRefreshTime_ = 0;

	void loadCurrentVideo();

	void renderIdle();
	void renderText();
	void renderImage();
	void renderVideo();
	void renderCamera();
	void renderSound();
	void renderNDIRecv();
	void renderNDISend();
	void drawPrimitiveMesh(const std::string & name);
};
