# 3D Clock Tower: Architectural Visualization in OpenGL

A comprehensive 3D scene built from scratch using **C++** and **Legacy OpenGL**, featuring a detailed clock tower and a dynamic urban environment.

## 🏛️ Project Overview
This project focuses on creating a realistic 3D landmark set within a bustling three-way junction.Every element, from the circular stepped base to the pyramid roof, is constructed using geometric primitive shapes.

### Key Architectural Features
* **Main Tower Body:** Textured with procedural brick patterns for an authentic feel.
* **Functional Clock Face:** Displays real-time using animated hour, minute, and second hands.
* **Cupola Section:** An open, arched structure near the top of the tower.
* **Arched Windows & Stone Bands:** Detailed accents replicated on all four sides.

## 🌍 Dynamic Environment
The scene implements a full **Day & Night Cycle** that impacts the entire visualization:
* **Daytime:** Features a bright sun with rays, a light blue sky gradient, and subtle atmospheric haze.
* **Nighttime:** Transitioning to a darker sky, dense fog, and illuminated street lamps with glow effects.
* **Urban Junction:** A three-way intersection (North, Southeast, Southwest) featuring road markings, a zebra crossing, traffic lights, and animated cars.

## 🛠️ Technical Implementation
* **Procedural Texturing:** Simulating brick surface details without external image files.
* **Alpha Blending:** Used for transparent effects in clouds, fog, and light halos.
* **Shadow Mapping:** Generating dynamic planar shadows cast by the tower for depth and realism.
* **Advanced Lighting:** Directional sunlight and localized point lights for street lamps.

## 🚀 How to Run
1. Ensure you have an OpenGL/GLUT environment configured (like FreeGLUT).
2. Compile `main.cpp`.
3. **Controls:**
   - `Arrow Keys`: Rotate Camera
   - `+/-`: Zoom In/Out
   - `W/A/S/D`: Transition the Day/Night cycle
