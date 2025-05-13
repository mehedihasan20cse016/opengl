#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <time.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Texture variables
GLuint roadTexture, buildingTexture, grassTexture;

// Window and scene parameters
int WINDOW_WIDTH = 1000;
int WINDOW_HEIGHT = 700;
enum LightState { RED, YELLOW, GREEN };
LightState currentLight = RED;
int lightTimer = 0;
bool automaticMode = false;
bool dayMode = true;
bool streetLightsManuallyToggled = false;

// Camera parameters
enum CameraView { DEFAULT, OVERHEAD, SIDE, FOLLOW, FRONT };
CameraView currentView = DEFAULT;
size_t followCarIndex = 0;
float yaw = 90.0f;
float pitch = 30.0f;
float distance = 280.0f;
float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 0.0f;
float scaleFactor = 1.0f;
int lastX = 0, lastY = 0;
bool leftButtonHeld = false;

// Scene objects and constants
bool allStreetLightsOn = true;
const int RED_DURATION = 200;
const int YELLOW_DURATION = 70;
const int GREEN_DURATION = 200;
struct Car {
    float x, y, z;
    float speed;
    float width, height, depth;
    float color[3];
    bool stopped;
};
std::vector<Car> cars;
float carGenerationTimer = 0;
float carGenerationRate = 0.03;
struct BuildingData {
    float x_offset, z_pos_factor;
    float width, height, depth;
    float color[3];
};
std::vector<BuildingData> building_definitions;
struct TreeData {
    float x_offset, z_pos_factor;
    float trunkHeight;
    float crownRadius;
    float crownColor[3];
};
std::vector<TreeData> tree_definitions;
struct StreetLightData {
    float x_offset, z_pos_factor;
    float poleHeight;
    float lightRadius;
    bool isOn;
};
std::vector<StreetLightData> streetLight_definitions;
const float ROAD_Y_LEVEL = 0.0f;
const float ROAD_WIDTH_X = 1200.0f;
const float ROAD_DEPTH_Z = 100.0f;
const float LIGHT_POLE_X = ROAD_WIDTH_X * 0.25f;
const float LIGHT_POLE_Z = -ROAD_DEPTH_Z / 2.0f - 10.0f;
const float LIGHT_Y_OFFSET = ROAD_Y_LEVEL;
const float LIGHT_POLE_HEIGHT = 70.0f;
const float LIGHT_BOX_HEIGHT = 45.0f;
const float LIGHT_RADIUS = 6.0f;
const float STOP_LINE_X_POS = LIGHT_POLE_X - 60.0f;
const float SUN_X = 0.0f;
const float SUN_Y = 250.0f;
const float SUN_Z = -50.0f;
const float SUN_RADIUS = 20.0f;

// Function to create a checkerboard texture
GLuint createCheckerboardTexture(float color1[3], float color2[3], int width, int height) {
    GLubyte image[64][64][3];
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            int c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));
            image[i][j][0] = c ? (GLubyte)(color1[0] * 255) : (GLubyte)(color2[0] * 255);
            image[i][j][1] = c ? (GLubyte)(color1[1] * 255) : (GLubyte)(color2[1] * 255);
            image[i][j][2] = c ? (GLubyte)(color1[2] * 255) : (GLubyte)(color2[2] * 255);
        }
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return textureID;
}

void drawCuboid(float width, float height, float depth) {
    float w = width / 2.0f;
    float h = height / 2.0f;
    float d = depth / 2.0f;
    glBegin(GL_QUADS);
        glNormal3f(0,0,1); glTexCoord2f(0,0); glVertex3f(-w, -h, d); glTexCoord2f(1,0); glVertex3f(w, -h, d); glTexCoord2f(1,1); glVertex3f(w, h, d); glTexCoord2f(0,1); glVertex3f(-w, h, d);
        glNormal3f(0,0,-1); glTexCoord2f(0,0); glVertex3f(-w, -h, -d); glTexCoord2f(1,0); glVertex3f(-w, h, -d); glTexCoord2f(1,1); glVertex3f(w, h, -d); glTexCoord2f(0,1); glVertex3f(w, -h, -d);
        glNormal3f(0,1,0); glTexCoord2f(0,0); glVertex3f(-w, h, d); glTexCoord2f(1,0); glVertex3f(w, h, d); glTexCoord2f(1,1); glVertex3f(w, h, -d); glTexCoord2f(0,1); glVertex3f(-w, h, -d);
        glNormal3f(0,-1,0); glTexCoord2f(0,0); glVertex3f(-w, -h, d); glTexCoord2f(1,0); glVertex3f(-w, -h, -d); glTexCoord2f(1,1); glVertex3f(w, -h, -d); glTexCoord2f(0,1); glVertex3f(w, -h, d);
        glNormal3f(1,0,0); glTexCoord2f(0,0); glVertex3f(w, -h, d); glTexCoord2f(1,0); glVertex3f(w, -h, -d); glTexCoord2f(1,1); glVertex3f(w, h, -d); glTexCoord2f(0,1); glVertex3f(w, h, d);
        glNormal3f(-1,0,0); glTexCoord2f(0,0); glVertex3f(-w, -h, d); glTexCoord2f(1,0); glVertex3f(-w, h, d); glTexCoord2f(1,1); glVertex3f(-w, h, -d); glTexCoord2f(0,1); glVertex3f(-w, -h, -d);
    glEnd();
}

void initScene(void) {
    srand(time(NULL));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_TEXTURE_2D);
    // Create checkerboard textures
    float roadColor1[3] = {0.5f, 0.5f, 0.5f}; // Gray
    float roadColor2[3] = {0.3f, 0.3f, 0.3f}; // Dark gray
    roadTexture = createCheckerboardTexture(roadColor1, roadColor2, 64, 64);
    float buildingColor1[3] = {0.8f, 0.2f, 0.2f}; // Red
    float buildingColor2[3] = {0.6f, 0.6f, 0.6f}; // Gray
    buildingTexture = createCheckerboardTexture(buildingColor1, buildingColor2, 64, 64);
    float grassColor1[3] = {0.0f, 0.6f, 0.0f}; // Green
    float grassColor2[3] = {0.0f, 0.4f, 0.0f}; // Dark green
    grassTexture = createCheckerboardTexture(grassColor1, grassColor2, 64, 64);
    // Initialize lighting
    if (dayMode) {
        glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
        glEnable(GL_LIGHT0);
        GLfloat light_pos[] = {ROAD_WIDTH_X / 4.0f, 150.0f, 300.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
        GLfloat ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        GLfloat diffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    } else {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        if (allStreetLightsOn) {
            glEnable(GL_LIGHT0);
            GLfloat light_pos[] = {ROAD_WIDTH_X / 4.0f, 150.0f, 300.0f, 1.0f};
            glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
            GLfloat ambient[] = {0.1f, 0.1f, 0.1f, 1.0f};
            glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
            GLfloat diffuse[] = {0.3f, 0.3f, 0.3f, 1.0f};
            glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        } else {
            glDisable(GL_LIGHT0);
            GLfloat ambient[] = {0.1f, 0.1f, 1.0f};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
        }
    }
    // Initialize scene objects
    for (int i = 0; i < 3; i++) {
        Car car;
        car.x = (-ROAD_WIDTH_X / 2.0f) + (rand() % (int)(ROAD_WIDTH_X * 0.4f));
        car.y = ROAD_Y_LEVEL + 7.5f;
        car.z = (rand() % (int)(ROAD_DEPTH_Z * 0.5f)) - ROAD_DEPTH_Z * 0.25f;
        car.speed = 1.0f + (rand() % 15) / 10.0f;
        car.depth = 10.0f + (rand() % 10);
        car.width = car.depth * (2.5f + (rand() % 10) / 10.0f);
        car.height = car.depth * (0.7f + (rand() % 10) / 10.0f);
        car.color[0] = (rand() % 101) / 100.0f;
        car.color[1] = (rand() % 101) / 100.0f;
        car.color[2] = (rand() % 101) / 100.0f;
        float carFrontX = car.x + car.width / 2.0f;
        if (currentLight == RED && carFrontX < STOP_LINE_X_POS && (carFrontX + car.speed * 5) > STOP_LINE_X_POS) {
            car.stopped = true;
            car.x = STOP_LINE_X_POS - car.width / 2.0f - 2.0f;
        } else {
            car.stopped = false;
        }
        cars.push_back(car);
    }
    for (float x_base = -ROAD_WIDTH_X / 2.0f - 100.0f; x_base < ROAD_WIDTH_X / 2.0f + 100.0f; x_base += (120.0f + rand()%50)) {
        BuildingData bd;
        bd.x_offset = x_base;
        bd.height = 80.0f + (rand() % 150);
        bd.width = 60.0f + (rand() % 30);
        bd.depth = 50.0f + (rand() % 20);
        bd.z_pos_factor = ROAD_DEPTH_Z / 2.0f + bd.depth / 2.0f + 10.0f + (rand()%20);
        bd.color[0] = 0.5f + (rand() % 51) / 100.0f;
        bd.color[1] = 0.5f + (rand() % 51) / 100.0f;
        bd.color[2] = 0.5f + (rand() % 51) / 100.0f;
        building_definitions.push_back(bd);
    }
    for (float x_base = -ROAD_WIDTH_X / 2.0f - 50.0f; x_base < ROAD_WIDTH_X / 2.0f + 50.0f; x_base += (180.0f + rand()%100)) {
        TreeData td;
        td.x_offset = x_base;
        td.trunkHeight = 30.0f + (rand() % 20);
        td.crownRadius = 15.0f + (rand() % 10);
        td.z_pos_factor = ROAD_DEPTH_Z / 2.0f + td.crownRadius + 5.0f + (rand()%30);
        td.crownColor[0] = 0.0f;
        td.crownColor[1] = 0.4f + (rand() % 51) / 100.0f;
        td.crownColor[2] = 0.0f;
        tree_definitions.push_back(td);
    }
    for (const auto& td : tree_definitions) {
        StreetLightData sld;
        sld.x_offset = td.x_offset + 50.0f;
        sld.z_pos_factor = ROAD_DEPTH_Z / 2.0f + 10.0f + (rand()%20);
        sld.poleHeight = 50.0f;
        sld.lightRadius = 4.0f;
        sld.isOn = !dayMode;
        streetLight_definitions.push_back(sld);
    }
    allStreetLightsOn = !dayMode;
}

void updateStreetLights() {
    if (!streetLightsManuallyToggled) {
        allStreetLightsOn = !dayMode;
        for (auto& sld : streetLight_definitions) {
            sld.isOn = allStreetLightsOn;
        }
    }
}

void drawSun_3D() {
    if (!dayMode) return;
    glPushMatrix();
    glTranslatef(SUN_X, SUN_Y, SUN_Z);
    glColor3f(1.0f, 1.0f, 0.0f);
    GLUquadric* q = gluNewQuadric();
    gluSphere(q, SUN_RADIUS, 16, 16);
    gluDeleteQuadric(q);
    glPopMatrix();
}

void drawCar_3D(const Car& car) {
    glPushMatrix();
    glTranslatef(car.x, car.y, car.z);
    glColor3f(car.color[0], car.color[1], car.color[2]);
    glPushMatrix();
    glTranslatef(0.0f, -car.height * 0.3f, 0.0f);
    drawCuboid(car.width * 0.95f, car.height * 0.4f, car.depth * 0.95f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, car.height * 0.2f, 0.0f);
    drawCuboid(car.width * 0.6f, car.height * 0.6f, car.depth * 0.9f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(car.width * 0.35f, -car.height * 0.2f, 0.0f);
    glRotatef(-20.0f, 0.0f, 0.0f, 1.0f);
    drawCuboid(car.width * 0.3f, car.height * 0.3f, car.depth * 0.95f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-car.width * 0.35f, -car.height * 0.2f, 0.0f);
    glRotatef(20.0f, 0.0f, 0.0f, 1.0f);
    drawCuboid(car.width * 0.3f, car.height * 0.3f, car.depth * 0.95f);
    glPopMatrix();
    glColor3f(0.1f, 0.1f, 0.1f);
    float wheelRadius = car.height * 0.15f;
    float wheelWidth = car.depth * 0.15f;
    float wheelOffsetFront = car.width * 0.35f;
    float wheelOffsetRear = -car.width * 0.35f;
    float wheelDepthOffset = car.depth * 0.475f;
    GLUquadric* q = gluNewQuadric();
    for (int i = 0; i < 4; ++i) {
        glPushMatrix();
        float xPos = (i < 2) ? wheelOffsetFront : wheelOffsetRear;
        float zPos = (i % 2 == 0) ? wheelDepthOffset : -wheelDepthOffset;
        glTranslatef(xPos, -car.height * 0.5f + wheelRadius, zPos);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
#pragma GCC diagnostic pop
        gluCylinder(q, wheelRadius, wheelRadius, wheelWidth, 12, 1);
        glColor3f(0.7f, 0.7f, 0.7f);
        gluCylinder(q, wheelRadius * 0.6f, wheelRadius * 0.6f, wheelWidth * 0.8f, 12, 1);
        glColor3f(0.1f, 0.1f, 0.1f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, wheelWidth / 2);
        gluDisk(q, 0.0f, wheelRadius, 12, 1);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -wheelWidth / 2);
        gluDisk(q, 0.0f, wheelRadius, 12, 1);
        glPopMatrix();
        glPopMatrix();
    }
    gluDeleteQuadric(q);
    glColor4f(0.1f, 0.1f, 0.1f, 0.7f);
    glPushMatrix();
    glTranslatef(car.width * 0.15f, car.height * 0.3f, 0.0f);
    glRotatef(-35.0f, 0.0f, 0.0f, 1.0f);
    drawCuboid(car.width * 0.3f, car.height * 0.4f, car.depth * 0.85f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-car.width * 0.15f, car.height * 0.3f, 0.0f);
    glRotatef(35.0f, 0.0f, 0.0f, 1.0f);
    drawCuboid(car.width * 0.3f, car.height * 0.4f, car.depth * 0.85f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, car.height * 0.3f, car.depth * 0.45f);
    drawCuboid(car.width * 0.4f, car.height * 0.3f, 0.2f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, car.height * 0.3f, -car.depth * 0.45f);
    drawCuboid(car.width * 0.4f, car.height * 0.3f, 0.2f);
    glPopMatrix();
    glColor3f(1.0f, 1.0f, 0.9f);
    glPushMatrix();
    glTranslatef(car.width * 0.47f, -car.height * 0.15f, car.depth * 0.4f);
    drawCuboid(2.5f, 2.0f, 0.5f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(car.width * 0.47f, -car.height * 0.15f, -car.depth * 0.4f);
    drawCuboid(2.5f, 2.0f, 0.5f);
    glPopMatrix();
    if (!dayMode && !allStreetLightsOn) {
        static int lightIndex = 0;
        GLenum light = GL_LIGHT1 + (lightIndex % 7);
        glEnable(light);
        GLfloat lightPos[] = {car.x + car.width * 0.47f, car.y - car.height * 0.15f, car.z + car.depth * 0.4f, 1.0f};
        GLfloat diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
        GLfloat direction[] = {1.0f, 0.0f, 0.0f};
        glLightfv(light, GL_POSITION, lightPos);
        glLightfv(light, GL_DIFFUSE, diffuse);
        glLightf(light, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(light, GL_LINEAR_ATTENUATION, 0.05f);
        glLightf(light, GL_QUADRATIC_ATTENUATION, 0.01f);
        glLightf(light, GL_SPOT_CUTOFF, 30.0f);
        glLightfv(light, GL_SPOT_DIRECTION, direction);
        glLightf(light, GL_SPOT_EXPONENT, 2.0f);
        GLenum light2 = GL_LIGHT1 + ((lightIndex + 1) % 7);
        glEnable(light2);
        lightPos[2] = car.z - car.depth * 0.4f;
        glLightfv(light2, GL_POSITION, lightPos);
        glLightfv(light2, GL_DIFFUSE, diffuse);
        glLightf(light2, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(light2, GL_LINEAR_ATTENUATION, 0.05f);
        glLightf(light2, GL_QUADRATIC_ATTENUATION, 0.01f);
        glLightf(light2, GL_SPOT_CUTOFF, 30.0f);
        glLightfv(light2, GL_SPOT_DIRECTION, direction);
        glLightf(light2, GL_SPOT_EXPONENT, 2.0f);
        lightIndex += 2;
        glDisable(light);
        glDisable(light2);
    }
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-car.width * 0.47f, -car.height * 0.15f, car.depth * 0.4f);
    drawCuboid(2.5f, 2.0f, 0.5f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-car.width * 0.47f, -car.height * 0.15f, -car.depth * 0.4f);
    drawCuboid(2.5f, 2.0f, 0.5f);
    glPopMatrix();
    glColor3f(0.3f, 0.3f, 0.3f);
    glPushMatrix();
    glTranslatef(car.width * 0.47f, -car.height * 0.25f, 0.0f);
    drawCuboid(4.0f, 3.0f, car.depth * 0.85f);
    glColor3f(0.5f, 0.5f, 0.5f);
    glTranslatef(0.0f, 0.0f, car.depth * 0.3f);
    drawCuboid(2.0f, 1.0f, 1.0f);
    glPopMatrix();
    glColor3f(car.color[0], car.color[1], car.color[2]);
    glPushMatrix();
    glTranslatef(car.width * 0.2f, car.height * 0.2f, car.depth * 0.5f);
    drawCuboid(2.0f, 1.0f, 1.0f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(car.width * 0.2f, car.height * 0.2f, -car.depth * 0.5f);
    drawCuboid(2.0f, 1.0f, 1.0f);
    glPopMatrix();
    glPopMatrix();
}

void drawTrafficLight_3D() {
    glPushMatrix();
    glTranslatef(LIGHT_POLE_X, LIGHT_Y_OFFSET, LIGHT_POLE_Z);
    glColor3f(0.3f, 0.3f, 0.3f);
    GLUquadric* quad = gluNewQuadric();
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(quad, 2.0f, 2.0f, LIGHT_POLE_HEIGHT, 16, 16);
    glPopMatrix();
    glTranslatef(0, LIGHT_POLE_HEIGHT + LIGHT_BOX_HEIGHT / 2.0f, 0);
    glColor3f(0.1f, 0.1f, 0.1f);
    drawCuboid(15.0f, LIGHT_BOX_HEIGHT, 15.0f);
    float lightSpacing = LIGHT_BOX_HEIGHT / 3.5f;
    float lightBoxFrontOffset = 7.5f;
    glColor3f(currentLight == RED ? 1.0f : 0.3f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0, lightSpacing, lightBoxFrontOffset);
    gluSphere(quad, LIGHT_RADIUS, 16, 16);
    glPopMatrix();
    glColor3f(currentLight == YELLOW ? 1.0f : 0.3f, currentLight == YELLOW ? 1.0f : 0.3f, 0.0f);
    glPushMatrix();
    glTranslatef(0, 0, lightBoxFrontOffset);
    gluSphere(quad, LIGHT_RADIUS, 16, 16);
    glPopMatrix();
    glColor3f(0.0f, currentLight == GREEN ? 1.0f : 0.3f, 0.0f);
    glPushMatrix();
    glTranslatef(0, -lightSpacing, lightBoxFrontOffset);
    gluSphere(quad, LIGHT_RADIUS, 16, 16);
    glPopMatrix();
    gluDeleteQuadric(quad);
    glPopMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(STOP_LINE_X_POS, ROAD_Y_LEVEL + 0.1f, 0);
    drawCuboid(5.0f, 0.2f, ROAD_DEPTH_Z);
    glPopMatrix();
}

void drawRoad_3D() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, roadTexture);
    glPushMatrix();
    glTranslatef(0, ROAD_Y_LEVEL, 0);
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-ROAD_WIDTH_X/2, ROAD_Y_LEVEL, -ROAD_DEPTH_Z/2);
        glTexCoord2f(10.0f, 0.0f); glVertex3f(ROAD_WIDTH_X/2, ROAD_Y_LEVEL, -ROAD_DEPTH_Z/2);
        glTexCoord2f(10.0f, 2.0f); glVertex3f(ROAD_WIDTH_X/2, ROAD_Y_LEVEL, ROAD_DEPTH_Z/2);
        glTexCoord2f(0.0f, 2.0f); glVertex3f(-ROAD_WIDTH_X/2, ROAD_Y_LEVEL, ROAD_DEPTH_Z/2);
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (float x_mark = -ROAD_WIDTH_X / 2.0f; x_mark < ROAD_WIDTH_X / 2.0f; x_mark += 100.0f) {
        glPushMatrix();
        glTranslatef(x_mark + 25.0f, ROAD_Y_LEVEL + 0.15f, 0);
        drawCuboid(50.0f, 0.1f, 4.0f);
        glPopMatrix();
    }
}

void drawBuildings_3D() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, buildingTexture);
    for (const auto& bd : building_definitions) {
        glPushMatrix();
        glTranslatef(bd.x_offset, ROAD_Y_LEVEL + bd.height / 2.0f, -bd.z_pos_factor);
        glColor3fv(bd.color);
        float texScaleX = bd.width / 50.0f;
        float texScaleY = bd.height / 50.0f;
        float texScaleZ = bd.depth / 50.0f;
        glBegin(GL_QUADS);
            glNormal3f(0,0,1);
            glTexCoord2f(0,0); glVertex3f(-bd.width/2, -bd.height/2, bd.depth/2);
            glTexCoord2f(texScaleX,0); glVertex3f(bd.width/2, -bd.height/2, bd.depth/2);
            glTexCoord2f(texScaleX,texScaleY); glVertex3f(bd.width/2, bd.height/2, bd.depth/2);
            glTexCoord2f(0,texScaleY); glVertex3f(-bd.width/2, bd.height/2, bd.depth/2);
            glNormal3f(0,0,-1);
            glTexCoord2f(0,0); glVertex3f(-bd.width/2, -bd.height/2, -bd.depth/2);
            glTexCoord2f(texScaleX,0); glVertex3f(-bd.width/2, bd.height/2, -bd.depth/2);
            glTexCoord2f(texScaleX,texScaleY); glVertex3f(bd.width/2, bd.height/2, -bd.depth/2);
            glTexCoord2f(0,texScaleY); glVertex3f(bd.width/2, -bd.height/2, -bd.depth/2);
            glNormal3f(0,1,0);
            glTexCoord2f(0,0); glVertex3f(-bd.width/2, bd.height/2, bd.depth/2);
            glTexCoord2f(texScaleX,0); glVertex3f(bd.width/2, bd.height/2, bd.depth/2);
            glTexCoord2f(texScaleX,texScaleZ); glVertex3f(bd.width/2, bd.height/2, -bd.depth/2);
            glTexCoord2f(0,texScaleZ); glVertex3f(-bd.width/2, bd.height/2, -bd.depth/2);
            glNormal3f(0,-1,0);
            glTexCoord2f(0,0); glVertex3f(-bd.width/2, -bd.height/2, bd.depth/2);
            glTexCoord2f(texScaleX,0); glVertex3f(-bd.width/2, -bd.height/2, -bd.depth/2);
            glTexCoord2f(texScaleX,texScaleZ); glVertex3f(bd.width/2, -bd.height/2, -bd.depth/2);
            glTexCoord2f(0,texScaleZ); glVertex3f(bd.width/2, -bd.height/2, bd.depth/2);
            glNormal3f(1,0,0);
            glTexCoord2f(0,0); glVertex3f(bd.width/2, -bd.height/2, bd.depth/2);
            glTexCoord2f(texScaleZ,0); glVertex3f(bd.width/2, -bd.height/2, -bd.depth/2);
            glTexCoord2f(texScaleZ,texScaleY); glVertex3f(bd.width/2, bd.height/2, -bd.depth/2);
            glTexCoord2f(0,texScaleY); glVertex3f(bd.width/2, bd.height/2, bd.depth/2);
            glNormal3f(-1,0,0);
            glTexCoord2f(0,0); glVertex3f(-bd.width/2, -bd.height/2, bd.depth/2);
            glTexCoord2f(texScaleZ,0); glVertex3f(-bd.width/2, bd.height/2, bd.depth/2);
            glTexCoord2f(texScaleZ,texScaleY); glVertex3f(-bd.width/2, bd.height/2, -bd.depth/2);
            glTexCoord2f(0,texScaleY); glVertex3f(-bd.width/2, -bd.height/2, -bd.depth/2);
        glEnd();
        glColor3f(0.1f, 0.1f, 0.2f);
        int numFloors = (int)(bd.height / 20.0f);
        int numWindowsPerRow = (int)(bd.width / 18.0f);
        for(int r=0; r < numFloors; ++r) {
            for(int c=0; c < numWindowsPerRow; ++c) {
                if (numWindowsPerRow <=0 || numFloors <=0) continue;
                float winWidth = bd.width * 0.1f; float winHeight = bd.height * 0.1f;
                float winX = -bd.width/2.0f + (bd.width / (numWindowsPerRow + 1.0f)) * (c + 1.0f);
                float winY = -bd.height/2.0f + (bd.height / (numFloors + 1.0f)) * (r + 1.0f);
                glPushMatrix();
                glTranslatef(winX, winY, bd.depth/2.0f * 1.005f);
                drawCuboid(winWidth, winHeight, 0.5f);
                glPopMatrix();
            }
        }
        glPopMatrix();
    }
    glDisable(GL_TEXTURE_2D);
}

void drawScenery_3D() {
    GLUquadric* q = gluNewQuadric();
    for (const auto& td : tree_definitions) {
        glPushMatrix();
        glTranslatef(td.x_offset, ROAD_Y_LEVEL, -td.z_pos_factor);
        glColor3f(0.5f, 0.35f, 0.05f);
        glPushMatrix(); glRotatef(-90, 1,0,0);
        gluCylinder(q, 1.5f + td.crownRadius*0.1f, 1.0f + td.crownRadius*0.05f, td.trunkHeight, 10, 8);
        glPopMatrix();
        glColor3fv(td.crownColor);
        glTranslatef(0, td.trunkHeight, 0);
        gluSphere(q, td.crownRadius, 12, 10);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(td.x_offset, ROAD_Y_LEVEL, td.z_pos_factor);
        glColor3f(0.5f, 0.35f, 0.05f);
        glPushMatrix(); glRotatef(-90, 1,0,0);
        gluCylinder(q, 1.5f + td.crownRadius*0.1f, 1.0f + td.crownRadius*0.05f, td.trunkHeight, 10, 8);
        glPopMatrix();
        glColor3fv(td.crownColor);
        glTranslatef(0, td.trunkHeight, 0);
        gluSphere(q, td.crownRadius, 12, 10);
        glPopMatrix();
    }
    gluDeleteQuadric(q);
}

void drawStreetLights_3D() {
    GLUquadric* q = gluNewQuadric();
    for (const auto& sld : streetLight_definitions) {
        glPushMatrix();
        glTranslatef(sld.x_offset, ROAD_Y_LEVEL, sld.z_pos_factor);
        glColor3f(0.3f, 0.3f, 0.3f);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        gluCylinder(q, 1.5f, 1.5f, sld.poleHeight, 12, 8);
        glPopMatrix();
        glTranslatef(0, sld.poleHeight, 0);
        glColor3f(sld.isOn ? 1.0f : 0.3f, sld.isOn ? 1.0f : 0.3f, sld.isOn ? 0.0f : 0.3f);
        gluSphere(q, sld.lightRadius, 12, 10);
        glPopMatrix();
    }
    gluDeleteQuadric(q);
}

void drawGrass_3D() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    for (const auto& td : tree_definitions) {
        glPushMatrix();
        glTranslatef(td.x_offset, ROAD_Y_LEVEL + 0.05f, -td.z_pos_factor);
        glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-td.crownRadius, 0, -td.crownRadius);
            glTexCoord2f(2.0f, 0.0f); glVertex3f(td.crownRadius, 0, -td.crownRadius);
            glTexCoord2f(2.0f, 2.0f); glVertex3f(td.crownRadius, 0, td.crownRadius);
            glTexCoord2f(0.0f, 2.0f); glVertex3f(-td.crownRadius, 0, td.crownRadius);
        glEnd();
        glPopMatrix();
        glPushMatrix();
        glTranslatef(td.x_offset, ROAD_Y_LEVEL + 0.05f, td.z_pos_factor);
        glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-td.crownRadius, 0, -td.crownRadius);
            glTexCoord2f(2.0f, 0.0f); glVertex3f(td.crownRadius, 0, -td.crownRadius);
            glTexCoord2f(2.0f, 2.0f); glVertex3f(td.crownRadius, 0, td.crownRadius);
            glTexCoord2f(0.0f, 2.0f); glVertex3f(-td.crownRadius, 0, td.crownRadius);
        glEnd();
        glPopMatrix();
    }
    for (const auto& sld : streetLight_definitions) {
        glPushMatrix();
        glTranslatef(sld.x_offset, ROAD_Y_LEVEL + 0.05f, sld.z_pos_factor);
        glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 0, -10.0f);
            glTexCoord2f(2.0f, 0.0f); glVertex3f(10.0f, 0, -10.0f);
            glTexCoord2f(2.0f, 2.0f); glVertex3f(10.0f, 0, 10.0f);
            glTexCoord2f(0.0f, 2.0f); glVertex3f(-10.0f, 0, 10.0f);
        glEnd();
        glPopMatrix();
    }
    glDisable(GL_TEXTURE_2D);
}

void drawHelpText_3D() {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING);
    glColor3f(dayMode ? 0.0f : 1.0f, 0.0f, 0.0f);
    int yPos = WINDOW_HEIGHT - 20;
    const char* line1 = "Controls: R/Y/G=Light, A=Auto, H=Horn, L=Streetlights, D=Day/Night, Arrows=Rotate, WASQ=Move, ZX=Scale, ESC=Exit";
    glRasterPos2f(10, yPos); for (const char* c = line1; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    yPos -= 15;
    const char* line2 = "+/- = Adjust Car Generation Rate";
    glRasterPos2f(10, yPos); for (const char* c = line2; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    yPos -= 15;
    const char* line3 = "Mouse: Left Click=Cycle Views, Right Click=Cycle Cars, Drag=Rotate";
    glRasterPos2f(10, yPos); for (const char* c = line3; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    yPos -= 20;
    std::string modeTextStr = automaticMode ? "Mode: Automatic" : "Mode: Manual";
    const char* modeText = modeTextStr.c_str();
    glRasterPos2f(10, yPos); for (const char* c = modeText; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    glEnable(GL_LIGHTING);
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
}
void mehedi() {
    glScalef(scaleFactor, scaleFactor, scaleFactor);
    drawSun_3D();
    drawRoad_3D();
    drawBuildings_3D();
    drawScenery_3D();
    drawStreetLights_3D();
    drawGrass_3D();
    drawTrafficLight_3D();
    for (const auto& car : cars) {
        drawCar_3D(car);
    }
    drawHelpText_3D();
}

void displayScene() {
    if (dayMode) {
        glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
    } else {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    if (dayMode || allStreetLightsOn) {
        glEnable(GL_LIGHT0);
        GLfloat light_pos[] = {ROAD_WIDTH_X / 4.0f, 150.0f, 300.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
        GLfloat ambient[] = {dayMode ? 0.3f : 0.1f, dayMode ? 0.3f : 0.1f, dayMode ? 0.3f : 0.1f, 1.0f};
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        GLfloat diffuse[] = {dayMode ? 0.7f : 0.3f, dayMode ? 0.7f : 0.3f, dayMode ? 0.7f : 0.3f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    } else {
        glDisable(GL_LIGHT0);
        GLfloat ambient[] = {0.1f, 0.1f, 0.1f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    }
    switch (currentView) {
        case DEFAULT: {
            float centerX = ROAD_WIDTH_X / 4.0 + cameraX;
            float centerY = 0.0 + cameraY;
            float centerZ = 0.0 + cameraZ;
            float radYaw = yaw * M_PI / 180.0f;
            float radPitch = pitch * M_PI / 180.0f;
            float eyeX = centerX + distance * cos(radPitch) * cos(radYaw);
            float eyeY = centerY + distance * sin(radPitch);
            float eyeZ = centerZ + distance * cos(radPitch) * sin(radYaw);
            gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, 0.0, 1.0, 0.0);
            break;
        }
        case OVERHEAD: {
            float centerX = 0.0 + cameraX;
            float centerY = 0.0 + cameraY;
            float centerZ = 0.0 + cameraZ;
            float radYaw = yaw * M_PI / 180.0f;
            float radPitch = pitch * M_PI / 180.0f;
            float eyeX = centerX + distance * cos(radPitch) * cos(radYaw);
            float eyeY = centerY + distance * sin(radPitch);
            float eyeZ = centerZ + distance * cos(radPitch) * sin(radYaw);
            gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, 0.0, 1.0, 0.0);
            break;
        }
        case SIDE: {
            float centerX = ROAD_WIDTH_X / 4.0 + cameraX;
            float centerY = 0.0 + cameraY;
            float centerZ = 0.0 + cameraZ;
            float radYaw = yaw * M_PI / 180.0f;
            float radPitch = pitch * M_PI / 180.0f;
            float eyeX = centerX + distance * cos(radPitch) * cos(radYaw);
            float eyeY = centerY + distance * sin(radPitch);
            float eyeZ = centerZ + distance * cos(radPitch) * sin(radYaw);
            gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, 0.0, 1.0, 0.0);
            break;
        }
        case FOLLOW:
            if (!cars.empty() && followCarIndex < cars.size()) {
                const Car& car = cars[followCarIndex];
                gluLookAt(car.x - 30.0 + cameraX, car.y + 15.0 + cameraY, car.z + cameraZ,
                          car.x, car.y, car.z, 0.0, 1.0, 0.0);
            } else {
                gluLookAt(-ROAD_WIDTH_X / 4.0 + cameraX, 50.0 + cameraY, ROAD_DEPTH_Z / 2.0 + 50.0 + cameraZ,
                          ROAD_WIDTH_X / 4.0, 0.0, 0.0, 0.0, 1.0, 0.0);
            }
            break;
        case FRONT:
            if (!cars.empty() && followCarIndex < cars.size()) {
                const Car& car = cars[followCarIndex];
                gluLookAt(car.x + 50.0 + cameraX, car.y + 15.0 + cameraY, car.z + cameraZ,
                          car.x, car.y, car.z, 0.0, 1.0, 0.0);
            } else {
                gluLookAt(-ROAD_WIDTH_X / 4.0 + cameraX, 50.0 + cameraY, ROAD_DEPTH_Z / 2.0 + 50.0 + cameraZ,
                          ROAD_WIDTH_X / 4.0, 0.0, 0.0, 0.0, 1.0, 0.0);
            }
            break;
    }
    mehedi();
    glutSwapBuffers();
}

void updateTrafficLight_Automatic() {
    if (!automaticMode) return;
    lightTimer++;
    if (currentLight == RED && lightTimer >= RED_DURATION) {
        currentLight = GREEN; lightTimer = 0;
    } else if (currentLight == GREEN && lightTimer >= GREEN_DURATION) {
        currentLight = YELLOW; lightTimer = 0;
    } else if (currentLight == YELLOW && lightTimer >= YELLOW_DURATION) {
        currentLight = RED; lightTimer = 0;
    }
}

void generateNewCars() {
    carGenerationTimer += carGenerationRate;
    if (carGenerationTimer >= 1.0f) {
        carGenerationTimer -= 1.0f;
        Car car;
        car.x = -ROAD_WIDTH_X / 2.0f - (20.0f + rand()%20);
        car.y = ROAD_Y_LEVEL + 7.5f;
        car.z = (rand() % (int)(ROAD_DEPTH_Z * 0.6f)) - ROAD_DEPTH_Z * 0.3f;
        car.speed = 1.2f + (rand() % 18) / 10.0f;
        car.depth = 10.0f + (rand() % 10);
        car.width = car.depth * (2.5f + (rand() % 10) / 10.0f);
        car.height = car.depth * (0.7f + (rand() % 10) / 10.0f);
        car.color[0] = (rand() % 101) / 100.0f;
        car.color[1] = (rand() % 101) / 100.0f;
        car.color[2] = (rand() % 101) / 100.0f;
        car.stopped = false;
        bool spawnOK = true;
        for(const auto& existingCar : cars) {
            if (fabs(existingCar.x - car.x) < car.width + 10.0f && fabs(existingCar.z - car.z) < car.depth) {
                spawnOK = false;
                break;
            }
        }
        if(spawnOK) {
             cars.push_back(car);
        }
    }
}

void updateCarsState() {
    for (auto it = cars.begin(); it != cars.end(); ) {
        float carFrontX = it->x + it->width / 2.0f;
        if (carFrontX > ROAD_WIDTH_X / 2.0f + it->width * 2) {
            it = cars.erase(it);
            if (followCarIndex >= cars.size()) {
                followCarIndex = cars.empty() ? 0 : cars.size() - 1;
            }
            continue;
        }
        if (!it->stopped) {
            if (currentLight == RED && carFrontX < STOP_LINE_X_POS && (carFrontX + it->speed) >= STOP_LINE_X_POS) {
                float closestRearX = STOP_LINE_X_POS;
                bool foundCar = false;
                for (const auto& other : cars) {
                    if (&(*it) == &other) continue;
                    float otherRearX = other.x - other.width / 2.0f;
                    if (fabs(it->z - other.z) < (it->depth + other.depth) / 2.0f) {
                        if (otherRearX > it->x && otherRearX < closestRearX && (other.x + other.width / 2.0f <= STOP_LINE_X_POS + 2.0f || other.stopped)) {
                            closestRearX = otherRearX;
                            foundCar = true;
                        }
                    }
                }
                it->stopped = true;
                if (foundCar) {
                    it->x = closestRearX - it->width / 2.0f - 2.0f;
                } else {
                    it->x = STOP_LINE_X_POS - it->width / 2.0f - 1.0f;
                }
            } else {
                float newX = it->x + it->speed;
                float newFrontX = newX + it->width / 2.0f;
                bool canMove = true;
                float closestRearX = ROAD_WIDTH_X / 2.0f;
                for (const auto& other : cars) {
                    if (&(*it) == &other) continue;
                    float otherRearX = other.x - other.width / 2.0f;
                    float otherFrontX = other.x + other.width / 2.0f;
                    if (fabs(it->z - other.z) < (it->depth + other.depth) / 2.0f) {
                        if (newFrontX > otherRearX && newFrontX < otherFrontX + 5.0f) {
                            canMove = false;
                            if (otherRearX < closestRearX) {
                                closestRearX = otherRearX;
                            }
                        }
                    }
                }
                if (!canMove) {
                    it->stopped = true;
                    it->x = closestRearX - it->width / 2.0f - 2.0f;
                } else {
                    it->x = newX;
                }
            }
        } else {
            if (currentLight == GREEN) {
                float newX = it->x + it->speed;
                float newFrontX = newX + it->width / 2.0f;
                bool canMove = true;
                for (const auto& other : cars) {
                    if (&(*it) == &other) continue;
                    float otherRearX = other.x - other.width / 2.0f;
                    float otherFrontX = other.x + other.width / 2.0f;
                    if (fabs(it->z - other.z) < (it->depth + other.depth) / 2.0f) {
                        if (newFrontX > otherRearX && newFrontX < otherFrontX + 5.0f) {
                            canMove = false;
                            break;
                        }
                    }
                }
                if (canMove) {
                    it->stopped = false;
                    it->x = newX;
                }
            } else if (currentLight == YELLOW && carFrontX <= STOP_LINE_X_POS + 2.0f) {
                bool canMove = true;
                float newX = it->x + it->speed;
                float newFrontX = newX + it->width / 2.0f;
                for (const auto& other : cars) {
                    if (&(*it) == &other) continue;
                    float otherRearX = other.x - other.width / 2.0f;
                    if (fabs(it->z - other.z) < (it->depth + other.depth) / 2.0f) {
                        if (newFrontX > otherRearX && newFrontX < otherRearX + 5.0f) {
                            canMove = false;
                            break;
                        }
                    }
                }
                if (canMove) {
                    it->stopped = false;
                    it->x = newX;
                }
            }
        }
        ++it;
    }
}

void animateScene(int value) {
    updateTrafficLight_Automatic();
    updateCarsState();
    generateNewCars();
    glutPostRedisplay();
    glutTimerFunc(30, animateScene, 0);
}

void reshapeWindow(int w, int h) {
    WINDOW_WIDTH = w; WINDOW_HEIGHT = h;
    if (h == 0) h = 1;
    float ratio = 1.0f * w / h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(45.0f, ratio, 10.0f, 1500.0f);
    glMatrixMode(GL_MODELVIEW);
}

void keyboardInput(unsigned char key, int x, int y) {
    switch (key) {
        case 'D': case 'd': {
            dayMode = !dayMode;
            if (dayMode) {
                glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
                glEnable(GL_LIGHT0);
                GLfloat light_pos[] = {ROAD_WIDTH_X / 4.0f, 150.0f, 300.0f, 1.0f};
                glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
                GLfloat ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
                glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
                GLfloat diffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
                glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
            } else {
                glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
                if (allStreetLightsOn) {
                    glEnable(GL_LIGHT0);
                    GLfloat light_pos[] = {ROAD_WIDTH_X / 4.0f, 150.0f, 300.0f, 1.0f};
                    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
                    GLfloat ambient[] = {0.1f, 0.1f, 0.1f, 1.0f};
                    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
                    GLfloat diffuse[] = {0.3f, 0.3f, 0.3f, 1.0f};
                    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
                } else {
                    glDisable(GL_LIGHT0);
                    GLfloat ambient[] = {0.1f, 0.1f, 0.1f, 1.0f};
                    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
                }
            }
            updateStreetLights();
            glFlush();
            glutPostRedisplay();
            break;
        }
        case 'r': case 'R':
            currentLight = RED; lightTimer = 0; if(automaticMode) automaticMode=false;
            break;
        case 'y': case 'Y':
            currentLight = YELLOW; lightTimer = 0; if(automaticMode) automaticMode=false;
            break;
        case 'g': case 'G':
            currentLight = GREEN; lightTimer = 0; if(automaticMode) automaticMode=false;
            break;
        case 'a': case 'A':
            automaticMode = !automaticMode;
            lightTimer = 0;
            break;
        case '+':
            carGenerationRate += 0.005f;
            if (carGenerationRate > 0.2f) carGenerationRate = 0.2f;
            break;
        case '-':
            carGenerationRate -= 0.005f;
            if (carGenerationRate < 0.005f) carGenerationRate = 0.005f;
            break;
        case 'h': case 'H':
            Beep(600, 150);
            break;
        case 'l': case 'L':
            streetLightsManuallyToggled = true;
            allStreetLightsOn = !allStreetLightsOn;
            for (auto& sld : streetLight_definitions) {
                sld.isOn = allStreetLightsOn;
            }
            if (!dayMode) {
                if (allStreetLightsOn) {
                    glEnable(GL_LIGHT0);
                    GLfloat light_pos[] = {ROAD_WIDTH_X / 4.0f, 150.0f, 300.0f, 1.0f};
                    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
                    GLfloat ambient[] = {0.1f, 0.1f, 0.1f, 1.0f};
                    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
                    GLfloat diffuse[] = {0.3f, 0.3f, 0.3f, 1.0f};
                    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
                } else {
                    glDisable(GL_LIGHT0);
                    GLfloat ambient[] = {0.1f, 0.1f, 0.1f, 1.0f};
                    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
                }
                glFlush();
            }
            glutPostRedisplay();
            break;
        case 'w': case 'W':
            cameraY += 10.0f;
            break;
        case 's': case 'S':
            cameraY -= 10.0f;
            break;
        case 'q': case 'Q':
            cameraX -= 10.0f;
            break;
        case 'e': case 'E':
            cameraX += 10.0f;
            break;
        case 'z': case 'Z':
            scaleFactor += 0.1f;
            if (scaleFactor > 2.0f) scaleFactor = 2.0f;
            break;
        case 'x': case 'X':
            scaleFactor -= 0.1f;
            if (scaleFactor < 0.5f) scaleFactor = 0.5f;
            break;
        case 27:
            glDeleteTextures(1, &roadTexture);
            glDeleteTextures(1, &buildingTexture);
            glDeleteTextures(1, &grassTexture);
            cars.clear(); building_definitions.clear(); tree_definitions.clear(); streetLight_definitions.clear();
            followCarIndex = 0;
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void specialKeyboardInput(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            pitch += 2.0f;
            if (pitch > 89.0f) pitch = 89.0f;
            break;
        case GLUT_KEY_DOWN:
            pitch -= 2.0f;
            if (pitch < -89.0f) pitch = -89.0f;
            break;
        case GLUT_KEY_LEFT:
            yaw += 2.0f;
            break;
        case GLUT_KEY_RIGHT:
            yaw -= 2.0f;
            break;
    }
    glutPostRedisplay();
}

void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            leftButtonHeld = true;
            lastX = x;
            lastY = y;
        } else if (state == GLUT_UP) {
            leftButtonHeld = false;
            currentView = static_cast<CameraView>((currentView + 1) % 5);
            distance = 280.0f;
            glutPostRedisplay();
        }
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
        if (currentView == FOLLOW || currentView == FRONT) {
            if (!cars.empty()) {
                followCarIndex = (followCarIndex + 1) % cars.size();
            }
            glutPostRedisplay();
        }
    }
}

void mouseMotion(int x, int y) {
    if (leftButtonHeld && currentView != FOLLOW && currentView != FRONT) {
        int dx = x - lastX;
        int dy = y - lastY;
        yaw -= dx * 0.2f;
        pitch -= dy * 0.2f;
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
        lastX = x;
        lastY = y;
        glutPostRedisplay();
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("3D Animated Traffic Simulation");
    initScene();
    glutDisplayFunc(displayScene);
    glutReshapeFunc(reshapeWindow);
    glutKeyboardFunc(keyboardInput);
    glutSpecialFunc(specialKeyboardInput);
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMotion);
    glutTimerFunc(30, animateScene, 0);
    glutMainLoop();
    return 0;
}
