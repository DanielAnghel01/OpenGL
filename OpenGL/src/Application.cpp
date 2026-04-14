#include <windows.h>
#include <freeglut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "freeglut.lib")

const float PI = 3.1415926535f;

// ---------------------------
// Textures
// ---------------------------
GLuint texGrass = 0;
GLuint texSky = 0;
GLuint texMountains = 0;
GLuint texRoad = 0;
GLuint texBuilding = 0;
GLuint texBark = 0;
GLuint texLeaves = 0;
GLuint texHorizon = 0;


struct RandomMover
{
    float x, z;
    float angle;
    float speed;
    int changeTimer;
};

RandomMover randomMovers[5];
const int randomMoverCount = 5;

float autoCarAngle = 0.0f;
float autoCarRadiusX = 110.0f;
float autoCarRadiusZ = 75.0f;
// ---------------------------
// Camera
// ---------------------------
float camX = 0.0f;
float camY = 12.0f;
float camZ = 180.0f;
float yaw = 0.0f;
float pitch = -5.0f;

float camDistance = 28.0f;
float camHeight = 10.0f;
float camYawOffset = 0.0f;
float camPitchOffset = 15.0f;

bool keys[256] = { false };
bool specialKeys[256] = { false };

// ---------------------------
// Lighting / Shadows
// ---------------------------
bool enableLighting = true;
bool enableShadows = true;

struct StreetLight
{
    float x, y, z;
};

StreetLight streetLights[] =
{
    {-60.0f, 8.0f, -40.0f},
    { 60.0f, 8.0f, -40.0f},
    {-60.0f, 8.0f,  40.0f},
    { 60.0f, 8.0f,  40.0f}
};

const int streetLightCount = sizeof(streetLights) / sizeof(streetLights[0]);
float mainLightPos[4] = { 80.0f, 120.0f, 60.0f, 1.0f };
// ---------------------------
// Buildings for collisions
// ---------------------------
struct BuildingData
{
    float x, z;
    float w, h, d;
};

BuildingData buildings[128];
int buildingCount = 0;
// ---------------------------
// Controllable car
// ---------------------------
float carX = 0.0f;
float carZ = 110.0f;
float carY = 0.12f;
float carAngle = 180.0f;

float carWidth = 8.0f;
float carDepth = 14.0f;
float carSpeed = 0.0f;
// Functions
void DrawGround();
void DrawSkyCylinder();
void DrawTerrain();
void DrawRoad();
void DrawRoadWithSidewalk(float x, float z, float w, float d, float rotDeg);
void DrawCityRoads();

void DrawBuilding(float x, float z, float w, float h, float d);
void DrawBuildingRowX(float startX, float z, int count, float spacing);
void DrawBuildingRowZ(float x, float startZ, int count, float spacing);
void DrawCityBuildings();

void DrawTree(float x, float z);
void DrawExtraTrees();

void DrawBenchModel();
void DrawBench(float x, float z, float rot);

void DrawFlower(float x, float z, float scale);
void DrawFlowerPatch(float cx, float cz, int count, float radius);

void DrawParkDecorations();
void DrawStaticObjects();
void DrawShadowCasters();

void DrawStreetLight(float x, float y, float z);
void DrawStreetLights();

void SetupLighting();
void BuildShadowMatrix(float shadowMat[16], const float groundPlane[4], const float lightPos[4]);
void RenderShadowFromLight(const float lightPos[4]);
void ComputeTerrainNormal(float x, float z, float& nx, float& ny, float& nz);

bool CheckAABBCollision(float x1, float z1, float w1, float d1,
    float x2, float z2, float w2, float d2);
bool CarCollidesWithBuildings(float testX, float testZ);
void AddBuildingData(float x, float z, float w, float h, float d);
void InitBuildings();

void DrawCar();
void UpdateCar();

void InitRandomMovers();
void DrawRandomMover(float x, float z);
void DrawRandomMovers();
void UpdateRandomMovers();
bool RandomMoverCollides(float x, float z);

void DrawAutoCar();
void UpdateAutoCar();
void DrawTexturedBox(float w, float h, float d, float uRepeat = 1.0f, float vRepeat = 1.0f);

// ---------------------------
// Texture loader (JPG/PNG)
// ---------------------------
GLuint LoadTexture(const char* path, bool repeat = true)
{
    int width = 0, height = 0, channels = 0;
    stbi_set_flip_vertically_on_load(1);

    unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
    if (!data)
    {
        printf("Failed to load texture: %s\n", path);
        printf("stb reason: %s\n", stbi_failure_reason());
        return 0;
    }

    GLuint textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    gluBuild2DMipmaps(
        GL_TEXTURE_2D,
        GL_RGBA,
        width,
        height,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data
    );

    stbi_image_free(data);
    return textureID;
}


void LoadAllTextures()
{
    texGrass = LoadTexture("textures/grass.jpg", true);
    texSky = LoadTexture("textures/sky.jpg", false);
    texMountains = LoadTexture("textures/mountains.jpg", false);
    texRoad = LoadTexture("textures/road.jpg", true);
    texBuilding = LoadTexture("textures/building.jpg", true);
    texBark = LoadTexture("textures/bark.jpg", true);
    texLeaves = LoadTexture("textures/leaves.jpg", true);

    texHorizon = LoadTexture("textures/horizon.jpg", true);
    if (!texHorizon) texHorizon = texMountains;
}

// ---------------------------
// Helpers
// ---------------------------
void BindTexture(GLuint tex)
{
    glBindTexture(GL_TEXTURE_2D, tex);
}

float TerrainHeight(float x, float z)
{
    return 2.2f * sinf(x * 0.10f) * cosf(z * 0.12f);
}

void ComputeTerrainNormal(float x, float z, float& nx, float& ny, float& nz)
{
    float eps = 0.2f;

    float hL = TerrainHeight(x - eps, z);
    float hR = TerrainHeight(x + eps, z);
    float hD = TerrainHeight(x, z - eps);
    float hU = TerrainHeight(x, z + eps);

    nx = hL - hR;
    ny = 2.0f * eps;
    nz = hD - hU;

    float len = sqrtf(nx * nx + ny * ny + nz * nz);
    if (len > 0.0001f)
    {
        nx /= len;
        ny /= len;
        nz /= len;
    }
    else
    {
        nx = 0.0f;
        ny = 1.0f;
        nz = 0.0f;
    }
}
void DrawAutoCar()
{
    float x = cosf(autoCarAngle * PI / 180.0f) * autoCarRadiusX;
    float z = sinf(autoCarAngle * PI / 180.0f) * autoCarRadiusZ;

    float dx = -sinf(autoCarAngle * PI / 180.0f) * autoCarRadiusX;
    float dz = cosf(autoCarAngle * PI / 180.0f) * autoCarRadiusZ;
    float angle = atan2f(dx, -dz) * 180.0f / PI;

    glPushMatrix();
    glTranslatef(x, 0.12f, z);
    glRotatef(angle, 0, 1, 0);

    glDisable(GL_TEXTURE_2D);

    glColor3f(0.1f, 0.8f, 0.1f);
    glPushMatrix();
    DrawTexturedBox(7.0f, 2.0f, 12.0f);
    glPopMatrix();

    glColor3f(0.7f, 0.9f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, -1.0f);
    DrawTexturedBox(5.0f, 1.8f, 5.0f);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);

    glPopMatrix();
}

void UpdateAutoCar()
{
    autoCarAngle += 0.6f;
    if (autoCarAngle >= 360.0f)
        autoCarAngle -= 360.0f;
}

void AddBuildingData(float x, float z, float w, float h, float d)
{
    if (buildingCount < 128)
    {
        buildings[buildingCount].x = x;
        buildings[buildingCount].z = z;
        buildings[buildingCount].w = w;
        buildings[buildingCount].h = h;
        buildings[buildingCount].d = d;
        buildingCount++;
    }
}
// Box anchored at ground level (y starts from 0)
void DrawTexturedBox(float w, float h, float d, float uRepeat, float vRepeat)
{
    float x = w * 0.5f;
    float z = d * 0.5f;

    glBegin(GL_QUADS);

    // Front
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0, 0);               glVertex3f(-x, 0, z);
    glTexCoord2f(uRepeat, 0);         glVertex3f(x, 0, z);
    glTexCoord2f(uRepeat, vRepeat);   glVertex3f(x, h, z);
    glTexCoord2f(0, vRepeat);         glVertex3f(-x, h, z);

    // Back
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0, 0);               glVertex3f(x, 0, -z);
    glTexCoord2f(uRepeat, 0);         glVertex3f(-x, 0, -z);
    glTexCoord2f(uRepeat, vRepeat);   glVertex3f(-x, h, -z);
    glTexCoord2f(0, vRepeat);         glVertex3f(x, h, -z);

    // Left
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0, 0);               glVertex3f(-x, 0, -z);
    glTexCoord2f(uRepeat, 0);         glVertex3f(-x, 0, z);
    glTexCoord2f(uRepeat, vRepeat);   glVertex3f(-x, h, z);
    glTexCoord2f(0, vRepeat);         glVertex3f(-x, h, -z);

    // Right
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0, 0);               glVertex3f(x, 0, z);
    glTexCoord2f(uRepeat, 0);         glVertex3f(x, 0, -z);
    glTexCoord2f(uRepeat, vRepeat);   glVertex3f(x, h, -z);
    glTexCoord2f(0, vRepeat);         glVertex3f(x, h, z);

    // Top
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0, 0);               glVertex3f(-x, h, z);
    glTexCoord2f(uRepeat, 0);         glVertex3f(x, h, z);
    glTexCoord2f(uRepeat, vRepeat);   glVertex3f(x, h, -z);
    glTexCoord2f(0, vRepeat);         glVertex3f(-x, h, -z);

    // Bottom
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0, 0);               glVertex3f(-x, 0, -z);
    glTexCoord2f(uRepeat, 0);         glVertex3f(x, 0, -z);
    glTexCoord2f(uRepeat, vRepeat);   glVertex3f(x, 0, z);
    glTexCoord2f(0, vRepeat);         glVertex3f(-x, 0, z);

    glEnd();
}
void InitRandomMovers()
{
    for (int i = 0; i < randomMoverCount; i++)
    {
        randomMovers[i].x = -50.0f + i * 20.0f;
        randomMovers[i].z = -30.0f + i * 15.0f;
        randomMovers[i].angle = (float)(rand() % 360);
        randomMovers[i].speed = 0.15f + (rand() % 10) * 0.01f;
        randomMovers[i].changeTimer = 50 + rand() % 100;
    }
}

// ---------------------------
// P1 - Scene inside a cube
// ---------------------------
void DrawGround()
{
    BindTexture(texGrass);
    glColor3f(1, 1, 1);

    float size = 450.0f;
    float repeat = 30.0f;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0, 0);               glVertex3f(-size, 0.0f, -size);
    glTexCoord2f(repeat, 0);          glVertex3f(size, 0.0f, -size);
    glTexCoord2f(repeat, repeat);     glVertex3f(size, 0.0f, size);
    glTexCoord2f(0, repeat);          glVertex3f(-size, 0.0f, size);
    glEnd();
}

void DrawSkyCylinder()
{
    glDisable(GL_LIGHTING);

    const float radius = 430.0f;
    const float height = 140.0f;
    const int segments = 72;

    glColor3f(1, 1, 1);

    // Panoramic horizon around the scene
    BindTexture(texHorizon);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++)
    {
        float t = 2.0f * PI * i / segments;
        float x = cosf(t) * radius;
        float z = sinf(t) * radius;
        float u = (float)i / segments;

        glTexCoord2f(u, 0.0f); glVertex3f(x, 0.0f, z);
        glTexCoord2f(u, 1.0f); glVertex3f(x, height, z);
    }
    glEnd();

    // Top sky
    BindTexture(texSky);
    float s = 460.0f;
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-s, height, -s);
    glTexCoord2f(1, 0); glVertex3f(s, height, -s);
    glTexCoord2f(1, 1); glVertex3f(s, height, s);
    glTexCoord2f(0, 1); glVertex3f(-s, height, s);
    glEnd();

    if (enableLighting)
        glEnable(GL_LIGHTING);
}

void DrawTerrain()
{
    BindTexture(texGrass);
    glColor3f(1, 1, 1);

    const int N = 60;
    const float halfSize = 65.0f;
    const float texRepeat = 8.0f;

    for (int iz = 0; iz < N; iz++)
    {
        float z0 = -halfSize + (2.0f * halfSize * iz) / N;
        float z1 = -halfSize + (2.0f * halfSize * (iz + 1)) / N;

        glBegin(GL_TRIANGLE_STRIP);
        for (int ix = 0; ix <= N; ix++)
        {
            float x = -halfSize + (2.0f * halfSize * ix) / N;

            float y0 = 0.35f + TerrainHeight(x, z0);
            float y1 = 0.35f + TerrainHeight(x, z1);

            float nx0, ny0, nz0;
            float nx1, ny1, nz1;
            ComputeTerrainNormal(x, z0, nx0, ny0, nz0);
            ComputeTerrainNormal(x, z1, nx1, ny1, nz1);

            float u = (float)ix / N * texRepeat;
            float v0 = (float)iz / N * texRepeat;
            float v1 = (float)(iz + 1) / N * texRepeat;

            glNormal3f(nx0, ny0, nz0);
            glTexCoord2f(u, v0); glVertex3f(x, y0, z0);

            glNormal3f(nx1, ny1, nz1);
            glTexCoord2f(u, v1); glVertex3f(x, y1, z1);
        }
        glEnd();
    }
}

// ---------------------------
// P2 - Road circuit
// ---------------------------
void DrawRoad()
{
    BindTexture(texRoad);
    glColor3f(1, 1, 1);

    const int segments = 180;
    const float outerRX = 120.0f;
    const float outerRZ = 85.0f;
    const float innerRX = 100.0f;
    const float innerRZ = 65.0f;
    const float y = 0.12f;

    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segments; i++)
    {
        float t = 2.0f * PI * i / segments;
        float c = cosf(t);
        float s = sinf(t);

        float u = (float)i / segments * 12.0f;

        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(1.0f, u);
        glVertex3f(outerRX * c, y, outerRZ * s);

        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, u);
        glVertex3f(innerRX * c, y, innerRZ * s);
    }
    glEnd();
}

// ---------------------------
// Static objects
// ---------------------------
void DrawBuilding(float x, float z, float w, float h, float d)
{
    AddBuildingData(x, z, w, h, d);

    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    BindTexture(texBuilding);
    DrawTexturedBox(w, h, d, 2.0f, 2.0f);

    // Simple roof
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.25f, 0.1f, 0.1f);

    glBegin(GL_TRIANGLES);
    // Front
    glNormal3f(0.0f, 0.7f, 0.7f);
    glVertex3f(-w / 2, h, d / 2);
    glVertex3f(w / 2, h, d / 2);
    glVertex3f(0, h + 5, d / 2);

    // Back
    glNormal3f(0.0f, 0.7f, -0.7f);
    glVertex3f(w / 2, h, -d / 2);
    glVertex3f(-w / 2, h, -d / 2);
    glVertex3f(0, h + 5, -d / 2);
    glEnd();

    glBegin(GL_QUADS);
    // Left roof side
    glNormal3f(-0.7f, 0.7f, 0.0f);
    glVertex3f(-w / 2, h, -d / 2);
    glVertex3f(-w / 2, h, d / 2);
    glVertex3f(0, h + 5, d / 2);
    glVertex3f(0, h + 5, -d / 2);

    // Right roof side
    glNormal3f(0.7f, 0.7f, 0.0f);
    glVertex3f(w / 2, h, d / 2);
    glVertex3f(w / 2, h, -d / 2);
    glVertex3f(0, h + 5, -d / 2);
    glVertex3f(0, h + 5, d / 2);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);

    glPopMatrix();
}

void DrawTree(float x, float z)
{
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    // Trunk
    BindTexture(texBark);
    DrawTexturedBox(2.0f, 8.0f, 2.0f, 1.0f, 2.0f);

    // Leaves - lower block
    glPushMatrix();
    glTranslatef(0.0f, 7.0f, 0.0f);
    BindTexture(texLeaves);
    DrawTexturedBox(6.0f, 4.0f, 6.0f, 1.0f, 1.0f);
    glPopMatrix();

    // Leaves - upper block
    glPushMatrix();
    glTranslatef(0.0f, 10.0f, 0.0f);
    BindTexture(texLeaves);
    DrawTexturedBox(4.0f, 3.5f, 4.0f, 1.0f, 1.0f);
    glPopMatrix();

    glPopMatrix();
}

// ---------------------------
// Rendering
// ---------------------------
void ApplyCamera()
{
    float totalYaw = carAngle + camYawOffset;
    float yawRad = totalYaw * PI / 180.0f;
    float pitchRad = camPitchOffset * PI / 180.0f;

    float targetX = carX;
    float targetY = carY + 3.0f;
    float targetZ = carZ;

    float camOffsetX = sinf(yawRad) * cosf(pitchRad) * camDistance;
    float camOffsetY = sinf(pitchRad) * camDistance + camHeight;
    float camOffsetZ = -cosf(yawRad) * cosf(pitchRad) * camDistance;

    float eyeX = targetX - camOffsetX;
    float eyeY = targetY + camOffsetY;
    float eyeZ = targetZ - camOffsetZ;

    gluLookAt(
        eyeX, eyeY, eyeZ,
        targetX, targetY, targetZ,
        0.0f, 1.0f, 0.0f
    );
}

void SetupLighting()
{
    if (!enableLighting)
    {
        glDisable(GL_LIGHTING);
        return;
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat globalAmbient[] = { 0.22f, 0.22f, 0.22f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    // Main light
    glEnable(GL_LIGHT0);
    GLfloat light0Pos[] = { mainLightPos[0], mainLightPos[1], mainLightPos[2], mainLightPos[3] };
    GLfloat light0Diffuse[] = { 0.95f, 0.95f, 0.90f, 1.0f };
    GLfloat light0Specular[] = { 0.75f, 0.75f, 0.70f, 1.0f };
    GLfloat light0Ambient[] = { 0.10f, 0.10f, 0.10f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient);

    // Disable extra lights first
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4);

    // Streetlights
    for (int i = 0; i < streetLightCount && i < 4; i++)
    {
        GLenum lid = GL_LIGHT1 + i;
        glEnable(lid);

        GLfloat pos[] = { streetLights[i].x + 1.7f, streetLights[i].y - 0.7f, streetLights[i].z, 1.0f };
        GLfloat diff[] = { 1.0f, 0.92f, 0.70f, 1.0f };
        GLfloat spec[] = { 0.8f, 0.75f, 0.55f, 1.0f };
        GLfloat amb[] = { 0.02f, 0.02f, 0.01f, 1.0f };

        glLightfv(lid, GL_POSITION, pos);
        glLightfv(lid, GL_DIFFUSE, diff);
        glLightfv(lid, GL_SPECULAR, spec);
        glLightfv(lid, GL_AMBIENT, amb);

        glLightf(lid, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(lid, GL_LINEAR_ATTENUATION, 0.025f);
        glLightf(lid, GL_QUADRATIC_ATTENUATION, 0.004f);
    }
}

void DrawRoadWithSidewalk(float x, float z, float w, float d, float rotDeg)
{
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(rotDeg, 0, 1, 0);

    // Sidewalk
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.72f, 0.72f, 0.72f);

    float sw = w + 8.0f;
    float sd = d + 8.0f;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-sw * 0.5f, 0.10f, -sd * 0.5f);
    glVertex3f(sw * 0.5f, 0.10f, -sd * 0.5f);
    glVertex3f(sw * 0.5f, 0.10f, sd * 0.5f);
    glVertex3f(-sw * 0.5f, 0.10f, sd * 0.5f);
    glEnd();

    // Road
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    BindTexture(texRoad);

    float uRep = w / 20.0f;
    float vRep = d / 20.0f;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0, 0);         glVertex3f(-w * 0.5f, 0.12f, -d * 0.5f);
    glTexCoord2f(uRep, 0);      glVertex3f(w * 0.5f, 0.12f, -d * 0.5f);
    glTexCoord2f(uRep, vRep);   glVertex3f(w * 0.5f, 0.12f, d * 0.5f);
    glTexCoord2f(0, vRep);      glVertex3f(-w * 0.5f, 0.12f, d * 0.5f);
    glEnd();

    // Simple yellow center marks
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.95f, 0.85f, 0.15f);

    if (w > d)
    {
        for (float sx = -w * 0.35f; sx < w * 0.35f; sx += 16.0f)
        {
            glBegin(GL_QUADS);
            glNormal3f(0.0f, 1.0f, 0.0f);
            glVertex3f(sx - 3.0f, 0.13f, -0.22f);
            glVertex3f(sx + 3.0f, 0.13f, -0.22f);
            glVertex3f(sx + 3.0f, 0.13f, 0.22f);
            glVertex3f(sx - 3.0f, 0.13f, 0.22f);
            glEnd();
        }
    }
    else
    {
        for (float sz = -d * 0.35f; sz < d * 0.35f; sz += 16.0f)
        {
            glBegin(GL_QUADS);
            glNormal3f(0.0f, 1.0f, 0.0f);
            glVertex3f(-0.22f, 0.13f, sz - 3.0f);
            glVertex3f(0.22f, 0.13f, sz - 3.0f);
            glVertex3f(0.22f, 0.13f, sz + 3.0f);
            glVertex3f(-0.22f, 0.13f, sz + 3.0f);
            glEnd();
        }
    }

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    glPopMatrix();
}

void DrawCityRoads()
{
    // Outer boulevards
    DrawRoadWithSidewalk(0.0f, -145.0f, 280.0f, 18.0f, 0.0f);
    DrawRoadWithSidewalk(0.0f, 145.0f, 280.0f, 18.0f, 0.0f);
    DrawRoadWithSidewalk(-165.0f, 0.0f, 18.0f, 240.0f, 0.0f);
    DrawRoadWithSidewalk(165.0f, 0.0f, 18.0f, 240.0f, 0.0f);

    // Diagonal connectors
    DrawRoadWithSidewalk(-235.0f, -115.0f, 110.0f, 16.0f, 25.0f);
    DrawRoadWithSidewalk(235.0f, 115.0f, 110.0f, 16.0f, 25.0f);
    DrawRoadWithSidewalk(-235.0f, 115.0f, 110.0f, 16.0f, -25.0f);
    DrawRoadWithSidewalk(235.0f, -115.0f, 110.0f, 16.0f, -25.0f);
}

void DrawBuildingRowX(float startX, float z, int count, float spacing)
{
    for (int i = 0; i < count; i++)
    {
        float x = startX + i * spacing;
        float w = 16.0f + (i % 4) * 4.0f;
        float h = 18.0f + (i % 5) * 6.0f;
        float d = 14.0f + (i % 3) * 5.0f;
        DrawBuilding(x, z, w, h, d);
    }
}

void DrawBuildingRowZ(float x, float startZ, int count, float spacing)
{
    for (int i = 0; i < count; i++)
    {
        float z = startZ + i * spacing;
        float w = 14.0f + (i % 3) * 5.0f;
        float h = 20.0f + (i % 4) * 7.0f;
        float d = 16.0f + (i % 4) * 4.0f;
        DrawBuilding(x, z, w, h, d);
    }
}

void DrawCityBuildings()
{
    // Border rows
    DrawBuildingRowX(-300.0f, -280.0f, 9, 75.0f);
    DrawBuildingRowX(-300.0f, 280.0f, 9, 75.0f);
    DrawBuildingRowZ(-340.0f, -200.0f, 6, 80.0f);
    DrawBuildingRowZ(340.0f, -200.0f, 6, 80.0f);

    // Mid-city blocks
    DrawBuilding(-240.0f, -210.0f, 24.0f, 40.0f, 18.0f);
    DrawBuilding(-180.0f, -210.0f, 20.0f, 34.0f, 18.0f);
    DrawBuilding(180.0f, -210.0f, 22.0f, 38.0f, 18.0f);
    DrawBuilding(240.0f, -210.0f, 26.0f, 42.0f, 20.0f);

    DrawBuilding(-240.0f, 210.0f, 24.0f, 36.0f, 18.0f);
    DrawBuilding(-180.0f, 210.0f, 18.0f, 30.0f, 16.0f);
    DrawBuilding(180.0f, 210.0f, 22.0f, 40.0f, 18.0f);
    DrawBuilding(240.0f, 210.0f, 26.0f, 34.0f, 20.0f);

    // Corner clusters
    DrawBuilding(-285.0f, -95.0f, 20.0f, 26.0f, 16.0f);
    DrawBuilding(-285.0f, 95.0f, 18.0f, 28.0f, 16.0f);
    DrawBuilding(285.0f, -95.0f, 22.0f, 32.0f, 18.0f);
    DrawBuilding(285.0f, 95.0f, 18.0f, 24.0f, 16.0f);
}

void DrawBenchModel()
{
    glDisable(GL_TEXTURE_2D);

    // Legs
    glColor3f(0.22f, 0.18f, 0.14f);

    glPushMatrix(); glTranslatef(-1.7f, 0.0f, -0.45f); DrawTexturedBox(0.22f, 1.2f, 0.22f); glPopMatrix();
    glPushMatrix(); glTranslatef(1.7f, 0.0f, -0.45f); DrawTexturedBox(0.22f, 1.2f, 0.22f); glPopMatrix();
    glPushMatrix(); glTranslatef(-1.7f, 0.0f, 0.45f); DrawTexturedBox(0.22f, 1.2f, 0.22f); glPopMatrix();
    glPushMatrix(); glTranslatef(1.7f, 0.0f, 0.45f); DrawTexturedBox(0.22f, 1.2f, 0.22f); glPopMatrix();

    // Seat
    glColor3f(0.56f, 0.33f, 0.14f);
    glPushMatrix(); glTranslatef(0.0f, 1.2f, 0.0f); DrawTexturedBox(4.2f, 0.22f, 1.1f); glPopMatrix();

    // Backrest supports
    glColor3f(0.22f, 0.18f, 0.14f);
    glPushMatrix(); glTranslatef(-1.6f, 1.2f, -0.38f); DrawTexturedBox(0.18f, 1.3f, 0.18f); glPopMatrix();
    glPushMatrix(); glTranslatef(1.6f, 1.2f, -0.38f); DrawTexturedBox(0.18f, 1.3f, 0.18f); glPopMatrix();

    // Backrest planks
    glColor3f(0.56f, 0.33f, 0.14f);
    glPushMatrix(); glTranslatef(0.0f, 2.0f, -0.38f); DrawTexturedBox(4.0f, 0.16f, 0.16f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 2.35f, -0.38f); DrawTexturedBox(4.0f, 0.16f, 0.16f); glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
}

void DrawBench(float x, float z, float rot)
{
    float y = 0.35f + TerrainHeight(x, z);

    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rot, 0, 1, 0);
    DrawBenchModel();
    glPopMatrix();
}

void DrawFlower(float x, float z, float scale)
{
    float y = 0.35f + TerrainHeight(x, z);

    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale, scale, scale);

    glDisable(GL_TEXTURE_2D);

    // Stem
    glColor3f(0.12f, 0.65f, 0.18f);
    DrawTexturedBox(0.08f, 0.75f, 0.08f);

    // Center
    glPushMatrix();
    glTranslatef(0.0f, 0.68f, 0.0f);
    glColor3f(0.95f, 0.85f, 0.10f);
    DrawTexturedBox(0.18f, 0.18f, 0.18f);
    glPopMatrix();

    // Petals
    glColor3f(0.95f, 0.25f, 0.55f);
    glPushMatrix(); glTranslatef(0.18f, 0.66f, 0.0f);   DrawTexturedBox(0.16f, 0.16f, 0.16f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.18f, 0.66f, 0.0f);  DrawTexturedBox(0.16f, 0.16f, 0.16f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.66f, 0.18f);   DrawTexturedBox(0.16f, 0.16f, 0.16f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.66f, -0.18f);  DrawTexturedBox(0.16f, 0.16f, 0.16f); glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);

    glPopMatrix();
}

void DrawFlowerPatch(float cx, float cz, int count, float radius)
{
    for (int i = 0; i < count; i++)
    {
        float a = i * 2.39996323f;
        float r = radius * (0.25f + 0.75f * fmodf(i * 0.37f, 1.0f));
        float x = cx + cosf(a) * r;
        float z = cz + sinf(a) * r;
        float s = 0.75f + 0.08f * (i % 4);

        DrawFlower(x, z, s);
    }
}

void DrawExtraTrees()
{
    DrawTree(-110.0f, -95.0f);
    DrawTree(110.0f, -95.0f);
    DrawTree(-110.0f, 95.0f);
    DrawTree(110.0f, 95.0f);

    DrawTree(-185.0f, -150.0f);
    DrawTree(-215.0f, -40.0f);
    DrawTree(-205.0f, 55.0f);
    DrawTree(-185.0f, 150.0f);

    DrawTree(185.0f, -150.0f);
    DrawTree(215.0f, -40.0f);
    DrawTree(205.0f, 55.0f);
    DrawTree(185.0f, 150.0f);
}

void DrawParkDecorations()
{
    // Benches inside the oval / park area
    DrawBench(-35.0f, -18.0f, 25.0f);
    DrawBench(35.0f, -18.0f, -25.0f);
    DrawBench(-10.0f, 28.0f, 180.0f);
    DrawBench(10.0f, -32.0f, 0.0f);

    // Flower patches
    DrawFlowerPatch(-28.0f, 18.0f, 14, 10.0f);
    DrawFlowerPatch(26.0f, 22.0f, 12, 9.0f);
    DrawFlowerPatch(-20.0f, -24.0f, 15, 11.0f);
    DrawFlowerPatch(22.0f, -20.0f, 13, 10.0f);
}

void DrawStreetLight(float x, float y, float z)
{
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glDisable(GL_TEXTURE_2D);

    // Pole
    glColor3f(0.18f, 0.18f, 0.18f);
    glPushMatrix();
    DrawTexturedBox(0.5f, y, 0.5f);
    glPopMatrix();

    // Arm
    glPushMatrix();
    glTranslatef(0.9f, y - 0.5f, 0.0f);
    DrawTexturedBox(1.8f, 0.18f, 0.18f);
    glPopMatrix();

    // Lamp bulb
    glPushMatrix();
    glTranslatef(1.7f, y - 0.7f, 0.0f);
    glColor3f(1.0f, 0.95f, 0.65f);
    glutSolidSphere(0.35, 12, 12);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    glPopMatrix();
}

void DrawStreetLights()
{
    for (int i = 0; i < streetLightCount; i++)
    {
        DrawStreetLight(streetLights[i].x, streetLights[i].y, streetLights[i].z);
    }
}

void DrawStaticObjects()
{
    DrawCityBuildings();
    DrawExtraTrees();
    DrawParkDecorations();
    DrawStreetLights();
}

void DrawShadowCasters()
{
    DrawCityBuildings();
    DrawExtraTrees();
    DrawParkDecorations();
    DrawStreetLights();
}

void DrawCar()
{
    glPushMatrix();
    glTranslatef(carX, carY, carZ);
    glRotatef(carAngle, 0, 1, 0);

    glDisable(GL_TEXTURE_2D);

    // Body
    glColor3f(0.85f, 0.1f, 0.1f);
    glPushMatrix();
    DrawTexturedBox(8.0f, 2.2f, 14.0f);
    glPopMatrix();

    // Cabin
    glColor3f(0.75f, 0.85f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, 2.2f, -1.0f);
    DrawTexturedBox(6.0f, 2.0f, 6.0f);
    glPopMatrix();

    // Wheels
    glColor3f(0.1f, 0.1f, 0.1f);

    glPushMatrix(); glTranslatef(-3.5f, 0.0f, -4.5f); DrawTexturedBox(1.2f, 1.2f, 2.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(3.5f, 0.0f, -4.5f); DrawTexturedBox(1.2f, 1.2f, 2.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-3.5f, 0.0f, 4.5f); DrawTexturedBox(1.2f, 1.2f, 2.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(3.5f, 0.0f, 4.5f); DrawTexturedBox(1.2f, 1.2f, 2.0f); glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);

    glPopMatrix();
}

void UpdateCar()
{
    float turnSpeed = 2.0f;
    float accel = 0.08f;
    float maxSpeed = 1.2f;
    float friction = 0.04f;
    float brakeForce = 0.12f;

    if (keys['w'] || keys['W']) carSpeed += accel;
    if (keys['s'] || keys['S']) carSpeed -= accel;

    if (carSpeed > maxSpeed) carSpeed = maxSpeed;
    if (carSpeed < -maxSpeed * 0.6f) carSpeed = -maxSpeed * 0.6f;

    // Turn only when moving
    if (fabs(carSpeed) > 0.01f)
    {
        if (keys['a'] || keys['A'])
            carAngle -= turnSpeed * (carSpeed >= 0.0f ? 1.0f : -1.0f);

        if (keys['d'] || keys['D'])
            carAngle += turnSpeed * (carSpeed >= 0.0f ? 1.0f : -1.0f);
    }

    // Friction
    if (!(keys['w'] || keys['W']) && !(keys['s'] || keys['S']))
    {
        if (carSpeed > 0.0f)
        {
            carSpeed -= friction;
            if (carSpeed < 0.0f) carSpeed = 0.0f;
        }
        else if (carSpeed < 0.0f)
        {
            carSpeed += friction;
            if (carSpeed > 0.0f) carSpeed = 0.0f;
        }
    }

    if (keys[' ']) // Space = brake
    {
        if (carSpeed > 0.0f)
        {
            carSpeed -= brakeForce;
            if (carSpeed < 0.0f) carSpeed = 0.0f;
        }
        else if (carSpeed < 0.0f)
        {
            carSpeed += brakeForce;
            if (carSpeed > 0.0f) carSpeed = 0.0f;
        }
    }

    float rad = carAngle * PI / 180.0f;
    float nextX = carX + sinf(rad) * carSpeed;
    float nextZ = carZ - cosf(rad) * carSpeed;

    if (!CarCollidesWithBuildings(nextX, nextZ))
    {
        carX = nextX;
        carZ = nextZ;
    }
    else
    {
        carSpeed = 0.0f;
    }
}

void BuildShadowMatrix(float shadowMat[16], const float groundPlane[4], const float lightPos[4])
{
    float dot =
        groundPlane[0] * lightPos[0] +
        groundPlane[1] * lightPos[1] +
        groundPlane[2] * lightPos[2] +
        groundPlane[3] * lightPos[3];

    shadowMat[0] = dot - lightPos[0] * groundPlane[0];
    shadowMat[4] = 0.0f - lightPos[0] * groundPlane[1];
    shadowMat[8] = 0.0f - lightPos[0] * groundPlane[2];
    shadowMat[12] = 0.0f - lightPos[0] * groundPlane[3];

    shadowMat[1] = 0.0f - lightPos[1] * groundPlane[0];
    shadowMat[5] = dot - lightPos[1] * groundPlane[1];
    shadowMat[9] = 0.0f - lightPos[1] * groundPlane[2];
    shadowMat[13] = 0.0f - lightPos[1] * groundPlane[3];

    shadowMat[2] = 0.0f - lightPos[2] * groundPlane[0];
    shadowMat[6] = 0.0f - lightPos[2] * groundPlane[1];
    shadowMat[10] = dot - lightPos[2] * groundPlane[2];
    shadowMat[14] = 0.0f - lightPos[2] * groundPlane[3];

    shadowMat[3] = 0.0f - lightPos[3] * groundPlane[0];
    shadowMat[7] = 0.0f - lightPos[3] * groundPlane[1];
    shadowMat[11] = 0.0f - lightPos[3] * groundPlane[2];
    shadowMat[15] = dot - lightPos[3] * groundPlane[3];
}

bool CheckAABBCollision(float x1, float z1, float w1, float d1,
    float x2, float z2, float w2, float d2)
{
    float left1 = x1 - w1 * 0.5f;
    float right1 = x1 + w1 * 0.5f;
    float front1 = z1 + d1 * 0.5f;
    float back1 = z1 - d1 * 0.5f;

    float left2 = x2 - w2 * 0.5f;
    float right2 = x2 + w2 * 0.5f;
    float front2 = z2 + d2 * 0.5f;
    float back2 = z2 - d2 * 0.5f;

    if (right1 < left2) return false;
    if (left1 > right2) return false;
    if (front1 < back2) return false;
    if (back1 > front2) return false;

    return true;
}

bool CarCollidesWithBuildings(float testX, float testZ)
{
    for (int i = 0; i < buildingCount; i++)
    {
        if (CheckAABBCollision(
            testX, testZ, carWidth, carDepth,
            buildings[i].x, buildings[i].z, buildings[i].w, buildings[i].d))
        {
            return true;
        }
    }
    return false;
}

void RenderShadowFromLight(const float lightPos[4])
{
    float groundPlane[4] = { 0.0f, 1.0f, 0.0f, -0.05f };
    float shadowMat[16];
    BuildShadowMatrix(shadowMat, groundPlane, lightPos);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, 0.28f);

    glPushMatrix();
    glMultMatrixf(shadowMat);
    DrawShadowCasters();
    glPopMatrix();

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    if (enableLighting)
        glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
}

void Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    ApplyCamera();
    SetupLighting();

    DrawSkyCylinder();
    DrawGround();
    DrawTerrain();

    DrawRoad();       // main oval circuit
    DrawCityRoads();  // extra city roads

    void DrawCar();
    void UpdateCar();

    if (enableShadows)
    {
        RenderShadowFromLight(mainLightPos);

        for (int i = 0; i < streetLightCount; i++)
        {
            float lp[4] =
            {
                streetLights[i].x + 1.7f,
                streetLights[i].y - 0.7f,
                streetLights[i].z,
                1.0f
            };
            RenderShadowFromLight(lp);
        }
    }
    DrawRandomMovers();
    DrawAutoCar();

    buildingCount = 0;

    DrawStaticObjects();

    DrawCar();

    glutSwapBuffers();
}

void InitBuildings()
{
    buildingCount = 0;

    // Border rows
    for (int i = 0; i < 9; i++)
    {
        float x = -300.0f + i * 75.0f;
        float w = 16.0f + (i % 4) * 4.0f;
        float h = 18.0f + (i % 5) * 6.0f;
        float d = 14.0f + (i % 3) * 5.0f;
        AddBuildingData(x, -280.0f, w, h, d);
    }

    for (int i = 0; i < 9; i++)
    {
        float x = -300.0f + i * 75.0f;
        float w = 16.0f + (i % 4) * 4.0f;
        float h = 18.0f + (i % 5) * 6.0f;
        float d = 14.0f + (i % 3) * 5.0f;
        AddBuildingData(x, 280.0f, w, h, d);
    }

    for (int i = 0; i < 6; i++)
    {
        float z = -200.0f + i * 80.0f;
        float w = 14.0f + (i % 3) * 5.0f;
        float h = 20.0f + (i % 4) * 7.0f;
        float d = 16.0f + (i % 4) * 4.0f;
        AddBuildingData(-340.0f, z, w, h, d);
    }

    for (int i = 0; i < 6; i++)
    {
        float z = -200.0f + i * 80.0f;
        float w = 14.0f + (i % 3) * 5.0f;
        float h = 20.0f + (i % 4) * 7.0f;
        float d = 16.0f + (i % 4) * 4.0f;
        AddBuildingData(340.0f, z, w, h, d);
    }

    AddBuildingData(-240.0f, -210.0f, 24.0f, 40.0f, 18.0f);
    AddBuildingData(-180.0f, -210.0f, 20.0f, 34.0f, 18.0f);
    AddBuildingData(180.0f, -210.0f, 22.0f, 38.0f, 18.0f);
    AddBuildingData(240.0f, -210.0f, 26.0f, 42.0f, 20.0f);

    AddBuildingData(-240.0f, 210.0f, 24.0f, 36.0f, 18.0f);
    AddBuildingData(-180.0f, 210.0f, 18.0f, 30.0f, 16.0f);
    AddBuildingData(180.0f, 210.0f, 22.0f, 40.0f, 18.0f);
    AddBuildingData(240.0f, 210.0f, 26.0f, 34.0f, 20.0f);

    AddBuildingData(-285.0f, -95.0f, 20.0f, 26.0f, 16.0f);
    AddBuildingData(-285.0f, 95.0f, 18.0f, 28.0f, 16.0f);
    AddBuildingData(285.0f, -95.0f, 22.0f, 32.0f, 18.0f);
    AddBuildingData(285.0f, 95.0f, 18.0f, 24.0f, 16.0f);
}

void DrawRandomMover(float x, float z)
{
    glPushMatrix();
    glTranslatef(x, 0.2f, z);

    glDisable(GL_TEXTURE_2D);

    // body
    glColor3f(0.2f, 0.2f, 0.9f);
    glPushMatrix();
    DrawTexturedBox(1.2f, 2.5f, 1.2f);
    glPopMatrix();

    // head
    glColor3f(1.0f, 0.8f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 2.5f, 0.0f);
    glutSolidSphere(0.5, 12, 12);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);

    glPopMatrix();
}

void DrawRandomMovers()
{
    for (int i = 0; i < randomMoverCount; i++)
    {
        DrawRandomMover(randomMovers[i].x, randomMovers[i].z);
    }
}

bool RandomMoverCollides(float x, float z)
{
    for (int i = 0; i < buildingCount; i++)
    {
        if (CheckAABBCollision(x, z, 1.2f, 1.2f,
            buildings[i].x, buildings[i].z,
            buildings[i].w, buildings[i].d))
        {
            return true;
        }
    }
    return false;
}

void UpdateRandomMovers()
{
    for (int i = 0; i < randomMoverCount; i++)
    {
        randomMovers[i].changeTimer--;

        if (randomMovers[i].changeTimer <= 0)
        {
            randomMovers[i].angle = (float)(rand() % 360);
            randomMovers[i].changeTimer = 50 + rand() % 100;
        }

        float rad = randomMovers[i].angle * PI / 180.0f;
        float nextX = randomMovers[i].x + sinf(rad) * randomMovers[i].speed;
        float nextZ = randomMovers[i].z - cosf(rad) * randomMovers[i].speed;

        // keep inside map and avoid buildings
        if (nextX < -350 || nextX > 350 || nextZ < -350 || nextZ > 350 ||
            RandomMoverCollides(nextX, nextZ))
        {
            randomMovers[i].angle = (float)(rand() % 360);
            continue;
        }

        randomMovers[i].x = nextX;
        randomMovers[i].z = nextZ;
    }
}

void Reshape(int w, int h)
{
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, aspect, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ---------------------------
// Movement
// ---------------------------
void UpdateCamera()
{
    if (specialKeys[GLUT_KEY_LEFT])  camYawOffset -= 2.0f;
    if (specialKeys[GLUT_KEY_RIGHT]) camYawOffset += 2.0f;
    if (specialKeys[GLUT_KEY_UP])    camPitchOffset += 1.0f;
    if (specialKeys[GLUT_KEY_DOWN])  camPitchOffset -= 1.0f;

    if (camPitchOffset > 45.0f) camPitchOffset = 45.0f;
    if (camPitchOffset < 5.0f)  camPitchOffset = 5.0f;
}

void Timer(int value)
{
    UpdateCamera();
    UpdateCar();
    UpdateRandomMovers();
    UpdateAutoCar();
    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0);
}

// ---------------------------
// Input
// ---------------------------
void KeyboardDown(unsigned char key, int x, int y)
{
    keys[key] = true;

    if (key == 27) // ESC
        exit(0);

    if (key == 'l' || key == 'L')
        enableLighting = !enableLighting;

    if (key == 'k' || key == 'K')
        enableShadows = !enableShadows;
}

void KeyboardUp(unsigned char key, int x, int y)
{
    keys[key] = false;
}

void SpecialDown(int key, int x, int y)
{
    if (key >= 0 && key < 256)
        specialKeys[key] = true;
}

void SpecialUp(int key, int x, int y)
{
    if (key >= 0 && key < 256)
        specialKeys[key] = false;
}

// ---------------------------
// Init
// ---------------------------
void Init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.55f, 0.78f, 0.95f, 1.0f);
    InitBuildings();
    InitRandomMovers();

    LoadAllTextures();
}

// ---------------------------
// Main
// ---------------------------
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("OpenGL Project - P3");

    Init();

    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(KeyboardDown);
    glutKeyboardUpFunc(KeyboardUp);
    glutSpecialFunc(SpecialDown);
    glutSpecialUpFunc(SpecialUp);
    glutTimerFunc(16, Timer, 0);

    srand((unsigned int)time(0));

    glutMainLoop();
    return 0;
}