#include "stdafx.h"

#include "GLTools.h"
#include "glut.h"
#include <math.h>

#define BUFFER_OFFSET(offset) ((GLvoid*)NULL + offset)
#define  NumberOf(array) (sizeof(array)/ sizeof(array[0]))
#define _PI       3.14159265358979323846
typedef struct Vec3
{
	GLfloat x, y, z;
}Vec3;

typedef struct XForm
{
	Vec3 xlate;
	GLfloat angel;
	Vec3 axis;
}XForm;

enum {
	Cube,
	Cone,
	NumVAOs
};

GLuint VAO[NumVAOs];
GLenum PrimType[NumVAOs];
GLsizei NumElements[NumVAOs];
XForm xform[NumVAOs] = {
	{ { -2.0, 0.0, 0.0 }, 0.0, { 0.0, 1.0, 0.0 } },
	{ { 0.0, 0.0, 2.0 }, 0.0, { 1.0, 0.0, 0.0 } }
};

GLfloat Angel = 0.0;

void changeSize(int w, int h)
{
	glViewport(0, 0, w, h);
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluOrtho2D(0.0f, (GLdouble)w, 0.0f, (GLdouble)h);
}



void init()
{
	enum{ Vertices, Colors, Elements, NumVBOs };
	GLuint buffers[NumVBOs];
	glGenVertexArrays(NumVAOs, VAO);

	GLfloat cubeVerts[][3] = {
			{ -1.0, -1.0, -1.0 },
			{ -1.0, -1.0, 1.0 },
			{ -1.0, 1.0, -1.0 },
			{ -1.0, 1.0, 1.0 },
			{ 1.0, -1.0, -1.0 },
			{ 1.0, -1.0, 1.0 },
			{ 1.0, 1.0, -1.0 },
			{ 1.0, 1.0, 1.0 },
	};

	GLfloat cubeColors[][3] = {
			{ 0.0, 0.0, 0.0 },
			{ 0.0, 0.0, 1.0 },
			{ 0.0, 1.0, 0.0 },
			{ 0.0, 1.0, 1.0 },
			{ 1.0, 0.0, 0.0 },
			{ 1.0, 0.0, 1.0 },
			{ 1.0, 1.0, 0.0 },
			{ 1.0, 1.0, 1.0 },
	};

	GLbyte cubeIndices[] =
	{
		0, 1, 3, 2,
		4, 6, 7, 5,
		2, 3, 7, 6,
		0, 4, 5, 1,
		0, 2, 6, 4,
		1, 5, 7, 3,
	};

	{
		glBindVertexArray(VAO[Cube]);
		glGenBuffers(2, buffers);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[Vertices]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
		glVertexPointer(3, GL_FLOAT, 0, cubeVerts);
		glEnableClientState(GL_VERTEX_ARRAY);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Colors]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);
		glColorPointer(3, GL_FLOAT, 0, cubeColors);
		glEnableClientState(GL_COLOR_ARRAY);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[Elements]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

		PrimType[Cube] = GL_QUADS;
		NumElements[Cube] = NumberOf(cubeIndices);

	}

	{

		int i, idx;
		float dTheta;
#define NumConePoints 36

		//we add one more vertex for the cone's apex
		GLfloat coneVerts[NumConePoints + 1][3] = 
		{
			{0.0, 0.0, 1.0}
		}; 

		GLfloat coneColors[NumConePoints + 1][3] =
		{
			{ 1.0, 1.0, 1.0 }
		};

		GLubyte coneIndices[NumConePoints + 1];
		dTheta = 2 * _PI / (NumConePoints - 1);

		idx = 1;

		for (i = 0; i < NumConePoints; ++i, ++idx)
		{
			float theta = i * dTheta;
			coneVerts[idx][0] = cos(theta);
			coneVerts[idx][1] = sin(theta);
			coneVerts[idx][2] = 0.0;

			coneColors[idx][0] = cos(theta);
			coneColors[idx][1] = sin(theta);
			coneColors[idx][2] = 0.0;

			coneIndices[idx] = idx;
		}

		glBindVertexArray(VAO[Cone]);
		glGenBuffers(2, buffers);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[Vertices]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
		glVertexPointer(3, GL_FLOAT, 0, cubeVerts);
		glEnableClientState(GL_VERTEX_ARRAY);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Colors]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);
		glColorPointer(3, GL_FLOAT, 0, cubeColors);
		glEnableClientState(GL_COLOR_ARRAY);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[Elements]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

		PrimType[Cone] = GL_TRIANGLE_FAN;
		NumElements[Cone] = NumberOf(cubeIndices);
	}
	glEnable(GL_DEPTH_TEST);

}

void setupRC()
{
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	//glShadeModel(GL_FLAT);
	init();
}


void renderScene()
{
	int i;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glRotatef(Angel, 0.0, 1.0, 0.0);

	for (i = 0; i < NumVAOs; ++i)
	{
		glPushMatrix();
		glTranslatef(xform[i].xlate.x, xform[i].xlate.y, xform[i].xlate.z);
		glRotatef(xform[i].angel, xform[i].xlate.x, xform[i].xlate.y, xform[i].xlate.z);
		glBindVertexArray(VAO[i]);
		glDrawElements(PrimType[i], NumElements[i], GL_UNSIGNED_BYTE, VAO);
	}
	glPushMatrix();
	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(600, 300);
	glutCreateWindow("Vao");
	glutReshapeFunc(changeSize);
	glutDisplayFunc(renderScene);
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		return 1;
	}
	setupRC();
	glutMainLoop();
	return 0;
}