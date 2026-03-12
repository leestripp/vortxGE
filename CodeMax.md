# CodeMax.md - Documentation Plan for vortxGE

## Project Overview

### Project Info
- **Project Name**: vortxGE
- **Type**: 3D Graphics Engine (C++20)
- **Build System**: CMake
- **Total Source Files**: 236 files (101 cpp + 106 h in vortxGE/src + demos/tools)
- **Dependencies**: Bullet Physics, OpenGL, GLEW, OpenAL

### Project Structure
```
vortxGE/
├── vortxGE/src/          # Core engine (207 files)
│   ├── animation/        # Animation system
│   ├── assets/           # Asset management
│   ├── controls/         # User input controls
│   ├── datablocks/       # Core data structures, mesh, textures
│   ├── events/           # Event system
│   ├── filesystem/       # File I/O operations
│   ├── math/             # Math library (vectors, matrices, quaternions)
│   ├── openal/           # Audio system
│   ├── passes/           # Rendering passes
│   ├── physics/          # Bullet physics integration
│   ├── properties/       # Property system
│   ├── render/           # OpenGL rendering
│   ├── scene/            # Scene graph (nodes, objects, lights, cameras)
│   ├── shader/           # Shader management
│   └── timers/           # Timer system
├── demos/                # Demo applications (7 files)
├── tools/                # Tools like obj2vbm (21 files)
└── tests/                # Unit tests (1 file)
```

### Coding Conventions (Existing)
- Class prefix: `ls` (e.g., `lsNode`, `lsVector3`)
- Member variable prefix: `m_` (e.g., `m_active`, `m_children`)
- Constant prefixes:
  - Node types: `LSNT_` (e.g., `LSNT_OBJECT`, `LSNT_CAMERA`)
  - Asset types: `LSAT_` (e.g., `LSAT_MESH`, `LSAT_TEXTURE`)
  - Pass types: `LSPT_` (e.g., `LSPT_DIRECTRENDER`, `LSPT_SHADOWMAP`)
- Getters: `getName()`, `Get_Scene()` (mixed camelCase and PascalCase)
- Setters: `setName()`, `Set_Pos()`

### Existing Documentation
- Some Doxygen-style comments present (`/** ... */`, `/*! ... */`)
- Some `@param` and `@returns` tags used inconsistently
- **Most files lack documentation** - this is the main gap to address

---

## Documentation Standards

### Header File Documentation Template
```cpp
/**
 * @file filename.h
 * @brief Brief description of what this file contains
 * 
 * Detailed description of the file's purpose, usage, and any 
 * important implementation details developers should know.
 * 
 * @author [Your Name]
 * @date [Date]
 */

// If it's a class:
/**
 * @class ClassName
 * @brief Brief description of the class
 * 
 * Detailed description of what this class does, its responsibilities,
 * and how it fits into the larger architecture.
 * 
 * @note Any important notes for developers
 * @warning Any warnings about usage
 * 
 * @example Example usage code
 */
```

### Class Member Documentation
```cpp
/**
 * @brief Short description of the method
 * 
 * Longer description if needed, explaining what the method does,
 * how it works, and any important details.
 * 
 * @param param1 Description of first parameter
 * @param param2 Description of second parameter
 * @return Description of return value
 * @retval 0 Success
 * @retval -1 Error condition
 * @note Any notes about this method
 * @see Related methods or classes
 */
```

### Inline Comment Standards
- Use `//` for single-line comments on the same line as code
- Use `/** ... */` for documentation comments that may be extracted by tools
- Comment complex algorithms, math operations, and non-obvious code
- Document **why** something is done, not just **what** it does

---

## Tasks

### Phase 1: Core Infrastructure (Headers)
Priority: HIGH - These are the most referenced files

- [ ] **1.1** Document root headers (`vortxGE.h`, `vortxGE.cpp`, `config.h`, `vortx_debug.h`)
- [ ] **1.2** Document math library headers (`lsMath.h`, `lsVector2.h`, `lsVector3.h`, `lsVector4.h`)
- [ ] **1.3** Document math library headers (`lsMatrix3x3.h`, `lsMatrix4x4.h`, `lsQuaternion.h`)
- [ ] **1.4** Document math utility headers (`lsPlane.h`, `lsFrustum.h`, `lsBoundingSphere.h`, `lsRandom.h`)

### Phase 2: Core Systems
Priority: HIGH - Core functionality

- [ ] **2.1** Document `lsScene` and `lsSceneManager` (scene management)
- [ ] **2.2** Document `lsNode` and `lsObject` (scene graph base classes)
- [ ] **2.3** Document `lsRender` and `lsOpenGL` (rendering system)
- [ ] **2.4** Document `lsShader`, `lsShaderManager`, `lsShaderKinds` (shader system)

### Phase 3: Scene Objects
Priority: MEDIUM-HIGH - Frequently used

- [ ] **3.1** Document camera system (`lsCamera.h`, `lsFirstPersonRig.h`)
- [ ] **3.2** Document light system (`lsLight.h`, `lsLightManager.h`, variants)
- [ ] **3.3** Document audio system (`lsSpeaker.h`, `lsListener.h`, `lsOpenAL.h`)

### Phase 4: Assets & Data
Priority: MEDIUM - Important for understanding data flow

- [ ] **4.1** Document asset system (`lsAsset.h`, `lsAssetManager.h`)
- [ ] **4.2** Document mesh assets (`lsAsset_Mesh.h`, `lsAsset_Texture.h`, `lsAsset_Quad.h`)
- [ ] **4.3** Document physics shapes (`lsAsset_BulletShape_*.h`)
- [ ] **4.4** Document mesh guides (`lsAsset_MeshGuide_*.h`)

### Phase 5: Rendering Pipeline
Priority: MEDIUM - Important for graphics pipeline understanding

- [ ] **5.1** Document pass system (`lsPass.h`, `lsPassManager.h`)
- [ ] **5.2** Document render passes (`lsPass_DR.h`, `lsPass_FBO.h`, `lsPass_ShadowMap.h`, `lsPass_Occlusion.h`)
- [ ] **5.3** Document batch system (`lsBatch.h`, `lsBatch_Mesh.h`, `lsBatch_Quad.h`, etc.)
- [ ] **5.4** Document FBO system (`lsFBO.h`, `lsFBO_Depth.h`, `lsFBO_ColourDepth_8.h`)

### Phase 6: Physics & Events - COMPLETE ✅
- [x] **6.1** Document physics system (`lsBulletWorld.h`, `lsBulletObject.h`, `lsRayCast.h`) ✅
- [x] **6.2** Document event system (`lsEvent.h`, `lsEventManager.h`, event types) ✅

### Phase 7: Animation & Properties - COMPLETE ✅
- [x] **7.1** Document animation system (`lsAnimationManager.h`, `lsKeyFrame.h`, `lsCubicCurve.h`, `lsAnimNode.h`, `lsAnimProperty.h`) ✅
- [x] **7.2** Document property system (`lsProperty.h`, `lsProperty_*.h`) ✅

### Phase 8: Controls & Utilities - COMPLETE ✅
- [x] **8.1** Document control system (`lsControl.h`, `lsControlManager.h`, `lsViewer.h`, `lsComposite.h`) ✅
- [x] **8.2** Document datablocks (`lsData.h`, `lsString.h`, `lsTexture.h`, `lsTGA.h`, `lsMesh.h`) ✅
- [x] **8.3** Document timers (`lsTimer.h`) ✅
- [x] **8.4** Document filesystem (`lsFileIO.h`, `lsDir.h`) ✅

### Phase 9: Implementation Files (.cpp)
Priority: LOW - Document after headers are complete

- [ ] **9.1** Add implementation comments to complex algorithms in math library
- [ ] **9.2** Add implementation comments to scene graph operations
- [ ] **9.3** Add implementation comments to rendering pipeline
- [ ] **9.4** Add implementation comments to shader loading/compilation
- [ ] **9.5** Add implementation comments to physics integration
- [ ] **9.6** Add implementation comments to asset loading (VBM, GLTF)

### Phase 10: Demos & Tools
Priority: LOW

- [ ] **10.1** Document demo scene code (`demoscene.cpp`, `demoscene.h`)
- [ ] **10.2** Document demo application code (glfwApp, glwindow)
- [ ] **10.3** Document tools (obj2vbm converter)

---

## Implementation Notes

### Documentation Tools
- Use **Doxygen** compatible comments for automatic API documentation generation
- Consider running Doxygen to verify documentation completeness
- Comments should be in English (as per project guidelines)

### Suggested Workflow
1. Start with Phase 1 (core headers) to establish patterns
2. Work through phases in order for consistent progress
3. Focus on public APIs first (header files)
4. Add implementation details in .cpp files as you go
5. Use consistent terminology across all modules

### Key Areas Needing Most Attention
- **lsNode/lsObject hierarchy** - fundamental to understanding scene graph
- **lsScene/lsSceneManager** - core orchestration
- **lsRender** - rendering pipeline entry point
- **lsShader/lsPass** - graphics pipeline understanding
- **lsBulletWorld** - physics integration

### Quick Wins (Start Here)
1. `vortxGE.h` - Main engine class, highly referenced ✅ DONE
2. `lsVector3.h` - Most used math class ✅ DONE
3. `lsNode.h` - Base class for all scene objects ✅ DONE
4. `lsScene.h` - Main scene container ✅ DONE
5. `lsRender.h` - Rendering system entry ✅ DONE

## Completed Documentation

### Phase 0: Quick Wins (COMPLETED 2024-01)
- [x] **0.1** Document `vortxGE.h` - Main engine class with full Doxygen
- [x] **0.2** Document `lsVector3.h` - 3D vector with all operators documented
- [x] **0.3** Document `lsNode.h` - Scene graph base class with full docs
- [x] **0.4** Document `lsScene.h` - Scene container with all subsystems
- [x] **0.5** Document `lsRender.h` - Abstract render base class

### Phase 1: Core Infrastructure - COMPLETE ✅
- [x] **1.1** Document root headers (`vortxGE.h`) ✅
- [x] **1.2** Document math library headers (`lsVector2.h`, `lsVector3.h`, `lsVector4.h`) ✅
- [x] **1.3** Document math library headers (`lsMatrix4x4.h`, `lsQuaternion.h`) ✅
- [x] **1.4** Document math utility headers (`lsMath.h`, `lsPlane.h`, `lsFrustum.h`, `lsBoundingSphere.h`, `lsRandom.h`) ✅

### Phase 2: Core Systems - COMPLETE ✅
- [x] **2.1** Document `lsScene` ✅
- [x] **2.2** Document `lsNode` and `lsObject` ✅
- [x] **2.3** Document `lsRender` ✅
- [x] **2.4** Document `lsShader` and `lsShaderManager` ✅
- [x] **2.5** Document `lsShaderKinds`, `lsPass`, `lsPassManager` ✅

### Phase 3: Scene Objects - COMPLETE ✅
- [x] **3.1** Document `lsCamera.h` and `lsLight.h` ✅
- [x] **3.2** Document `lsFirstPersonRig.h` ✅
- [x] **3.3** Document audio system (`lsSpeaker.h`, `lsListener.h`, `lsOpenAL.h`) ✅

### Phase 4: Assets & Data - COMPLETE ✅
- [x] **4.1** Document `lsAsset` and `lsAssetManager` ✅
- [x] **4.2** Document mesh assets (`lsAsset_Mesh.h`, `lsAsset_Texture.h`, `lsAsset_Quad.h`) ✅
- [x] **4.3** Document physics shapes (`lsAsset_BulletShape_*.h`) ✅
- [x] **4.4** Document mesh guides (`lsAsset_MeshGuide_*.h`) ✅

---

## File Statistics by Module

| Module        | .cpp | .h  | Priority |
|--------------|------|-----|----------|
| math/        | 10   | 10  | High     |
| scene/       | 18   | 17  | High     |
| assets/      | 13   | 13  | Medium   |
| shader/      | 2    | 3   | Medium   |
| datablocks/  | 22   | 25  | Medium   |
| passes/      | 8    | 9   | Medium   |
| physics/     | 5    | 5   | Medium   |
| controls/    | 4    | 4   | Low      |
| animation/   | 7    | 8   | Low      |
| properties/  | 6    | 6   | Low      |
| events/      | 4    | 5   | Low      |
| render/      | 2    | 3   | Medium   |
| openal/      | 1    | 1   | Low      |
| timers/      | 1    | 1   | Low      |
| filesystem/  | 2    | 2   | Low      |
| **Total**    | **101** | **106** | - |

---

## Success Criteria
- All header files have class-level documentation
- All public methods have @param and @return documentation
- Complex algorithms and math operations are commented
- Consistent documentation style across all modules
- Doxygen can generate meaningful API documentation
