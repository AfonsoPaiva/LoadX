<div align="center">


![OpenGL Modular Engine](https://i.postimg.cc/Vs786DFg/LoadX-WT.png)

*A Modern 3D Graphics Engine for Model Visualization and Rendering*

![OpenGL](https://img.shields.io/badge/OpenGL-3.3-blue.svg)
![C++](https://img.shields.io/badge/C++-17-red.svg)
![GLFW](https://img.shields.io/badge/GLFW-3.x-green.svg)
![GLAD](https://img.shields.io/badge/GLAD-Latest-yellow.svg)
![GLM](https://img.shields.io/badge/GLM-Latest-purple.svg)
![Assimp](https://img.shields.io/badge/Assimp-Latest-lightblue.svg)
![STB](https://img.shields.io/badge/STB-Image-brightgreen.svg)
![ImGui](https://img.shields.io/badge/ImGui-Latest-orange.svg)
![ImGuiFileDialog](https://img.shields.io/badge/ImGuiFileDialog-Latest-red.svg)

</div>

## Features

### Advanced Rendering
- Modern OpenGL 3.3 pipeline with programmable shaders
- PBR (Physically Based Rendering) material system
- Real-time lighting with multiple light types (Directional, Point, Spot)
- Normal mapping and height mapping support
- Multi-texture support with automatic texture detection
- Real-time grid rendering with infinite appearance

### Multi-Format Model Support
- Native OBJ loader with MTL material support
- Assimp integration for 20+ formats (FBX, GLTF, 3DS, DAE, etc.)
- Progress tracking for large model loading
- Automatic model centering and scaling
- UV coordinate manipulation

### Interactive Controls
- 6DOF camera system with smooth movement
- Real-time transform controls (Position, Rotation, Scale)
- Mouse and keyboard navigation
- Screenshot capture (F12 or UI button)
- Multiple lighting presets

### Professional UI
- ImGui-based interface with dark theme
- Real-time performance monitoring
- Debug console with timestamped messages
- Texture management with folder loading
- Material property editor
- System information display

### Performance Features
- Optimized model loading with memory management
- Efficient texture caching
- Frame rate monitoring and statistics
- Memory usage tracking
- GPU information display

## Technical Stack

| Component | Technology | Purpose |
|-----------|------------|---------|
| Graphics API | OpenGL 3.3+ | Core rendering |
| Window Management | GLFW 3.x | Window creation and input |
| Math Library | GLM | 3D mathematics |
| Model Loading | Assimp + Custom OBJ | 3D file format support |
| Texture Loading | STB Image | Image file handling |
| User Interface | Dear ImGui | Real-time GUI |
| File Dialogs | ImGuiFileDialog | Native file browser |

## Installation

### Prerequisites
- Visual Studio 2022 (Windows) or compatible C++17 compiler
- CMake 3.15+ (if building with CMake)
- Git for cloning repositories

### Dependencies Included
All required libraries are included in the project:

- GLFW 3.x
- GLAD (OpenGL loader)
- GLM (Mathematics)
- Assimp (Model loading)
- STB Image (Texture loading)
- Dear ImGui (User interface)
- ImGuiFileDialog (File browser)

### Build Instructions

#### Visual Studio 2022
1. Clone the repository
2. Open `OpenGL-Modular-Engine.sln` in Visual Studio 2022
3. Set build configuration to Release for optimal performance
4. Build the solution (Ctrl+Shift+B)
5. Run the executable from `bin/Release/`

<div align="center">

## Demonstration

[![Watch Demo Video](https://img.shields.io/badge/_Watch_Demo-YouTube-red?style=for-the-badge&logo=youtube)](https://www.youtube.com/watch?v=YOUR_VIDEO_ID)

![Engine Demo](https://i.postimg.cc/QM4M3hVg/video-gif-github.gif)

*Interactive demonstration of the OpenGL Modular Engine showing model loading, real-time rendering, and UI controls*

</div>

## Usage Guide

### Basic Operation
1. Launch the application
2. Load a model using "Select Model File" button
3. Navigate using mouse (hold left-click) and WASD keys
4. Transform the model using the UI sliders
5. Adjust lighting in the right panel
6. Take screenshots with F12 or the UI button

### Supported File Formats

| Format | Extension | Features |
|--------|-----------|----------|
| Wavefront OBJ | .obj | MTL materials, Custom loader |
| GLTF | .gltf, .glb | PBR materials, Animations |
| FBX | .fbx | Complete scenes, Materials |
| Collada | .dae | XML-based, Materials |
| 3DS Max | .3ds | Legacy format support |
| STL | .stl | 3D printing models |
| PLY | .ply | Point clouds, Meshes |

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| WASD | Camera movement (when enabled) |
| Mouse + Left Click | Look around |
| Mouse Scroll | Zoom in/out |
| F12 | Take screenshot |
| ESC | Exit application |

## UI Panels

### Model Loader
- Load 3D models from file
- Auto-detect MTL materials for OBJ files
- Progress tracking for large files
- Model information display

### Camera Controls
- Toggle camera movement
- Reset camera position
- Take screenshots
- Movement instructions

### Object Transform
- Position, rotation, scale controls
- Uniform and individual scaling
- Auto-sizing and centering
- Model dimension display

### Engine Stats
- Real-time FPS monitoring
- Memory usage tracking
- GPU information
- Performance graphs

### Lighting Controls
- Directional light settings
- Point light configuration
- Spot light parameters
- Lighting presets

## Architecture

### Modular Design

#### Key Classes
- **Window**: GLFW window management and input handling
- **Camera**: 6DOF camera with smooth movement
- **Model**: 3D model representation with multi-format support
- **Mesh**: Individual mesh with vertices, indices, and materials
- **Render**: Core rendering functions and lighting updates
- **UI**: Complete ImGui interface with all panels
- **Transform**: 3D transformation matrices and operations

## Shader System

### Vertex Shader Features
- Model-View-Projection transformations
- Normal matrix calculations
- Tangent space computation for normal mapping
- Multiple texture coordinate support

### Fragment Shader Features
- Blinn-Phong lighting model
- PBR material properties
- Multi-texture sampling (Diffuse, Normal, Specular, etc.)
- Dynamic texture availability checking
- Metallic workflow support

### Grid Shader
- Infinite grid appearance
- Distance-based fading
- Axis highlighting (X=Red, Z=Blue)
- Multi-scale rendering

## Performance

### Optimizations
- Vertex Array Objects for efficient rendering
- Texture caching to avoid redundant loading
- Frustum culling ready architecture
- Memory pooling for large models
- Progressive loading with progress feedback

### Monitoring
- Real-time FPS counter
- Frame time statistics (min/max/average)
- Memory usage tracking
- GPU information display
- Draw call counting

## Configuration

### Build Configurations
- **Debug**: Full debugging, slower performance
- **Release**: Optimized for performance
- **RelWithDebInfo**: Optimized with debug symbols

### Customization
- Modify shader files in `shaders` directory
- Adjust lighting presets in `Lighting.h`
- Configure UI layout in `Ui.cpp`
- Add new file formats in model loaders

## Contributing

We welcome contributions from developers of all skill levels! Whether you're fixing bugs, adding new features, improving documentation, or suggesting enhancements, your help is appreciated.

### How to Contribute

1. **Fork the repository** to your GitHub account
2. **Create a feature branch** from the main branch (`git checkout -b feature/your-feature-name`)
3. **Make your changes** and test them thoroughly
4. **Commit your changes** with clear, descriptive messages
5. **Push to your fork** (`git push origin feature/your-feature-name`)
6. **Submit a pull request** with a detailed description of your changes


## Acknowledgments

- [LearnOpenGL](https://learnopengl.com/) - Excellent OpenGL tutorials
- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate mode GUI
- [Assimp](https://github.com/assimp/assimp) - Asset import library
- [GLFW](https://www.glfw.org/) - Window and input management
- [GLM](https://github.com/g-truc/glm) - OpenGL mathematics
- [STB](https://github.com/nothings/stb) - Single-file libraries
