#include <windows.h>
#include <freeglut.h>
#include <cmath>
#include <cstdio>

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

// ---------------------------
// Camera
// ---------------------------
float camX = 0.0f;
float camY = 12.0f;
float camZ = 180.0f;
float yaw = 0.0f;
float pitch = -5.0f;

bool keys[256] = { false };
bool specialKeys[256] = { false };

//Functions
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

// Box anchored at ground level (y starts from 0)
void DrawTexturedBox(float w, float h, float d, float uRepeat = 1.0f, float vRepeat = 1.0f)
{
    float x = w * 0.5f;
    float z = d * 0.5f;

    glBegin(GL_QUADS);

    // Front
    glTexCoord2f(0, 0);        glVertex3f(-x, 0, z);
    glTexCoord2f(uRepeat, 0);  glVertex3f(x, 0, z);
    glTexCoord2f(uRepeat, vRepeat); glVertex3f(x, h, z);
    glTexCoord2f(0, vRepeat);  glVertex3f(-x, h, z);

    // Back
    glTexCoord2f(0, 0);        glVertex3f(x, 0, -z);
    glTexCoord2f(uRepeat, 0);  glVertex3f(-x, 0, -z);
    glTexCoord2f(uRepeat, vRepeat); glVertex3f(-x, h, -z);
    glTexCoord2f(0, vRepeat);  glVertex3f(x, h, -z);

    // Left
    glTexCoord2f(0, 0);        glVertex3f(-x, 0, -z);
    glTexCoord2f(uRepeat, 0);  glVertex3f(-x, 0, z);
    glTexCoord2f(uRepeat, vRepeat); glVertex3f(-x, h, z);
    glTexCoord2f(0, vRepeat);  glVertex3f(-x, h, -z);

    // Right
    glTexCoord2f(0, 0);        glVertex3f(x, 0, z);
    glTexCoord2f(uRepeat, 0);  glVertex3f(x, 0, -z);
    glTexCoord2f(uRepeat, vRepeat); glVertex3f(x, h, -z);
    glTexCoord2f(0, vRepeat);  glVertex3f(x, h, z);

    // Top
    glTexCoord2f(0, 0);        glVertex3f(-x, h, z);
    glTexCoord2f(uRepeat, 0);  glVertex3f(x, h, z);
    glTexCoord2f(uRepeat, vRepeat); glVertex3f(x, h, -z);
    glTexCoord2f(0, vRepeat);  glVertex3f(-x, h, -z);

    // Bottom
    glTexCoord2f(0, 0);        glVertex3f(-x, 0, -z);
    glTexCoord2f(uRepeat, 0);  glVertex3f(x, 0, -z);
    glTexCoord2f(uRepeat, vRepeat); glVertex3f(x, 0, z);
    glTexCoord2f(0, vRepeat);  glVertex3f(-x, 0, z);

    glEnd();
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
    glTexCoord2f(0, 0);         glVertex3f(-size, 0.0f, -size);
    glTexCoord2f(repeat, 0);    glVertex3f(size, 0.0f, -size);
    glTexCoord2f(repeat, repeat); glVertex3f(size, 0.0f, size);
    glTexCoord2f(0, repeat);    glVertex3f(-size, 0.0f, size);
    glEnd();
}

void DrawSkyCylinder()
{
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

            float u = (float)ix / N * texRepeat;
            float v0 = (float)iz / N * texRepeat;
            float v1 = (float)(iz + 1) / N * texRepeat;

            glTexCoord2f(u, v0); glVertex3f(x, y0, z0);
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

        glTexCoord2f(1.0f, u);
        glVertex3f(outerRX * c, y, outerRZ * s);

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
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    BindTexture(texBuilding);
    DrawTexturedBox(w, h, d, 2.0f, 2.0f);

    // Simple roof
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.25f, 0.1f, 0.1f);
    glBegin(GL_TRIANGLES);

    // Front
    glVertex3f(-w / 2, h, d / 2);
    glVertex3f(w / 2, h, d / 2);
    glVertex3f(0, h + 5, d / 2);

    // Back
    glVertex3f(w / 2, h, -d / 2);
    glVertex3f(-w / 2, h, -d / 2);
    glVertex3f(0, h + 5, -d / 2);

    glEnd();

    glBegin(GL_QUADS);
    // Left roof side
    glVertex3f(-w / 2, h, -d / 2);
    glVertex3f(-w / 2, h, d / 2);
    glVertex3f(0, h + 5, d / 2);
    glVertex3f(0, h + 5, -d / 2);

    // Right roof side
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
    glRotatef(-pitch, 1, 0, 0);
    glRotatef(-yaw, 0, 1, 0);
    glTranslatef(-camX, -camY, -camZ);
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

    glPushMatrix(); glTranslatef(0.18f, 0.66f, 0.0f);  DrawTexturedBox(0.16f, 0.16f, 0.16f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.18f, 0.66f, 0.0f);  DrawTexturedBox(0.16f, 0.16f, 0.16f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.66f, 0.18f);  DrawTexturedBox(0.16f, 0.16f, 0.16f); glPopMatrix();
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

void DrawStaticObjects()
{
    DrawCityBuildings();
    DrawExtraTrees();
    DrawParkDecorations();
}

void Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    ApplyCamera();

    DrawSkyCylinder();
    DrawGround();
    DrawTerrain();

    DrawRoad();       // main oval circuit
    DrawCityRoads();  // extra city roads

    DrawStaticObjects();

    glutSwapBuffers();
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
    float rad = yaw * PI / 180.0f;
    float speed = 1.6f;
    float strafeSpeed = 1.4f;
    float verticalSpeed = 1.2f;

    // Forward / backward
    if (keys['w'] || keys['W'])
    {
        camX += sinf(rad) * speed;
        camZ -= cosf(rad) * speed;
    }
    if (keys['s'] || keys['S'])
    {
        camX -= sinf(rad) * speed;
        camZ += cosf(rad) * speed;
    }

    // Strafe
    if (keys['a'] || keys['A'])
    {
        camX -= cosf(rad) * strafeSpeed;
        camZ -= sinf(rad) * strafeSpeed;
    }
    if (keys['d'] || keys['D'])
    {
        camX += cosf(rad) * strafeSpeed;
        camZ += sinf(rad) * strafeSpeed;
    }

    // Up / down
    if (keys['q'] || keys['Q']) camY += verticalSpeed;
    if (keys['e'] || keys['E']) camY -= verticalSpeed;

    // Rotation
    if (specialKeys[GLUT_KEY_LEFT])  yaw -= 1.5f;
    if (specialKeys[GLUT_KEY_RIGHT]) yaw += 1.5f;
    if (specialKeys[GLUT_KEY_UP])    pitch += 1.0f;
    if (specialKeys[GLUT_KEY_DOWN])  pitch -= 1.0f;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

void Timer(int value)
{
    UpdateCamera();
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
    glutCreateWindow("OpenGL Project");

    Init();

    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(KeyboardDown);
    glutKeyboardUpFunc(KeyboardUp);
    glutSpecialFunc(SpecialDown);
    glutSpecialUpFunc(SpecialUp);
    glutTimerFunc(16, Timer, 0);

    glutMainLoop();
    return 0;
}