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

void passOne(void);
void passTwo(void);

float toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }

#define numVAOs 1
#define numVBOs 5

#define vShaderPath1 "c:\\Users\\oscar\\source\\repos\\OPENGLBOOK\\OPENGLBOOK\\vertShader.glsl"
#define fShaderPath1 "C:\\Users\\oscar\\source\\repos\\OPENGLBOOK\\OPENGLBOOK\\fragShader.glsl"
#define vShaderPath2 "c:\\Users\\oscar\\source\\repos\\OPENGLBOOK\\OPENGLBOOK\\vertShader2.glsl"
#define fShaderPath2 "C:\\Users\\oscar\\source\\repos\\OPENGLBOOK\\OPENGLBOOK\\fragShader2.glsl"

unsigned int renderingProgram1, renderingProgram2;
unsigned int vao[numVAOs];
unsigned int vbo[numVBOs];
unsigned int brickTexture;

ImportedModel pyramid("pyr.obj");
Torus myTorus(0.6f, 0.4f, 48);
int numPyramidVertices, numTorusVertices, numTorusIndices;

// locations of torus, pyramid, camera, and light
glm::vec3 torusLoc(1.6f, 0.0f, -0.3f);
glm::vec3 pyrLoc(-1.0f, 0.1f, 0.3f);
glm::vec3 cameraLoc(0.0f, 0.2f, 6.0f);
glm::vec3 lightLoc(-3.8f, 2.2f, 1.1f);

// white light properties
float globalAmbient[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

// gold material properties for the pyramid
float* goldMatAmb = utils::goldAmbient();
float* goldMatDif = utils::goldDiffuse();
float* goldMatSpe = utils::goldSpecular();
float goldMatShi = utils::goldShininess();

// bronze material for the torus
float* bronzeMatAmb = utils::bronzeAmbient();
float* bronzeMatDif = utils::bronzeDiffuse();
float* bronzeMatSpe = utils::bronzeSpecular();
float bronzeMatShi = utils::bronzeShininess();

// variables used in display() for transferring light to shaders
float curAmb[4], curDif[4], curSpe[4], matAmb[4], matDif[4], matSpe[4];
float curShi, matShi;

// shadow-related variables
int screenSizeX, screenSizeY;
unsigned int shadowTex, shadowBuffer;
glm::mat4 lightVmatrix;
glm::mat4 lightPmatrix;
glm::mat4 shadowMVP1;
glm::mat4 shadowMVP2;
glm::mat4 b;

// ALLOCATE VARIABLES USED IN DISPLAY() FUNCTION, SO NO NEED TO ALLOCATE DURING RENDERING.
GLuint mvLoc, projLoc, nLoc, sLoc;
int width, height;
float aspect;
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat;
glm::vec3 currentLightPos, transformed;
float lightPos[3];
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;
glm::vec3 origin(0.0f, 0.0f, 0.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

void installLights(unsigned int prog, glm::mat4 vMatrix)
{
	// convert light's position to view space, and save it in a float array
	transformed = glm::vec3(vMatrix * glm::vec4(currentLightPos, 1.0));
	lightPos[0] = transformed.x;
	lightPos[1] = transformed.y;
	lightPos[2] = transformed.z;

	matAmb[0] = curAmb[0]; matAmb[1] = curAmb[1]; matAmb[2] = curAmb[2]; matAmb[3] = curAmb[3];
	matDif[0] = curDif[0]; matDif[1] = curDif[1]; matDif[2] = curDif[2]; matDif[3] = curDif[3];
	matSpe[0] = curSpe[0]; matSpe[1] = curSpe[1]; matSpe[2] = curSpe[2]; matSpe[3] = curSpe[3];
	matShi = curShi;

	// get the locations of the light and material fields in the shader
	globalAmbLoc = glGetUniformLocation(prog, "globalAmbient");
	ambLoc = glGetUniformLocation(prog, "light.ambient");
	diffLoc = glGetUniformLocation(prog, "light.diffuse");
	specLoc = glGetUniformLocation(prog, "light.specular");
	posLoc = glGetUniformLocation(prog, "light.position");
	mambLoc = glGetUniformLocation(prog, "material.ambient");
	mdiffLoc = glGetUniformLocation(prog, "material.diffuse");
	mspecLoc = glGetUniformLocation(prog, "material.specular");
	mshiLoc = glGetUniformLocation(prog, "material.shininess");

	// set the uniform light and material values in the shader
	glProgramUniform4fv(prog, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(prog, ambLoc, 1, lightAmbient);
	glProgramUniform4fv(prog, diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(prog, specLoc, 1, lightSpecular);
	glProgramUniform3fv(prog, posLoc, 1, lightPos);
	glProgramUniform4fv(prog, mambLoc, 1, matAmb);
	glProgramUniform4fv(prog, mdiffLoc, 1, matDif);
	glProgramUniform4fv(prog, mspecLoc, 1, matSpe);
	glProgramUniform1f(prog, mshiLoc, matShi);
}

void setupVertices(void)
{
	// pyramid definition

	numPyramidVertices = pyramid.getNumVertices();
	std::vector<glm::vec3> vert = pyramid.getVertices();
	std::vector<glm::vec3> norm = pyramid.getNormals();

	std::vector<float> pyramidPvalues;
	std::vector<float> pyramidNvalues;

	for (int i = 0; i < numPyramidVertices; i++)
	{
		pyramidPvalues.push_back((vert[i]).x);
		pyramidPvalues.push_back((vert[i]).y);
		pyramidPvalues.push_back((vert[i]).z);
		pyramidNvalues.push_back((norm[i]).x);
		pyramidNvalues.push_back((norm[i]).y);
		pyramidNvalues.push_back((norm[i]).z);
	}

	// torus definition

	numTorusVertices = myTorus.getNumVertices();
	numTorusIndices = myTorus.getNumIndices();
	std::vector<int> ind = myTorus.getIndices();
	vert = myTorus.getVertices();
	norm = myTorus.getNormals();

	std::vector<float> torusPvalues;
	std::vector<float> torusNvalues;

	for (int i = 0; i < numTorusVertices; i++)
	{
		torusPvalues.push_back((vert[i]).x);
		torusPvalues.push_back((vert[i]).y);
		torusPvalues.push_back((vert[i]).z);
		torusNvalues.push_back((norm[i]).x);
		torusNvalues.push_back((norm[i]).y);
		torusNvalues.push_back((norm[i]).z);
	}

	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, torusPvalues.size() * 4, &torusPvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, pyramidPvalues.size() * 4, &pyramidPvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, torusNvalues.size() * 4, &torusNvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, pyramidNvalues.size() * 4, &pyramidNvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * 4, &ind[0], GL_STATIC_DRAW);
}

void setupShadowBuffers(GLFWwindow* window)
{
	glfwGetFramebufferSize(window, &width, &height);
	screenSizeX = width;
	screenSizeY = height;

	// create the custom frame buffer
	glGenFramebuffers(1, &shadowBuffer);

	// create the shadow texture and configure it to hold depth information.
	// these steps are similar to those in Program 5.2
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, screenSizeX, screenSizeY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}

void init(GLFWwindow* window) 
{
	renderingProgram1 = utils::createShaderProgram(vShaderPath1, fShaderPath1);
	renderingProgram2 = utils::createShaderProgram(vShaderPath2, fShaderPath2);

	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);

	setupVertices();
	setupShadowBuffers(window);

	b = glm::mat4
	(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f
	);
}

void display(GLFWwindow* window, double currTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);

	currentLightPos = glm::vec3(lightLoc);

	// set up view and perspective matrix from the light point of view, for pass 1
	lightVmatrix = glm::lookAt(currentLightPos, origin, up); // vector from light to origin
	lightPmatrix = glm::perspective(toRadians(60.0f), aspect, 0.1f, 1000.0f);

	// make the custom frame buffer current, and associate it with the shadow texture
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTex, 0);

	// disable drawing colors, but enable the depth computation
	glDrawBuffer(GL_NONE);
	glEnable(GL_DEPTH_TEST);

	passOne();

	// restore the default display buffer, and re-enable drawing
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	glDrawBuffer(GL_FRONT); // re-enables drawing colors

	passTwo();
}

void passOne(void)
{
	glUseProgram(renderingProgram1);

	// draw the torus

	mMat = glm::translate(glm::mat4(1.0f), torusLoc);
	mMat = glm::rotate(mMat, toRadians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	shadowMVP1 = lightPmatrix * lightVmatrix * mMat;
	sLoc = glGetUniformLocation(renderingProgram1, "shadowMVP");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
	glDrawElements(GL_TRIANGLES, numTorusIndices, GL_UNSIGNED_INT, 0);

	// draw the pyramid

	mMat = glm::translate(glm::mat4(1.0f), pyrLoc);
	mMat = glm::rotate(mMat, toRadians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	mMat = glm::rotate(mMat, toRadians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	shadowMVP1 = lightPmatrix * lightVmatrix * mMat;
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, numPyramidVertices);
}

void passTwo(void)
{
	glUseProgram(renderingProgram2);

	mvLoc = glGetUniformLocation(renderingProgram2, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram2, "proj_matrix");
	nLoc = glGetUniformLocation(renderingProgram2, "norm_matrix");
	sLoc = glGetUniformLocation(renderingProgram2, "shadowMVP");

	// draw the torus

	curAmb[0] = bronzeMatAmb[0]; curAmb[1] = bronzeMatAmb[1]; curAmb[2] = bronzeMatAmb[2];
	curDif[0] = bronzeMatDif[0]; curDif[1] = bronzeMatDif[1]; curDif[2] = bronzeMatDif[2];
	curSpe[0] = bronzeMatSpe[0]; curSpe[1] = bronzeMatSpe[1]; curSpe[2] = bronzeMatSpe[2];
	curShi = bronzeMatShi;

	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraLoc.x, -cameraLoc.y, -cameraLoc.z));

	mMat = glm::translate(glm::mat4(1.0f), torusLoc);
	mMat = glm::rotate(mMat, toRadians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	currentLightPos = glm::vec3(lightLoc);
	installLights(renderingProgram2, vMat);

	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));
	shadowMVP2 = b * lightPmatrix * lightVmatrix * mMat;

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	// set up torus vertices and normals buffers (and texture coordinates buffer if used)
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // torus vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]); // torus normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
	glDrawElements(GL_TRIANGLES, numTorusIndices, GL_UNSIGNED_INT, 0);

	// draw the pyramid
	
	curAmb[0] = goldMatAmb[0]; curAmb[1] = goldMatAmb[1]; curAmb[2] = goldMatAmb[2];
	curDif[0] = goldMatDif[0]; curDif[1] = goldMatDif[1]; curDif[2] = goldMatDif[2];
	curSpe[0] = goldMatSpe[0]; curSpe[1] = goldMatSpe[1]; curSpe[2] = goldMatSpe[2];
	curShi = goldMatShi;

	mMat = glm::translate(glm::mat4(1.0f), pyrLoc);
	mMat = glm::rotate(mMat, toRadians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	mMat = glm::rotate(mMat, toRadians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	currentLightPos = glm::vec3(lightLoc);
	installLights(renderingProgram2, vMat);

	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));
	shadowMVP2 = b * lightPmatrix * lightVmatrix * mMat;

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]); 
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, pyramid.getNumVertices());
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
	GLFWwindow* window = glfwCreateWindow(600, 600, "Chapter 8 - program 1", NULL, NULL);
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