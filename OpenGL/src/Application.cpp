#include <windows.h>
#include <GL/glut.h>
#include <iostream>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint grassTexture;
GLuint skyTexture;

/* camera angles */
double cameraAngleX = 65;
double cameraAngleY = 7.5;
double cameraAngleZ = 30;

/* ball position */
double ballPositionX = 5;
double ballPositionY = 5;
double ballPositionZ = 5;

/* window size */
int windowWidth = 800;
int windowHeight = 600;

/* bouncing ball */
double width = 5.0;
double height = 8.5;
bool ballMove = true;

/* camera limits */
float cameraMinX = -20.0f;
float cameraMaxX = 20.0f;
float cameraMinY = 0.0f;
float cameraMaxY = 20.0f;

/* texture loader */
GLuint loadTexture(const char* filename)
{
    GLuint texture = 0;

    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);

    if (data)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D,
            0,
            channels == 4 ? GL_RGBA : GL_RGB,
            width,
            height,
            0,
            channels == 4 ? GL_RGBA : GL_RGB,
            GL_UNSIGNED_BYTE,
            data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        std::cout << "Loaded texture: " << filename << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture: " << filename << std::endl;
    }

    stbi_image_free(data);

    return texture;
}

/* OpenGL initialization */
void initGL()
{
    glClearColor(0, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPos[] = { 0.0f,10.0f,0.0f,1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_TEXTURE_2D);

    grassTexture = loadTexture("grass.jpg");
    skyTexture = loadTexture("sky.jpg");
}

/* draw terrain */
void drawTerrain()
{
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    glBegin(GL_TRIANGLES);

    for (float x = -25; x < 25; x += 1.0)
    {
        for (float z = -25; z < 25; z += 1.0)
        {
            float y1 = sin(x * 0.2) * cos(z * 0.2);
            float y2 = sin((x + 1) * 0.2) * cos(z * 0.2);
            float y3 = sin(x * 0.2) * cos((z + 1) * 0.2);
            float y4 = sin((x + 1) * 0.2) * cos((z + 1) * 0.2);

            glTexCoord2f(0, 0); glVertex3f(x, y1, z);
            glTexCoord2f(1, 0); glVertex3f(x + 1, y2, z);
            glTexCoord2f(0, 1); glVertex3f(x, y3, z + 1);

            glTexCoord2f(1, 0); glVertex3f(x + 1, y2, z);
            glTexCoord2f(1, 1); glVertex3f(x + 1, y4, z + 1);
            glTexCoord2f(0, 1); glVertex3f(x, y3, z + 1);
        }
    }

    glEnd();
}

/* draw sky wall */
void drawSky()
{
    glBindTexture(GL_TEXTURE_2D, skyTexture);

    glBegin(GL_QUADS);

    glTexCoord2f(0, 0); glVertex3f(-50, 0, -50);
    glTexCoord2f(1, 0); glVertex3f(50, 0, -50);
    glTexCoord2f(1, 1); glVertex3f(50, 50, -50);
    glTexCoord2f(0, 1); glVertex3f(-50, 50, -50);

    glEnd();
}

/* display scene */
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(-10 + cameraAngleX,
        9 + cameraAngleY,
        -10 + cameraAngleZ,
        ballPositionX,
        ballPositionY,
        ballPositionZ,
        0, 6, 0);

    drawSky();
    drawTerrain();

    /* first sphere */
    glPushMatrix();
    glTranslatef(7, width + 1.5, 4);
    glColor3f(1, 0, 1);
    glutSolidSphere(1.9f, 50, 50);
    glPopMatrix();

    /* second sphere */
    glPushMatrix();
    glTranslatef(7, width + 1, 9);
    glColor3f(1, 1, 1);
    glutSolidSphere(1.9f, 50, 50);
    glPopMatrix();

    glutSwapBuffers();
}

/* reshape window */
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0, (float)w / (float)h, 0.1, 100.0);
}

/* keyboard control */
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
        break;

    case 'a':
        ballPositionX += 0.8;
        break;

    case 'd':
        ballPositionX -= 0.3;
        break;

    case 'w':
        ballPositionY += 0.3;
        break;

    case 's':
        ballPositionY -= 0.3;
        break;

    case 'q':
        ballPositionZ += 0.8;
        break;

    case 'e':
        ballPositionZ -= 0.8;
        break;
    }

    glutPostRedisplay();
}

/* special keyboard */
void special_keyboard(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        cameraAngleX -= 0.5f;
        if (cameraAngleX < cameraMinX)
            cameraAngleX = cameraMinX;
        break;

    case GLUT_KEY_DOWN:
        cameraAngleX += 0.5f;
        if (cameraAngleX > cameraMaxX)
            cameraAngleX = cameraMaxX;
        break;

    case GLUT_KEY_LEFT:
        cameraAngleY -= 0.5f;
        if (cameraAngleY < cameraMinY)
            cameraAngleY = cameraMinY;
        break;

    case GLUT_KEY_RIGHT:
        cameraAngleY += 0.5f;
        if (cameraAngleY > cameraMaxY)
            cameraAngleY = cameraMaxY;
        break;
    }

    glutPostRedisplay();
}

/* bouncing animation */
void bouncing_balls()
{
    if (width <= 0 || width >= height)
        ballMove = !ballMove;

    if (ballMove)
        width += 0.1;
    else
        width -= 0.1;

    glutPostRedisplay();
}

/* main */
int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);

    glutCreateWindow("Bile Saltarete");

    initGL();

    glutDisplayFunc(display);
    glutIdleFunc(bouncing_balls);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special_keyboard);
    glutReshapeFunc(reshape);

    glutMainLoop();

    return 0;
}