#ifdef _WIN32
#include <iostream>
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif
#include "gsub.h"
#include "video.h"
#include "param.h"
#include "ar.h"

using namespace std;
//
// Camera configuration.
//
#ifdef _WIN32
char		*vconf = "Data\\WDM_camera_flipV.xml";
#else
char		*vconf = "";
#endif

int			xsize, ysize;
int			thresh = 100;
int			count = 0;

char		*cparam_name    = "Data/camera_para.dat";
ARParam		cparam;

char		*patt_name      = "Data/patt.hiro";
int			patt_id;
double		patt_width     = 80.0;
double		patt_center[2] = {0.0, 0.0};
double		patt_trans[3][4];

static void   init(void);
static void   cleanup(void);
static void	  mousePlot(int, int, int, int);
static void   keyEvent( unsigned char key, int x, int y);
static void   mainLoop(void);
static void   draw( void );
void initTransformation();
void initViewParam();
void printError (char*);
void openGLCleanup();
void saveTransformation();

#define BOUNDARY 300 

struct POINTS_3D {
	GLfloat x, y, z;
};

struct COLOR {
	GLfloat red, green, blue;
};

// Set 3D viewing variables
struct VIEWING {
	struct POINTS_3D view;		// Original viewing-coordinate origin.
	struct POINTS_3D ref;		// original look-at point
	struct POINTS_3D viewUp;	// view up vector
	GLfloat xwMin, xwMax, ywMin, ywMax, zwMin, zwMax; // Set coordinate limits for the clipping window, only allow multiples of 50
	GLfloat dnear, dfar;	// Set positions for near and far clipping planes
};

VIEWING viewParam;

struct GUIKEYBOARD { 	// Set keyboard interactive variables
	GLfloat xDir;			// rotation direction about x-axis
	GLfloat xAngle;			// rotation angle about x-axis
	GLfloat yDir;			// rotation direction about y-aixs
	GLfloat yAngle;			// rotation angle about y-axis
	GLfloat zDir;			// rotation direction about z-aixs
	GLfloat zAngle;			// rotation angle about z-axis
	GLfloat scaleFactor;	// Scaling Factor
	GLfloat incVal;			// the incremental value of user interaction
	GLfloat incValScale;	// the incremental value for scaling
};

GUIKEYBOARD keyboard;

// Set drawing variables
GLfloat red, green, blue;
int   i, j;

//-------------- object
struct OBJPOINTS {
	GLfloat x, y, z;
	OBJPOINTS *prev, *next;
};


struct OBJECT {
	int mode;
	COLOR color;
	OBJPOINTS *points;
	OBJECT *prev, *next;
};

OBJECT *objData;

//-------------- transformation structure
#define MAXTRANSFORMATION 10
#define TOL 0.0001

int transIndex = 0;
enum TRANSMODE { translation, rotation, scaling };
enum STATUS { running, paused, stopped }; 


struct TRANSFORMATION {
	int index;
	COLOR color;
	TRANSMODE mode;
	GLfloat x, y, z;
	GLfloat angle;
	TRANSFORMATION *prev, *next;
}; 

struct ANIMATE {
	bool x, y, z;
};

enum BUTTONS {
	start,		// 0, animate the transformation till the end, non-stop. Used for restarting also after pause
	stop,		// 1, unused
	pause,		// 2, pause the animation, press start/nextStep/nextTransf continue the animation from where it paused
	nextStep,	// 3, pause at every stage of transformation, including pivot point transformation
	prevStep,	// 4, goes back to prev transformation, incl. pivot point transf, reverse of nextStep
	nextTransf,	// 5, pause at every stage of transformation, excluding pivot point
	prevTransf,	// 6, goes back to prev transformation, excl. pivot point transf, reverse of nextTransf
	resetTransf,// 7, reset the animation to initial stage, i.e. without transformation, viewParam remain
	resetView,	// 8, reset the viewParam to initial stage, object transformation remain

	none		// 9, no button pressed yet
};

struct GUIPANEL { 
	BUTTONS btnStatus; 
	int slower, faster;			// setting the animation slower or faster by changing the value of GUIPANEL.speedStep
	bool axis,					
		ar,						// inidicate if the display is AR or VR
		gridOriObj,				// grid for the original object
		gridAllTrans,			// grid for all transformation
		gridX, gridY, gridZ;	// grids parallel to X, Y and Z axis
	STATUS progStatus;	
	float speedStep;			// speed of the animation, higher value shows slower speed
	float gridXSpace, gridYSpace, gridZSpace; // grid space, only allow multiples of 50
	
};

TRANSFORMATION *transData[MAXTRANSFORMATION]; // main transformation data
TRANSFORMATION *tmpTrans = new TRANSFORMATION; // temporary transformation used for animation
TRANSFORMATION *curTrans; // used to navigate through the pivot transformation
int numTransData = 0;
int curTransDataIndex;

GUIPANEL panel;


/******************* FUNCTION DEFINITIONS *************************/


void draw_triangle(COLOR color)
{
	glLineWidth(2);
	glColor3f (color.red, color.green, color.blue);
	
	/*glBegin (GL_TRIANGLES);
		glVertex3f( 30.0,  50.0,  50.0);
		glVertex3f(100.0, 100.0,  0.0);
		glVertex3f( 10.0, 140.0,  -50.0);
	glEnd(); */

	glBegin (GL_TRIANGLES);
		glVertex3f(  0.0,   0.0,  50.0);
		glVertex3f( 50.0,   0.0,  50.0);
		glVertex3f( 25.0,  50.0,  50.0);
	glEnd();

}


void draw_square(GLfloat r, GLfloat g, GLfloat b)
{
	glLineWidth(2);
	glColor3f (r, g, b);
	glBegin (GL_QUADS);
		glVertex3f(  0.0,   0.0,  50.0);
		glVertex3f( 50.0,   0.0,  50.0);
		glVertex3f( 50.0,  50.0,  50.0);
		glVertex3f(  0.0,  50.0,  50.0);
	glEnd();
}

void draw_cube(void)
{
	float xmin=3, xmax=5, ymin=2, ymax=4;

	glBegin (GL_LINES);
		glVertex3f(xmin*5, ymin*5, 0); glVertex3f( xmin*5,  ymin*5, 2.0*5);
		glVertex3f(xmin*5, ymax*5, 0); glVertex3f( xmin*5,  ymax*5, 2.0*5);
		glVertex3f(xmax*5, ymin*5, 0); glVertex3f( xmax*5,  ymin*5, 2.0*5);
		glVertex3f(xmax*5, ymax*5, 0); glVertex3f( xmax*5,  ymax*5, 2.0*5);

		glVertex3f(xmin*5, ymin*5, 0); glVertex3f( xmax*5,  ymin*5, 0);
		glVertex3f(xmin*5, ymax*5, 0); glVertex3f( xmax*5,  ymax*5, 0);
		glVertex3f(xmin*5, ymin*5, 0); glVertex3f( xmin*5,  ymax*5, 0);
		glVertex3f(xmax*5, ymin*5, 0); glVertex3f( xmax*5,  ymax*5, 0);
				
		glVertex3f(xmin*5, ymin*5, 2.0*5); glVertex3f( xmax*5,  ymin*5, 2.0*5);
		glVertex3f(xmin*5, ymax*5, 2.0*5); glVertex3f( xmax*5,  ymax*5, 2.0*5);
		glVertex3f(xmin*5, ymin*5, 2.0*5); glVertex3f( xmin*5,  ymax*5, 2.0*5);
		glVertex3f(xmax*5, ymin*5, 2.0*5); glVertex3f( xmax*5,  ymax*5, 2.0*5);
	glEnd();
	
	// Flush drawing commands
	glFlush();
}

void draw_grid (COLOR color)
{
	int i, j, k;
	float xframe = panel.gridXSpace;
	float yframe = panel.gridYSpace;
	float zframe = panel.gridZSpace;
	
	//if (!panel.gridAllTrans) return;

	glEnable(GL_LINE_STIPPLE);
	glLineStipple (1, 0xFFFF);
	glLineWidth(1);
	glColor3f(color.red, color.green, color.blue);
	glBegin (GL_LINES);
		
	//------------ draw positive lines begins -----------------
		// draw lines parallel to x-axis
		if (panel.gridX) {
			for (k=0; k<=viewParam.zwMax-panel.gridZSpace; k+=panel.gridZSpace) { // for +tive z region
				for (j=0; j<=viewParam.ywMax-panel.gridYSpace; j+=panel.gridYSpace) { // draw +tive lines
					glVertex3i(viewParam.xwMin+xframe, j, k);
					glVertex3i(viewParam.xwMax-xframe, j, k);
				}
				for (j=0; j>=viewParam.ywMin+panel.gridYSpace; j-=panel.gridYSpace) { // draw -tive lines
					glVertex3i(viewParam.xwMin+xframe, j, k);
					glVertex3i(viewParam.xwMax-xframe, j, k);
				}
			}

			for (k=0; k>=viewParam.zwMin+panel.gridZSpace; k-=panel.gridZSpace) { // for -tive z region
				for (j=0; j<=viewParam.ywMax-panel.gridYSpace; j+=panel.gridYSpace) { // draw +tive lines
					glVertex3i(viewParam.xwMin+xframe, j, k);
					glVertex3i(viewParam.xwMax-xframe, j, k);
				}
				for (j=0; j>=viewParam.ywMin+panel.gridYSpace; j-=panel.gridYSpace) { // draw -tive lines
					glVertex3i(viewParam.xwMin+xframe, j, k);
					glVertex3i(viewParam.xwMax-xframe, j, k);
				}
			}

		} // if (panel.gridX) 

		// draw lines parallel to y-axis
		if (panel.gridY) {
			for (k=0; k<=viewParam.zwMax-panel.gridZSpace; k+=panel.gridZSpace) { // draw +tive z region
				for (i=0; i<=viewParam.xwMax-panel.gridXSpace; i+=panel.gridXSpace) { // draw +tive lines
					glVertex3i(i, viewParam.ywMin+yframe, k);
					glVertex3i(i, viewParam.ywMax-yframe, k);
				}
				for (i=0; i>=viewParam.xwMin+panel.gridXSpace; i-=panel.gridXSpace) { // draw -tive lines
					glVertex3i(i, viewParam.ywMin+yframe, k);
					glVertex3i(i, viewParam.ywMax-yframe, k);
				}
			}
			for (k=0; k>=viewParam.zwMin+panel.gridZSpace; k-=panel.gridZSpace) { // draw -tive z region
				for (i=0; i<=viewParam.xwMax-panel.gridXSpace; i+=panel.gridXSpace) { // draw +tive lines
					glVertex3i(i, viewParam.ywMin+yframe, k);
					glVertex3i(i, viewParam.ywMax-yframe, k);
				}
				for (i=0; i>=viewParam.xwMin+panel.gridXSpace; i-=panel.gridXSpace) { // draw -tive lines
					glVertex3i(i, viewParam.ywMin+yframe, k);
					glVertex3i(i, viewParam.ywMax-yframe, k);
				}
			}
	
		} // if (panel.gridY)

		// draw lines parallel to z-axis
		if (panel.gridZ) {
			for (j=0; j<=viewParam.ywMax-panel.gridYSpace; j+=panel.gridYSpace) { // draw +tive y region
				for (i=0; i<=viewParam.xwMax-panel.gridXSpace; i+=panel.gridXSpace) { // draw +tive lines
					glVertex3i(i, j, viewParam.zwMin+zframe);
					glVertex3i(i, j, viewParam.zwMax-zframe);
				}
				for (i=0; i>=viewParam.xwMin+panel.gridXSpace; i-=panel.gridXSpace) { // draw -tive lines
					glVertex3i(i, j, viewParam.zwMin+zframe);
					glVertex3i(i, j, viewParam.zwMax-zframe);
				}
			}

			for (j=0; j>=viewParam.ywMin+panel.gridYSpace; j-=panel.gridYSpace) { // draw -tive y region
				for (i=0; i<=viewParam.xwMax-panel.gridXSpace; i+=panel.gridXSpace) { // draw +tive lines
					glVertex3i(i, j, viewParam.zwMin+zframe);
					glVertex3i(i, j, viewParam.zwMax-zframe);
				}
				for (i=0; i>=viewParam.xwMin+panel.gridXSpace; i-=panel.gridXSpace) { // draw -tive lines
					glVertex3i(i, j, viewParam.zwMin+zframe);
					glVertex3i(i, j, viewParam.zwMax-zframe);
				}
			}
		} // if (panel.gridZ)

	glEnd();

	glDisable(GL_LINE_STIPPLE);
}

void drawAxis(COLOR color)
{
	if (!panel.axis) return;

	glLineWidth(4);
	glColor3f(color.red, color.green, color.blue);
	glBegin(GL_LINES);
		glVertex3f(-100.0,    0.0,    0.0); // x-axis
		glVertex3f( 100.0,    0.0,    0.0); // x-axis
		glVertex3f(   0.0, -100.0,    0.0); // y-axis
		glVertex3f(   0.0,  100.0,    0.0); // y-axis
		glVertex3f(   0.0,    0.0, -100.0); // z-axis
		glVertex3f(   0.0,    0.0,  100.0); // z-axis

		// arrow for x-axis
		glVertex3f( 90.0,  10.0, 0.0);
		glVertex3f(100.0,   0.0, 0.0);
		glVertex3f(100.0,   0.0, 0.0);
		glVertex3f( 90.0, -10.0, 0.0); 

		// arrow for y-axis
		glVertex3f( 10.0,  90.0, 0.0);
		glVertex3f(  0.0, 100.0, 0.0);
		glVertex3f(  0.0, 100.0, 0.0);
		glVertex3f(-10.0,  90.0, 0.0);

		// arrow for z-axis
		glVertex3f( 0.0,  10.0,  90.0);
		glVertex3f( 0.0,   0.0, 100.0);
		glVertex3f( 0.0,   0.0, 100.0);
		glVertex3f( 0.0, -10.0,  90.0);
	glEnd();

	glRasterPos3i(105, 0, 0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 'x');
	glRasterPos3i(0, 105, 0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 'y');
	glRasterPos3i(0, 0, 105);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 'z');
}

//---------------- transformation functions starts ------------
void transformObj (TRANSFORMATION *trans)
{
	if (!trans) printError ("Error in transformObj()");

	// previous transformation
	//if ((curTransDataIndex != 0) && (trans->index == 0)) { 
	if (curTransDataIndex != 0) { 
		for (int i=0; i<=curTransDataIndex-1; i++) {
			TRANSFORMATION *trans = transData[i];
			while (trans) {
				switch (trans->mode) {
					case translation	:
						glTranslatef (trans->x, trans->y, trans->z);
						break;
					case rotation		:
						glRotatef(trans->angle, trans->x, trans->y, trans->z);
						break;
					case scaling		:
						glScalef(trans->x, trans->y, trans->z);
						break;
				} // switch (trans->mode)
				trans = trans->next;
			} // while (trans)
		} //  for (int i=0; i<curTransData-1; i++)
	} // if ((curTransDataIndex != 0) && (trans->index == 0))

	// transformation pointed to by curTrans
	switch (trans->mode) {
		case translation	:
			if (trans->prev) {
				glTranslatef(trans->prev->prev->x, trans->prev->prev->y, trans->prev->prev->z);
				if (trans->prev->mode == rotation) glRotatef(trans->prev->angle, trans->prev->x, trans->prev->y, trans->prev->z);
				else if (trans->prev->mode == scaling) glScalef(trans->prev->x, trans->prev->y, trans->prev->z);
			}
			glTranslatef (trans->x, trans->y, trans->z);
			break;

		case rotation		:
			if (trans->prev) glTranslatef(trans->prev->x, trans->prev->y, trans->prev->z);
			glRotatef(trans->angle, trans->x, trans->y, trans->z);
			break;

		case scaling		:
			if (trans->prev) glTranslatef(trans->prev->x, trans->prev->y, trans->prev->z);
			glScalef(trans->x, trans->y, trans->z);
			break;
	} // switch (trans->mode
}

TRANSFORMATION *writeTransTable (int index, TRANSMODE mode, GLfloat x, GLfloat y, GLfloat z, GLfloat angle, GLfloat Px, GLfloat Py, GLfloat Pz, GLfloat r, GLfloat g, GLfloat b)
{
	TRANSFORMATION *head = NULL, *p;


	switch (mode) {
		case translation	:
			p = new TRANSFORMATION;
			if (!p) printError("Error in writeTransTable()");
			p->index = 0;
			p->mode = translation;
			p->x = x;
			p->y = y;
			p->z = z;
			p->angle = 0;
			p->prev = p->next = NULL;
			head = p;
			p->color.red = r;
			p->color.green = g;
			p->color.blue = b;
			break;

		case rotation		:
		case scaling		:
			// if no pivot point
			if ((fabs(Px)<TOL) && (fabs(Py)<TOL) && (fabs(Pz)<TOL)) {
				p = new TRANSFORMATION;
				if (!p) printError("Error in writeTransTable()");
				p->index = 0;
				p->mode = mode;
				p->x = x;
				p->y = y;
				p->z = z;
				p->angle = angle;
				p->prev = p->next = NULL;
				head = p;
				p->color.red = r;
				p->color.green = g;
				p->color.blue = b;

			}
			else {
				p = new TRANSFORMATION;
				if (!p) printError("Error in writeTransTable()");
				p->index = 0;
				p->mode = translation;
				p->x = Px;
				p->y = Py;
				p->z = Pz;
				p->angle = 0;
				p->prev = p->next = NULL;
				head = p;
				p->color.red = r;
				p->color.green = g;
				p->color.blue = b;


				p = new TRANSFORMATION;
				if (!p) printError("Error in writeTransTable()");
				p->index = 1;
				p->mode = mode;
				p->x = x;
				p->y = y;
				p->z = z;
				if (p->mode == rotation) p->angle = angle;
				else p->angle = 0;
				head->next = p;
				p->prev = head;
				p->next = NULL;
				p->color.red = r;
				p->color.green = g;
				p->color.blue = b;

				p = new TRANSFORMATION;
				if (!p) printError("Error in writeTransTable()");
				p->index = 2;
				p->mode = translation;
				p->x = -Px;
				p->y = -Py;
				p->z = -Pz;
				p->angle = 0;
				p->prev = head->next;
				p->next = NULL;
				p->prev->next = p;
				p->color.red = r;
				p->color.green = g;
				p->color.blue = b;
			} 
			break;
	} // 	switch (mode)
	return head;
}


void copyTransData (TRANSFORMATION *source, TRANSFORMATION *target)
{
	if (!source || !target) printError("Error in copyTransData()");

	curTrans = source; // set current pointer to point to the source
	target->mode = source->mode;
	target->index = source->index;
	target->color.red = source->color.red;
	target->color.green = source->color.green;
	target->color.blue = source->color.blue;
	target->prev = source->prev;
	target->next = source->next;
	switch (source->mode) {
		case translation	:
			target->x = 0;	// will be changed during animation
			target->y = 0;	// will be changed during animation
			target->z = 0;	// will be changed during animation
			target->angle = 0;	// unuse
			break;

		case rotation		:
			target->x = source->x;
			target->y = source->y;
			target->z = source->z;
			target->angle = 0;	// will be changed during animation
			break;

		case scaling		:
			target->x = 1;	// will be changed during animation
			target->y = 1;	// will be changed during animation
			target->z = 1;	// will be changed during animation
			target->angle = 0;	// unuse
			break;
	}
}
// ----------------------- transformation functions end



void display(void)
{
	COLOR color;

	if (!panel.ar) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
		glLoadIdentity (); 
	}

	gluLookAt (viewParam.view.x, viewParam.view.y, viewParam.view.z, viewParam.ref.x, viewParam.ref.y, viewParam.ref.z, viewParam.viewUp.x, viewParam.viewUp.y, viewParam.viewUp.z);

	color.red = 1.0, color.green = 0.0, color.blue = 0.0;

	glPushMatrix();
		glRotatef(keyboard.xAngle, 1, 0, 0);
		glRotatef(keyboard.yAngle, 0, 1, 0);
		glRotatef(keyboard.zAngle, 0, 0, 1);
		glScalef(keyboard.scaleFactor, keyboard.scaleFactor, keyboard.scaleFactor);
		if (panel.gridAllTrans || panel.gridOriObj)  draw_grid(color);
		drawAxis(color);
		draw_triangle (color);  
		//draw_square (red, green, blue);
		
		if (panel.progStatus == running) {
		
			if ((panel.btnStatus==prevStep) || (panel.btnStatus==prevTransf)) transformObj (curTrans);
			else transformObj (tmpTrans);
			if (panel.gridAllTrans) draw_grid(curTrans->color);
			drawAxis(curTrans->color);
			draw_triangle (curTrans->color); 
		
		}

	glPopMatrix();
		 
	if (!panel.ar) glutSwapBuffers(); 
}

void idleFunc()
{
	ANIMATE animPt;


	if (panel.progStatus == running) {
		if (!tmpTrans || !curTrans) printError ("Error in idle()");

		//---------------------- translation
		if (tmpTrans->mode == translation) {
			// x-coord
			if (fabs(curTrans->x - tmpTrans->x) > TOL) {
				tmpTrans->x += curTrans->x/panel.speedStep;
				animPt.x = true;
			}
			else animPt.x = false;
			// y-coord
			if (fabs(curTrans->y - tmpTrans->y) > TOL) {
				tmpTrans->y += curTrans->y/panel.speedStep;
				animPt.y = true;
			}
			else animPt.y = false;
			// z-coord
			if (fabs(curTrans->z - tmpTrans->z) > TOL) {
				tmpTrans->z += curTrans->z/panel.speedStep;
				animPt.z = true;
			}
			else animPt.z = false;
			// check if animation is done
			if (!animPt.x && !animPt.y && !animPt.z) {
				if (panel.btnStatus==start) {
					if (tmpTrans->next) copyTransData(tmpTrans->next, tmpTrans);
					else { // goto next transformation in transData[]
						if ((curTransDataIndex+1) < numTransData)
							copyTransData(transData[++curTransDataIndex], tmpTrans);
					}
				} 
				else if (panel.btnStatus==nextTransf) {
					if (tmpTrans->next) copyTransData(tmpTrans->next, tmpTrans);
				}			
			} // if (!animPt.x && !animPt.y && !animPt.z)
		} // if (tmpTrans.mode == translation) 

		//---------------------- rotation
		else if (tmpTrans->mode == rotation) {

			if (fabs(tmpTrans->angle - curTrans->angle) > TOL) tmpTrans->angle += curTrans->angle/panel.speedStep;
			else if (panel.btnStatus==start) {
				if (tmpTrans->next)	copyTransData(tmpTrans->next, tmpTrans);
				else { // goto next transformation in transData[]
					if ((curTransDataIndex+1) < numTransData)
						copyTransData(transData[++curTransDataIndex], tmpTrans);
				}
			}
			else if (panel.btnStatus==nextTransf) {
				if (tmpTrans->next) copyTransData(tmpTrans->next, tmpTrans);
			}			
			
		} // else if (tmpTrans.mode == rotation)

		//---------------------- scaling
		else if (tmpTrans->mode == scaling) {

			// x coordinate
			if (curTrans->x > 1.0) {
				if (tmpTrans->x < curTrans->x) {
					tmpTrans->x = tmpTrans->x + curTrans->x/panel.speedStep;
					animPt.x = true;
				}
				else animPt.x = false;
			}
			else {
				if (tmpTrans->x > curTrans->x) {
					tmpTrans->x = tmpTrans->x - fabs(curTrans->x)/panel.speedStep;
					animPt.x = true;
				}
				else animPt.x = false;
			}			
			// y coordinate
			if (curTrans->y > 1.0) {
				if (tmpTrans->y < curTrans->y) {
					tmpTrans->y = tmpTrans->y + curTrans->y/panel.speedStep;
					animPt.y = true;
				}
				else animPt.y = false;
			}
			else {
				if (tmpTrans->y > curTrans->y) {
					tmpTrans->y = tmpTrans->y - fabs(curTrans->y)/panel.speedStep;
					animPt.y = true;
				}
				else animPt.y = false;
			}
			// z- coordinate
			if (curTrans->z > 1.0) {
				if (tmpTrans->z < curTrans->z) {
					tmpTrans->z = tmpTrans->z + curTrans->z/panel.speedStep;
					animPt.z = true;
				}
				else animPt.z = false;
			}
			else {
				if (tmpTrans->z > curTrans->z) {
					tmpTrans->z = tmpTrans->z - fabs(curTrans->z)/panel.speedStep;
					animPt.z = true;
				}
				else animPt.z = false;
			}
			// check if animation has done
			if (!animPt.x && !animPt.y && !animPt.z) {

				if (panel.btnStatus==start) {
					if (tmpTrans->next)	copyTransData(tmpTrans->next, tmpTrans);
					else { // goto next transformation in transData[]
						if ((curTransDataIndex+1) < numTransData)
							copyTransData(transData[++curTransDataIndex], tmpTrans);
					}
				}
				else if (panel.btnStatus==nextTransf) {
					if (tmpTrans->next)	copyTransData(tmpTrans->next, tmpTrans);
				}
			} // if (!animPt.x && !animPt.y && !animPt.z && )

		} // else if (tmpTrans.mode == scaling)
	} // if (panel.start) 

	glutPostRedisplay();
}


void reshape (int w, int h)
{

	int frame = 10;

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	//glOrtho (-BOUNDARY-10, BOUNDARY+10, -BOUNDARY-10, BOUNDARY+10, -BOUNDARY-10, BOUNDARY+10);
	glOrtho (
		viewParam.xwMin-panel.gridXSpace, viewParam.xwMax+panel.gridXSpace, 
		viewParam.ywMin-panel.gridYSpace, viewParam.ywMax+panel.gridYSpace, 
		viewParam.zwMin-panel.gridZSpace, viewParam.zwMax+panel.gridZSpace);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
}

int main(int argc, char **argv)
{

	// variables cannot be reset
	panel.ar = false;
	
	// initialise main transData
	for (int i=0; i < MAXTRANSFORMATION; i++) transData[i] = NULL;

	// to be moved to proper position in this program when GUI panel is done
	saveObject();
	saveTransformation();

	if (panel.ar) {
		glutInit(&argc, argv);
		init();
		arVideoCapStart(); 
		argMainLoop( mousePlot, keyEvent, mainLoop ); 
		return (0);
	}
	else {
		glutInit(&argc, argv);
		glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
		glutInitWindowSize (700, 700); 
		glutInitWindowPosition (0, 0);
		glutCreateWindow (argv[0]);
		init ();
		glutDisplayFunc(display); 
		glutIdleFunc(idleFunc);
		glutReshapeFunc(reshape);
		glutKeyboardFunc (keyEvent);
		glutMouseFunc(mousePlot);
		glutMainLoop();
		return 0;
	}

}

static void mousePlot(int button, int action, int xMouse, int yMouse)
{
/*	if (button == GLUT_LEFT_BUTTON && action == GLUT_DOWN) {
		if (panel.start==1 && start2==1) start3 = 1;
		else if (panel.start==1) start2 = 1;
		panel.start = 1;
		copyTransData(&pivotTrans[0], &tmpTrans);
		
	}		
	else if (button == GLUT_RIGHT_BUTTON && action == GLUT_DOWN)
		panel.start = false;
	else if (button == GLUT_MIDDLE_BUTTON && action == GLUT_DOWN)
		exit(0);
*/
}

static void keyEvent( unsigned char key, int x, int y)
{
   	switch (key) {
		/* quit if the ESC key is pressed */
		case 27 :	printf("*** %f (frame/sec)\n", (double)count/arUtilTimer());
					cleanup();
					exit(0);
					break;

		// rotation about x-axis
		case 'i' :
		case 'I' :	keyboard.xDir = -keyboard.incVal;
					keyboard.xAngle = keyboard.xAngle + keyboard.xDir;
					if (keyboard.xAngle <= 0.0) keyboard.xAngle = 360.0;
					break;
		case 'm' :
		case 'M' :	keyboard.xDir = +keyboard.incVal;
					keyboard.xAngle = keyboard.xAngle + keyboard.xDir;
					if (keyboard.xAngle > 360.0) keyboard.xAngle = keyboard.xAngle - 360.0;
					break;

		// rotation about y-axis
		case 'j' :
		case 'J' :	keyboard.yDir = -keyboard.incVal;
					keyboard.yAngle = keyboard.yAngle + keyboard.yDir;
					if (keyboard.yAngle < 0.0) keyboard.yAngle = 360.0;
					break;
		case 'k' :
		case 'K' :	keyboard.yDir = +keyboard.incVal;
					keyboard.yAngle = keyboard.yAngle + keyboard.yDir;
					if (keyboard.yAngle > 360.0) keyboard.yAngle = keyboard.yAngle - 360.0;
					break;

		// rotation about z-axis
		case 'o' :
		case 'O' :	keyboard.zDir = -keyboard.incVal;
					keyboard.zAngle = keyboard.zAngle + keyboard.zDir;
					if (keyboard.zAngle < 0.0) keyboard.zAngle = 360.0;
					break;
		case 'p' :
		case 'P' :	keyboard.zDir = +keyboard.incVal;
					keyboard.zAngle = keyboard.zAngle + keyboard.zDir;
					if (keyboard.zAngle > 360.0) keyboard.zAngle = keyboard.zAngle - 360.0;
					break;

		// zoom in and out
		case '+' :	keyboard.scaleFactor = keyboard.scaleFactor + keyboard.incValScale;
					break;
		case '-' :	keyboard.scaleFactor = keyboard.scaleFactor - keyboard.incValScale;
					break;

		// turn axis or grid on/off
		case 'a' :	
		case 'A' : panel.axis = (panel.axis)? false : true;
					break;

		case 'r' :	
		case 'R' : panel.gridOriObj = (panel.gridOriObj)? false : true;
					break;

		case 'g' :	
		case 'G' : panel.gridAllTrans = (panel.gridAllTrans)? false : true;
					break;

		case 'x' :
		case 'X' :	panel.gridX = (panel.gridX)? false : true;
					break;

		case 'y' :
		case 'Y' :	panel.gridY = (panel.gridY)? false : true;
					break;

		case 'z' :
		case 'Z' :	panel.gridZ = (panel.gridZ)? false : true;
					break;

		//case 'r' :
		//case 'R' :	panel.ar = (panel.ar)? false : true;
					break;


		case '0' :	panel.btnStatus = start; // running from the 1st transformation till the end
					//if (panel.progStatus == stopped) {
						panel.progStatus = running;
						copyTransData(transData[0], tmpTrans);
						curTransDataIndex = 0;
					//}
					break;

		case '1' :	// unused
					break;

		case '2' :	panel.btnStatus = pause; // pause the transformation
					//curTransDataIndex=0;
					break;

		case '3' :	panel.btnStatus = nextStep; // pause at every transformation, incl. pivot point
					if (panel.progStatus == stopped) {
						panel.progStatus = running;
						copyTransData(transData[0], tmpTrans);
						curTransDataIndex = 0;
					}
					else if (panel.progStatus==running) {
						if (tmpTrans->next) copyTransData(tmpTrans->next, tmpTrans);
						else if ((curTransDataIndex+1) < numTransData) 
							copyTransData(transData[++curTransDataIndex], tmpTrans);
					}
					break;

		case '4' :	panel.btnStatus = prevStep; // go one step back
					if (panel.progStatus==running) {
						if (tmpTrans->prev) {
							copyTransData(tmpTrans->prev, tmpTrans);
						}
						else if (curTransDataIndex > 0) { 
							curTransDataIndex--;
							TRANSFORMATION *trans = transData[curTransDataIndex];
							while (trans->next) trans = trans->next;
							copyTransData(trans, tmpTrans);
						}
					}
					break;

		case '5' :	panel.btnStatus = nextTransf; // pause at every transformation
					if (panel.progStatus == stopped) {
						panel.progStatus = running;
						copyTransData(transData[0], tmpTrans);
						++curTransDataIndex;
					}
					else if (panel.progStatus == running) {
						if ((curTransDataIndex+1) < numTransData) 
							copyTransData(transData[++curTransDataIndex], tmpTrans);
					}
					break;
					
		case '6' :	panel.btnStatus = prevTransf; // go one transformation back
					if (panel.progStatus == running) {
						if (curTransDataIndex > 0) { 
							curTransDataIndex--;
							copyTransData(transData[curTransDataIndex], tmpTrans);
						}
					}
					break;

		case '7' :	panel.btnStatus = resetTransf;
					copyTransData(transData[0], tmpTrans);
					initTransformation();
					break;

		case '8' :	panel.btnStatus = resetView;
					initViewParam();
					break;

   }

}

/* main loop */
static void mainLoop(void)
{
    ARUint8         *dataPtr;
    ARMarkerInfo    *marker_info;
    int             marker_num;
    int             j, k;

    /* grab a vide frame */
    if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL ) {
        arUtilSleep(2);
        return;
    }
    if( count == 0 ) arUtilTimerReset();
    count++;

    argDrawMode2D();
    argDispImage( dataPtr, 0,0 );

    /* detect the markers in the video frame */
    if( arDetectMarker(dataPtr, thresh, &marker_info, &marker_num) < 0 ) {
        cleanup();
        exit(0);
    }

    arVideoCapNext();

    /* check for object visibility */
    k = -1;
    for( j = 0; j < marker_num; j++ ) {
        if( patt_id == marker_info[j].id ) {
            if( k == -1 ) k = j;
            else if( marker_info[k].cf < marker_info[j].cf ) k = j;
        }
    }
    if( k == -1 ) {
        argSwapBuffers();
        return;
    }

    /* get the transformation between the marker and the real camera */
    arGetTransMat(&marker_info[k], patt_center, patt_width, patt_trans);

	idleFunc();

   draw(); 

	//glutReshapeFunc(reshape);
	
    argSwapBuffers();
}

static void init( void )
{
	initTransformation();
	initViewParam();

	if (panel.ar) {
		ARParam  wparam;
		
		// open the video path 
		if( arVideoOpen( vconf ) < 0 ) exit(0);
		// find the size of the window 
		if( arVideoInqSize(&xsize, &ysize) < 0 ) exit(0);
		printf(" Image size (x,y) = (%d,%d)\n", xsize, ysize);

		// set the initial camera parameters 
		if( arParamLoad(cparam_name, 1, &wparam) < 0 ) {
			printf("Camera parameter load error !!\n");
			exit(0);
		}
		arParamChangeSize( &wparam, xsize, ysize, &cparam );
		arInitCparam( &cparam );
		printf("*** Camera Parameter ***\n");
		arParamDisp( &cparam );

		if( (patt_id=arLoadPatt(patt_name)) < 0 ) {
			printf("pattern load error !!\n");
			exit(0);
		}

		// open the graphics window 
		argInit( &cparam, 1.0, 0, 0, 0, 0 );
	} // 	if (panel.ar) 

	else {
		glEnable(GL_DEPTH_TEST); 
		glPolygonMode (GL_FRONT, GL_FILL); // front-back test, coord drawn counter clockwise
		glPolygonMode (GL_BACK, GL_LINE);  // front-back test, coord drawn clockwise
		glFrontFace(GL_CCW);
	} // if (panel.ar) {...} else 

}


void initTransformation()
{
	glClearColor (1.0, 1.0, 1.0, 0.0); // tgy

	// panel initialization
	panel.axis = true;
	panel.gridOriObj = true;
	panel.gridAllTrans = false;
	panel.gridX = true;
	panel.gridY = true;
	panel.gridZ = true;
	panel.btnStatus = none;
	panel.progStatus = stopped;
	panel.speedStep = 50.0; // the higher value the slower
	panel.gridXSpace = 100;
	panel.gridYSpace = 100;
	panel.gridZSpace = 100;
	curTransDataIndex = -1;


}

void saveTransformation()
{
	transData[0] = writeTransTable(0, scaling,		1.0,   2.0,   1.0,	  0,	 0.0, 50.0,  0.0, 0, 1, 0);
	transData[1] = writeTransTable(0, rotation,		0.0,   1.0,   0.0,	 90,	50.0,  0.0,  0.0, 0, 0, 1);
	transData[2] = writeTransTable(0, translation,	0.0,  50.0,  -50.0,	  0,	 0.0,  0.0,  0.0, 0, 1, 1);
	transData[3] = writeTransTable(0, rotation,		1.0,   1.0,   0.0,	-90,	 0.0, 50.0, 50.0, 1, 0, 1);

	numTransData = 4;
}

OBJECT *writeObject()
{
	OBJECT *obj = new OBJECT;

	if (!obj) printError("Error in writeObject");

	obj->mode = triangle;
	obj->color.red = red;
	obj->color.green = green;
	obj->color.blue = blue;
	obj->prev = obj->next = NULL;

	OBJPOINTS *pts = new OBJPOINTS;
	pts->x = x;
	pts->y = y;
	pts->z = z;
	pts->prev = NULL;
	pts-next = NULL;
	obj->points= pts;

	return obj;

}

void saveObject()
{
	objData = writeObject();
}


void initViewParam()
{
	// viewing parameters initialization
	viewParam.view.x   =  0.0, viewParam.view.y   =   0.0, viewParam.view.z   = 200.0;	//  viewing-coordinate origin.
	viewParam.ref.x    =  0.0, viewParam.ref.y    =   0.0, viewParam.ref.z    =   0.0;	//  Look-at point.
	viewParam.viewUp.x =  0.0, viewParam.viewUp.y =   1.0, viewParam.viewUp.z =   0.0;  //  View UP vector
	viewParam.dnear    = 25.0, viewParam.dfar     = 275.0;								// set positions for near and far clipping planes
	viewParam.xwMin = -250.0, viewParam.xwMax = 250.0;	// Set coordinate limits for the clipping volume
	viewParam.ywMin = -250.0, viewParam.ywMax = 250.0;	// used by glFrustrum & glOrtho
	viewParam.zwMin = -250.0, viewParam.zwMax = 250.0;

	// initialize interactive variables in keyboard
	keyboard.xDir = keyboard.yDir = keyboard.zDir = 0;
	keyboard.xAngle = keyboard.yAngle = keyboard.zAngle = 0;
	keyboard.incVal = 5.0;		// incremental value for user interactions
	keyboard.incValScale = 0.1;	// incremental value for scaling
	keyboard.scaleFactor = 1.0;	// zoom factor
}

/* cleanup function called when program exits */
static void cleanup(void)
{
    arVideoCapStop();
    arVideoClose();
    argCleanup();
	openGLCleanup();
	objCleanup();
}

void openGLCleanup()
{
	TRANSFORMATION *p, *q;

	for (int i=0; i<MAXTRANSFORMATION; i++) {
		for (p = transData[i]; p != NULL; p = q) {
			q = p->next;
			delete p;
		}
	}
	if (tmpTrans) delete tmpTrans;
}

void objCleanup ()
{
	OBJECT *p, *q;
	OBJPOINTS *c, *d;

	for (p=objData; p != NULL; p=q) {
		q = p->next;

		for (c=p->points; c!= NULL; c=d) { // delete points in the object
			d=c->next;
			delete c;
		}

		delete p;
	}
}


static void draw( void )
{
    double    gl_para[16];
    GLfloat   mat_ambient[]     = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_flash[]       = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_flash_shiny[] = {50.0};
    GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};
    
    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glClearDepth( 1.0 );
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    /* load the camera transformation matrix */
    argConvGlpara(patt_trans, gl_para);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd( gl_para );

	glEnable(GL_COLOR_MATERIAL); // added by tgy
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);	
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMatrixMode(GL_MODELVIEW);
    
	display();

    glDisable( GL_LIGHTING );

    glDisable( GL_DEPTH_TEST );
}

void printError (char *msg)
{
	cout << endl << "*****" << msg << "*****" << endl << endl;
	exit(-1);
}