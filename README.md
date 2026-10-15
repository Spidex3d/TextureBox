# BoxTextureEditor

BoxTextureEditor is a personal C++ tool for generating normal maps from 2D textures.

## Current status

So far, the project includes:

- GLFW + OpenGL context setup
- ImGui integration with docking enabled
- File dialog support for selecting texture files
- Source texture loading with `stb_image`
- Texture preview in the editor UI
- CPU-based normal map generation (Sobel filter)
- Normal map settings:
  - Strength
  - Invert Y
  - Wrap/tile edges
- Normal map preview in the main viewport

## Project structure

- `inc/` and `src/` folders
- `core/` for image processing and normal map generation
- `window/` for ImGui panels and rendering-related UI

## Next steps

- Save generated normal maps to disk (`stb_image_write`)
- Improve UX (status messages, buttons, error feedback)
- Add optional 3D preview for lighting/material validation

## Notes

This is a personal utility project built to support my BoxEditor workflow.