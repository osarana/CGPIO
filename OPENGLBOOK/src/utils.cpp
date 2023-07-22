#include "utils.h"

string utils::readShaderSource(const char* filePath)
{
	string content;
	ifstream fileStream(filePath, ios::in);

	string line = "";

	while (!fileStream.eof())
	{
		getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

void utils::printShaderLog(unsigned int shader)
{
	int len = 0;
	int chWritten = 0;
	char* log;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0)
	{
		log = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, &chWritten, log);
		cout << "Shader Info Log: " << log << endl;
		free(log);
	}
}

void utils::printProgramLog(int prog)
{
	int len = 0;
	int chWritten = 0;
	char* log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0)
	{
		log = (char*)malloc(len);
		glGetProgramInfoLog(prog, len, &chWritten, log);
		cout << "Program Info Log: " << prog << endl;
		free(log);
	}
}

bool utils::checkOpenGLError()
{
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		cout << "glError: " << glErr << endl;
		foundError = true;
		glErr = glGetError();
	}

	return foundError;
}

unsigned int utils::createShaderProgram(const char* vp, const char* fp)
{
	int vertCompiled = 0;
	int fragCompiled = 0;
	int linked = 0;

	string vertShaderStr = readShaderSource(vp);
	string fragShaderStr = readShaderSource(fp);

	const char* vshaderSource = vertShaderStr.c_str();
	const char* fshaderSource = fragShaderStr.c_str();

	unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vShader, 1, &vshaderSource, NULL);
	glShaderSource(fShader, 1, &fshaderSource, NULL);

	glCompileShader(vShader);
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
	if (vertCompiled != 1)
	{
		cout << "vertex compilation failed" << endl;
		printShaderLog(vShader);
	}

	glCompileShader(fShader);
	checkOpenGLError();
	glGetShaderiv(fShader, GL_COMPILE_STATUS, &fragCompiled);
	if (fragCompiled != 1)
	{
		cout << "fragment compilation failed" << endl;
		printShaderLog(fShader);
	}

	unsigned int vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);

	glLinkProgram(vfProgram);
	checkOpenGLError();
	glGetProgramiv(vfProgram, GL_LINK_STATUS, &linked);
	if (linked != 1)
	{
		cout << "linking failed" << endl;
		printProgramLog(vfProgram);
	}

	return vfProgram;
}

unsigned int utils::loadTexture(const char* texImagePath)
{
	unsigned int textureID;
	textureID = SOIL_load_OGL_texture(texImagePath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	
	if (textureID == 0)
	{
		cout << "could not find texture file" << texImagePath << endl;
	}

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	if (glewIsSupported("GL_EXT_texture_filter_anisotropic"))
	{
		float anisoSetting = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoSetting);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisoSetting);
	}

	return textureID;
}

unsigned int utils::loadCubeMap(const char* mapDir)
{
	unsigned int textureRef;

	// assumes that the six texture image files are named xp, xn, yp, yn, zp, zn and are JPG
	string xp = mapDir; xp = xp + "/xp.jpg";
	string xn = mapDir; xn = xn + "/xn.jpg";
	string yp = mapDir; yp = yp + "/yp.jpg";
	string yn = mapDir; yn = yn + "/yn.jpg";
	string zp = mapDir; zp = zp + "/zp.jpg";
	string zn = mapDir; zn = zn + "/zn.jpg";

	textureRef = SOIL_load_OGL_cubemap(xp.c_str(), xn.c_str(), yp.c_str(), yn.c_str(), zp.c_str(), zn.c_str(),
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	if (textureRef == 0)
	{
		cout << "didnt find cube map image file" << endl;
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureRef);

	// reduce seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureRef;
}

float* utils::goldAmbient()
{
	static float a[4] = { 0.2473f, 0.1995f, 0.0745f, 1 };
	return (float*)a;
}

float* utils::goldDiffuse()
{
	static float a[4] = { 0.7516f, 0.6065f, 0.2265f, 1 };
	return (float*)a;
}

float* utils::goldSpecular()
{
	static float a[4] = { 0.6283f, 0.5558f, 0.3661f, 1 };
	return (float*)a;
}

float utils::goldShininess()
{
	return 51.2f;
}

float* utils::silverAmbient()
{
	static float a[4] = { 0.1923f, 0.1923f, 0.1923f, 1 };
	return (float*)a;
}

float* utils::silverDiffuse()
{
	static float a[4] = { 0.5075f, 0.5075f, 0.5075f, 1 };
	return (float*)a;
}

float* utils::silverSpecular()
{
	static float a[4] = { 0.5083f, 0.5083f, 0.5083f, 1 };
	return (float*)a;
}

float utils::silverShininess()
{
	return 51.2f;
}

float* utils::bronzeAmbient()
{
	static float a[4] = { 0.2125f, 0.1275f, 0.0540f, 1 };
	return (float*)a;
}

float* utils::bronzeDiffuse()
{
	static float a[4] = { 0.7140f, 0.4284f, 0.1814f, 1 };
	return (float*)a;
}

float* utils::bronzeSpecular()
{
	static float a[4] = { 0.3935f, 0.2719f, 0.1667f, 1 };
	return (float*)a;
}

float utils::bronzeShininess()
{
	return 25.6f;
}