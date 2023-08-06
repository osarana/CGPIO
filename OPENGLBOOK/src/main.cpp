#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <stack>
#include <iostream>
#include <fstream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL2/SOIL2.h>
#include "utils.h"
#include "Sphere.h"
#include "Torus.h"
#include "ImportedModel.h"

using namespace std;

float toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }

#define numVAOs 1
#define numVBOs 3

#define vShaderPath1 "c:\\Users\\oscar\\source\\repos\\OPENGLBOOK\\OPENGLBOOK\\vertShader.glsl"
#define fShaderPath1 "C:\\Users\\oscar\\source\\repos\\OPENGLBOOK\\OPENGLBOOK\\fragShader.glsl"

float cameraX, cameraY, cameraZ;
float gndLocX, gndLocY, gndLocZ;
unsigned int renderingProgram;
unsigned int vao[numVAOs];
unsigned int vbo[numVBOs];

// ALLOCATE VARIABLES USED IN DISPLAY() FUNCTION, SO NO NEED TO ALLOCATE DURING RENDERING.
GLuint mvLoc, projLoc;
int width, height;
float aspect;
glm::mat4 pMat, vMat, mMat, mvMat;

ImportedModel ground("grid.obj");
int numGroundVertices;

unsigned int heightMap, heightTexture;

void setupVertices(void)
{
	numGroundVertices = ground.getNumVertices();
	std::vector<glm::vec3> vert = ground.getVertices();
	std::vector<glm::vec2> tex = ground.getTextureCoordinates();
	std::vector<glm::vec3> norm = ground.getNormals();

	std::vector<float> pvalues;
	std::vector<float> tvalues;
	std::vector<float> nvalues;

	for (int i = 0; i < numGroundVertices; i++) {
		pvalues.push_back((vert[i]).x);
		pvalues.push_back((vert[i]).y);
		pvalues.push_back((vert[i]).z);
		tvalues.push_back((tex[i]).s);
		tvalues.push_back((tex[i]).t);
		nvalues.push_back((norm[i]).x);
		nvalues.push_back((norm[i]).y);
		nvalues.push_back((norm[i]).z);
	}

	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);
}

void init(GLFWwindow* window) 
{
	renderingProgram = utils::createShaderProgram(vShaderPath1, fShaderPath1);
	cameraX = 0.03f; cameraY = 0.03f; cameraZ = 0.8f;
	gndLocX = 0.0f; gndLocY = 0.0f; gndLocZ = -1.0f;

	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);

	setupVertices();

	heightTexture = utils::loadTexture("heightTexture.jpg");
	heightMap = utils::loadTexture("height.jpg");
}

void display(GLFWwindow* window, double currTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(renderingProgram);

	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");

	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(gndLocX, gndLocY, gndLocZ));
	mMat = glm::rotate(mMat, toRadians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	mvMat = vMat * mMat;

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightMap);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, numGroundVertices);
}


void window_reshape_callback(GLFWwindow* window, int newWidth, int newHeight)
{
	aspect = (float)newWidth / (float)newHeight;
	glViewport(0, 0, newWidth, newHeight);
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
}

int main(void)
{
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(600, 600, "Chapter 10 - program 4", NULL, NULL);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		exit(EXIT_FAILURE);
	}

	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(window, window_reshape_callback);

	init(window);

	while (!glfwWindowShouldClose(window))
	{
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}