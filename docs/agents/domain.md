# Domain Docs

## Glossary

| Term | Definition |
|------|------------|
| **GuiApp** | The ImGui-based control window running in windowed mode. |
| **ProjectorApp** | The full-screen output window rendered by the Engine. |
| **Engine** | Shared compute/render engine owned by both apps, allocated after GL context is ready. |
| **AppState** | Shared state object: current `State`, events, debug flags. |
| **State** | Enum representing an active use case: `STATE_IDLE`, `STATE_TEXT`, `STATE_IMAGE`, etc. |

## Architecture Decision Records (ADR)

ADRs are stored in `docs/adr/` at the repo root. Each ADR is a Markdown file with the format:

```
# ADR-NNN: Title

## Status
Proposed / Accepted / Deprecated / Superseded

## Context
What problem are we solving?

## Decision
What did we decide?

## Consequences
What are the trade-offs?
```
