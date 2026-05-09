

# Solar System with HDR Bloom (OpenGL C++)

A modern OpenGL 3D project that simulates the Solar System with a strong focus on **HDR rendering and bloom effects**.  
The goal is to achieve a solid rendering pipeline to learn how to manipulate OpenGL correctly.

---

### Core Rendering
- Full 3D rendering of:
  - Sun
  - 8 planets
  - Earth’s Moon
- Realistic textures for all planets
- Full cubemap skybox filled with stars (texture available here https://opengameart.org/)

## Basic Features

### Lights
- Phong lighting
- diffuse
- specular
- ambient
- emissive Sun
- HDR exposure

### Textures
- texture for sun and each planet
- moon
- night map Earth
- cubemap skybox

### Multiple Models
- multiple sphere instances
- different textures
- separate moon

### Cubemap
- real cubemap skybox
- 6 textures

### Game Logic
- orbits system
- hierarchy Earth/Moon
- time control
- UI runtime
- toggle for bloom and orbits

### Object Moving
- orbits
- rotations
- camera

### Free Navigation
- FPS camera
- moule look
- WASD system
- zoom

### Reflection
- skybox texture reflect on Earth

## Intermediate Features

### Frame Buffer Object effect
- HDR framebuffer
- ping pong blur
- bloom pipeline



### Lighting & Effects
- Phong lighting model
- HDR rendering pipeline
- Bloom effect applied to the Sun
- Light spill and glow on nearby planets
- Emission-based rendering for bright objects

### Simulation
- Orbital motion for planets and moon
- Scaled (but visually coherent) distances and sizes
- Adjustable time speed *(planned / optional)*

### Camera System
- Free-roaming camera
- Mouse look
- WASD movement

### Advanced (Planned for now)
- Click on a planet to follow it
- UI controls (ImGui)
- Exposure control
- Bloom intensity tuning

---

## Project Tree Folder
```
SolarSystem/
│
├── include/
│   ├── glad/
│   ├── GLFW/
│   ├── glm/
│   ├── KHR/
│   └── stb_image.h
│
├── lib/
│   └── libglfw3.a
│
├── resources/
│   ├── textures/
│   └── hdr/                
│
├── shaders/
│   ├── scene.vert
│   ├── scene.frag
│   ├── light.vert
│   ├── light.frag
│   ├── blur.vert           
│   ├── blur.frag           
│   ├── bloom_final.vert    
│   └── bloom_final.frag
│   ├── quad.vert    
│   └── quad.frag
│   ├── skybox.vert    
│   └── skybox.frag
│
├── src/
│   ├── main.cpp
│   ├── glad.c
│
│   ├── core/               
│   │   ├── shader.h / shader.cpp
│   │   ├── camera.h / camera.cpp
│   │   ├── framebuffer.h / framebuffer.cpp
│
│   ├── rendering/          
│   │   ├── sphere.h / sphere.cpp
│   │   ├── renderer.h / renderer.cpp
│
│   ├── utils/              
│   │   ├── texture.h / texture.cpp
│   │   ├── cubemap.h / cubemap.cpp
│
├── CMakeLists.txt          
└── glfw3.dll
└── README.md
```

---

## Dependencies

The project uses the following libraries:

- **GLFW** → windowing and input  
- **GLAD** → OpenGL loader  
- **GLM** → mathematics (vectors, matrices)  
- **stb_image** → texture loading  

Optional (future):
- ImGui → UI controls

---

## Requirements

- C++17 compatible compiler (e.g. `g++`, `clang`, MSVC)
- CMake ≥ 3.10
- OpenGL 3.3+

---

## Clone the Repository

```bash
git clone https://github.com/NiccoBene00/SolarSystem.git
cd SolarSystem
```

---

## Building Instruction

Create build directory
```bash
mkdir build
cd build
```

Generate project files
```bash
cmake ..
```

Compile
```bash
cmake --build .
```

Run the project
```bash
./SolarSystem.exe
```
