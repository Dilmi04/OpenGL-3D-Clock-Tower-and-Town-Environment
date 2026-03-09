#include <GL/glut.h>
#include <GL/glu.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include<stdbool.h>

#define PI 3.14159265358979323846

bool isNightTime = false;
int numStreetLamps = 3;

//Lamps
GLfloat lampLightPositions[3][4] = {
    {5.0f, 7.5f, -5.0f, 1.0f},    // North-East corner
    {-5.0f, 7.5f, -5.0f, 1.0f},   // North-West corner
    {0.0f, 7.5f, 7.0f, 1.0f}      // Southern split
};

// Shadow mapping variables
#define SHADOW_MAP_SIZE 2048
GLuint shadowMapTexture;
GLfloat shadowMatrix[16];

// Tower dimensions
float towerHeight = 10.0f;
float towerBaseSize = 2.0f;
float clockRadius = 0.65f;

// Camera control
float camAngleY = 45.0f;
float camAngleX = 25.0f;
float zoom = 20.0f;

// Sun position
float sunAngleY = 45.0f;
float sunAngleX = 35.0f;

void drawCloud(float x, float y, float scaleX, float scaleY, float alpha);

// Brick colors
float brickColor1[] = { 0.72f, 0.42f, 0.32f }; 
float brickColor2[] = { 0.68f, 0.38f, 0.28f }; 
float brickColor3[] = { 0.75f, 0.45f, 0.35f }; 
float mortarColor[] = { 0.88f, 0.85f, 0.82f }; 
float windowColor[] = { 0.05f, 0.05f, 0.08f };
float roofColor[] = { 0.35f, 0.30f, 0.28f };
float roadColor[] = { 0.2f, 0.2f, 0.22f };
float grassColor[] = { 0.2f, 0.6f, 0.2f };
float buildingColor1[] = { 0.75f, 0.7f, 0.65f };
float buildingColor2[] = { 0.85f, 0.8f, 0.75f };
float stoneBaseColor[] = { 0.75f, 0.75f, 0.70f };

// Brick pattern simulation
void drawBrickPattern(float width, float height, float depth) {
    float brickHeight = 0.25f;
    float brickWidth = 0.50f;
     float mortarThickness = 0.015f;

    for (float y = 0; y < height; y += brickHeight) {
        bool offset = ((int)(y / brickHeight) % 2 == 0);
        float startX = offset ? -brickWidth / 2 : 0;

        for (float x = startX; x < width / 2; x += brickWidth) {
            // Vary brick color slightly
            int colorChoice = ((int)(x * 10 + y * 10)) % 3;
            if (colorChoice == 0) glColor3fv(brickColor1);
            else if (colorChoice == 1) glColor3fv(brickColor2);
            else glColor3fv(brickColor3);

            // individual brick
            glPushMatrix();
                glTranslatef(x, y, depth / 2 + 0.005f);
                glBegin(GL_QUADS);
                    glVertex3f(-brickWidth/2 + 0.01f, 0, 0);
                    glVertex3f(brickWidth/2 - 0.01f, 0, 0);
                    glVertex3f(brickWidth/2 - 0.01f, brickHeight - 0.01f, 0);
                    glVertex3f(-brickWidth/2 + 0.01f, brickHeight - 0.01f, 0);
                glEnd();
            glPopMatrix();

            // Mirror on negative side
            if (x != 0) {
                glPushMatrix();
                    glTranslatef(-x, y, depth / 2 + 0.005f);
                    glBegin(GL_QUADS);
                        glVertex3f(-brickWidth/2 + 0.01f, 0, 0);
                        glVertex3f(brickWidth/2 - 0.01f, 0, 0);
                        glVertex3f(brickWidth/2 - 0.01f, brickHeight - 0.01f, 0);
                        glVertex3f(-brickWidth/2 + 0.01f, brickHeight - 0.01f, 0);
                    glEnd();
                glPopMatrix();
            }
        }
    }
}

void drawTaperedHand(float length, float width, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLES);
        glVertex3f(-width / 2, 0.0f, 0.0f);
        glVertex3f(width / 2, 0.0f, 0.0f);
        glVertex3f(0.0f, length, 0.0f);
    glEnd();
}

// tall arched windows
void drawArchedWindow(float yPos, float xOffset) {
    glColor3fv(windowColor);
    float winWidth = 0.35f;
    float winHeight = 1.2f;
    float archRadius = winWidth / 2;

    glPushMatrix();
        glTranslatef(xOffset, yPos, towerBaseSize / 2 + 0.01f);

        // Window rectangle
        glBegin(GL_QUADS);
            glVertex3f(-winWidth/1, -winHeight/2, 0);
            glVertex3f(winWidth/1, -winHeight/2, 0);
            glVertex3f(winWidth/1, winHeight/2 - archRadius, 0);
            glVertex3f(-winWidth/1, winHeight/2 - archRadius, 0);
        glEnd();

        // Arched top
        glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0, winHeight/2 - archRadius, 0);
            for (int i = 0; i <= 20; i++) {
                float angle = PI * i / 20.0f;
                glVertex3f(archRadius * cosf(angle), winHeight/2 - archRadius + archRadius * sinf(angle), 0);
            }
        glEnd();
    glPopMatrix();
}

void drawClockFace() {
    // White clock face
    glColor3f(0.98f, 0.98f, 0.95f);
    GLUquadric* quad = gluNewQuadric();
    gluDisk(quad, 0.0f, clockRadius, 50, 1);
    // Black outer ring
    glColor3f(0.05f, 0.05f, 0.05f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    for(int i=0; i<360; i++) {
        float rad = i * PI / 180.0f;
        glVertex2f(cosf(rad) * clockRadius, sinf(rad) * clockRadius);
    }
    glEnd();

    // Hour markers
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    for(int i=0; i<12; i++) {
        float angle = i * 30.0f * PI / 180.0f;
        float innerR = clockRadius - 0.12f;
        glVertex2f(cosf(angle) * innerR, sinf(angle) * innerR);
        glVertex2f(cosf(angle) * clockRadius, sinf(angle) * clockRadius);
    }
    glEnd();

    // Minute markers
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    for(int i=0; i<60; i++) {
        if (i % 5 != 0) { // Skip hour positions
            float angle = i * 6.0f * PI / 180.0f;
            float innerR = clockRadius - 0.06f;
            glVertex2f(cosf(angle) * innerR, sinf(angle) * innerR);
            glVertex2f(cosf(angle) * clockRadius, sinf(angle) * clockRadius);
        }
    }
    glEnd();

    // Clock hands with actual time
    time_t timer;
    time(&timer);
    struct tm* tm_info = localtime(&timer);
    float h_angle = (360.0f/12.0f)*(tm_info->tm_hour%12) + (tm_info->tm_min/60.0f)*30.0f;
    float m_angle = (360.0f/60.0f)*tm_info->tm_min;
    float s_angle = (360.0f/60.0f)*tm_info->tm_sec;

    // Hour hand (short, thick)
    glPushMatrix();
        glTranslatef(0,0,0.01f);
        glRotatef(-h_angle, 0, 0, 1);
        drawTaperedHand(0.35f, 0.09f, 0.1f, 0.1f, 0.1f);
    glPopMatrix();

    // Minute hand (long, medium)
    glPushMatrix();
        glTranslatef(0,0,0.02f);
        glRotatef(-m_angle, 0, 0, 1);
        drawTaperedHand(0.5f, 0.07f, 0.1f, 0.1f, 0.1f);
    glPopMatrix();

    //second hand
    glPushMatrix();
        glTranslatef(0,0,0.03f);
        glRotatef(-s_angle, 0, 0, 1);
        drawTaperedHand(0.52f, 0.06f, 0.1f, 0.1f, 0.1f);
    glPopMatrix();

    // Center cap
    glColor3f(0.1f, 0.1f, 0.1f);
    glTranslatef(0, 0, 0.03f);
    gluDisk(quad, 0.0f, 0.04f, 20, 1);

    gluDeleteQuadric(quad);
}

//circular stepped base
void drawCircularBase() {
    int steps = 4;
    float stepHeight = 0.2f;

    for (int i = 0; i < steps; i++) {
        float radius = 3.0f - i * 0.4f;
        glColor3fv(stoneBaseColor);

        glPushMatrix();
            glTranslatef(0, i * stepHeight, 0);
            glRotatef(-90, 1, 0, 0);

            //step as cylinder
            GLUquadric* quad = gluNewQuadric();
            gluCylinder(quad, radius, radius, stepHeight, 40, 1);

            //top cap
            glPushMatrix();
                glTranslatef(0, 0, stepHeight);
                gluDisk(quad, 0, radius, 40, 1);
            glPopMatrix();

            gluDeleteQuadric(quad);
        glPopMatrix();
    }
}

void drawTowerStructure() {
    // Circular stepped base platform
    drawCircularBase();

    // Main tower body with brick texture
    glColor3fv(brickColor2);
    glPushMatrix();
        glTranslatef(0, towerHeight / 2 + 0.8f, 0);
        glScalef(towerBaseSize, towerHeight, towerBaseSize);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Apply brick pattern on all 4 sides
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
            glRotatef(i * 90.0f, 0, 1, 0);
            drawBrickPattern(towerBaseSize, towerHeight, towerBaseSize);
        glPopMatrix();
    }

    // Decorative stone bands (horizontal accents)
    glColor3f(0.80f, 0.75f, 0.70f);
    for (float h = 2.5f; h < towerHeight; h += 2.5f) {
        glPushMatrix();
            glTranslatef(0, h + 0.8f, 0);
            glScalef(towerBaseSize + 0.08f, 0.12f, towerBaseSize + 0.08f);
            glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Windows and Clocks on all 4 sides
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
            glRotatef(i * 90.0f, 0, 1, 0);

            // Lower arched windows
            drawArchedWindow(2.5f, 0);
            drawArchedWindow(4.8f, 0);

            // Clock face near top
            glTranslatef(0.0f, towerHeight - 0.8f, towerBaseSize/2 + 0.02f);
            drawClockFace();
        glPopMatrix();
    }

// Clock housing cap
    glPushMatrix();
        glTranslatef(0, towerHeight + 0.8f, 0);
        glColor3f(0.26f, 0.14f, 0.14f);
        glScalef(towerBaseSize + 0.15f, 1.0f, towerBaseSize + 0.15f);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Upper decorative band (base of roof structure)
    glPushMatrix();
        glTranslatef(0, towerHeight + 1.15f, 0);
        glColor3fv(stoneBaseColor);
        glScalef(towerBaseSize + 0.25f, 0.13f, towerBaseSize + 0.25f);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Bell tower / cupola section (open arched structure)
    float cupolaHeight = 0.5f;
    float cupolaWidth = towerBaseSize * 0.7f;

    // Cupola pillars
    glColor3f(0.75f, 0.70f, 0.65f);
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
            glRotatef(i * 90.0f, 0, 1, 0);
            glTranslatef(cupolaWidth / 2, towerHeight + 1.25f + cupolaHeight / 2, 0);
            glScalef(0.15f, cupolaHeight, 0.15f);
            glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Cupola top band
    glPushMatrix();
        glTranslatef(0, towerHeight + 1.25f + cupolaHeight, 0);
        glColor3f(0.26f, 0.14f, 0.14f);
        glScalef(cupolaWidth + 0.2f, 0.15f, cupolaWidth + 0.2f);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Main pyramid roof
    glPushMatrix();
        glTranslatef(0, towerHeight + 1.25f + cupolaHeight + 0.15f, 0);
        glColor3f(0.35f, 0.32f, 0.30f); // Dark grey roof color
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(cupolaWidth / 1.2f, 1.8f, 4, 1);
    glPopMatrix();

    // Roof peak decoration (finial)
    glPushMatrix();
        glTranslatef(0, towerHeight + 1.25f + cupolaHeight + 1.95f, 0);
        glColor3f(0.6f, 0.55f, 0.5f);
        glutSolidSphere(0.12f, 10, 10);
    glPopMatrix();

    // Pointed top
    glPushMatrix();
        glTranslatef(0, towerHeight + 1.25f + cupolaHeight + 2.07f, 0);
        glColor3f(0.5f, 0.45f, 0.4f);
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(0.08f, 0.4f, 8, 1);
    glPopMatrix();
}

// Draw a building
void drawBuilding(float width, float height, float depth, float* color) {
    glColor3fv(color);
    glPushMatrix();
        glTranslatef(0, height/2, 0);
        glScalef(width, height, depth);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Windows
    glColor3f(0.3f, 0.4f, 0.5f);
    for(float y = 1.0f; y < height - 0.5f; y += 1.5f) {
        for(float x = -width/2 + 0.6f; x < width/2 - 0.4f; x += 0.9f) {
            glPushMatrix();
                glTranslatef(x, y, depth/2 + 0.01f);
                glScalef(0.4f, 0.8f, 0.01f);
                glutSolidCube(1.0f);
            glPopMatrix();
            glPushMatrix();
                glTranslatef(x, y, -depth/2 - 0.02f); // Back windows
                glScalef(0.5f, 0.8f, 0.01f);
                glutSolidCube(1.0f);
            glPopMatrix();
        }
    }

    // Roof
    glColor3f(0.3f, 0.2f, 0.1f);
    glPushMatrix();
        glTranslatef(0, height, 0);
        glRotatef(-90, 1, 0, 0);
        glRotatef(45, 0, 0, 1);
        float roofRadius = width * 0.707f;
        float roofHeight = 2.0f;
        glutSolidCone(roofRadius, roofHeight, 4, 1);
    glPopMatrix();
}
void drawStreetLamp(float x, float z, float rotation) {
    glPushMatrix();
        glDisable(GL_LIGHTING); // Ensure visibility
        glTranslatef(x, 0, z);
        glRotatef(rotation, 0, 1, 0);

        // Pole (Dark Grey)
        glEnable(GL_LIGHTING); // Changed: enable lighting for pole
        glColor3f(0.25f, 0.25f, 0.25f);
        glPushMatrix();
            glTranslatef(0, 4.0f, 0); // Center of the 8-unit pole
            glScalef(0.15f, 8.0f, 0.15f);
            glutSolidCube(1.0f);
        glPopMatrix();

        // Arm (the part that sticks out)
        glPushMatrix();
            glTranslatef(0, 8.0f, 0.5f);
            glScalef(0.12f, 0.12f, 1.0f);
            glutSolidCube(1.0f);
        glPopMatrix();

        // Lamp housing (darker)
        glColor3f(0.15f, 0.15f, 0.15f);
        glPushMatrix();
            glTranslatef(0, 7.8f, 1.0f);
            glScalef(0.5f, 0.3f, 0.5f);
            glutSolidCube(1.0f);
        glPopMatrix();

        //Improve blub with glow effect
        glDisable(GL_LIGHTING); // Disable for emissive effect
        
        if(isNightTime) {
            // Outer glow (larger, semi-transparent)
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 0.95f, 0.7f, 0.3f);
            glPushMatrix();
                glTranslatef(0, 7.5f, 1.0f);
                glutSolidSphere(0.7f, 16, 16);
            glPopMatrix();
            
            // Middle glow
            glColor4f(1.0f, 0.95f, 0.7f, 0.6f);
            glPushMatrix();
                glTranslatef(0, 7.5f, 1.0f);
                glutSolidSphere(0.5f, 16, 16);
            glPopMatrix();
            
            // Inner bright core
            glColor4f(1.0f, 1.0f, 0.9f, 1.0f);
            glPushMatrix();
                glTranslatef(0, 7.5f, 1.0f);
                glutSolidSphere(0.35f, 12, 12);
            glPopMatrix();
            glDisable(GL_BLEND);
            
        } else {
            // Dim bulb during day
            glColor3f(0.7f, 0.7f, 0.5f);
            glPushMatrix();
                glTranslatef(0, 7.5f, 1.0f);
                glutSolidSphere(0.35f, 12, 12);
            glPopMatrix();
        }
        
        glEnable(GL_LIGHTING);
    glPopMatrix();
}
void drawTrafficLight(float x, float z, float rotation) {
    glPushMatrix();
        glDisable(GL_LIGHTING);
        glTranslatef(x, 0, z);
        glRotatef(rotation, 0, 1, 0);

        // Pole
        glColor3f(0.1f, 0.1f, 0.1f);
        glPushMatrix();
            glTranslatef(0, 3.0f, 0);
            glScalef(0.3f, 6.0f, 0.3f);
            glutSolidCube(1.0f);
        glPopMatrix();

        // Light Box
        glPushMatrix();
            glTranslatef(0, 6.0f, 0);
            glScalef(1.0f, 2.5f, 1.0f);
            glutSolidCube(1.0f);
        glPopMatrix();

        // Red Light
        glColor3f(1.0f, 0.0f, 0.0f);
        glPushMatrix();
            glTranslatef(0, 6.8f, 0.55f);
            glutSolidSphere(0.3f, 10, 10);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    glPopMatrix();
}
void drawCar(float x, float z, float rotation, float r, float g, float b) {
    glPushMatrix();
        glTranslatef(x, 1.0f, z); // Move car to position (slightly above road)
        glRotatef(rotation, 0, 1, 0);

        // Body (Lower part)
        glColor3f(r, g, b);
        glPushMatrix();
            glScalef(2.0f, 0.8f, 4.0f);
            glutSolidCube(1.0f);
        glPopMatrix();

        // Roof (Upper part)
        glPushMatrix();
            glTranslatef(0, 0.7f, -0.2f);
            glScalef(1.8f, 0.7f, 2.0f);
            glutSolidCube(1.0f);
        glPopMatrix();

        // Wheels (Black)
        glColor3f(0.1f, 0.1f, 0.1f);
        float wheelPos[4][2] = {{0.9, 1.2}, {-0.9, 1.2}, {0.9, -1.2}, {-0.9, -1.2}};
        for(int i = 0; i < 4; i++) {
            glPushMatrix();
                glTranslatef(wheelPos[i][0], -0.4f, wheelPos[i][1]);
                glutSolidSphere(0.4f, 10, 10);
            glPopMatrix();
        }
    glPopMatrix();
}

// Draw roads
void drawRoads() {
    glColor3fv(roadColor);

    // Road going North
    glPushMatrix();
        glTranslatef(0, 0.01f, -30);
        glScalef(8.0f, 0.01f, 60.0f);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Road going Southeast
    glPushMatrix();

        glRotatef(45, 0, 1, 0);
         glTranslatef(0, 0.01f, 40.0f);
        glScalef(8.0f, 0.01f, 82.0f);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Road going Southwest
    glPushMatrix();

        glRotatef(-45, 0, 1, 0);
        glTranslatef(0, 0.01f, 40.0f);
        glScalef(8.0f, 0.01f, 82.0f);
        glutSolidCube(1.0f);
    glPopMatrix();

    // Central junction
    glPushMatrix();
        glTranslatef(0, 0.01f, 0);
        glScalef(8.0f, 0.01f, 8.0f);
        glutSolidCube(1.0f);
    glPopMatrix();

    //draw north road crossing
    glColor3f(1.0f, 1.0f, 1.0f); // Pure white

    float crossingZ = -25.0f; // Positioned at the start of the North Road
    float stripeWidth = 0.6f;
    float stripeLength = 3.0f;
    float gap = 0.4f;

    // loop from -4 to 4 (the 8-unit width of road)
    for (float x = -3.5f; x <= 3.5f; x += (stripeWidth + gap)) {
        glPushMatrix();
            glTranslatef(x, 0.025f, crossingZ); // Slightly higher than road and center markings
            glScalef(stripeWidth, 0.01f, stripeLength);
            glutSolidCube(1.0f);
        glPopMatrix();
    }

    //Draw road markings
    glColor3f(1.0f, 1.0f, 1.0f); // Pure white
    glLineWidth(2.0f);

    // Marking for North Road
    glPushMatrix();
        glTranslatef(0, 0.02f, 0); // Lift slightly above road
        glBegin(GL_LINES);
            glVertex3f(0, 0, -6.0f);  // Start near center
            glVertex3f(0, 0, -60.0f); // End at road tip
        glEnd();
    glPopMatrix();

    // Marking for Southeast Road
    glPushMatrix();
        glRotatef(45, 0, 1, 0);
        glTranslatef(0, 0.02f, 0);
        glBegin(GL_LINES);
            glVertex3f(0, 0, 4.0f);   // Start near center
            glVertex3f(0, 0, 80.0f);  // End at road tip
        glEnd();
    glPopMatrix();

    // Marking for Southwest Road
    glPushMatrix();
        glRotatef(-45, 0, 1, 0);
        glTranslatef(0, 0.02f, 0);
        glBegin(GL_LINES);
            glVertex3f(0, 0, 4.0f);   // Start near center
            glVertex3f(0, 0, 80.0f);  // End at road tip
        glEnd();
    glPopMatrix();

    //ADDING STREET LAMPS
    // Place them at the junction corners
    drawStreetLamp(5.0f, -5.0f, 45.0f);   // North-East corner
    drawStreetLamp(-5.0f, -5.0f, -45.0f); // North-West corner
    drawStreetLamp(0.0f, 7.0f, 180.0f);   // Southern split

    //ADDING TRAFFIC LIGHT
    // Place one at the stop line of the North road
    drawTrafficLight(4.0f, -23.0f, 0.0f);

    // Car 1: Red car on the North Road 
    drawCar(2.0f, -30.0f, 0.0f, 0.8f, 0.1f, 0.1f);

    // Car 2: Blue car on the Southeast Road
    glPushMatrix();
        glRotatef(45, 0, 1, 0);
        drawCar(-2.0f, 40.0f, 0.0f, 0.1f, 0.3f, 0.8f);
    glPopMatrix();

    // Car 3: Green car on the Southwest Road
    glPushMatrix();
        glRotatef(-45, 0, 1, 0);
        drawCar(2.0f, 15.0f, 180.0f, 0.1f, 0.6f, 0.2f);
    glPopMatrix();

    // Car 4: Yellow car
    drawCar(-2.0f, -10.0f, 180.0f, 0.9f, 0.8f, 0.1f);

    float redBuilding[] = {0.6f, 0.2f, 0.2f};
glPushMatrix();
    glTranslatef(15, 0, -20); // Position it 15 units right and 20 units back
    drawBuilding(6.0f, 10.0f, 6.0f, redBuilding);
glPopMatrix();

 float newBuilding[] = {0.99f, 0.92f, 0.82f};
glPushMatrix();
    glTranslatef(-15, 0, -20); // Position it 15 units right and 20 units back
    drawBuilding(5.0f, 10.0f, 6.0f, newBuilding);
glPopMatrix();
}

void drawLampLightPools() {
    if(!isNightTime) return;
    
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw light pools on ground beneath each lamp
    float lampGroundPos[3][2] = {
        {5.0f, -5.0f},
        {-5.0f, -5.0f},
        {0.0f, 7.0f}
    };
    
    for(int i = 0; i < 3; i++) {
        glPushMatrix();
            glTranslatef(lampGroundPos[i][0], 0.03f, lampGroundPos[i][1]);
            glRotatef(-90, 1, 0, 0);
            
            //gradient circle for light pool
            glBegin(GL_TRIANGLE_FAN);
                // Bright center
                glColor4f(1.0f, 0.95f, 0.7f, 0.4f);
                glVertex3f(0, 0, 0);
                
                // Fade to transparent at edges
                glColor4f(1.0f, 0.95f, 0.7f, 0.0f);
                for(int angle = 0; angle <= 360; angle += 20) {
                    float rad = angle * PI / 180.0f;
                    float radius = 8.0f; // Light pool size
                    glVertex3f(radius * cosf(rad), radius * sinf(rad), 0);
                }
            glEnd();
        glPopMatrix();
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}
void drawTree(float x, float z) {
    glPushMatrix();
        glTranslatef(x, 0, z);

        //Trunk
        glColor3f(0.35f, 0.22f, 0.15f); // Slightly darker brown
        glPushMatrix();
            glTranslatef(0, 1.0f, 0); 
            glScalef(0.4f, 2.0f, 0.4f);
            glutSolidCube(1.0f);
        glPopMatrix();

       
        glColor3f(0.13f, 0.45f, 0.18f); // Rich forest green
        
        // Center mass
        glPushMatrix();
            glTranslatef(0, 2.5f, 0);
            glutSolidSphere(1.2f, 16, 16);
        glPopMatrix();

        // Side puffs for irregular shape
        float offsets[4][3] = {
            {0.7f, 2.2f, 0.5f}, {-0.7f, 2.4f, -0.3f}, 
            {0.4f, 2.8f, -0.6f}, {-0.5f, 2.1f, 0.7f}
        };

        for(int i = 0; i < 4; i++) {
            glPushMatrix();
                glTranslatef(offsets[i][0], offsets[i][1], offsets[i][2]);
                glutSolidSphere(0.8f, 12, 12);
            glPopMatrix();
        }
    glPopMatrix();
}

void drawSidewalk(float length, float width) {
    glColor3f(0.7f, 0.7f, 0.7f); // Light grey concrete
    glPushMatrix();
        // The sidewalk is a thin slab
        glScalef(width, 0.15f, length);
        glutSolidCube(1.0f);
    glPopMatrix();

    //Add cracks to the sidewalk
    glColor3f(0.5f, 0.5f, 0.5f); // Darker grey for lines
    glLineWidth(1.0f);
    for(float i = -length/2; i <= length/2; i += 2.0f) {
        glBegin(GL_LINES);
            glVertex3f(-width/2, 0.08f, i);
            glVertex3f(width/2, 0.08f, i);
        glEnd();
    }
}

void drawGround() {
    float groundSize = 60.0f;
    float tileSize = 4.0f;
    
    glEnable(GL_LIGHTING); // Changed from glDisable
    
    // Draw concrete tiles
    for (float x = -groundSize; x < groundSize; x += tileSize) {
        for (float z = -groundSize; z < groundSize; z += tileSize) {
            
            if (((int)(x/tileSize) + (int)(z/tileSize)) % 2 == 0)
                glColor3f(0.45f, 0.45f, 0.47f);
            else
                glColor3f(0.48f, 0.48f, 0.50f);

            glBegin(GL_QUADS);
                glNormal3f(0, 1, 0);
                glVertex3f(x, 0, z);
                glVertex3f(x + tileSize, 0, z);
                glVertex3f(x + tileSize, 0, z + tileSize);
                glVertex3f(x, 0, z + tileSize);
            glEnd();
        }
    }
    
    // Add dirt/grass patches
    srand(42); // Fixed seed for consistent patches
    for(int i = 0; i < 30; i++) {
        float x = -50 + (rand() % 100);
        float z = -50 + (rand() % 100);
        
        // Skip if on road area
        if((abs(x) < 5 && z < 0) || 
           (abs(x - z) < 7 && x > 0 && z > 0) ||
           (abs(x + z) < 7 && x < 0 && z > 0)) continue;
        
        // Random dirt or grass
        if(i % 3 == 0)
            glColor3f(0.3f, 0.5f, 0.25f); // Grass patch
        else
            glColor3f(0.38f, 0.32f, 0.28f); // Dirt patch
        
        glBegin(GL_POLYGON);
        for(int j = 0; j < 6; j++) {
            float angle = j * PI / 3.0f;
            float r = 0.8f + (rand() % 10) / 10.0f;
            glVertex3f(x + r * cos(angle), 0.01f, z + r * sin(angle));
        }
        glEnd();
    }
}

// Draw town buildings
void drawTown() {

float b1[] = {0.98f, 0.92f, 0.57f}; 
    float b2[] = {0.95f, 0.90f, 0.79f}; 
    float b3[] = {0.73f, 0.56f, 0.82f}; 

    glPushMatrix();
        glTranslatef(8.5, 0, -10);
        drawBuilding(3.0f, 4.0f, 3.0f, buildingColor2);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-8.5, 0, -17);
        drawBuilding(3.2f, 5.2f, 3.2f, buildingColor2);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(8.5, 0, -18);
        drawBuilding(3.8f, 4.8f, 3.5f, buildingColor1);
    glPopMatrix();

    //SOUTHEAST ROAD
    glPushMatrix();
        glRotatef(45, 0, 1, 0); // Align coordinate system to SE road
        for (int i = 1; i < 5; i++) {
            float zPos = 10.0f + (i * 9.0f);
            // "Left" side of the SE road (looking away from center)
            glPushMatrix();
                glTranslatef(-9.0f, 0, zPos);
                drawBuilding(5.0f, 6.0f, 5.0f, b2);
            glPopMatrix();
            // "Right" side of the SE road
            glPushMatrix();
                glTranslatef(9.0f, 0, zPos);
                drawBuilding(5.0f, 5.0f, 5.0f, b3);
            glPopMatrix();
        }
    glPopMatrix();

    //SOUTHWEST ROAD
    glPushMatrix();
        glRotatef(-45, 0, 1, 0); // Align coordinate system to SW road
        for (int i = 1; i < 5; i++) {
            float zPos = 10.0f + (i * 9.0f);
            // "Left" side
            glPushMatrix();
                glTranslatef(-9.0f, 0, zPos);
                drawBuilding(5.0f, 7.0f, 5.0f, b1);
            glPopMatrix();
            // "Right" side
            glPushMatrix();
                glTranslatef(9.0f, 0, zPos);
                drawBuilding(5.0f, 4.0f, 5.0f, b2);
            glPopMatrix();
        }
    glPopMatrix();
}
void drawAllSidewalks() {
    float swWidth = 2.0f; // Width of the sidewalk
    float roadHalfWidth = 4.0f; // Half of 8-unit road
    float offset = roadHalfWidth + swWidth/2;

    //junction_edge + (total_length / 2)
    float diagCenterZ = 45.0f;
    float diagLength = 82.0f;

    // 1. North Road Sidewalks (Left and Right)
    glPushMatrix();
        glTranslatef(-(roadHalfWidth + swWidth/2), 0.05f, -31.50f);
        drawSidewalk(60.0f, swWidth);
    glPopMatrix();
    glPushMatrix();
        glTranslatef((roadHalfWidth + swWidth/2), 0.05f, -31.50f);
        drawSidewalk(60.0f, swWidth);
    glPopMatrix();

    // 2. Southeast Road Sidewalks
    glPushMatrix();
        glRotatef(45, 0, 1, 0);
        glPushMatrix();
        glTranslatef(-offset , 0.05f, 43.0f);
        drawSidewalk(78.0f, swWidth);
    glPopMatrix();
    glPushMatrix();

        glTranslatef(offset, 0.05f, 41.0f);
        drawSidewalk(78.0f, swWidth);
    glPopMatrix();
    glPopMatrix();

    // 3. Southwest Road Sidewalks
    glPushMatrix();
        glRotatef(-45, 0, 1, 0);
        glPushMatrix();
        glTranslatef(-offset, 0.05f, 41.0f);
        drawSidewalk(78.0f, swWidth);

     glPopMatrix();
        glPushMatrix();
            glTranslatef(offset, 0.05f, 43.0f);
            drawSidewalk(78.0f, swWidth);
        glPopMatrix();
    glPopMatrix();

}
// cloudy sky
void drawSky() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // UPDATE SKY GRADIENT FOR DAY/NIGHT
    glBegin(GL_QUADS);
        if(isNightTime) {
            // Night horizon - dark blue
            glColor3f(0.05f, 0.08f, 0.15f);
            glVertex3f(-1, -1, -0.99f);
            glVertex3f(1, -1, -0.99f);
            
            // Night sky - very dark blue
            glColor3f(0.01f, 0.02f, 0.08f);
            glVertex3f(1, 1, -0.99f);
            glVertex3f(-1, 1, -0.99f);
        } else {
            // Day horizon - light blue with slight warmth
            glColor3f(0.75f, 0.85f, 0.95f);
            glVertex3f(-1, -1, -0.99f);
            glVertex3f(1, -1, -0.99f);
            
            // Day top - deeper blue
            glColor3f(0.40f, 0.65f, 0.90f);
            glVertex3f(1, 1, -0.99f);
            glVertex3f(-1, 1, -0.99f);
        }
    glEnd();

    // Add atmospheric haze near horizon
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBegin(GL_QUADS);
        glColor4f(0.95f, 0.95f, 0.98f, 0.3f); // White haze
        glVertex3f(-1, -1, -0.98f);
        glVertex3f(1, -1, -0.98f);
        
        glColor4f(0.95f, 0.95f, 0.98f, 0.0f); // Fade to transparent
        glVertex3f(1, -0.3f, -0.98f);
        glVertex3f(-1, -0.3f, -0.98f);
    glEnd();

    // clouds using random-looking
    srand(12345); // Fixed seed for consistent clouds
    
    for(int i = 0; i < 35; i++) {
        float x = -1.2f + (rand() % 240) / 100.0f;
        float y = -0.6f + (rand() % 170) / 100.0f;
        float scaleX = 0.15f + (rand() % 25) / 100.0f;
        float scaleY = 0.08f + (rand() % 15) / 100.0f;
        float alpha = 0.3f + (rand() % 40) / 100.0f;
        
        drawCloud(x, y, scaleX, scaleY, alpha);
    }

    glDisable(GL_BLEND);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    // PARK AREAS (Between Roads)
    // Park 1: Between North Road and Southeast Road
    for(int i = 0; i < 5; i++) {
        drawTree(14.0f + (i * 2), 5.0f - (i * 3));
        drawTree(21.0f + (i * 2), 0.0f);
    }

    // Park 2: Between North Road and Southwest Road
    for(int i = 0; i < 5; i++) {
        drawTree(-14.0f - (i * 2), 5.0f - (i * 3));
        drawTree(-21.0f - (i * 2), 0.0f);
    }

    // Park 3: The "Deep South"
    for(float x = -25; x <= 20; x += 5) {
        for(float z = 20; z <= 50; z += 5) {
            if(abs(x) < 4)
                drawTree(x, z + 10);
        }
    }
}

// Perlin-like noise function for organic variation
float noise2D(float x, float y) {
    int n = (int)x + (int)y * 57;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float smoothNoise(float x, float y) {
    float corners = (noise2D(x-1, y-1) + noise2D(x+1, y-1) + 
                     noise2D(x-1, y+1) + noise2D(x+1, y+1)) / 16.0f;
    float sides = (noise2D(x-1, y) + noise2D(x+1, y) + 
                   noise2D(x, y-1) + noise2D(x, y+1)) / 8.0f;
    float center = noise2D(x, y) / 4.0f;
    return corners + sides + center;
}

// Draw a single cloud puff with gradient
void drawCloudPuff(float cx, float cy, float radius, float alpha, float brightness) {
    const int segments = 32;
    
    glBegin(GL_TRIANGLE_FAN);
        // Bright center
        glColor4f(brightness, brightness, brightness, alpha * 0.9f);
        glVertex3f(cx, cy, -0.97f);
        
        // Soft gradient edges
        for(int i = 0; i <= segments; i++) {
            float angle = (i * 2.0f * PI) / segments;
            float x = cx + radius * cosf(angle);
            float y = cy + radius * sinf(angle);
            
            // Add noise to edge
            float noiseVal = smoothNoise(x * 10.0f, y * 10.0f);
            float edgeVariation = 0.85f + noiseVal * 0.15f;
            
            // Softer edges with gradient
            glColor4f(brightness * 0.95f, brightness * 0.95f, brightness * 0.95f, 
                     alpha * 0.3f * edgeVariation);
            glVertex3f(cx + radius * edgeVariation * cosf(angle), 
                      cy + radius * edgeVariation * sinf(angle), -0.97f);
        }
    glEnd();
}

// Main cloud drawing function 
void drawCloud(float x, float y, float scaleX, float scaleY, float alpha) {
    // Base brightness with subtle variation
    float baseBrightness = 0.95f + (rand() % 10) / 100.0f;
    
    // Define multiple puffs for volumetric appearance
    struct CloudPuff {
        float offsetX, offsetY;
        float radius;
        float brightness;
    };
    
    // Create irregular cloud structure with 7-12 puffs
    int numPuffs = 7 + rand() % 6;
    CloudPuff puffs[12];
    
    // Generate puff positions using pseudo-random distribution
    for(int i = 0; i < numPuffs; i++) {
        float angle = (i * 2.0f * PI / numPuffs) + (rand() % 100) / 100.0f;
        float distance = (0.3f + (rand() % 50) / 100.0f) * scaleX;
        
        puffs[i].offsetX = distance * cosf(angle);
        puffs[i].offsetY = distance * sinf(angle) * 0.7f; // Slightly flatter
        puffs[i].radius = scaleX * (0.4f + (rand() % 40) / 100.0f);
        puffs[i].brightness = baseBrightness - (rand() % 15) / 100.0f;
    }
    
    // Draw puffs from back to front for depth
    for(int i = 0; i < numPuffs; i++) {
        drawCloudPuff(x + puffs[i].offsetX, 
                     y + puffs[i].offsetY, 
                     puffs[i].radius, 
                     alpha * (0.7f + (rand() % 30) / 100.0f),
                     puffs[i].brightness);
    }
    
    // Add central mass for density
    drawCloudPuff(x, y, scaleX * 0.65f, alpha * 1.0f, baseBrightness);
    
    // Add highlight puffs on top
    for(int i = 0; i < 3; i++) {
        float highlightX = x + (scaleX * 0.3f * cosf(i * 2.1f));
        float highlightY = y + (scaleY * 0.2f * sinf(i * 1.8f));
        
        drawCloudPuff(highlightX, highlightY, 
                     scaleX * 0.35f, 
                     alpha * 0.6f, 
                     0.98f);
    }
    
    // Subtle wispy edges
    for(int i = 0; i < 5; i++) {
        float angle = (rand() % 360) * PI / 180.0f;
        float wispX = x + scaleX * 1.1f * cosf(angle);
        float wispY = y + scaleY * 0.8f * sinf(angle);
        
        drawCloudPuff(wispX, wispY, 
                     scaleX * 0.25f, 
                     alpha * 0.2f, 
                     0.92f);
    }
}

void drawSun() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 1. Position the Sun
    float sunDist = 45.0f; 
    float sunX = sunDist * sinf(sunAngleY * 3.14159f / 180.0f) * cosf(sunAngleX * 3.14159f / 180.0f);
    float sunY = sunDist * sinf(sunAngleX * 3.14159f / 180.0f);
    float sunZ = sunDist * cosf(sunAngleY * 3.14159f / 180.0f) * cosf(sunAngleX * 3.14159f / 180.0f);
    
    sunY -= 15.0f;  // Move sun down by 15 units 
    glPushMatrix();
    glTranslatef(sunX, sunY, sunZ);
    float scale = 1.2f; 
    
    // 3. Draw Wavy Sun Rays 
    int numRays = 16;
    float rayBaseRadius = 0.8f * scale;
    float rayTipRadius = 2.2f * scale;
    
    glBegin(GL_TRIANGLES);
    for(int i = 0; i < numRays; i++) {
        float angle1 = (i * 360.0f / numRays) * 3.14159f / 180.0f;
        float angle2 = ((i + 1) * 360.0f / numRays) * 3.14159f / 180.0f;
        float midAngle = (angle1 + angle2) / 2.0f;
        
        // Create wavy effect
        float waveOffset = 0.3f * scale * sinf(i * 2.5f);
        float currentRayTip = rayTipRadius + waveOffset;
        
        // Base points
        float x1Base = cosf(angle1) * rayBaseRadius;
        float y1Base = sinf(angle1) * rayBaseRadius;
        float x2Base = cosf(angle2) * rayBaseRadius;
        float y2Base = sinf(angle2) * rayBaseRadius;
        
        // Tip point
        float xTip = cosf(midAngle) * currentRayTip;
        float yTip = sinf(midAngle) * currentRayTip;
        
        glColor4f(1.0f, 0.7f, 0.1f, 0.9f);
        glVertex3f(x1Base, y1Base, 0.0f);
        
        glColor4f(1.0f, 0.7f, 0.1f, 0.9f);
        glVertex3f(x2Base, y2Base, 0.0f);
        
        glColor4f(1.0f, 0.9f, 0.3f, 0.3f);
        glVertex3f(xTip, yTip, 0.0f);
    }
    glEnd();
    
    // 4. Draw Outer Glow
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    float glowRadius = 2.8f * scale;
    
    glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 0.8f, 0.4f, 0.3f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        for(int i = 0; i <= 360; i += 15) {
            float rad = i * 3.14159f / 180.0f;
            glColor4f(1.0f, 0.7f, 0.2f, 0.0f);
            glVertex3f(cosf(rad) * glowRadius, sinf(rad) * glowRadius, 0.0f);
        }
    glEnd();
    
    // 5. Draw Main Solar Disc
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float coreRadius = 0.75f * scale;
    
    glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 0.95f, 0.6f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        for(int i = 0; i <= 360; i += 10) {
            float rad = i * 3.14159f / 180.0f;
            glColor4f(1.0f, 0.8f, 0.3f, 1.0f);
            glVertex3f(cosf(rad) * coreRadius, sinf(rad) * coreRadius, 0.0f);
        }
    glEnd();
    
    // 6. Draw Inner Highlight
    float highlightRadius = 0.4f * scale;
    
    glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 1.0f, 0.9f, 0.9f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        for(int i = 0; i <= 360; i += 10) {
            float rad = i * 3.14159f / 180.0f;
            glColor4f(1.0f, 0.95f, 0.7f, 0.7f);
            glVertex3f(cosf(rad) * highlightRadius, sinf(rad) * highlightRadius, 0.0f);
        }
    glEnd();
    
    glPopMatrix();
    glPopAttrib();
}
// shadow for clock tower
void drawClockTowerShadow() {
    // Calculate light position
    float lightX = 25.0f * sinf(sunAngleY * PI / 180.0f);
    float lightY = 25.0f * sinf(sunAngleX * PI / 180.0f);
    float lightZ = 25.0f * cosf(sunAngleY * PI / 180.0f);
    
    //draw shadow if sun is above horizon
    if(lightY <= 0.1f) return;
    
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.4f); // Semi-transparent shadow
    
    glPushMatrix();
    
    // Create shadow matrix for planar projection onto ground (y=0)
    float shadowMat[16] = {
        lightY, 0.0f, 0.0f, 0.0f,
        -lightX, 0.0f, -lightZ, 0.0f,
        0.0f, 0.0f, lightY, 0.0f,
        0.0f, 0.01f, 0.0f, lightY  // 0.01f lifts shadow slightly above ground
    };
    
    glMultMatrixf(shadowMat);
    
    // 1. Tower Base
    glPushMatrix();
    glTranslatef(0.0f, towerHeight/2 + 0.8f, 0.0f);
    glScalef(towerBaseSize, towerHeight, towerBaseSize);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // 2. Clock face position (front, back, left, right)
    for(int i = 0; i < 4; i++) {
        glPushMatrix();
        glRotatef(i * 90.0f, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, towerHeight + 0.5f, towerBaseSize/2 + 0.01f);
        glutSolidSphere(0.6f, 20, 20);
        glPopMatrix();
    }
    
    // 3. Upper tower section
    glPushMatrix();
    glTranslatef(0.0f, towerHeight + 1.5f, 0.0f);
    glScalef(towerBaseSize * 0.8f, 1.0f, towerBaseSize * 0.8f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // 4. Roof (cone)
    glPushMatrix();
    glTranslatef(0.0f, towerHeight + 2.0f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCone(towerBaseSize * 0.6f, 1.5f, 20, 20);
    glPopMatrix();
    
    // 5. Spire
    glPushMatrix();
    glTranslatef(0.0f, towerHeight + 3.5f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCone(towerBaseSize * 0.3f, 2.0f, 20, 20);
    glPopMatrix();
    
    // 6. Spire ball
    glPushMatrix();
    glTranslatef(0.0f, towerHeight + 5.5f, 0.0f);
    glutSolidSphere(0.3f, 15, 15);
    glPopMatrix();
    
    // 7. Door 
    glPushMatrix();
    glTranslatef(0.0f, 0.4f, towerBaseSize/2 + 0.01f);
    glScalef(0.5f, 0.8f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // 8. Windows 
    for(int level = 0; level < 3; level++) {
        for(int side = 0; side < 4; side++) {
            glPushMatrix();
            glRotatef(side * 90.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.3f, 1.5f + level * 1.2f, towerBaseSize/2 + 0.01f);
            glScalef(0.3f, 0.4f, 0.1f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }
    glPopMatrix();
    
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void setupLampLights() {
    if(sunAngleX < 30.0f) {
        isNightTime = true;
        
        // NIGHT FOG
        GLfloat nightFogColor[] = {0.02f, 0.03f, 0.08f, 1.0f};
        glFogfv(GL_FOG_COLOR, nightFogColor);
        glFogf(GL_FOG_DENSITY, 0.020f); // Denser at night
        
    } else {
        isNightTime = false;
        
        //DAY FOG
        GLfloat dayFogColor[] = {0.75f, 0.82f, 0.88f, 1.0f};
        glFogfv(GL_FOG_COLOR, dayFogColor);
        glFogf(GL_FOG_DENSITY, 0.012f); // Lighter during day
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawSky();
    glLoadIdentity();

    // Camera
    float lx = zoom * sinf(camAngleY * PI / 180.0f) * cosf(camAngleX * PI / 180.0f);
    float ly = zoom * sinf(camAngleX * PI / 180.0f);
    float lz = zoom * cosf(camAngleY * PI / 180.0f) * cosf(camAngleX * PI / 180.0f);
    gluLookAt(lx, ly + towerHeight/2, lz, 0, towerHeight/2, 0, 0, 1, 0);

    // Sun lighting
    GLfloat lightPos[4];
    lightPos[0] = 25.0f * sinf(sunAngleY * PI / 180.0f);
    lightPos[1] = 25.0f * sinf(sunAngleX * PI / 180.0f);
    lightPos[2] = 25.0f * cosf(sunAngleY * PI / 180.0f);
    lightPos[3] = 1.0f;
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    setupLampLights();
    drawSun();
    drawGround();
    drawLampLightPools(); 
    drawRoads();
    drawAllSidewalks();
    drawClockTowerShadow();
    drawTown();   
    drawTowerStructure();
    glutSwapBuffers();
}

void specialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_RIGHT) camAngleY += 5.0f;
    else if (key == GLUT_KEY_LEFT) camAngleY -= 5.0f;
    else if (key == GLUT_KEY_UP) camAngleX += 5.0f;
    else if (key == GLUT_KEY_DOWN) camAngleX -= 5.0f;
    glutPostRedisplay();
}

void normalKeys(unsigned char key, int x, int y) {
    if (key == '+' || key == '=') zoom -= 1.0f;
    else if (key == '-' || key == '_') zoom += 1.0f;
    else if (key == 'w') sunAngleX += 5.0f;
    else if (key == 's') sunAngleX -= 5.0f;
    else if (key == 'a') sunAngleY -= 5.0f;
    else if (key == 'd') sunAngleY += 5.0f;
    if (zoom < 5.0f) zoom = 5.0f;
    if (zoom > 50.0f) zoom = 50.0f;
    glutPostRedisplay();
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Warm daylight
    GLfloat ambient[] = {0.35f, 0.35f, 0.4f, 1.0f};
    GLfloat diffuse[] = {0.95f, 0.90f, 0.75f, 1.0f};
    GLfloat specular[] = {0.6f, 0.6f, 0.5f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

    GLfloat globalAmbient[] = {0.25f, 0.25f, 0.3f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);  
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w/h, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 900);
    glutCreateWindow("Realistic Brick Clock Tower - Three-Way Junction");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(normalKeys);
    glutIdleFunc(display);
    glutMainLoop();
    return 0;
}
