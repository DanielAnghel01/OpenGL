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

// ================= SISTEM DE PARTICULE PENTRU ZAPADA =================
#define MAX_SNOWFLAKES 1500 // Numarul de fulgi de zapada

struct Snowflake {
    float x, y, z;
    float speed;
    float drift; // Pentru a simula bataia vantului
};

Snowflake snow[MAX_SNOWFLAKES];

// Initializarea fulgilor in pozitii aleatoare
void initSnow() {
    for (int i = 0; i < MAX_SNOWFLAKES; i++) {
        snow[i].x = ((float)rand() / RAND_MAX) * 50.0f - 25.0f; // intre -25 si 25
        snow[i].y = ((float)rand() / RAND_MAX) * 25.0f + 5.0f;  // cad de sus
        snow[i].z = ((float)rand() / RAND_MAX) * 50.0f - 25.0f; // intre -25 si 25
        snow[i].speed = ((float)rand() / RAND_MAX) * 0.05f + 0.02f; // viteza diferita per fulg
        snow[i].drift = ((float)rand() / RAND_MAX) * 0.02f - 0.01f;
    }
}

// Desenarea zapezii pe ecran
void drawSnow() {
    glDisable(GL_LIGHTING); // Zapada e alba pur, nu vrem umbre pe ea
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(2.5f); // Dimensiunea unui fulg

    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_SNOWFLAKES; i++) {
        glVertex3f(snow[i].x, snow[i].y, snow[i].z);
    }
    glEnd();
}

// Miscarea zapezii in jos (apelata in timer)
void updateSnow() {
    for (int i = 0; i < MAX_SNOWFLAKES; i++) {
        snow[i].y -= snow[i].speed;
        snow[i].x += snow[i].drift + sin(snow[i].y) * 0.005f; // Un mic efect de miscare stanga-dreapta

        // Daca fulgul atinge pamantul, il readucem sus
        if (snow[i].y < 0.0f) {
            snow[i].y = 25.0f;
            snow[i].x = ((float)rand() / RAND_MAX) * 50.0f - 25.0f;
            snow[i].z = ((float)rand() / RAND_MAX) * 50.0f - 25.0f;
        }
    }
}
// =====================================================================

void initGL() {
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    GLfloat lightPos[] = { 10.0f, 20.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    initSnow(); // Gneram zapada la pornire
}

void drawMountain(float x, float z, float width, float height, float depth) {
    glBegin(GL_TRIANGLES);

    // FATA STANGA (Luminata de "soare")
    glColor3f(0.5f, 0.5f, 0.5f); // Baza stanga (gri mediu)
    glVertex3f(x - width, 0, z);
    glColor3f(0.6f, 0.6f, 0.6f); // Baza din fata (varful scos in afara spre camera)
    glVertex3f(x, 0, z + depth);
    glColor3f(1.0f, 1.0f, 1.0f); // Varf alb (zapada luminata)
    glVertex3f(x, height, z);

    // FATA DREAPTA (Aflata in umbra)
    glColor3f(0.4f, 0.4f, 0.4f); // Baza din fata (aceeasi cu cea de sus)
    glVertex3f(x, 0, z + depth);
    glColor3f(0.3f, 0.3f, 0.3f); // Baza dreapta (gri intunecat)
    glVertex3f(x + width, 0, z);
    glColor3f(0.75f, 0.75f, 0.8f); // Varf alb-albastrui (zapada in umbra)
    glVertex3f(x, height, z);

    glEnd();
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
    glColor3f(0.6f, 0.8f, 1.0f);
    glVertex3f(-25, 0, -25);
    glVertex3f(25, 0, -25);
    glColor3f(0.1f, 0.3f, 0.8f);
    glVertex3f(25, 25, -25);
    glVertex3f(-25, 25, -25);

    glColor3f(0.6f, 0.8f, 1.0f);
    glVertex3f(-25, 0, 25);
    glVertex3f(-25, 0, -25);
    glColor3f(0.1f, 0.3f, 0.8f);
    glVertex3f(-25, 25, -25);
    glVertex3f(-25, 25, 25);

    glColor3f(0.6f, 0.8f, 1.0f);
    glVertex3f(25, 0, -25);
    glVertex3f(25, 0, 25);
    glColor3f(0.1f, 0.3f, 0.8f);
    glVertex3f(25, 25, 25);
    glVertex3f(25, 25, -25);
    glEnd();

    // ================= MUNTII DE PE PERETELE DIN SPATE =================
     // Muntele din stanga
    drawMountain(-14.0f, -24.0f, 12.0f, 14.0f, 3.0f);
    // Muntele din dreapta 
    drawMountain(12.0f, -23.0f, 14.0f, 15.0f, 4.0f);
    // Muntele central 
    drawMountain(0.0f, -22.0f, 16.0f, 20.0f, 7.0f);


    // ================= MUNTII DE PE PERETELE DIN STANGA =================
    glPushMatrix(); // Salvam coordonatele actuale ale lumii

    // Rotim lumea cu 90 de grade in jurul axei Y (axa verticala)
    // Astfel, axa Z (peretele din spate) devine axa X (peretele din stanga)
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

    // Acum folosim aceeasi functie drawMountain.
    // Datorita rotatiei, cand noi ii spunem sa deseneze la Z = -24.5, 
    // OpenGL va desena de fapt pe axa X la -24.5 (adica fix pe peretele din stanga!)

    // Munte perete din stanga
    drawMountain(15.0f, -24.8f, 13.0f, 16.0f, 3.0f);

    // Munte situat la mijlocul peretelui din stanga
    drawMountain(0.0f, -24.5f, 15.0f, 19.0f, 4.0f);

    // Munte stang ppe peretele din stanga
    drawMountain(-12.0f, -24.0f, 10.0f, 12.0f, 6.0f);

    glPopMatrix();

    // ================= PAMANT =================
    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.2f, 0.1f);
    glVertex3f(-25, -0.1f, -25);
    glVertex3f(-25, -0.1f, 25);
    glVertex3f(25, -0.1f, 25);
    glVertex3f(25, -0.1f, -25);
    glEnd();

    // ================= IARBA/TEREN DENIVELAT =================
    glBegin(GL_TRIANGLES);
    for (float x = -25; x < 25; x += 0.5) {
        for (float z = -25; z < 25; z += 0.5) {
            float y1 = sin(x) * cos(z) * 0.4f;
            float y2 = sin(x + 0.5) * cos(z) * 0.4f;
            float y3 = sin(x) * cos(z + 0.5) * 0.4f;
            float y4 = sin(x + 0.5) * cos(z + 0.5) * 0.4f;

            glColor3f(0.1f, 0.6f + (y1 * 0.2f), 0.2f); // Iarba ceva mai rece/brumata

            glVertex3f(x, y1, z);
            glVertex3f(x + 0.5, y2, z);
            glVertex3f(x, y3, z + 0.5);

            glVertex3f(x + 0.5, y2, z);
            glVertex3f(x + 0.5, y4, z + 0.5);
            glVertex3f(x, y3, z + 0.5);
        }
    }
    glEnd();

    // Apelam functia care deseneaza fulgii de zapada
    drawSnow();

    glEnable(GL_LIGHTING);

    // ================= BILELE =================
    glPushMatrix();
    glTranslatef(7, width + 1.5, 4);
    glColor3f(1, 1, 0); // Galben
    glutSolidSphere(1.9f, 50, 50);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(7, width + 1, 9);
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
    updateSnow(); // Actualizam pozitia zapezii de fiecare data cand se redeseneaza cadrul
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
    glutCreateWindow("Peisaj de Iarna cu Bile Saltarete");

    initGL();

    glutDisplayFunc(display);
    glutIdleFunc(bouncing_balls);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special_keyboard);
    glutReshapeFunc(reshape);

    glutTimerFunc(16, timer, 0);

    glutMainLoop();
}