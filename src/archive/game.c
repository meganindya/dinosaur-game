#include <GL/freeglut.h> // basic GLUT library
#include <stdlib.h>      // for dynamic memory allocation
#include <stdbool.h>     // for boolean variables
#include <time.h>        // for seed in random generation

#include <stdio.h> // generic terminal printing (for debugging)

#define WW 800 // Window Width
#define WH 600 // Window Height

// -----------------------------------------------------------------------------

// Structs for Image Objects

typedef struct figureObjects
{
    int **dino0;
    int **dino1;
    int **dino2;
} figObj;

typedef struct sceneObjects
{
    int **cacti;
    int **cloud;
    int **tree;
} scnObj;

// Function Declarations

/* -- Initialization Functions -- */

void init(void);
void convColors(void);
void loadImages(void);
int **loadData(FILE *);

/* -- Event Trigger Functions -- */

void keyPress(unsigned char, int, int);

/* -- Action Functions -- */

void loop(int); // Callback Loop (timed)

void updateDino(void);
void updateCacti(void);

void placeCacti(void);

void disp(void); // Display Function

/* -- Drawing Functions -- */

void drawScene(void);
void drawFigure(int, int, int, int, int);

// Drawing Utilities

void drawLine(int, int, int, int);
void drawRect(int, int, int, int);

// -----------------------------------------------------------------------------

// Global Variables

/* -- Environment Variables -- */

int refreshPeriod = 10, runtime = 0, scale = 4;
bool halt = false;

/* -- Colour Definitions (RGB) -- */

GLfloat cols[4][3] = {
    {0, 0, 0},       // Black
    {230, 230, 230}, // Grey Dark
    {85, 85, 85},    // Grey Light
    {25, 50, 128}    // Blue
};

/* -- Image Objects -- */

figObj fig; // Dinosaur Figures
scnObj scn; // Scenery	Figures

/* -- Control Variables for Dinosaur -- */

// Figure

int dinoState = 1, dinoX = 100, dinoY = WH / 4, dinoHS = 0;
int dinoHeights[10] = {0, 9, 16, 21, 24, 25, 24, 21, 16, 9};

// Motion

bool dinoJumpEnable = false;
int dinoPeriod = 60;

/* -- Control Variables for Cactus -- */

// Figure

int cactusWidth = 20, cactusX = -10, cactusY = WH / 4;
int cactusPos[5] = {WW * 2, WW * 2, WW * 2, WW * 2, WW * 2}, lastPushed;
int winLeftEdge, winRightEdge;

// Motion

int cactiPeriod = 10, gapDistance = 300, gapDelta, gapPeriod;

// -----------------------------------------------------------------------------

// Function Definitions

/* -- Main Function -- */

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WW, WH);
    glutInitWindowPosition(930, 0);
    glutCreateWindow("DOWNASAUR GAME");
    init();
    glutKeyboardFunc(keyPress);
    glutDisplayFunc(disp);
    glutTimerFunc(refreshPeriod, loop, 0);
    glutMainLoop();

    return 0;
}

/* -- Initialization Functions  -- */

void init(void)
{
    gluOrtho2D(0.0, WW, 0.0, WH);
    // glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    // glClear(GL_COLOR_BUFFER_BIT);

    /*glMatrixMode(GL_PROJECTION);
    glLoadIdentity();*/

    cactusWidth *= scale;
    winLeftEdge = 0 - (cactusWidth / 2);
    winRightEdge = WW + (cactusWidth / 2);
    gapPeriod = cactiPeriod * 75;

    convColors();
    loadImages();
}

void convColors(void)
{
    for (int i = 0; i < (sizeof(cols) / (sizeof(GLfloat) * 3)); i++)
        for (int j = 0; j < 3; j++)
            cols[i][j] /= 255;
}

void loadImages(void)
{
    char **fn = (char **)malloc(sizeof(char *) * 4);
    fn[0] = "dino0.txt";
    fn[1] = "dino1.txt";
    fn[2] = "dino2.txt";
    fn[3] = "cacti.txt"; // fn[5] = "cloud.txt";

    for (int i = 0; i < 4; i++)
    {
        FILE *fp = fopen(fn[i], "r");
        if (fp == NULL)
        {
            printf(" -- file \"%s\" missing\n", fn[i]);
            break;
            exit(0);
        }
        else
        {
            int **temp = loadData(fp);
            switch (i)
            {
            case 0:
                fig.dino0 = temp;
                break;
            case 1:
                fig.dino1 = temp;
                break;
            case 2:
                fig.dino2 = temp;
                break;
            case 3:
                scn.cacti = temp;
                break;
            }
        }
    }
}

int **loadData(FILE *fp)
{
    int xa, ya, xb, yb, c = 0;
    int arr[4][128];
    while (fscanf(fp, "%d", &xa) == 1)
    {
        fscanf(fp, "%d", &ya);
        fscanf(fp, "%d", &xb);
        fscanf(fp, "%d", &yb);
        arr[0][c] = xa;
        arr[1][c] = ya;
        arr[2][c] = xb;
        arr[3][c] = yb;
        c++;
    }

    int **temp;
    temp = (int **)malloc(sizeof(int *) * 4);
    for (int i = 0; i < 4; i++)
        temp[i] = (int *)malloc(sizeof(int) * c + 1);
    for (int i = 0; i < c; i++)
    {
        for (int j = 0; j < 4; j++)
            temp[j][i] = arr[j][i];
    }
    for (int i = 0; i < 4; i++)
        temp[i][c] = -1;

    return temp;
}

/* -- Event Trigger Functions -- */

void keyPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // Key Esc
        exit(0);
        break;
    case 32: // Key Space
        dinoJumpEnable = true;
        break;
    }
}

/* -- Action Functions -- */

// Callback Loop (timed)

void loop(int val)
{
    runtime += refreshPeriod;

    if (runtime % dinoPeriod == 0)
        updateDino();
    if (runtime % cactiPeriod == 0)
        updateCacti();

    if (halt == false)
        glutPostRedisplay();

    glutTimerFunc(refreshPeriod, loop, 0);
}

void updateDino(void)
{
    dinoState = 1 - dinoState;
    if (dinoJumpEnable == true)
    {
        dinoHS = ((dinoHS + 1) % 9);
        if (dinoHS == 0)
            dinoJumpEnable = false;
    }
}

void updateCacti(void)
{
    if (cactusPos[4] == winRightEdge)
        lastPushed = runtime;

    for (int i = 0; i < 5; i++)
        cactusPos[i] -= 5;

    if (runtime == (lastPushed + cactiPeriod * 75))
    {
        // printf("%d\n", runtime);
        for (int i = 0; i < 4; i++)
            cactusPos[i] = cactusPos[i + 1];
        cactusPos[4] = winRightEdge;
        // srand(time(0));
        // gapDelta = rand() % 100;
        // gapPeriod = (gapDistance + gapDelta) / 5;
        // printf("%d\t", gapPeriod);
        // gapPeriod -= gapPeriod % 10;
        // printf("%d\n", gapPeriod);
    }
}

void placeCacti(void)
{
    glColor3fv(cols[0]);
    for (int i = 0; i < 5; i++)
        if (cactusPos[i] < (WW + cactusWidth))
            drawFigure(3, cactusPos[i] + cactusX, cactusY, 0, scale);
}

// Display Function

void disp(void)
{
    drawScene();

    // Draw Cacti

    placeCacti();

    // Draw Dinosaur

    glColor3fv(cols[3]);
    drawFigure(dinoState, dinoX, dinoY, dinoHeights[dinoHS], scale);

    glutSwapBuffers();
}

// Drawing Functions

void drawScene(void)
{

    /* -- Background -- */

    glColor3fv(cols[1]);
    drawRect(0, 0, WW, WH);

    /* -- Ground Line -- */

    glColor3fv(cols[2]);
    glLineWidth((float)scale);
    drawLine(0, WH / 4, WW, WH / 4);
}

void drawFigure(int f, int xpos, int ypos, int yoff, int bsize)
{
    int xa, ya, xb, yb, i = 0, **ar;
    switch (f)
    {
    case 0:
        ar = fig.dino0;
        break;
    case 1:
        ar = fig.dino1;
        break;
    case 2:
        ar = fig.dino2;
        break;
    case 3:
        ar = scn.cacti;
        break;
    }

    while (ar[0][i] != -1 && ar[1][i] != -1 && ar[2][i] != -1 && ar[3][i++] != -1)
    {
        xa = ar[0][i] * bsize + xpos;
        ya = ar[1][i] * bsize + ypos + yoff * bsize;
        xb = ar[2][i] * bsize + xpos;
        yb = ar[3][i] * bsize + ypos + yoff * bsize;
        drawRect(xa, ya, xb, yb);
    }
}

/* -- Drawing Utilities -- */

void drawLine(int xa, int ya, int xb, int yb)
{
    glBegin(GL_LINES);
    glVertex2i(xa, ya);
    glVertex2i(xb, yb);
    glEnd();
}

void drawRect(int xa, int ya, int xb, int yb)
{
    glBegin(GL_POLYGON);
    glVertex2i(xa, ya);
    glVertex2i(xa, yb);
    glVertex2i(xb, yb);
    glVertex2i(xb, ya);
    glEnd();
}