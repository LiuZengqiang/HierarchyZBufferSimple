// TODO:TEST!!!
// 重写坐标系？！！！
// 实数坐标 (-1,1)-(-1, 1)
// 像素坐标 (0, scr_width)-(0, scr_height)
// 函数参数顺序 (scr_width,scr_heigh)
#include"GL\freeglut.h"
#include"GL\glut.h"
#include"Scene.h"
#include"Pyramid.h"
#include"Point.h"
#include"Model.h"
#include"tool\globalFunction.cpp"
#include"Octree.h"
#include<iostream>
#include<string>
#include<stdlib.h>

// octree加上model旋转会怎样？
// 会出什么错误？

const unsigned int SCR_WIDTH = 720;
const unsigned int SCR_HEIGHT = 720;
const unsigned int SCR_DEPTH = 720;
std::string model_path = "E:\\VisualStudioProject\\HierarchyZBufferSimple\\resources\\bunny.obj";
// 最中用于显示的z buffer地址
GLubyte* z_buffer_data;

glm::vec3 lightPos = {-0.8f, 0.5f, 1.0f};
glm::vec3 cameraPos = {-1.0f, 0.5f, 1.0f};

// initial 

void init(void);

void display();
void reshape(int w, int h);
void keyBorad(unsigned char key, int x, int y);


int main(int argc, char ** argv) {
	
	Octree octree(SCR_WIDTH, SCR_HEIGHT, SCR_DEPTH, model_path, cameraPos, lightPos);

	octree.init();
	
	octree.beginRender();
	
	//octree.debugNodeZ();
	//octree.debugHierarichy();

	//// 参数窗口 wdth,height,model path
	//Scene scene(SCR_WIDTH, SCR_HEIGHT, model_path);
	//
	//scene.setCameraPosition(cameraPos);
	//
	//scene.setLightPosition(lightPos);
	//
	//scene.init();
	//
	//scene.beginRender();

	//z_buffer_data = scene.getZBufferData();
	z_buffer_data = octree.getZBufferData();
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("绘制像素");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyBorad);
	glutMainLoop();

	return 0;
}

void init(void ) {
	glClearColor(0.0, 0.0, 255, 0.0);
	glShadeModel(GL_FLAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}
void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	// 设置绘制起点
	glRasterPos2i(0, 0);



	glDrawPixels(SCR_WIDTH, SCR_HEIGHT, GL_RGB,
		GL_UNSIGNED_BYTE, z_buffer_data);
	/*
	在这里加上显示z buffer color的代码
	*/

	glFlush();
}
void reshape(int w, int h) {
	
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble)w, 0, (GLdouble)h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyBorad(unsigned char key, int x, int y) {
	
	switch (key) {
	case 'a':
		break;
	default:
		break;
	}

	glutPostRedisplay();
}