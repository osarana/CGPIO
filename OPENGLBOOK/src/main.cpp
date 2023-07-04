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

#define numVAOs 1
#define numVBOs 4
#define vShaderPath "c:\\Users\\oscar\\source\\repos\\OPENGLBOOK\\OPENGLBOOK\\vertShader.glsl"
#define fShaderPath "C:\\Users\\oscar\\source\\repos\\OPENGLBOOK\\OPENGLBOOK\\fragShader.glsl"
#define brickPath "C:\\Users\\oscar\\source\\repos\\OPENGLBOOK\\OPENGLBOOK\\white.jpg"

float cameraX, cameraY, cameraZ;
float cubeLocX, cubeLocY, cubeLocZ;
float pyrLocX, pyrLocY, pyrLocZ;
unsigned int renderingProgram;
unsigned int vao[numVAOs];
unsigned int vbo[numVBOs];
unsigned int brickTexture;

// ALLOCATE VARIABLES USED IN DISPLAY() FUNCTION, SO NO NEED TO ALLOCATE DURING RENDERING.
unsigned int mvLoc, projLoc;
int width, height, i;
float aspect, timeFactor;
glm::mat4 pMat, vMat, tMat, rMat, mMat, mvMat;
stack<glm::mat4> mvStack;

ImportedModel myModel("orbiter.obj");

// 36 VERTICES, 12 TRIANGLES, MAKES 2X2X2 CUBE PLACED AT ORIGIN
void setupVertices(void)
{
	std::vector<glm::vec3> vert = myModel.getVertices();
	std::vector<glm::vec2> tex = myModel.getTextureCoordinates();
	std::vector<glm::vec3> norm = myModel.getNormals();
	int numObjVertices = myModel.getNumVertices();

	std::vector<float> pvalues; // vertex positions
	std::vector<float> tvalues; // texture coordinates
	std::vector<float> nvalues; // normal vectors

	for (int i = 0; i < numObjVertices; i++)
	{
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

	// put the vertices into buffer #0
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);

	// put the texture coordinates into buffer #1
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW);

	// put the normals into buffer #2
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);
}

void init(GLFWwindow* window) 
{
	renderingProgram = utils::createShaderProgram(vShaderPath, fShaderPath);

	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);

	cameraX = 0.0f; cameraY = 2.0f; cameraZ = 1000.0f;
	cubeLocX = 0.0f; cubeLocY = -2.0f; cubeLocZ = 0.0f; // shift down y to see perspective
	pyrLocX = 2.0f; pyrLocY = 2.0f; pyrLocZ = 0.0f;
	brickTexture = utils::loadTexture(brickPath);

	setupVertices();
}

void display(GLFWwindow* window, double currTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(renderingProgram);
	glEnable(GL_CULL_FACE);

	// GET THE UNIFORM VARIABLES FOR THE V, MV, AND PROJECTION MATRICES
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");

	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
	mvStack.push(vMat); // push vMatrix onto stack

	// pyramid == sun
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currTime, glm::vec3(1.0f, 0.0f, 0.0f));
	
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(cubeLocX, cubeLocY, cubeLocZ));
	mvMat = vMat * mMat;

	// COPY PERSPECTIVE AND MV MATRICES TO CORRESPONDING UNIFORM VARIABLES
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	// ASSOCIATE VBO WITH THE CORRESPONDING VERTEX ATTRIBUTE IN THE VERTEX SHADER
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	// ADJUST OPENGL SETTINGS AND DRAW PYRAMID
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glFrontFace(GL_CCW);
	glDrawArrays(GL_TRIANGLES, 0, myModel.getNumVertices());
	mvStack.pop();

	// CUBE PLANET
	/*
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currTime) * 4.0, 0.0f, cos((float)currTime) * 4.0));
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currTime, glm::vec3(0.0, 1.0, 0.0));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glFrontFace(GL_CW);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	mvStack.pop();

	// CUBE MOON

	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currTime) * 2.0, cos((float)currTime) * 2.0));
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currTime, glm::vec3(0.0, 0.0, 1.0));

	mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 36);
	mvStack.pop();

	mvStack.pop(); mvStack.pop(); mvStack.pop();
	*/
}

void window_reshape_callback(GLFWwindow* winidow, int newWidth, int newHeight)
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
	GLFWwindow* window = glfwCreateWindow(600, 600, "Chapter 4 - program 1", NULL, NULL);
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