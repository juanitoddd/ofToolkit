#include "engine/Engine.h"
#include "gui/GuiApp.h"
#include "ofMain.h"
#include "projector/ProjectorApp.h"
#include "state/AppState.h"

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

//========================================================================

int main() {
	auto state = std::make_shared<AppState>();

	auto engine = std::make_shared<Engine>(state);

	ofGLFWWindowSettings guiSettings;
	guiSettings.setSize(1280, 800);
	guiSettings.setGLVersion(3, 3);
	guiSettings.windowMode = OF_WINDOW;
	guiSettings.title = "Starter - GUI";

	auto guiWindow = ofCreateWindow(guiSettings);

	ofGLFWWindowSettings projectorSettings;
	projectorSettings.setSize(1920, 1080);
	projectorSettings.setGLVersion(3, 3);
	projectorSettings.windowMode = OF_FULLSCREEN;
	projectorSettings.monitor = 1;
	projectorSettings.title = "Starter - Projector";
	projectorSettings.shareContextWith = guiWindow;

	auto projectorWindow = ofCreateWindow(projectorSettings);

	// TODO: Setup engine after GL context is created (required for FBO allocation)
	// engine->setup();

	ofRunApp(guiWindow, std::make_shared<GuiApp>(state, engine));
	ofRunApp(projectorWindow, std::make_shared<ProjectorApp>(state, engine));

	ofRunMainLoop();
}
