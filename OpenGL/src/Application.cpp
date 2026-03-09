#include <windows.h>  
#include <GL/glut.h> 
#include <iostream>
#include <math.h>
#include <stdlib.h> // Adaugat pentru functia rand() necesara zapezii

/* unghiurile camerei */
double cameraAngleX = 25;
double cameraAngleY = 1;
double cameraAngleZ = 25;

/* pozitia unei bile în spatiu / punctul in care se uita camera */
double ballPositionX = 5;
double ballPositionY = 5;
double ballPositionZ = 5;

/* dimensiuni fereastra */
int windowWidth = 800;
int windowHeight = 600;
double width = 5.0;
double height = 8.5;

bool ballMove = true;

// limite camera
float cameraMinX = -20.0f;
float cameraMaxX = 60.0f;
float cameraMinY = 0.0f;
float cameraMaxY = 20.0f;
float cameraMinZ = -20.0f;
float cameraMaxZ = 60.0f;

void initGL() {
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    GLfloat lightPos[] = { 10.0f, 20.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void drawSea()
{
    glDisable(GL_LIGHTING);

    glBegin(GL_TRIANGLES);

    for (float x = -25; x < 25; x += 1.0f)
    {
        for (float z = -25; z < -5; z += 1.0f) // ocean starts far away
        {
            float y1 = sin(x * 0.3 + glutGet(GLUT_ELAPSED_TIME) * 0.002) * 0.2f;
            float y2 = sin((x + 1) * 0.3 + glutGet(GLUT_ELAPSED_TIME) * 0.002) * 0.2f;
            float y3 = sin(z * 0.3 + glutGet(GLUT_ELAPSED_TIME) * 0.002) * 0.2f;

            glColor3f(0.0f, 0.4f, 0.8f);

            glVertex3f(x, y1, z);
            glVertex3f(x + 1, y2, z);
            glVertex3f(x, y3, z + 1);

            glVertex3f(x + 1, y2, z);
            glVertex3f(x + 1, y1, z + 1);
            glVertex3f(x, y3, z + 1);
        }
    }

    glEnd();

    glEnable(GL_LIGHTING);
}

void drawHorizon()
{
    glDisable(GL_LIGHTING);

    glBegin(GL_QUADS);

    glColor3f(0.0f, 0.5f, 0.9f);

    glVertex3f(-25, 0, -25);
    glVertex3f(25, 0, -25);
    glVertex3f(25, 0, -10);
    glVertex3f(-25, 0, -10);

    glEnd();

    glEnable(GL_LIGHTING);
}

void drawTree(float x, float z)
{
    glPushMatrix();
    glTranslatef(x, 0, z);

    // trunk
    glColor3f(0.4f, 0.25f, 0.1f);
    glPushMatrix();
    glTranslatef(0, 1.5f, 0);
    glScalef(0.5f, 3.0f, 0.5f);
    glutSolidCube(1);
    glPopMatrix();

    // leaves
    glColor3f(0.1f, 0.7f, 0.1f);
    glPushMatrix();
    glTranslatef(0, 4.0f, 0);
    glutSolidSphere(1.5, 20, 20);
    glPopMatrix();

    glPopMatrix();
}

void drawFlower(float x, float z)
{
    glPushMatrix();
    glTranslatef(x, 0.2f, z);

    // stem
    glColor3f(0.0f, 0.7f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    glEnd();

    // center
    glPushMatrix();
    glTranslatef(0, 1, 0);
    glColor3f(1.0f, 0.8f, 0.0f);
    glutSolidSphere(0.1, 10, 10);
    glPopMatrix();

    // petals
    glColor3f(1.0f, 0.4f, 0.7f);

    for (int i = 0; i < 6; i++)
    {
        float angle = i * 60 * 3.14159 / 180;

        glPushMatrix();
        glTranslatef(cos(angle) * 0.2, 1, sin(angle) * 0.2);
        glutSolidSphere(0.1, 10, 10);
        glPopMatrix();
    }

    glPopMatrix();
}

void drawSun()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float sunX = 15;
    float sunY = 20;
    float sunZ = -20;

    glPushMatrix();
    glTranslatef(sunX, sunY, sunZ);

    // rays
    glColor4f(1.0f, 0.9f, 0.2f, 0.4f);

    int rays = 40;
    float radius = 4.0f;

    glBegin(GL_TRIANGLES);

    for (int i = 0; i < rays; i++)
    {
        float angle = i * 2 * 3.14159 / rays;
        float nextAngle = (i + 1) * 2 * 3.14159 / rays;

        glVertex3f(0, 0, 0);
        glVertex3f(cos(angle) * radius, sin(angle) * radius, 0);
        glVertex3f(cos(nextAngle) * radius, sin(nextAngle) * radius, 0);
    }

    glEnd();

    // sun core
    glColor3f(1.0f, 0.9f, 0.0f);
    glutSolidSphere(2.5, 30, 30);

    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(-10 + cameraAngleX, 9 + cameraAngleY, -10 + cameraAngleZ,
        ballPositionX, ballPositionY, ballPositionZ,
        0, 1, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_LIGHTING);

    // ================= CERUL =================
    glBegin(GL_QUADS);

    // bottom horizon
    glColor3f(0.6f, 0.85f, 1.0f);
    glVertex3f(-25, 0, -25);
    glVertex3f(25, 0, -25);

    // upper sky
    glColor3f(0.2f, 0.6f, 1.0f);
    glVertex3f(25, 25, -25);
    glVertex3f(-25, 25, -25);

    glEnd();

	//SEA and HORIZON
    drawSea();
    drawHorizon();
    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.45f, 0.9f);
    glVertex3f(-25, 0, -25);
    glVertex3f(25, 0, -25);
    glVertex3f(25, 0, -10);
    glVertex3f(-25, 0, -10);
    glEnd();

    //SAND
    glBegin(GL_QUADS);
    glColor3f(0.95f, 0.9f, 0.6f);

    glVertex3f(-25, 0, -10);
    glVertex3f(25, 0, -10);
    glVertex3f(25, 0, 0);
    glVertex3f(-25, 0, 0);

    glEnd();


    // ================= PAMANT =================
    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.2f, 0.1f);

    glVertex3f(-25, -0.1f, 0);
    glVertex3f(-25, -0.1f, 25);
    glVertex3f(25, -0.1f, 25);
    glVertex3f(25, -0.1f, 0);

    glEnd();

    // ================= IARBA/TEREN DENIVELAT =================
    glBegin(GL_TRIANGLES);
    for (float x = -25; x < 25; x += 0.5) {
        for (float z = 0; z < 25; z += 0.5) {
            float y1 = sin(x) * cos(z) * 0.4f;
            float y2 = sin(x + 0.5) * cos(z) * 0.4f;
            float y3 = sin(x) * cos(z + 0.5) * 0.4f;
            float y4 = sin(x + 0.5) * cos(z + 0.5) * 0.4f;

            glColor3f(0.2f + y1 * 0.05f, 0.8f, 0.2f); // Iarba ceva mai rece/brumata

            glVertex3f(x, y1, z);
            glVertex3f(x + 0.5, y2, z);
            glVertex3f(x, y3, z + 0.5);

            glVertex3f(x + 0.5, y2, z);
            glVertex3f(x + 0.5, y4, z + 0.5);
            glVertex3f(x, y3, z + 0.5);
        }
    }
    glEnd();

    //TREES
    drawTree(-10, -5);
    drawTree(5, 6);
    drawTree(-6, 10);
    drawTree(12, -3);

    //FLOWERS
    drawFlower(-5, 3);
    drawFlower(-2, 7);
    drawFlower(3, -4);
    drawFlower(8, 2);
    drawFlower(-8, -3);

    //SUN
    drawSun();

    glEnable(GL_LIGHTING);

    // ================= BILELE =================
    glPushMatrix();
    glTranslatef(4, width + 1.5, 4);
    glColor3f(1, 1, 0); // Galben
    glutSolidSphere(1.9f, 50, 50);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2, width + 1, 9);
    glColor3f(1, 1, 1); // Alb
    glutSolidSphere(1.9f, 50, 50);
    glPopMatrix();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 0.1, 100.0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27:
        exit(0);
        break;
    case 'a': ballPositionX += 0.8; break;
    case 'd': ballPositionX -= 0.3; break;
    case 'w': ballPositionY += 0.3; break;
    case 's': ballPositionY -= 0.3; break;
    case 'q': ballPositionZ += 0.8; break;
    case 'e': ballPositionZ -= 0.8; break;
    }
    glutPostRedisplay();
}

void special_keyboard(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        cameraAngleX -= 0.5f;
        if (cameraAngleX < cameraMinX) cameraAngleX = cameraMinX;
        break;
    case GLUT_KEY_DOWN:
        cameraAngleX += 0.5f;
        if (cameraAngleX > cameraMaxX) cameraAngleX = cameraMaxX;
        break;
    case GLUT_KEY_LEFT:
        cameraAngleY -= 0.5f;
        if (cameraAngleY < cameraMinY) cameraAngleY = cameraMinY;
        break;
    case GLUT_KEY_RIGHT:
        cameraAngleY += 0.5f;
        if (cameraAngleY > cameraMaxY) cameraAngleY = cameraMaxY;
        break;
    }
}

void timer(int v) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // Aproximativ 60 cadre pe secunda
}

void bouncing_balls() {
    if (width <= 0 || width >= height) {
        ballMove ^= 1;
    }

    if (ballMove) {
        width += 0.1;
    }
    else {
        width -= 0.1;
    }
    Sleep(10);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Peisaj de primavara cu Bile Saltarete");

    initGL();

    glutDisplayFunc(display);
    glutIdleFunc(bouncing_balls);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special_keyboard);
    glutReshapeFunc(reshape);

    glutTimerFunc(16, timer, 0);

    glutMainLoop();
}