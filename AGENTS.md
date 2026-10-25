# OF Toolkit — Agent Brief

OpenFrameworks app for showcasing simple use cases with functionality and integrations

## Architecture

Tha application consists of two apps GuiApp `src/gui/*` and ProjectorApp `src/projector/*` that share state and events through `src/state/AppState.h` and also share a compute engine `src/engine` which takes care of additional computation that needs to be shared between the two apps.

### UI

The GuiApp uses ofImGui to manage the ui, input fields, selects, buttons, sliders, inside windows, top bar menu and rest of interactions.

### App Use Cases

from `src/state/AppState.h`, we have following states that represent use cases:

- STATE_IDLE
- STATE_TEXT
- STATE_IMAGE
- STATE_VIDEO
- STATE_CAMERA
- STATE_SOUND
- STATE_MESH
- STATE_NDI_RECV
- STATE_NDI_SEND

And may be subject to more use cases.

#### STATE_IDLE

Nothing is depicted, but a floating window with debug information about the app, like fps, size, and more relevant information

#### STATE_TEXT

Input fields, texts, text color picker, font if possible.

#### STATE_IMAGE

Loads a list of image files from `bin/data/images` and using a floating window it displays the image

#### STATE_VIDEO

Loads a list of video files from `bin/data/images` and using a floating window it displays the video with some basic buttons as playback controls.

#### STATE_CAMERA

Using the `ofVideoGrabber` it feed the camera video stream to a floating window. 

#### STATE_SOUND

Loads a list of image files from `bin/data/sounds` and using a floating window to playback the sounds,

#### STATE_MESH

A simple 3D Viewer with orbit controls.

#### STATE_NDI_RECV

Receives an NDI stream from the network. The GuiApp shows a source discovery list and connection controls. The ProjectorApp renders the received stream full-screen.

#### STATE_NDI_SEND

Broadcasts the ProjectorApp output as an NDI stream. The GuiApp shows sender controls (name, start/stop, connection count). The sender captures the ProjectorApp FBO and transmits it over the network.

### Engine

Inside the The `src/engine` folder is where the classes that manage the computer vision, sound processing, networking, helpers and more are located.

## Agent skills

Each state mentioned in the use cases should be an issue and therfore a branch to be developed.

### Issue tracker

Issues live as GitHub issues in `juanitoddd/ofToolkit`. See `docs/agents/issue-tracker.md`.

### Triage labels

Five canonical roles, unmapped (`needs-triage` through `wontfix`). See `docs/agents/triage-labels.md`.

### Domain docs

Single-context layout: one `GLOSSARY.md` + `docs/adr/` at repo root. See `docs/agents/domain.md`.