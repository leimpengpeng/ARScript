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

#include"obj_parse.h" //add by lpp

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
static void	  SpecialFunc( int key, int x, int y);//lpp
void DataInit();  //lpp
static void   mainLoop(void);
static void   draw( void );
void initTransformation();
void initViewParam();
void printError (char*);
void openGLCleanup();

#define BOUNDARY 300 

struct POINTS_3D {
	GLfloat x, y, z;
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



//-------------- transformation structure
#define MAXTRANSFORMATION 10
#define TOL 0.0001
#define UNUSE 999

int transIndex = 0;
enum TRANSMODE { translation, rotation, scaling };
enum STATUS { running, paused, stopped }; 

struct COLOR {
	GLfloat red, green, blue;
};

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
int loadObject(const char* obj_filename)
{

int i;
int FirstFace=0;
int LastFace=0;
int FaceId=0;
int ObjId=0;
int curmat=0;
int k=0;
char line[256];
char *arg[8];
char buf[256];
for (i=0; i<8;i++){
	 arg[i]=(char*)malloc(256);
	 if (arg[i]==NULL)
	 {printf("ERROR mallocing arg\n");exit(1);}
}
std::ifstream objfile;
objfile.open(obj_filename);
//_____READING OBJFILE_____
 if(!objfile.is_open())return -1;
//char buf[256];	
/*while (!objfile.eof()){
	objfile.getline(buf,256);
	coord.push_back(new std::string(buf));
	}
	*/
while (!objfile.eof())
{
		objfile.getline(line,256);
	if(line[0]!='#'){ 
			for(i=0;i<8;i++)strcpy(arg[i],"");
				int num_args=sscanf(line,"%s%s%s%s%s%s%s%s",arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],
							arg[6],arg[7]);	
			
	if(strcmp(arg[0],"g")==0){
		char obj_name[256];
			k++;
		sscanf(arg[1],"%s",obj_name);
	//	std::cout <<  (k/2) << obj_name << std::endl;
	

	//	std::cout << std::endl; 
		

	}

	if(strcmp(arg[0],"v")==0){
			float x,y,z; 
			sscanf(arg[1],"%f",&x);
			sscanf(arg[2],"%f",&y);
			sscanf(arg[3],"%f",&z);
	 		vertex.push_back(new coordinate(x,-z,y));
			//std::cout << vertex.size() << std::endl;
		
		}
	if (strcmp(arg[0],"mtllib")==0){
			char buff[200];
			char mread[256];
			std::ifstream mtlfile;
			int t=0;

			char mtl_objname[200];	//name of the material
			char filename[200];	//filename of the texture
			float amb[3],dif[3],spec[3],alpha,ns,ni;	//colors, shininess, and something else
			int illum;
			unsigned int texture;
			sscanf (arg[1], "%s./", mtlFile);
			std::cout<< "mtllib" << ":" << mtlFile ;
			mtlfile.open(mtlFile);
		/*	while(!mtlfile.eof()){
					mtlfile.getline(buff,256);
					mcoord.push_back(new std::string(buff));
					}*/

			while (!mtlfile.eof())
			{
				mtlfile.getline(buff,256);
		
				if(buff[0]!='#'){ 
					for(i=0;i<8;i++)strcpy(arg[i],"");
					int num_args=sscanf(buff,"%s%s%s%s%s%s%s%s",arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],
							arg[6],arg[7]);	
					if(strcmp(arg[0],"newmtl")==0){	//new material
						sscanf(arg[1],"%s",mtl_objname);
					
						while (!mtlfile.eof() && strcmp(arg[0],"#")!=0){
						mtlfile.getline(mread,256);
						
					for(i=0;i<8;i++)strcpy(arg[i],"");
					int num_args=sscanf(mread,"%s%s%s%s%s%s%s%s",arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],
								arg[6],arg[7]);
					if(strcmp(arg[0],"Ns")==0){	//the shininess
						sscanf(arg[1],"%f",&ns);
						}
					if(strcmp(arg[0],"Ka")==0){   //the ambient
						sscanf(arg[1],"%f",&amb[0]);
						sscanf(arg[2],"%f",&amb[1]);
						sscanf(arg[3],"%f",&amb[2]);
					}
					if(strcmp(arg[0],"Kd")==0){   //the diffuse
						sscanf(arg[1],"%f",&dif[0]);
						sscanf(arg[2],"%f",&dif[1]);
						sscanf(arg[3],"%f",&dif[2]);
					}
					if(strcmp(arg[0],"Ks")==0){   //the specular
						sscanf(arg[1],"%f",&spec[0]);
						sscanf(arg[2],"%f",&spec[1]);
						sscanf(arg[3],"%f",&spec[2]);
					}
					if(strcmp(arg[0],"Ni")==0){   //the I don't know what is this
						sscanf(arg[1],"%f",&ni);
					}
					if(strcmp(arg[0],"d")==0 ){	//the alpha
						sscanf(arg[1],"%f",&alpha);
					/*	if(alpha!=1 )
						alpha = 1 -  alpha;*/
						 
					}
					if(strcmp(arg[0],"illum")==0){	//the illum (I dont know what is this)
						sscanf(arg[1],"illum %d",&illum);
					}
					if(strcmp(arg[0],"map_Kd")==0){
						sscanf(arg[1],"%s",name);
					}
					//materials.push_back(new material(mtl_objname,alpha,ns,ni,dif,amb,spec,illum,-1));
			}
						
			 materials.push_back(new material(mtl_objname,alpha,ns,ni,dif,amb,spec,illum,-1));
		}
	}
}
}
	/*

	if(strcmp(arg[0],"vt")==0){
		float u,v;
		sscanf(arg[1],"%f",&u);
		sscanf(arg[2],"%f",&v);
		texturecoordinate.push_back(new texcoord(u,1-v));
	}*/
	if(strcmp(arg[0],"vn")==0){
		float nx,ny,nz;
		sscanf(arg[1],"%f",&nx);
		sscanf(arg[2],"%f",&ny);
		sscanf(arg[3],"%f",&nz);
		normals.push_back(new coordinate(nx,ny,nz));
		//std::cout << normals.size() << "\t"nx << "\t"ny << "\t"nz << std::endl;
	}
	if(strcmp(arg[0],"usemtl")==0){
		char obj_mtlname[200];
		char read[256];
		
		sscanf (arg[1],"%s",obj_mtlname);
		//std::cout <<obj_mtlname << std::endl;

		for(int k=0;k<materials.size();k++)	//go through all of the materials
		{
			if(strcmp(materials[k]->name.c_str(),obj_mtlname)==0)	//and compare the tmp with the name of the material
			{
				ObjId=k;	//if it's equal then set the current material to that
				
				break;
			}
		}
	while (!objfile.eof() && strcmp(arg[0],"#")!=0){
			objfile.getline(read,256);

			for(i=0;i<8;i++)strcpy(arg[i],"");
				int num_args=sscanf(read,"%s%s%s%s%s%s%s%s",arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],
								arg[6],arg[7]);
			if (strcmp(arg[0],"f")==0){ 
					  int v1,v2,v3,t1,t2,t3,n1,n2,n3;
			
						if (strcmp(arg[1],"%d//%d")==0){
							sscanf(arg[1],"%d//%d",&v1,&n1);
							sscanf(arg[2],"%d//%d",&v2,&n2);
							sscanf(arg[3],"%d//%d",&v3,&n3);
							faces.push_back(new face(v1-1,v2-1,v3-1,0,0,0,n1-1,n2-1,n3-1,curmat));
							FaceId++;
						}
						else { 

						sscanf(arg[1],"%d/%d/%d",&v1,&t1,&n1);
						sscanf(arg[2],"%d/%d/%d",&v2,&t2,&n2);
						sscanf(arg[3],"%d/%d/%d",&v3,&t3,&n3);
						faces.push_back(new face(v1-1,v2-1,v3-1,t1-1,t2-1,t3-1,n1-1,n2-1,n3-1,curmat));
						FaceId++;
					//	std::cout << faces.size() << std::endl;
					//	std::cout <<"a" << ":" << texturecoordinatfaces.at(3)->t1)->u <<  std::endl;
					//	std::cout <<"a" << ":" << texturecoordinate.at(faces.at(3)->t1)->u <<  std::endl;
					}
				}
			}
		//	faces.push_back(new face(v1-1,v2-1,v3-1,t1-1,t2-1,t3-1,n1-1,n2-1,n3-1,curmat));
		//	std::cout << faces.size() << std::endl;
			//	system("PAUSE");
			LastFace = FaceId;
			FaceMaterials.push_back(new FaceMaterial( ObjId,FirstFace, LastFace));
			FirstFace= LastFace;
			ObjId=ObjId+1;
		}
	

	else{	
		continue;	
	}
}
}
}

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
	if ((curTransDataIndex != 0) && (trans->index == 0)) { 
		for (int i=0; i<=curTransDataIndex-1; i++) {
			TRANSFORMATION *trans = transData[i];
			while (trans) {
				switch (trans->mode) {
					case translation	:
						glTranslatef (trans->x, trans->y, trans->z);
						break;
					case rotation		:
						if (trans->prev) glTranslatef(trans->prev->x, trans->prev->y, trans->prev->z);
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
			p->angle = UNUSE;
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
				p->angle = UNUSE;
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
				else p->angle = UNUSE;
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
				p->angle = UNUSE;
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
			target->angle = UNUSE;	// unuse
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
			target->angle = UNUSE;	// unuse
			break;
	}
}
// ----------------------- transformation functions end

void display(void)
{
	COLOR color;

	if (!panel.ar) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
		glLoadIdentity();
	}

	gluLookAt (viewParam.view.x, viewParam.view.y, viewParam.view.z, viewParam.ref.x, viewParam.ref.y, viewParam.ref.z, viewParam.viewUp.x, viewParam.viewUp.y, viewParam.viewUp.z);

	color.red = 1.0, color.green = 0.0, color.blue = 0.0;
 
	glPushMatrix();
		glRotatef(keyboard.xAngle, 1, 0, 0);
		glRotatef(keyboard.yAngle, 0, 1, 0);
		glRotatef(keyboard.zAngle, 0, 0, 1);
		glScalef(keyboard.scaleFactor, keyboard.scaleFactor, keyboard.scaleFactor);
	//	if (panel.gridAllTrans || panel.gridOriObj)  draw_grid(color);
		
	//	drawAxis(color);
	//	draw_triangle (color);  
	//	draw_square (red, green, blue);

		glTranslatef(moveX,moveY,0);
	//	my_model();
		car_model();
	
		//
	/*	if (panel.progStatus == running) {
		
			if ((panel.btnStatus==prevStep) || (panel.btnStatus==prevTransf)) transformObj (curTrans);
			else transformObj (tmpTrans);
			if (panel.gridAllTrans) draw_grid(curTrans->color);
			drawAxis(curTrans->color);
			draw_triangle (curTrans->color); 
		
		}*/

	//glPopMatrix();
		 
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
	panel.ar = true;
	
	// initialise main transData
	for (int i=0; i < MAXTRANSFORMATION; i++) transData[i] = NULL;

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
		glutSpecialFunc(SpecialFunc);
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
		case 'I' :		keyboard.xDir = -keyboard.incVal;
							keyboard.xAngle = keyboard.xAngle + keyboard.xDir;
							if (keyboard.xAngle <= 0.0) keyboard.xAngle = 360.0;
							break;
		case 'm' :
		case 'M' :		keyboard.xDir = +keyboard.incVal;
					keyboard.xAngle = keyboard.xAngle + keyboard.xDir;
					if (keyboard.xAngle > 360.0) keyboard.xAngle = keyboard.xAngle - 360.0;
					break;

		// rotation about y-axis
/*		case 'j' :
		case 'J' :	keyboard.yDir = -keyboard.incVal;
					keyboard.yAngle = keyboard.yAngle + keyboard.yDir;
					if (keyboard.yAngle < 0.0) keyboard.yAngle = 360.0;
					break;
	*/	case 'k' :
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

	
	case '1' :	DataInit(); break;
		case '2' :	Sca+=0.05; break;
			case '3' :	Sca-=0.05; break;
				case '5' :	moveY+=0.10; break;
					case '6' :	moveY-=0.10; break;
	case '4': rotate_+=2;break;
	case 'b' :  	
            toX+=0.5;
			toY-=0.5;
			toZ+=0.5;
				break;

	case 'c' :  
				
			typeX+=0.2;
		
				break;
	case 'd' :  
				
			hoodZ+=1;
			fenderX+=1;
			fenderY+=1;
			fenderZ+=1;

				break;
		case 'e' :  
				
			doorX+=1; 
			doorR-=1;

				break;

		case'f':
			rotate_d1+=2;
			rotate_d2-=2;
			break;
		case 'g' :  
			mirrorL+=0.5; 
			mirrorR-=0.5;
				break;

		case 'q' :  
			headlight-=1; 
			break;
		case 'r' :  
			foglight-=1; 
			break;
		case 's' :  
			taillight+=1; 
			break;
		case 't' :  
			roof_+=1; 
			break;
		case 'u' :  
			grilles_+=1; 
			break;
		case 'v' :  
			interior_+=1; 
			
				break;
		case 'w' :  
			exhaust_+=1; 
			break;
		case 'x' :  
			trunk_+=1; 
			break;
		case 'a' :	
           /* trunk_+=1; 
			exhaust_+=1; 
			grilles_+=1; 
			roof_+=1; 	
			taillight+=1;
			foglight-=1; 
			headlight-=1; 
			mirrorL+=0.5; 
			mirrorR-=0.5;
			hoodZ+=1;
			fenderX+=1;
			fenderY+=1;
			fenderZ+=1;
			doorX+=1; 
			doorR-=1;
			typeX+=0.2;*/
			toX+=1;
			toY-=1;
			toZ+=1;
			statical_+=2;
		}

}
static void SpecialFunc(int key, int x, int y)
{
	switch (key) {
		case GLUT_KEY_HOME	 :	DataInit(); break;
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
	cube= loadObject("car2.obj"); 
	indicate();
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
	
	transData[0] = writeTransTable(0, scaling,		1.0,   2.0,   1.0,		 0,		0.0, 50.0, 0.0,	0, 1, 0);
	transData[1] = writeTransTable(0, rotation,		0.0,   1.0,   1.0,		90,		50.0,  0.0, 0.0, 0, 0, 1);
	transData[2] = writeTransTable(0, translation,	0.0,  50.0, -50.0,		 0,		0.0,  0.0, 50.0, 0, 1, 1);
	numTransData = 3;
	curTransDataIndex = -1;

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

void DataInit(){
rotate_d1=0;
rotate_d2=0;
toX=0; 
toY=0;
toZ=0;
typeX=0;
hoodZ=0;
fenderX=0;
fenderY=0;
fenderZ=0;
doorX=0;
doorR=0;
mirrorL=0;
mirrorR=0;

headlight=0;
foglight=0;
taillight=0;
 interior_=0;
 exhaust_=0;
 trunk_=0;
 grilles_=0;
 roof_=0;
 statical_=0;
rotate_=0;Sca=0.80;


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
   
	glTranslatef( 0.0, 0, -50.0 );
//	glTranslatef(moveX,moveY,0.0);
//	my_model();
	car_model();
    glDisable( GL_LIGHTING );

    glDisable( GL_DEPTH_TEST );
}

void printError (char *msg)
{
	cout << endl << "*****" << msg << "*****" << endl << endl;
	exit(-1);
}

void indicate(){
	cout << "      \n\n\n";
	cout << "          *****************************************************************\n";
	cout << "          ***************        FINAL YEAR PROJECT          **************\n";
	cout << "          *****************************************************************\n";
	cout << "          *    Controllable Piecewise Component Animation of a 3D model   *\n";
	cout << "          *****************************************************************\n";
	cout << "          * Identify the each name of the 3Dmodel CAR component by ....   *\n";
	cout << "          ----------------------------------------------------------------*\n";
	cout << "          *   PRESS:                                                      #\n";
	cout << "          #   <a>                 -> Every the part of car                #\n"; 
	cout << "          #   <b>                 -> Bumper Molding                       #\n"; 
	cout << "          #   <c>                 -> Component of Type                    #\n"; 
	cout << "          #   <d>                 -> Hood and Fender                      #\n"; 
	cout << "          #   <f>                 -> Door and Window                      #\n"; 
//	cout << "#   <f>                 => -          #\n"; 
	cout << "          #   <g>                 -> Mirror                               #\n"; 
	cout << "          #   <q>                 -> HeadLight                            #\n"; 
	cout << "          #   <r>                 -> FogLight                             #\n"; 
	cout << "          #   <s>                 -> TailLight                            #\n"; 
	cout << "          #   <t>                 -> Roof                                 #\n"; 
	cout << "          #   <u>                 -> Grilles                              #\n"; 
	cout << "          #   <v>                 -> Interior                             #\n"; 
//	cout << "          #   <w>                 -> Exhaust                              #\n"; 
//	cout << "          #   <x>                 -> Trunk                                #\n"; 
	cout << "          #   <1>                 -> restore defaults                     #\n";
	cout << "          #   <2>                 -> Zoom In                              #\n"; 
	cout << "          #   <3>                 -> Zoom Out                             #\n"; 
	cout << "          #   ESC                 -> exit                                 #\n";
	cout << "          #                                                               #\n";
	cout << "          *****************************************************************\n";
	cout << "          #                      THANK YOU !!!                            #\n";
	cout << "          *****************************************************************\n";
}

void my_model(){

		sofa();
		tyre();
		meshObj();
		meshObj2();
		other();
			door3();
			door4();
}


void sofa(){
	 
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
for(int i=195;i<208;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}


glFlush();

}
void meshObj2(){

glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


for(int i=0;i<FaceMaterials.size();i++){
//float diffuse[]={materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->alpha};
//float ambient[]={materials[FaceMaterials[i]->ObjId]->amb[0],materials[FaceMaterials[i]->ObjId]->amb[1],materials[FaceMaterials[i]->ObjId]->amb[2],materials[FaceMaterials[i]->ObjId]->alpha};
//float specular[]={materials[FaceMaterials[i]->ObjId]->spec[i],materials[FaceMaterials[i]->ObjId]->spec[1],materials[FaceMaterials[i]->ObjId]->spec[2],materials[FaceMaterials[i]->ObjId]->alpha};
//			glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuse);
//			glMaterialfv(GL_FRONT,GL_AMBIENT,ambient);
//			glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
//			glMaterialf(GL_FRONT,GL_SHININESS,materials[FaceMaterials[i]->ObjId]->ns);	
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
 				
glBegin(GL_TRIANGLES);
	for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();

	
}
glFlush();
glDisable(GL_TEXTURE_2D);
//glDisable(GL_BLEND);
}
void meshObj(){
	
//_______________________________component inside car

for(int i=0;i<1;i+=1){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}


//___accelerator
for(int i=177;i<183;i+=1){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

//_____undefine.

for(int i=183;i<195;i+=1){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}


glFlush();



}

void door3(){

glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glTranslatef(doorR,0,0);
for(int i=84;i<86;i+=1){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}



for(int i=87;i<125;i+=37){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=137;i<216;i+=78){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=216;i<220;i+=3){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}



glFlush();

}
void door4(){

glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glTranslatef(doorX,0,0);
for(int i=82;i<83;i+=1){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}



for(int i=86;i<126;i+=39){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=139;i<218;i+=78){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=218;i<221;i+=2){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}



glFlush();

}
//void tyre(){
//
//// tyre1	
//for(int i=1;i<13;i+=1){
//float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
//glColor4fv(colorMesh);
//		glBegin(GL_TRIANGLES);
//			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
//			for(int b=0; b<3;b++){
//			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
//			}
//		}
//	glEnd();	
//}
//
//for(int i=145;i<230;i+=84){
//float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
//glColor4fv(colorMesh);
//		glBegin(GL_TRIANGLES);
//			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
//			for(int b=0; b<3;b++){
//			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
//			}
//		}
//	glEnd();	
//}
//
////tyre 2
//for(int i=13;i<25;i+=1){
//float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
//glColor4fv(colorMesh);
//		glBegin(GL_TRIANGLES);
//			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
//			for(int b=0; b<3;b++){
//			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
//			}
//		}
//	glEnd();	
//}
//
//for(int i=147;i<229;i+=81){
//float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
//glColor4fv(colorMesh);
//		glBegin(GL_TRIANGLES);
//			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
//			for(int b=0; b<3;b++){
//			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
//			}
//		}
//	glEnd();	
//}
////tyre 3
//for(int i=25;i<37;i+=1){
//float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
//glColor4fv(colorMesh);
//		glBegin(GL_TRIANGLES);
//			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
//			for(int b=0; b<3;b++){
//			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
//			}
//		}
//	glEnd();	
//}
//
//for(int i=150;i<227;i+=76){
//float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
//glColor4fv(colorMesh);
//		glBegin(GL_TRIANGLES);
//			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
//			for(int b=0; b<3;b++){
//			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
//			}
//		}
//	glEnd();	
//}
//
////tyre 3
//for(int i=37;i<48;i+=1){
//float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
//glColor4fv(colorMesh);
//		glBegin(GL_TRIANGLES);
//			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
//			for(int b=0; b<3;b++){
//			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
//			}
//		}
//	glEnd();	
//}
//
//for(int i=146;i<228;i+=81){
//float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
//glColor4fv(colorMesh);
//		glBegin(GL_TRIANGLES);
//			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
//			for(int b=0; b<3;b++){
//			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
//			}
//		}
//	glEnd();	
//}
//
//
//glFlush();
//
//
//
//
//}
void other(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


for(int i=37;i<76;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=88;i<108;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=118;i<123;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=128;i<136;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=141;i<145;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=148;i<150;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=151;i<177;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=208;i<212;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=223;i<225;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=208;i<209;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}


glFlush();

}

void car_model(){
	
	glRotatef(rotate_,0,0,1);
//	glScalef(0.8,0.8,0.8);
	glScalef(Sca,Sca,Sca);glTranslatef(0,moveY,0);
	
	glPushMatrix();
	Interior();
	glPopMatrix();

	glPushMatrix();
	 antenna();
	glPopMatrix();
	
	glPushMatrix();
	 bumper();
	glPopMatrix();
	
	glPushMatrix();
     tyre();
	glPopMatrix();

	glPushMatrix();
	hood();
	glPopMatrix();
	
	glPushMatrix();
//	window();
	glPopMatrix();
	
	glPushMatrix();
//	mirror();
	glPopMatrix();
	glPushMatrix();
			glTranslatef(-56,22,80);
		glRotatef(rotate_d1,0,0,1);
			glTranslatef(56,-22,-80);
				door1();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(53,22,80);
			glRotatef(rotate_d2,0,0,1);
			glTranslatef(-53,-22,-80);
			door2();
		glPopMatrix();

		glPushMatrix();
		door3();
		door4();
		glPopMatrix();




	glPushMatrix();
	Exhaust();
	glPopMatrix();

	glPushMatrix();
	Trunk();
	glPopMatrix();

	glPushMatrix();
	Roof();
	glPopMatrix();

	glPushMatrix();
	HeadLight();
	glPopMatrix();

	glPushMatrix();
	FogLight();
	glPopMatrix();
	
	glPushMatrix();
	TailLight();
	glPopMatrix();
	
	glPushMatrix();
	statical();
	glPopMatrix();
glFlush();

}

void antenna(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 glTranslatef(0,toY,toZ);
for(int i=151;i<152;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glDisable(GL_BLEND);
}
void bumper(){

glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glPushMatrix();
for(int i=54;i<58;i++){
	glTranslatef(0, toY,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();
glPushMatrix();
for(int i=70;i<72;i++){
	glTranslatef(0, -toY,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();
glDisable(GL_BLEND);
}

void tyre(){

glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glPushMatrix();
for(int i=1;i<25;i++){
glTranslatef(typeX, 0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=228;i<230;i++){
	glTranslatef(-typeX, 0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

glPopMatrix();
glPushMatrix();
for(int i=25;i<48;i++){
	glTranslatef(-typeX, 0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=226;i<228;i++){
	glTranslatef(typeX, 0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();
glDisable(GL_BLEND);
}
void hood(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//hood
glPushMatrix();
 glTranslatef(0,0,hoodZ);
for(int i=49;i<50;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();
glPushMatrix();
//fender
 glTranslatef(fenderX,fenderY,fenderZ);
for(int i=50;i<51;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();
glPushMatrix();
//fender
 glTranslatef(-fenderX,fenderY,fenderZ);
for(int i=51;i<52;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();
glDisable(GL_BLEND);
}
void door(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glPushMatrix();
for(int i=211;i<212;i++){
glTranslatef(doorX,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}


for(int i=217;i<219;i++){
glTranslatef(doorX,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}


glPopMatrix();
glTranslatef(doorX,0,0);
for(int i=220;i<222;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}


for(int i=76;i<79;i+=2){
	
 glTranslatef(doorX,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPushMatrix();
for(int i=79;i<83;i+=3){
glTranslatef(doorX,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=83;i<87;i+=3){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

//door_right
glPopMatrix();

glPushMatrix();
for(int i=213;i<217;i++){
glTranslatef(doorR,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}


for(int i=219;i<223;i+=3){
glTranslatef(doorR,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=77;i<81;i+=3){
glTranslatef(doorR,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=81;i<85;i+=3){
glTranslatef(doorR,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=85;i<88;i+=2){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();
glDisable(GL_BLEND);

}
void window(){
//window
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glPushMatrix();
glTranslatef(doorR,0,0);
for(int i=137;i<139;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glTranslatef(doorX,0,0);
for(int i=139;i<141;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=125;i<128;i++){
glTranslatef(doorX,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

//window_right
for(int i=121;i<124;i++){
glTranslatef(doorR,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glDisable(GL_BLEND);
}
void mirror(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glPushMatrix();

for(int i=108;i<113;i++){
glTranslatef(mirrorR,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();
glPushMatrix();
for(int i=113;i<118;i++){
	glTranslatef(mirrorL,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();
glDisable(GL_BLEND);
}

void HeadLight(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glPushMatrix();
glTranslatef(0,headlight,0);
glPushMatrix();
for(int i=99;i<101;i++){
glTranslatef(0,headlight,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();
for(int i=160;i<174;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}


glPopMatrix();

glDisable(GL_BLEND);

}

void FogLight(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glPushMatrix();
glTranslatef(0,foglight,0);
for(int i=91;i<99;i++){

float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();

glDisable(GL_BLEND);

}
void TailLight(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glPushMatrix();
glTranslatef(0,taillight,0);

for(int i=129;i<136;i++){

float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=152;i<160;i++){

float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

glPopMatrix();
glDisable(GL_BLEND);

}

void door1(){

glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glTranslatef(doorR,0,0);
for(int i=77;i<81;i+=3){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}


for(int i=81;i<109;i+=27){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=123;i<139;i+=15){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}


for(int i=222;i<223;i+=1){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

glPushMatrix();
for(int i=108;i<113;i+=1){
	glTranslatef(mirrorR,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();
glDisable(GL_BLEND);

}
void door2(){

glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	 glTranslatef(doorX,0,0);
for(int i=76;i<79;i+=2){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}



for(int i=79;i<114;i+=34){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=127;i<141;i+=13){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}




for(int i=212;i<222;i+=9){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPushMatrix();
for(int i=113;i<118;i++){
	glTranslatef(mirrorL,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glPopMatrix();

glDisable(GL_BLEND);
}

void Interior(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glTranslatef(interior_,0,0);
for(int i=176;i<202;i++){

float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=208;i<209;i++){
	
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glDisable(GL_BLEND);

}
void Exhaust(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glTranslatef(0,exhaust_,0);
for(int i=89;i<91;i++){

float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glDisable(GL_BLEND);


}
void Trunk(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glTranslatef(0,0,trunk_);
for(int i=105;i<145;i+=39){

float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glDisable(GL_BLEND);
}
void Grilles(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glTranslatef(0,-grilles_,0);
for(int i=101;i<103;i++){

float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=104;i<105;i++){
	glTranslatef(mirrorR,0,0);
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glDisable(GL_BLEND);
}
void Roof(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glTranslatef(0,0,roof_);
for(int i=128;i<224;i+=95){

float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

}
void statical(){
glEnable(GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glTranslatef(0,0,statical_);
for(int i=52;i<54;i++){
	float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=59;i<70;i+=10){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=72;i<74;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=88;i<104;i+=15){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=106;i<108;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=118;i<120;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
for(int i=141;i<144;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=148;i<150;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=174;i<176;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}

for(int i=209;i<211;i++){
float colorMesh[4] ={materials[FaceMaterials[i]->ObjId]->dif[2],materials[FaceMaterials[i]->ObjId]->dif[1],materials[FaceMaterials[i]->ObjId]->dif[0],materials[FaceMaterials[i]->ObjId]->alpha};
glColor4fv(colorMesh);
		glBegin(GL_TRIANGLES);
			for(int f=FaceMaterials[i]->FirstFace; f < FaceMaterials[i]->LastFace;f++){
			for(int b=0; b<3;b++){
			glVertex3f(vertex[faces[f]->v[b]]->x,vertex[faces[f]->v[b]]->y,vertex[faces[f]->v[b]]->z);
			}
		}
	glEnd();	
}
glDisable(GL_BLEND);
}