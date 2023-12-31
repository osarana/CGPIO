#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL2/SOIL2.h>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

class utils
{
	private:

	public:
		static string readShaderSource(const char* filePath);
		static void printShaderLog(unsigned int shader);
		static void printProgramLog(int prog);
		static bool checkOpenGLError();
		static unsigned int createShaderProgram(const char* vp, const char* fp);
		static unsigned int createShaderProgram(const char* vp, const char* gp, const char* fp);
		static unsigned int createShaderProgram(const char* vp, const char* tCS, const char* tES, const char* fp);
		static unsigned int createShaderProgram(const char* vp, const char *tCS, const char *tES, const char *gp, const char* fp);
		static unsigned int loadTexture(const char* texImagePath);
};
