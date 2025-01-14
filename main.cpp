//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>
#include "Skybox.hpp"

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(glm::vec3(0.0f, -0.5f, 5.5f),
                     glm::vec3(0.0f, 1.0f, 0.0f),   
                     glm::vec3(0.0f, 1.0f, 0.0f));  

float cameraSpeed = 0.09f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;
GLfloat angle;


gps::Model3D lightCube;
gps::Model3D screenQuad;


gps::Model3D scena;
gps::Model3D cat;


gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader skyboxShader;
gps::Shader rainShader;
gps::Shader snowShader;



GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::SkyBox mySkyBox;

bool showDepthMap;

//point light
glm::vec3 pointLightPos;
glm::vec3 pointLightColor;
float pointLightConstant;
float pointLightLinear;
float pointLightQuadratic;
bool enabledPL;
GLuint pointLightPosLoc;
GLuint pointLightColorLoc;
GLuint pointLightConstantLoc;
GLuint pointLightLinearLoc;
GLuint pointLightQuadraticLoc;
GLuint enabledPLoc;
GLuint viewLocLight;

glm::vec3 catPos = glm::vec3(0.0f, -1.05f, 0.0f);
glm::vec3 catCurrPos = catPos;
bool catKey=false;


//rain enable
bool enableRain = false;
std::vector<glm::vec3> rainPositions;
const int numRaindrops = 10000;
GLuint rainPositionBuffer;

//snow enable
bool enableSnow = false;
std::vector<glm::vec3> snowPositions;
const int numSnowflakes = 5000;
GLuint SnowPositionBuffer;

bool enabledSpot;
bool enableAnimation=false;

int currentRenderMode = 0;//pt moduri de vizualizare



GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}

//rain

void initRain() {
	rainPositions.resize(numRaindrops);

	for (int i = 0; i < numRaindrops; ++i) {
		float x = static_cast<float>(rand() % 400 - 200) / 10.0f; 
		float y = static_cast<float>(rand() % 100) / 10.0f + 10.0f; 
		float z = static_cast<float>(rand() % 400 - 200) / 10.0f; 
		rainPositions[i] = glm::vec3(x, y, z);
	}
}

void updateRain() {
	for (int i = 0; i < numRaindrops; ++i) {
		rainPositions[i].y -= 0.2f; 
		if (rainPositions[i].y < 0.0f) {
			rainPositions[i].y = static_cast<float>(rand() % 100) / 10.0f + 10.0f; 
			rainPositions[i].x = static_cast<float>(rand() % 400 - 200) / 10.0f; 
			rainPositions[i].z = static_cast<float>(rand() % 400 - 200) / 10.0f;
		}
	}
}

void renderRain(gps::Shader shader) {
	shader.useShaderProgram();

	GLuint modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	GLuint viewLoc = glGetUniformLocation(shader.shaderProgram, "view");
	GLuint projLoc = glGetUniformLocation(shader.shaderProgram, "projection");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < numRaindrops; ++i) {
		glm::mat4 model = glm::translate(glm::mat4(1.0f), rainPositions[i]);
		model = glm::scale(model, glm::vec3(0.01f, 0.1f, 0.1f)); 
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		screenQuad.Draw(shader); 
	}

	glDisable(GL_BLEND);
}

///SNOW 


void initSnow() {
	snowPositions.resize(numSnowflakes);

	for (int i = 0; i < numSnowflakes; ++i) {
		float x = static_cast<float>(rand() % 200 - 100) / 10.0f;
		float y = static_cast<float>(rand() % 100) / 10.0f + 10.0f;
		float z = static_cast<float>(rand() % 200 - 100) / 10.0f;
		snowPositions[i] = glm::vec3(x, y, z);
	}
}

void updateSnow() {
	float time = glfwGetTime(); 
	//folosim timp ca sa fie oscilatia neconstanta
	for (int i = 0; i < numSnowflakes; ++i) {
		snowPositions[i].y -= 0.05f; 

		//oscilam pe x si pe z pentru fulgi
		float offsetX = 0.1f * cos(time + i); 
		float offsetZ = 0.1f * sin(time + i); 
		snowPositions[i].x += offsetX;
		snowPositions[i].z += offsetZ;

		if (snowPositions[i].y < 0.0f) {
			snowPositions[i].y = static_cast<float>(rand() % 100) / 10.0f + 10.0f;
			snowPositions[i].x = static_cast<float>(rand() % 200 - 100) / 10.0f;
			snowPositions[i].z = static_cast<float>(rand() % 200 - 100) / 10.0f;
		}
	}
}

void renderSnow(gps::Shader shader) {
	shader.useShaderProgram();

	//obtinem locatia uniform din shader
	GLuint modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	GLuint viewLoc = glGetUniformLocation(shader.shaderProgram, "view");
	GLuint projLoc = glGetUniformLocation(shader.shaderProgram, "projection");

	//trimitem matricile spre shader
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glEnable(GL_BLEND); //activez amestecarea culorilor
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //alpha e pt transparenta texturii si GL_ONE_MINUS ALPHA Pt transparenta fundalului

	for (int i = 0; i < numSnowflakes; ++i) {
		glm::mat4 model = glm::translate(glm::mat4(1.0f), snowPositions[i]);
		model = glm::scale(model, glm::vec3(0.02f)); //dimensiune mai mică pentru fulgi
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		screenQuad.Draw(shader); 
	}

	glDisable(GL_BLEND);
}




void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		pressedKeys[GLFW_KEY_Q] = true;
	else if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
		pressedKeys[GLFW_KEY_Q] = false;

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		pressedKeys[GLFW_KEY_E] = true;
	else if (key == GLFW_KEY_E && action == GLFW_RELEASE)
		pressedKeys[GLFW_KEY_E] = false;
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		currentRenderMode = 0; // Solid
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		currentRenderMode = 1; // Wireframe
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		currentRenderMode = 2; // Poligonal
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		currentRenderMode = 3; // Smooth
	}

}

bool firstMouse = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float yaw = -90.0f;	
float pitch = 0.0f;
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		firstMouse = false;
		lastX = xpos;
		lastY = ypos;
	}

	float x_offset = xpos - lastX;
	float y_offset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	x_offset *= sensitivity;
	y_offset *= sensitivity;

	yaw += x_offset;
	pitch += y_offset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		angle -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle += 1.0f;		
	}

	
	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_P]) {
		enabledPL = !enabledPL;
		myCustomShader.useShaderProgram();
		glUniform1i(enabledPLoc, (int)enabledPL);
		pressedKeys[GLFW_KEY_P] = false;
	}

	if (pressedKeys[GLFW_KEY_K]) {
		
		catCurrPos.x += 0.1;
	}
	if (pressedKeys[GLFW_KEY_R]) {
		enableRain = !enableRain; //enableploaie
	}
	if (pressedKeys[GLFW_KEY_C]) {
		enableSnow = !enableSnow; //enable fulgi de nea
	}

	if (pressedKeys[GLFW_KEY_G]) {
		enableAnimation = !enableAnimation; //enable animatie prezentare
	}
}


void setRenderMode(int mode) {
	switch (mode) {
	case 0: //solid
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 1: //wireframe
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 2: //poligonal
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glLineWidth(2.0f); // Grosimea liniilor
		break;
	case 3: //smooth
		glShadeModel(GL_SMOOTH);
		break;
	default:
		break;
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
    glewExperimental = GL_TRUE;
    glewInit();
#endif

	const GLubyte* renderer = glGetString(GL_RENDERER); 
	const GLubyte* version = glGetString(GL_VERSION); 
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS); 
	glEnable(GL_CULL_FACE); 
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	//lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	scena.LoadModel("objects/scena/scenafinal3.obj");
	cat.LoadModel("objects/scena/pisica.obj");

}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/shadow.vert", "shaders/shadow.frag");
	depthMapShader.useShaderProgram();
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	rainShader.loadShader("shaders/rain.vert", "shaders/rain.frag");
	rainShader.useShaderProgram();
	snowShader.loadShader("shaders/snow.vert", "shaders/snow.frag");
	snowShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//stabilim directia luminii
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//culoarea luminii
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


	//lumina punctiforma
	pointLightPos = glm::vec3(-0.530921f, 3.14149f, -0.130871f); //am luat coordonatele stalpului de lumina
	//pointLightPos = glm::vec3(-2.078f, -0.5f, -3.212f);
	//pointLightPos = glm::vec3(-0.45f, 3.071f, -3.1f);
	pointLightColor = glm::vec3(1.0f, 1.0f, 0.0f);
	pointLightConstant = 1.0f;
	pointLightLinear = 0.7f;
	pointLightQuadratic = 1.8f;
	enabledPL = false;
	pointLightPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPos");
	pointLightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor");
	pointLightConstantLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightConstant");
	pointLightLinearLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLinear");
	pointLightQuadraticLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightQuadratic");
	viewLocLight = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	enabledPLoc = glGetUniformLocation(myCustomShader.shaderProgram, "enabledPL");


	myCustomShader.useShaderProgram();

	glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPos));
	glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));
	glUniform1f(pointLightConstantLoc, pointLightConstant); 
	glUniform1f(pointLightLinearLoc, pointLightLinear);
	glUniform1f(pointLightQuadraticLoc,pointLightQuadratic);
	glUniform1i(enabledPLoc, (int)enabledPL);
	glUniformMatrix4fv(viewLocLight, 1, GL_FALSE, glm::value_ptr(view));

}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//harta adancimilor la umbre
	glGenFramebuffers(1, &shadowMapFBO);
	
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,SHADOW_WIDTH,SHADOW_HEIGHT,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//atasam textura la fbo
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	//matrice de vizualizare 
	glm::mat4 lightView = glm::lookAt(glm::vec3(lightRotation * glm::vec4(lightDir,1.0f)),
							glm::vec3(0.0f),
							glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat nearPlane = 0.1f;
	const GLfloat farPlane = 50.0f;
	//matrice de proiectie ca sa vedem scena dpdv al luminii
	glm::mat4 lightProjection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f,nearPlane,farPlane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}
void  initSkybox() {
	//initializam tga-urile pentru skybox
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/right.tga");
	faces.push_back("skybox/left.tga");
	faces.push_back("skybox/top.tga");
	faces.push_back("skybox/bottom.tga");
	faces.push_back("skybox/back.tga");
	faces.push_back("skybox/front.tga");
	mySkyBox.Load(faces);
}
void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();
	
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}


	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}


	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	scena.Draw(shader);
	//randam pisica cu animatia de translatatre
	glm::mat4 catModel = glm::mat4(1.0f);
	catModel = glm::translate(catModel, catCurrPos);

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(catModel));
	cat.Draw(shader);

	if (!depthPass)
	{
		mySkyBox.Draw(skyboxShader, view, projection);
	}
	
}

void renderScene() {
	//cream harta de adancime
	depthMapShader.useShaderProgram();

	
	glm::mat4 lightSpaceMatrix = computeLightSpaceTrMatrix();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	//randadam scena in harta de adancime
	drawObjects(depthMapShader, true);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (showDepthMap) {
		//Afișeam harta de adancime pe quad-ul de ecran
		glViewport(0, 0, retina_width, retina_height);
		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		glActiveTexture(GL_TEXTURE0); 
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		
		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glLineWidth(1.0f); 
		setRenderMode(currentRenderMode);
		glViewport(0, 0, retina_width, retina_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 lightDirTransformed = glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir;
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTransformed));

		
		glActiveTexture(GL_TEXTURE3); 
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);


		myCustomShader.useShaderProgram();

		glm::vec3 fogColor = glm::vec3(0.5f, 0.5f, 0.5f); //culoare ceata gri
		float fogDensity = 0.05f; //cat de densa e ceata

		GLuint fogColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogColor");
		GLuint fogDensityLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");

		glUniform3fv(fogColorLoc, 1, glm::value_ptr(fogColor));
		glUniform1f(fogDensityLoc, fogDensity);

		drawObjects(myCustomShader, false);

		
		lightShader.useShaderProgram();
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = glm::translate(lightRotation, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader);

	}
	if (enableRain) {
		updateRain();
		renderRain(rainShader);
	}
	if (enableSnow) {
		updateSnow();
		renderSnow(snowShader);
	}
}


std::vector<glm::vec3> cameraPath = {
	glm::vec3(-1.7289f, - 0.5f, 9.26423f),  //start
	glm::vec3(-1.33801f, - 0.5f, - 2.39384f),  //p1
	glm::vec3(6.49394f, - 0.5f, - 0.474228f), //p2
	glm::vec3(6.88185f, - 0.5f, 9.6981f), //p3
	glm::vec3(-1.7289f, -0.5f, 9.26423f),  //start
};



glm::vec3 interpolate(const glm::vec3& start, const glm::vec3& end, float t) {
	return start + t * (end - start);
}
float animationTimer = 0.0f;//timp animatie
float animationDuration = 10.0f; //durata totala la animatie

void animateCamera(float deltaTime) {
	if (!enableAnimation) {
		return;
	}

	animationTimer += deltaTime;

	//durata fiecare segment
	float segmentDuration = animationDuration / cameraPath.size();

	//impartire pe fiecare segment al cadrului
	int currentSegment = static_cast<int>(animationTimer / segmentDuration) % cameraPath.size();
	int nextSegment = (currentSegment + 1) % cameraPath.size();
	float t = fmod(animationTimer, segmentDuration) / segmentDuration;

	//interpolare pentru tranzitie lina
	glm::vec3 newPos = interpolate(cameraPath[currentSegment], cameraPath[nextSegment], t);

	myCamera.setCameraPosition(newPos);

	
	glm::vec3 lookAtTarget = cameraPath[nextSegment];
	myCamera.setCameraTarget(lookAtTarget);

	//conditie de stop
	if (animationTimer >= animationDuration) {
		enableAnimation = false;
		animationTimer = 0.0f; 
	}
}


void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initSkybox();
	initUniforms();
	initFBO();
	initRain();
	initSnow();

	glCheckError();
	float startTime = glfwGetTime();
	float lastFrame = glfwGetTime();
	
	while (!glfwWindowShouldClose(glWindow)) {
		float currTime = glfwGetTime();
		float deltaTime = currTime - lastFrame;
		lastFrame = currTime;
		

		processMovement();
		if (enableAnimation) {
			animateCamera(deltaTime);
		}

		renderScene();		

		glfwPollEvents();
		glfwSwapBuffers(glWindow);

	/*	if (currTime - startTime >= 10.0f) {
			std::cout << myCamera.getCameraPosition().x << " " << myCamera.getCameraPosition().y << " "
				<< myCamera.getCameraPosition().z;
			std::cout << std::endl;
		}*/
	}

	cleanup();

	return 0;
}
