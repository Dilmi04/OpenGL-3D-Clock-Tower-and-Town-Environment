# 3D Clock Tower: Architectural Visualization in OpenGL

A comprehensive 3D scene built from scratch using **C++** and **Legacy OpenGL**, featuring a detailed clock tower and a dynamic urban environment.

## 🏛️ Project Overview
[cite_start]This project focuses on creating a realistic 3D landmark set within a bustling three-way junction[cite: 19, 64]. [cite_start]Every element, from the circular stepped base to the pyramid roof, is constructed using geometric primitive shapes[cite: 33, 44, 49].

### Key Architectural Features
* [cite_start]**Main Tower Body:** Textured with procedural brick patterns for an authentic feel[cite: 52, 57].
* [cite_start]**Functional Clock Face:** Displays real-time using animated hour, minute, and second hands[cite: 45, 55].
* [cite_start]**Cupola Section:** An open, arched structure near the top of the tower[cite: 50, 53].
* [cite_start]**Arched Windows & Stone Bands:** Detailed accents replicated on all four sides[cite: 48, 51, 56].

## 🌍 Dynamic Environment
[cite_start]The scene implements a full **Day & Night Cycle** that impacts the entire visualization[cite: 17, 62]:
* [cite_start]**Daytime:** Features a bright sun with rays, a light blue sky gradient, and subtle atmospheric haze[cite: 58, 59].
* [cite_start]**Nighttime:** Transitioning to a darker sky, dense fog, and illuminated street lamps with glow effects[cite: 60, 61].
* [cite_start]**Urban Junction:** A three-way intersection (North, Southeast, Southwest) featuring road markings, a zebra crossing, traffic lights, and animated cars[cite: 66, 70, 73, 76].

## 🛠️ Technical Implementation
* [cite_start]**Procedural Texturing:** Simulating brick surface details without external image files[cite: 34, 40].
* [cite_start]**Alpha Blending:** Used for transparent effects in clouds, fog, and light halos[cite: 36, 37].
* [cite_start]**Shadow Mapping:** Generating dynamic planar shadows cast by the tower for depth and realism[cite: 30, 38].
* [cite_start]**Advanced Lighting:** Directional sunlight and localized point lights for street lamps[cite: 35, 39].

## 🚀 How to Run
1. Ensure you have an OpenGL/GLUT environment configured (like FreeGLUT).
2. Compile `main.cpp`.
3. **Controls:**
   - [cite_start]`Arrow Keys`: Rotate Camera [cite: 83]
   - `+/-`: Zoom In/Out
   - `W/A/S/D`: Transition the Day/Night cycle
