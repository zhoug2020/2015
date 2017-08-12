#include "stdafx.h"

#include "GLTools.h"
#include "glut.h"


void setupRC()
{
	glClearColor(0.0, 0.0, 0.0,0.0);
	glShadeModel(GL_FLAT);
}

void changeSize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
	glMatrixMode(GL_MODELVIEW);
}

void renderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glScalef(1.0, 2.0, 1.0);
	glutWireCube(1.0);
	glFlush();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(600, 300);
	glutCreateWindow("Cube");
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
