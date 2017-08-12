// opgl_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "GLTools.h"
#include "GLShaderManager.h"
#include "glut.h"
#include "GLFrustum.h"
#include "StopWatch.h"
#include "math3d.h"
#include "GLTriangleBatch.h"

GLBatch triangleBatch;
GLBatch squarBatch;
GLTriangleBatch torusBatch;

GLShaderManager shaderManager;
GLFrustum viewfrutum;

float rotateFactor = 60.f;

void changeSize(int w, int h)
{
	if (h == 0)
		h = 1;

	glViewport(0, 0, w, h);

	viewfrutum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 1000.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void setupRC()
{
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	//glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	shaderManager.InitializeStockShaders();
	glEnable(GL_DEPTH_TEST);
	//GLfloat vVerts[] = { -0.5f, 0.0f, 0.0f,
	//	0.5f, 0.0f, 0.0f,
	//	0.0f, 0.5f, 0.0f };
	//triangleBatch.Begin(GL_TRIANGLES, 3);
	//triangleBatch.CopyVertexData3f(vVerts);
	//triangleBatch.End();
	gltMakeTorus(torusBatch, 0.4f, 0.15f, 20, 20); // 花托
	//gltMakeTorus(torusBatch, 40.0f, 20.0f, 50.0f, 40.0f);
}


void renderScene(void)
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	//shaderManager.UseStockShader(GLT_SHADER_IDENTITY, vRed);
	//triangleBatch.Draw();

	static CStopWatch rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * rotateFactor;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	M3DMatrix44f mTranslate, mRotate, mModelView, mModelViewProjection;
	m3dTranslationMatrix44(mTranslate, 0.0f, 0.0f, -2.5f);

	m3dRotationMatrix44(mRotate, m3dDegToRad(yRot), 0.0f, 1.0f, 0.0f);

	m3dMatrixMultiply44(mModelView, mTranslate, mRotate);

	m3dMatrixMultiply44(mModelViewProjection, viewfrutum.GetProjectionMatrix(), mModelView);

	GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	shaderManager.UseStockShader(GLT_SHADER_FLAT, mModelViewProjection, vBlack);


	torusBatch.Draw();
	

	glutSwapBuffers();
	glutPostRedisplay();


}

void specialKeys(int key, int x, int y)
{
	GLfloat stepsize = 2.5f;
	//GLfloat blockX = vVerts[0];
	printf("key:%d, x:%d, y:%d\n", key, x, y);
	if (key == GLUT_KEY_UP)
	{
		rotateFactor += stepsize;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		rotateFactor -= stepsize;
		if (rotateFactor < 30.0f)
		{
			rotateFactor = 30.0f;
		}
	}


}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(600, 300);
	glutCreateWindow("Triangle");
	glutReshapeFunc(changeSize);
	glutDisplayFunc(renderScene);
	glutSpecialFunc(specialKeys);
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		return 1;
	}
	setupRC();
	glutMainLoop();
	return 0;
}

