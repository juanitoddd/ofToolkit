#pragma once
#include "../engine/Engine.h"
#include "../state/AppState.h"
#include "ofMain.h"
#include "ofxImGui.h"

class GuiApp : public ofBaseApp {
public:
	GuiApp(std::shared_ptr<AppState> state, std::shared_ptr<Engine> engine)
		: appState(std::move(state))
		, engine_(std::move(engine)) { }

	void setup() override;
	void update() override;
	void draw() override;
	void exit() override;
	void keyPressed(int key) override;
	void keyReleased(int key) override;
	void mouseMoved(int x, int y) override;
	void mouseDragged(int x, int y, int button) override;
	void mousePressed(int x, int y, int button) override;
	void mouseReleased(int x, int y, int button) override;
	void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
	void mouseEntered(int x, int y) override;
	void mouseExited(int x, int y) override;
	void windowResized(int w, int h) override;
	void dragEvent(ofDragInfo dragInfo) override;
	void gotMessage(ofMessage msg) override;
	void drawTextPanel();
	void drawImagePanel();
	void drawVideoPanel();
	void drawCameraPanel();
	void drawSoundPanel();
	void drawNDIRecvPanel();
	void drawNDISendPanel();
	void drawMeshPanel();

	bool show_test_window;
	bool show_another_window;
	float floatValue;
	ImVec4 backgroundColor = ofColor(236, 239, 244);

private:
	std::shared_ptr<AppState> appState;
	std::shared_ptr<Engine> engine_;
	ofxImGui::Gui gui;
	ofImage cameraImage_;
	ofFbo cameraPreviewFbo_;
	bool debugDrawCamera_ = false;
	char textBuffer_[1024] = "Hello, World!";
	ofImage thumbnailImage_;
	int lastImageIndex_ = -1;
	int lastVideoIndex_ = -1;
	bool videoSeekDragging_ = false;
	int lastCameraDeviceIndex_ = -1;
	int lastCameraResolutionIndex_ = -1;
	int lastSoundIndex_ = -1;
	int lastMeshIndex_ = -1;
	bool meshDragging_ = false;
	float meshDragStartX_ = 0.0f;
	float meshDragStartY_ = 0.0f;
	float meshRotXStart_ = 0.0f;
	float meshRotYStart_ = 0.0f;
};
