#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "SceneHandler.h"
#include "cShaderManager.h"
#include "cMesh.h"
#include "cVAOMeshManager.h"
#include "cGameObject.h"
#include "cLightManager.h"
#include "cCamera.h"

#include <iPhysicsFactory.h>
#include <iPhysicsWorld.h>
#include <iRigidBody.h>
#include <iSoftBody.h>
#include <iShape.h>

typedef nPhysics::iPhysicsFactory*(*f_CreateFactory)();

nPhysics::iPhysicsFactory* g_bigFactory;
nPhysics::iPhysicsWorld* g_bigWorld;
int cameraFocus = 5;

//Setting up a camera GLOBAL
cCamera Camera(glm::vec3(0.0f, 5.0f, 14.0f),		//Camera Position
	glm::vec3(0.0f, 1.0f, 0.0f),		//World Up vector
	0.0f,								//Pitch
	-90.0f);							//Yaw
double deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 540, lastY = 360;

unsigned int SCR_WIDTH = 1080;
unsigned int SCR_HEIGHT = 720;

bool spaceLock = false;
bool enterLock = false;
bool drawMesh = false;

//Global handlers for shader, VAOs, game objects and lights
cShaderManager* g_pShaderManager = new cShaderManager();
cVAOMeshManager* g_pVAOManager = new cVAOMeshManager();
std::vector <cMesh> g_vecMesh;
std::vector <cGameObject*> g_vecGameObject;
std::vector <glm::vec3> g_vecObjStart;
cLightManager* g_pLightManager = new cLightManager();

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		if (!spaceLock)
		{
			spaceLock = true;
			cameraFocus++;
			if (cameraFocus >= ::g_vecGameObject.size())
			{
				cameraFocus = 5;
			}
		}
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		if (spaceLock)
			spaceLock = false;
	}

	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
	{
		if (!enterLock)
		{
			enterLock = true;
			drawMesh = !drawMesh;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE)
	{
		if (enterLock)
			enterLock = false;
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::RIGHT, deltaTime);

	const float ballSpeed = 2.5f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		::g_vecGameObject[cameraFocus]->myBody->applyForce(glm::vec3(0.0f, 0.0f, -ballSpeed));
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		::g_vecGameObject[cameraFocus]->myBody->applyForce(glm::vec3(0.0f, 0.0f, ballSpeed));
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		::g_vecGameObject[cameraFocus]->myBody->applyForce(glm::vec3(-ballSpeed, 0.0f, 0.0f));
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		::g_vecGameObject[cameraFocus]->myBody->applyForce(glm::vec3(ballSpeed, 0.0f, 0.0f));
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	//Space turns the blue guy (or whatever models was loaded second) into a wireframe and back
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		cameraFocus++;
		if (cameraFocus >= ::g_vecGameObject.size())
		{
			cameraFocus = 5;
		}
	}
	
	//Keys 1 through 8 will turn on and off the 8 point lights on the scene
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		::g_pLightManager->vecLights[0].lightIsOn = !(::g_pLightManager->vecLights[0].lightIsOn);
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		::g_pLightManager->vecLights[1].lightIsOn = !(::g_pLightManager->vecLights[1].lightIsOn);
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		::g_pLightManager->vecLights[2].lightIsOn = !(::g_pLightManager->vecLights[2].lightIsOn);
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
		::g_pLightManager->vecLights[3].lightIsOn = !(::g_pLightManager->vecLights[3].lightIsOn);
	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
		::g_pLightManager->vecLights[4].lightIsOn = !(::g_pLightManager->vecLights[4].lightIsOn);
	if (key == GLFW_KEY_6 && action == GLFW_PRESS)
		::g_pLightManager->vecLights[5].lightIsOn = !(::g_pLightManager->vecLights[5].lightIsOn);
	if (key == GLFW_KEY_7 && action == GLFW_PRESS)
		::g_pLightManager->vecLights[6].lightIsOn = !(::g_pLightManager->vecLights[6].lightIsOn);
	if (key == GLFW_KEY_8 && action == GLFW_PRESS)
		::g_pLightManager->vecLights[7].lightIsOn = !(::g_pLightManager->vecLights[7].lightIsOn);

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::FORWARD, deltaTime);
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::BACKWARD, deltaTime);
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::LEFT, deltaTime);
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) // this bool variable is initially set to true
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	Camera.processMouseMovement(xOffset, yOffset, true);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Camera.processMouseScroll(yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	return;
}

int main()
{

	std::string libraryFile = "PhysicsLibrary.dll";
	std::string createFactoryName = "CreateFactory";

	HINSTANCE hGetProckDll = LoadLibraryA(libraryFile.c_str());
	if (!hGetProckDll)
	{
		std::cout << "oh no!" << std::endl;
		system("pause");
		return 1;
	}

	f_CreateFactory CreateFactory = 0;

	CreateFactory = (f_CreateFactory)GetProcAddress(hGetProckDll, createFactoryName.c_str());
	if (!CreateFactory)
	{
		std::cout << "where's the CreateFactory???" << std::endl;
		system("pause");
		return 1;
	}

	::g_bigFactory = CreateFactory();

	::g_bigWorld = ::g_bigFactory->CreateWorld();

	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
		"Balls Party",
		NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	cShaderManager::cShader vertShader;
	cShaderManager::cShader fragShader;

	cShaderManager::cShader softVert;
	cShaderManager::cShader softFrag;

	vertShader.fileName = "simpleVert.glsl";
	fragShader.fileName = "simpleFrag.glsl";

	softVert.fileName = "softVert.glsl";
	softFrag.fileName = "softFrag.glsl";

	::g_pShaderManager->setBasePath("assets//shaders//");

	if (!::g_pShaderManager->createProgramFromFile(
		"simpleShader", vertShader, fragShader))
	{
		std::cout << "Failed to create shader program. Shutting down." << std::endl;
		std::cout << ::g_pShaderManager->getLastError() << std::endl;
		return -1;
	}
	if (!::g_pShaderManager->createProgramFromFile(
		"softShader", softVert, softFrag))
	{
		std::cout << "Failed to create shader program. Shutting down." << std::endl;
		std::cout << ::g_pShaderManager->getLastError() << std::endl;
		return -1;
	}
	std::cout << "The shaders comipled and linked OK" << std::endl;
	GLint shaderID = ::g_pShaderManager->getIDFromFriendlyName("simpleShader");
	GLint softShaderID = ::g_pShaderManager->getIDFromFriendlyName("softShader");

	std::string* plyDirects;
	std::string* plyNames;
	int numPlys;

	if (!readPlysToLoad(&plyDirects, &plyNames, &numPlys))
	{
		std::cout << "Couldn't find files to read. Giving up hard.";
		return -1;
	}

	for (int i = 0; i < numPlys; i++)
	{	//Load each file into a VAO object
		cMesh newMesh;
		newMesh.name = plyNames[i];
		if (!LoadPlyFileIntoMesh(plyDirects[i], newMesh))
		{
			std::cout << "Didn't load model" << std::endl;
		}
		g_vecMesh.push_back(newMesh);
		if (!::g_pVAOManager->loadMeshIntoVAO(newMesh, shaderID))
		{
			std::cout << "Could not load mesh into VAO" << std::endl;
		}
	}

	int numEntities;
	if (!readEntityDetails(&g_vecGameObject, &numEntities, &g_vecObjStart))
	{
		std::cout << "Unable to find game objects for placement." << std::endl;
	}

	//Creating a soft body
	nPhysics::sSoftBodyDesc myDesc;

	//Makes a rectangle of nodes using these dimensions
	int dimensionX = 8;
	int dimensionY = 10;

	float nodeHeight = dimensionY + 2;
	float nodePos = 0;
	for (int i = 0; i < dimensionX * dimensionY; i++)
	{
		myDesc.vertices.push_back(glm::vec3(nodePos, nodeHeight, -2.0f));
		nodePos += 1.0f;
		if (nodePos == (float)dimensionX)
		{
			nodePos = 0.0f;
			nodeHeight --;
		}
	}
	
	if (dimensionX == 0)
	{
		std::cout << "How could you betray me?" << std::endl;
	}

	else if (dimensionX == 1)
	{
		myDesc.staticIndices.push_back(0);
	}

	else if (dimensionX < 3)
	{
		myDesc.staticIndices.push_back(0);
		myDesc.staticIndices.push_back(dimensionX - 1);
	}

	else if (dimensionX >= 3)
	{
		int leftPin = 0;
		int rightPin = dimensionX - 1;
		for (int i = 0; i < dimensionX / 3; i++)
		{
			myDesc.staticIndices.push_back(leftPin++);
			myDesc.staticIndices.push_back(rightPin--);
		}
	}

	

	for (int i = 0; i < dimensionX * dimensionY; i++)
	{
		if ((i % dimensionX < dimensionX - 1) && (i < (dimensionX * dimensionY) - dimensionX))
		{
			myDesc.triangulatedIndices.push_back(i);
			myDesc.triangulatedIndices.push_back(i + dimensionX);
			myDesc.triangulatedIndices.push_back(i + dimensionX + 1);

			myDesc.triangulatedIndices.push_back(i);
			myDesc.triangulatedIndices.push_back(i + 1);
			myDesc.triangulatedIndices.push_back(i + dimensionX);
		}

		if ((i % dimensionX > 0) && (i < (dimensionX * dimensionY) - dimensionX))
		{
			myDesc.triangulatedIndices.push_back(i);
			myDesc.triangulatedIndices.push_back(i + dimensionX - 1);
			myDesc.triangulatedIndices.push_back(i + dimensionX);

			myDesc.triangulatedIndices.push_back(i - 1);
			myDesc.triangulatedIndices.push_back(i);
			myDesc.triangulatedIndices.push_back(i + dimensionX);
		}
	}

	nPhysics::iSoftBody* myBody = ::g_bigFactory->CreateSoftBody(myDesc);
	::g_bigWorld->AddSoftBody(myBody);


	std::cout << glGetString(GL_VENDOR) << " "
		<< glGetString(GL_RENDERER) << ", "
		<< glGetString(GL_VERSION) << std::endl;
	std::cout << "Shader language version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	GLint currentProgID = ::g_pShaderManager->getIDFromFriendlyName("simpleShader");

	GLint uniLoc_mModel = glGetUniformLocation(currentProgID, "mModel");
	GLint uniLoc_mView = glGetUniformLocation(currentProgID, "mView");
	GLint uniLoc_mProjection = glGetUniformLocation(currentProgID, "mProjection");

	currentProgID = ::g_pShaderManager->getIDFromFriendlyName("softShader");

	GLint soft_mModel = glGetUniformLocation(currentProgID, "mModel");
	GLint soft_mView = glGetUniformLocation(currentProgID, "mView");
	GLint soft_mProjection = glGetUniformLocation(currentProgID, "mProjection");

	currentProgID = ::g_pShaderManager->getIDFromFriendlyName("simpleShader");

	GLint uniLoc_materialDiffuse = glGetUniformLocation(currentProgID, "materialDiffuse");

	::g_pLightManager->createLights();	// There are 10 lights in the shader
	::g_pLightManager->LoadShaderUniformLocations(currentProgID);

	glEnable(GL_DEPTH);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	double lastTimeStep = glfwGetTime();

	unsigned int softVAO, softVBO, softEBO;

	glGenVertexArrays(1, &softVAO);
	glGenBuffers(1, &softVBO);
	glGenBuffers(1, &softEBO);

	while (!glfwWindowShouldClose(window))
	{

		processInput(window);

		float ratio;
		int width, height;
		glm::mat4x4 m, p;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);

		SCR_WIDTH = width;
		SCR_HEIGHT = height;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		::g_pLightManager->CopyInfoToShader();

		unsigned int sizeOfVector = ::g_vecGameObject.size();
		for (int index = 0; index != sizeOfVector; index++)
		{
			// Is there a game object? 
			if (::g_vecGameObject[index] == 0)	//if ( ::g_GameObjects[index] == 0 )
			{	// Nothing to draw
				continue;		// Skip all for loop code and go to next
			}

			// Was near the draw call, but we need the mesh name
			std::string meshToDraw = ::g_vecGameObject[index]->meshName;		//::g_GameObjects[index]->meshName;

			sVAOInfo VAODrawInfo;
			if (::g_pVAOManager->lookupVAOFromName(meshToDraw, VAODrawInfo) == false)
			{	// Didn't find mesh
				continue;
			}

			m = glm::mat4x4(1.0f);	

			glm::vec3 myPosition;
			::g_vecGameObject[index]->myBody->getPosition(myPosition);

			glm::mat4 trans = glm::mat4x4(1.0f);
			trans = glm::translate(trans,
				myPosition);
			m = m * trans;

			glm::vec3 myRotation;
			::g_vecGameObject[index]->myBody->getRotataion(myRotation);

			glm::mat4 matPostRotZ = glm::mat4x4(1.0f);
			matPostRotZ = glm::rotate(matPostRotZ, myRotation.z,
				glm::vec3(0.0f, 0.0f, 1.0f));
			m = m * matPostRotZ;

			glm::mat4 matPostRotY = glm::mat4x4(1.0f);
			matPostRotY = glm::rotate(matPostRotY, myRotation.y,
				glm::vec3(0.0f, 1.0f, 0.0f));
			m = m * matPostRotY;

			glm::mat4 matPostRotX = glm::mat4x4(1.0f);
			matPostRotX = glm::rotate(matPostRotX, myRotation.x,
				glm::vec3(1.0f, 0.0f, 0.0f));
			m = m * matPostRotX;

			if (::g_vecGameObject[index]->myBody->getShape()->getShapeType() == nPhysics::SHAPE_SPHERE)
			{
				float finalScale;
				::g_vecGameObject[index]->myBody->getShape()->sphereGetRadius(finalScale);
				glm::mat4 matScale = glm::mat4x4(1.0f);
				matScale = glm::scale(matScale,
					glm::vec3(finalScale,
						finalScale,
						finalScale));
				m = m * matScale;
			}

			p = glm::perspective(glm::radians(Camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

							// View or "camera" matrix
			glm::mat4 v = glm::mat4(1.0f);	// identity

			v = Camera.getViewMatrix();

			if (index != cameraFocus && index > 4)
			{
				glUniform4f(uniLoc_materialDiffuse,
					0.5f,
					0.5f,
					0.5f,
					::g_vecGameObject[index]->diffuseColour.a);
			}
			else
			{
				glUniform4f(uniLoc_materialDiffuse,
					::g_vecGameObject[index]->diffuseColour.r,
					::g_vecGameObject[index]->diffuseColour.g,
					::g_vecGameObject[index]->diffuseColour.b,
					::g_vecGameObject[index]->diffuseColour.a);
			}



			//        glUseProgram(program);
			::g_pShaderManager->useShaderProgram("simpleShader");

			glUniformMatrix4fv(uniLoc_mModel, 1, GL_FALSE,
				(const GLfloat*)glm::value_ptr(m));

			glUniformMatrix4fv(uniLoc_mView, 1, GL_FALSE,
				(const GLfloat*)glm::value_ptr(v));

			glUniformMatrix4fv(uniLoc_mProjection, 1, GL_FALSE,
				(const GLfloat*)glm::value_ptr(p));

			if(::g_vecGameObject[index]->wireFrame)
				glPolygonMode( GL_FRONT, GL_LINE );
			else
				glPolygonMode(GL_FRONT, GL_FILL);
			if (index < 5)
				glEnable(GL_CULL_FACE);
			else
				glDisable(GL_CULL_FACE);

			glBindVertexArray(VAODrawInfo.VAO_ID);

			glDrawElements(GL_TRIANGLES,
				VAODrawInfo.numberOfIndices,
				GL_UNSIGNED_INT,
				0);

			glBindVertexArray(0);

		}//for ( int index = 0...

		if (!drawMesh)
		{
			for (int index = 0; index < myBody->numNodes(); index++)
			{
				// Each node will be a ball
				std::string meshToDraw = "Ball";

				sVAOInfo VAODrawInfo;
				if (::g_pVAOManager->lookupVAOFromName(meshToDraw, VAODrawInfo) == false)
				{	// Didn't find mesh
					continue;
				}

				m = glm::mat4x4(1.0f);

				//Grab the position of this node
				glm::vec3 myPosition;
				myBody->getNodePosition(index, myPosition);

				glm::mat4 trans = glm::mat4x4(1.0f);
				trans = glm::translate(trans,
					myPosition);
				m = m * trans;

				//Nodes are created with a radius of 0.1f, so we must scale them down here.
				glm::mat4 matScale = glm::mat4x4(1.0f);
				matScale = glm::scale(matScale, glm::vec3(0.1f));
				m = m * matScale;

				//Leave all this the same as everything else
				p = glm::perspective(glm::radians(Camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

				// View or "camera" matrix
				glm::mat4 v = glm::mat4(1.0f);	// identity

				v = Camera.getViewMatrix();

				//Make the nodes white, I guess
				glUniform4f(uniLoc_materialDiffuse,
					1.0f,
					1.0f,
					1.0f,
					1.0f);



				//        glUseProgram(program);
				::g_pShaderManager->useShaderProgram("simpleShader");

				glUniformMatrix4fv(uniLoc_mModel, 1, GL_FALSE,
					(const GLfloat*)glm::value_ptr(m));

				glUniformMatrix4fv(uniLoc_mView, 1, GL_FALSE,
					(const GLfloat*)glm::value_ptr(v));

				glUniformMatrix4fv(uniLoc_mProjection, 1, GL_FALSE,
					(const GLfloat*)glm::value_ptr(p));

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				glBindVertexArray(VAODrawInfo.VAO_ID);

				glDrawElements(GL_TRIANGLES,
					VAODrawInfo.numberOfIndices,
					GL_UNSIGNED_INT,
					0);

				glBindVertexArray(0);
			}
		}

		else if (drawMesh)
		{
			float* springVerts = new float[myDesc.vertices.size() * 6];
			int vertsIndex = 0;
			for (int index = 0; index < myDesc.vertices.size(); index++)
			{
				glm::vec3 myPosition;
				myBody->getNodePosition(index, myPosition);
				springVerts[vertsIndex++] = myPosition.x;	//Vertex Position
				springVerts[vertsIndex++] = myPosition.y;
				springVerts[vertsIndex++] = myPosition.z;
				springVerts[vertsIndex++] = 0.8f;						//Vertex Colour (it'll be all one colour)
				springVerts[vertsIndex++] = 0.6f;
				springVerts[vertsIndex++] = 0.3f;
			}

			int* springIndices = new int[myDesc.triangulatedIndices.size()];
			for (int index = 0; index < myDesc.triangulatedIndices.size(); index++)
			{
				springIndices[index] = myDesc.triangulatedIndices[index];
			}

			::g_pShaderManager->useShaderProgram("softShader");

			glBindVertexArray(softVAO);

			glBindBuffer(GL_ARRAY_BUFFER, softVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertsIndex, springVerts, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, softEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * myDesc.triangulatedIndices.size(), springIndices, GL_STATIC_DRAW);

			GLuint vpos_location = glGetAttribLocation(softShaderID, "vPos");
			GLuint vcol_location = glGetAttribLocation(softShaderID, "vCol");

			glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(vpos_location);
			glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(vcol_location);

			glBindVertexArray(0);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			m = glm::mat4x4(1.0f);
			//Leave all this the same as everything else
			p = glm::perspective(glm::radians(Camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
			// View or "camera" matrix
			glm::mat4 v = Camera.getViewMatrix();

			glBindVertexArray(softVAO);

			glUniformMatrix4fv(soft_mModel, 1, GL_FALSE,
				(const GLfloat*)glm::value_ptr(m));

			glUniformMatrix4fv(soft_mView, 1, GL_FALSE,
				(const GLfloat*)glm::value_ptr(v));

			glUniformMatrix4fv(soft_mProjection, 1, GL_FALSE,
				(const GLfloat*)glm::value_ptr(p));

			glDrawElements(GL_TRIANGLES,
				myDesc.triangulatedIndices.size(),
				GL_UNSIGNED_INT,
				0);

			glBindVertexArray(0);
		}

		double curTime = glfwGetTime();
		deltaTime = curTime - lastTimeStep;

		::g_bigWorld->TimeStep(deltaTime);

		lastTimeStep = curTime;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}