#include "stdafx.h"

#include "GLTools.h"
#include "glut.h"

void drawlines(float x1, float y1, float x2, float y2)
{ 
	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

void draw_triangle()
{
	glBegin(GL_TRIANGLES);                            // 绘制三角形
	glVertex3f(100.0f, 50.0f, 125.0f);                    // 上顶点
	glVertex3f(200.0f, 60.0f, 145.0f);                    // 左下
	glVertex3f(130.0f, 23.0f, 45.0f);                    // 右下
	glEnd();
}


void renderScene()
{
	int i;
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0x0101);
	drawlines(5.0f, 125.0f, 150.f, 125.0f);
	glLineStipple(1, 0x00FF);
	drawlines(150.0f, 125.0f, 250.f, 125.0f);
	glLineStipple(1, 0x1C47);
	drawlines(250.0f, 125.0f, 350.f, 125.0f);

	glLineWidth(5.0f);
	glLineStipple(1, 0x0101);
	drawlines(50.0f, 100.0f, 150.f, 100.0f);
	glLineStipple(1, 0x00FF);
	drawlines(150.0f, 100.0f, 250.f, 125.0f);
	glLineStipple(1, 0x1C47);
	drawlines(250.0f, 100.0f, 350.f, 100.0f);
	glLineWidth(1.0f);

	glBegin(GL_LINE_STRIP);

	for (i = 0; i < 7; i++)
	{
		glVertex2f(50.0f + (GLfloat)i * 50.0f, 75.0f);
	}
	glEnd();

	for (i = 0; i < 6; ++i)
	{
		drawlines(50 + (GLfloat)i * 50.0f, 50.0f, 50 + GLfloat(i + 1) * 50.0f, 50.0f);
	}

	glLineStipple(5, 0x1c47);
	drawlines(50.0f, 25.0f, 350.0f, 25.0f);
	glDisable(GL_LINE_STIPPLE);
	glFlush();
}

void changeSize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0f, (GLdouble)w, 0.0f, (GLdouble)h);
}

void setupRC()
{
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glShadeModel(GL_FLAT);
}



int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(600, 300);
	glutCreateWindow("Lines");
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
