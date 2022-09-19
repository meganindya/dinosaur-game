/*
	Anindya Kundu
	ID: 510817020
	IT Sem 4
*/

#include <GL/freeglut.h> // basic GLUT library
#include <stdlib.h>		 // for dynamic memory allocation
#include <stdbool.h>	 // for boolean variables
#include <time.h>		 // for seed in random generation
#include <string.h>		 // for text operation

#include <stdio.h> // generic terminal printing (for debugging)

#define WW 800 // Window	Width
#define WH 600 // Window	Height
#define DW 150 // Dinosaur	Width
#define DH 100 // Dinosaur	Height
#define CW 60  // Cactus	Width
#define CH 80  // Cactus	Height
#define LW 252 // Cloud	Width
#define LH 140 // Cloud	Height

// -----------------------------------------------------------------------------

// Struct for Image Objects

typedef struct figureObjects
{
	unsigned char **r, **g, **b;
} figure;

// -----------------------------------------------------------------------------

// Function Declarations

/* -- Initialization Functions -- */

void init(void);
void convColors(void);
void loadImages(void);
figure loadArray(FILE *, int, int);

/* -- Event Trigger Functions -- */

void keyPress(unsigned char, int, int);
void reset(void);

/* -- Action Functions -- */

void loop(int); // Callback Loop (timed)

void checkCollision(void);
void eventCollision(void);

void updateDino(void);
void updateCacti(void);

void placeCacti(void);

void disp(void); // Display Function

/* -- Drawing Functions -- */

void drawScene(void);
void drawFigure(int, int, int, int, int, int);

// Drawing Utilities

void drawLine(int, int, int, int);
void drawRect(int, int, int, int);
void drawText(void);
char *intToStr(int);

// -----------------------------------------------------------------------------

// Global Variables

/* -- Environment Variables -- */

int refreshPeriod = 5, runtime = 0, score = 0, hiscore;
bool halt = true, started = false;

/* -- Colour Definitions (RGB) -- */

GLfloat cols[7][3] = {
	{0, 0, 0},		 // Black
	{230, 230, 230}, // Grey Light
	{85, 85, 85},	 // Grey Dark
	{25, 50, 128},	 // Blue
	{25, 75, 25},	 // Green
	{76, 38, 10},	 // Brown
	{255, 255, 255}	 // White
};

/* -- Image Objects -- */

figure dino1, dino2, dino3, dino4, dino5, dino6; // Dinosaur
figure cacti, cloud;							 // Scenery

/* -- Control Variables for Dinosaur -- */

// Figure

int dinoState = 0, dinoX = 100, dinoY = WH / 4, dinoHS = 0;
int dinoHeights[20] = {
	0, 6, 12, 17, 21, 25, 28, 30, 32, 33,
	34, 33, 32, 30, 28, 25, 21, 17, 12, 6};
int dinoLeftEdge, dinoRightEdge;

// Motion

bool dinoJumpEnable = false;
int dinoPeriod = 20;

/* -- Control Variables for Cactus -- */

// Figure

int cactusOffset = WH / 4;
int cactiPos[5] = {WW * 2, WW * 2, WW * 2, WW * 2, WW * 2}, cactiLastPushed = -999;
int winLeftEdge, winRightEdge;

// Motion

int cactiPeriod = 5, cactiShift = 5;
int gapPeriodOrig = 800, gapDelta = 0, gapPeriod = 800;

/* -- Control Variables for Cloud -- */

// Figure

int cloudOffset = 13 * WH / 20, cloudOffsetDelta = 0;
int cloudPos = WW / 3, cloudLastPushed = -999;

// Motion

int cloudPeriod = 20, cloudShift = 2;

// -----------------------------------------------------------------------------

// Function Definitions

/* -- Main Function -- */

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WW, WH);
	glutInitWindowPosition(940, 0);
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

	dinoLeftEdge = 0 - (CW / 2) + dinoX;
	dinoRightEdge = DW + (CW / 2) + dinoX;
	winLeftEdge = 0 - (CW / 2);
	winRightEdge = WW + (CW / 2);
	gapPeriod = cactiPeriod * 100;

	convColors();
	loadImages();

	FILE *fp = fopen("res/hiscore", "r");
	fscanf(fp, "%d", &hiscore);
}

void convColors(void)
{
	for (int i = 0; i < (sizeof(cols) / (sizeof(GLfloat) * 3)); i++)
		for (int j = 0; j < 3; j++)
			cols[i][j] /= 255;
}

void loadImages(void)
{
	char **fn = (char **)malloc(sizeof(char *) * 8);
	fn[0] = "res/dino1.pbm";
	fn[1] = "res/dino2.pbm";
	fn[2] = "res/dino3.pbm";
	fn[3] = "res/dino4.pbm";
	fn[4] = "res/dino5.pbm";
	fn[5] = "res/dino6.pbm";
	fn[6] = "res/cacti.pbm";
	fn[7] = "res/cloud.pbm";

	for (int i = 0; i < 8; i++)
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
			switch (i)
			{
			case 0:
				dino1 = loadArray(fp, DW, DH);
				break;
			case 1:
				dino2 = loadArray(fp, DW, DH);
				break;
			case 2:
				dino3 = loadArray(fp, DW, DH);
				break;
			case 3:
				dino4 = loadArray(fp, DW, DH);
				break;
			case 4:
				dino5 = loadArray(fp, DW, DH);
				break;
			case 5:
				dino6 = loadArray(fp, DW, DH);
				break;
			case 6:
				cacti = loadArray(fp, CW, CH);
				break;
			case 7:
				cloud = loadArray(fp, LW, LH);
				break;
			}
		}
	}
}

figure loadArray(FILE *fp, int w, int h)
{
	unsigned char **r, **g, **b;
	r = malloc(sizeof(unsigned char *) * h);
	g = malloc(sizeof(unsigned char *) * h);
	b = malloc(sizeof(unsigned char *) * h);
	for (int i = 0; i < h; i++)
	{
		r[i] = malloc(sizeof(unsigned char) * w);
		g[i] = malloc(sizeof(unsigned char) * w);
		b[i] = malloc(sizeof(unsigned char) * w);
	}

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			r[h - i - 1][w - j - 1] = fgetc(fp);
			g[h - i - 1][w - j - 1] = fgetc(fp);
			b[h - i - 1][w - j - 1] = fgetc(fp);
		}
	}

	figure img;
	img.r = r;
	img.g = g;
	img.b = b;
	return img;
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
		if (halt == true)
			reset();
		else
			dinoJumpEnable = true;
		break;
	case 112: // Key 'p'
		halt = !halt;
		break;
	}
}

void reset(void)
{
	runtime = 0;
	halt = false;
	started = true;
	score = 0;
	dinoState = 0;
	dinoHS = 0;
	cactiPos[0] = cactiPos[1] = cactiPos[2] = cactiPos[3] = cactiPos[4] = WW * 2;
	cactiLastPushed = -999;
}

/* -- Action Functions -- */

// Callback Loop (timed)

void loop(int val)
{
	runtime += refreshPeriod;

	if (halt == true)
		goto skip;

	if (runtime % dinoPeriod == 0)
		updateDino();
	if (runtime % cactiPeriod == 0)
		updateCacti();
	checkCollision();

	if (runtime % (10 * cactiPeriod) == 0)
		score++;

	glutPostRedisplay();

skip:
	glutTimerFunc(refreshPeriod, loop, 0);
}

void checkCollision(void)
{
	for (int i = 0; i < 5; i++)
		if (cactiPos[i] > dinoLeftEdge && cactiPos[i] < dinoRightEdge)
		{
			if (cactiPos[i] > dinoX + 2 * DW / 5 && cactiPos[i] < dinoX + 3 * DW / 5)
			{
				if (dinoHeights[dinoHS] * 4 < CH)
				{
					eventCollision();
					break;
				}
			}
			else
			{
				if (dinoHeights[dinoHS] * 4 < CH - (13 * DH / 20))
				{
					eventCollision();
					break;
				}
			}
		}
}

void eventCollision(void)
{
	halt = true;
	if (score <= hiscore)
		return;
	FILE *fp = fopen("res/hiscore", "w");
	fprintf(fp, "%d", score);
	fclose(fp);
}

void updateDino(void)
{
	dinoState = ((dinoState + 1) % 6);
	if (dinoJumpEnable == true)
	{
		dinoHS = ((dinoHS + 1) % 19);
		if (dinoHS == 0)
			dinoJumpEnable = false;
	}
}

void updateCacti(void)
{
	if (cactiPos[4] == winRightEdge)
		cactiLastPushed = runtime;

	for (int i = 0; i < 5; i++)
		cactiPos[i] -= cactiShift;

	if (runtime == (cactiLastPushed + gapPeriod))
	{
		for (int i = 0; i < 4; i++)
			cactiPos[i] = cactiPos[i + 1];
		cactiPos[4] = winRightEdge;
		srand(time(0));
		gapDelta = rand() % 70;
		gapPeriod = gapPeriodOrig + gapDelta * 10;
	}
}

// Display Function

void disp(void)
{
	drawScene();

	// Draw Dinosaur

	drawFigure(dinoState, dinoX, dinoY, dinoHeights[dinoHS] * 4, DW, DH);

	// Draw Cacti

	placeCacti();

	// Draw Vvisible Text Content

	drawText();

	glutSwapBuffers();
}

/* -- Drawing Functions -- */

void drawScene(void)
{

	/* -- Background -- */

	glColor3fv(cols[1]);
	drawRect(0, 0, WW, WH);

	/* -- Clouds -- */

	if (runtime % cloudPeriod == 0)
	{
		if (cloudPos >= -(LW / 2) && cloudPos <= WW + (LW / 2))
			cloudPos -= cloudShift;
		else
		{
			cloudPos = WW + (LW / 2);
			srand(time(0));
			cloudOffsetDelta = (rand() % (WH / 40)) * 10;
		}
	}
	drawFigure(7, cloudPos - (LW / 2), cloudOffset + cloudOffsetDelta, 0, LW, LH);

	/* -- Ground -- */

	glColor3fv(cols[5]);
	drawRect(0, 0, WW, WH / 4);
}

void drawFigure(int f, int xpos, int ypos, int yoff, int w, int h)
{
	figure temp;
	if (yoff > 0)
	{
		temp = dino4;
		goto skip;
	}
	switch (f)
	{
	case 0:
		temp = dino1;
		break;
	case 1:
		temp = dino2;
		break;
	case 2:
		temp = dino3;
		break;
	case 3:
		temp = dino4;
		break;
	case 4:
		temp = dino5;
		break;
	case 5:
		temp = dino6;
		break;
	case 6:
		temp = cacti;
		break;
	case 7:
		temp = cloud;
		break;
	}

skip:
	glBegin(GL_POINTS);
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			glColor3ub(temp.r[i][j], temp.g[i][j], temp.b[i][j]);
			glVertex2f(xpos + j, ypos + yoff + i);
		}
	}
	glEnd();
}

void placeCacti(void)
{
	for (int i = 0; i < 5; i++)
	{
		if (cactiPos[i] > winLeftEdge && cactiPos[i] < winRightEdge)
			drawFigure(6, cactiPos[i] - (CW / 2), cactusOffset, 0, CW, CH);
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

void drawText(void)
{
	glColor3fv(cols[3]);
	glRasterPos2f(3 * WW / 4, 7 * WH / 8);
	unsigned char points[] = "Score : ";
	strcat(points, intToStr(score));
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, points);

	if (score > hiscore)
		hiscore = score;
	glRasterPos2f(3 * WW / 4, 13 * WH / 16);
	unsigned char hipoint[] = "Hi-Score : ";
	strcat(hipoint, intToStr(hiscore));
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, hipoint);

	glColor3fv(cols[1]);
	glRasterPos2f(11 * WW / 28, WH / 10);
	unsigned char pause[] = "Press P for Play/Pause";
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, pause);

	if (halt == false)
		return;

	glColor3fv(cols[4]);
	glRasterPos2f(23 * WW / 56, WH / 2);
	unsigned char success[] = "Press Spacebar";
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, success);
}

char *intToStr(int n)
{
	if (n == 0)
		return "0\0";

	char s[8];
	int k = 0;
	while (n)
	{
		s[k++] = (n % 10) + '0';
		n /= 10;
	}
	char *tem = (char *)malloc(sizeof(char) * k + 1);
	for (int i = 0; i < k; i++)
		tem[i] = s[k - i - 1];
	tem[k] = '\0';

	return tem;
}
