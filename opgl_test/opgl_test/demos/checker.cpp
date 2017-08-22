#include "stdafx.h"
#include "GLTools.h"
#include "glut.h"

static GLbyte checkImage[64][64][4];
static GLuint textName;

void makeCheckImage(void)
{
	int i, j, c;
	for (i = 0; i < 64; i++)
	{
		for (j = 0; j < 64; j++)
		{
			c = (((i & 0x8) == 0) ^ ((j & 0x8)) == 0) * 255;
			checkImage[i][j][0] = (GLbyte)c;
			checkImage[i][j][1] = (GLbyte)c;
			checkImage[i][j][2] = (GLbyte)c;
			checkImage[i][j][3] = (GLbyte)255;
		}
	}
}

void init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	makeCheckImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &textName);
	glBindTexture(GL_TEXTURE_2D, textName);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
}

void changeSize(int w, int h)
{

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, w / h, 1, 30.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -3.6);

}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, textName);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-2.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-2.0, 1.0, 0.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.0, 1.0, 0.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(0.0, -1.0, 0.0);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.0, 1.0, 0.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(2.41421, 1.0, -1.414121);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(2.41421, -1.0, -1.41421);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_LINE_STRIP);
	glLineWidth(10.0);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(-2.0, -1.0, 0.0);
	glVertex3f(-2.0, 1.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, -1.0, 0.0);
	glEnd();

	glBegin(GL_POINTS);
	glPointSize(20.0);
	glColor3f(1.0, 1.0, 0.0);
	glVertex3f(1.0, -1.0, 0.0);
	glVertex3f(1.0, 1.0, 0.0);
	glVertex3f(2.41421, 1.0, -1.414121);
	glVertex3f(2.41421, -1.0, -1.41421);
	glEnd();

	glFlush();

}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(600, 300);
	glutCreateWindow("Checker");
	glutReshapeFunc(changeSize);
	glutDisplayFunc(display);
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		return 1;
	}
	init();
	glutMainLoop();
	return 0;
}