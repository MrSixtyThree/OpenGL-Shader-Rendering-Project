#include "utilities.h"
#include "Camera.h"
#include "SimpleModel.h"
#include "Texture.h"

/*
Declan Greenwell
7557036
dmg613@uowmail.edu.au

Summary
- The texture is not rendered on the floor - I tried and you can see the code and custom shader files
- The blend factor on the torus doesnt work - No idea why. Cube mapping is working though.
- Everything else is done to requirements
*/

static void render_scene();

// global variables
// settings
unsigned int gWindowWidth = 800;
unsigned int gWindowHeight = 700;
const float gCamMoveSensitivity = 1.0f;
const float gCamRotateSensitivity = 0.1f;

// frame stats
float gFrameRate = 60.0f;
float gFrameTime = 1 / gFrameRate;

// scene content
std::map<std::string, ShaderProgram> gShader;
GLuint gVBO[2];
GLuint gVAO[2];

Camera gCamera;					// camera object

// Instantiate your Camera objects
Camera cameraTopRight, cameraBottomLeft;

std::map<std::string, glm::mat4> gModelMatrix;	// object matrix

Light gLight;					// light properties
std::map<std::string, Material>  gMaterial;		// material properties
//SimpleModel gModel;				// object model
std::map<std::string, SimpleModel> gModels;

// controls
bool gWireframe = false;	// wireframe control
bool gMultiView = false;	// multi-view control
float gAlpha = 0.5f;		// reflective amount
float gAlphaObject = 0.5f;	// reflective amount for object

std::map<std::string, Texture> gTexture;

float wallOffset = 2.5f;

// Torus rotation speed
float torusRotationSpeed = 2.0f;
float accumulatedTorusRotation = 0.0f;

Texture gCubeEnvMap;			// cube environment map

// function initialise scene and render settings
static void init(GLFWwindow* window)
{
	// set the color the color buffer should be cleared to
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	glEnable(GL_DEPTH_TEST);    // enable depth buffer test

	// compile and link shaders
	gShader["Reflection"].compileAndLink("lighting.vert", "reflection.frag");
	gShader["NormalMap"].compileAndLink("normalMap.vert", "normalMap.frag");
	gShader["CubeMap"].compileAndLink("lighting.vert", "lighting_cubemap.frag");
	gShader["Texture"].compileAndLink("lightingAndTexture.vert", "pointLightTexture.frag");

	gShader["ReflectionTexture"].compileAndLink("lightingAndTexture.vert", "reflectionTexture.frag"); // Tried to combine the shaders for reflection and texture but it didn't work

	// initialise view and projection matrices
	gCamera.setViewMatrix(glm::vec3(0.0f, 4.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	gCamera.setProjMatrix(glm::perspective(glm::radians(45.0f),
		static_cast<float>(gWindowWidth) / gWindowHeight, 0.1f, 20.0f));

	// Set up camera positions and orientations
	cameraTopRight.setViewMatrix(glm::vec3(1.0f, 7.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)); // Top right camera
	cameraTopRight.setProjMatrix(glm::perspective(glm::radians(45.0f),
		static_cast<float>(gWindowWidth) / gWindowHeight, 0.1f, 20.0f));

	cameraBottomLeft.setViewMatrix(glm::vec3(0.0f, 1.0f, 2.5f), glm::vec3(0.0f, 0.5f, 0.0f)); // Bottom left camera
	cameraBottomLeft.setProjMatrix(glm::perspective(glm::radians(55.0f),
		static_cast<float>(gWindowWidth) / gWindowHeight, 0.1f, 20.0f));

	// initialise light and material properties
	gLight.pos = glm::vec3(2.0f, 5.0f, 0.0f);
	gLight.dir = glm::vec3(0.3f, -0.7f, -0.5f);
	gLight.La = glm::vec3(0.3f);
	gLight.Ld = glm::vec3(1.0f);
	gLight.Ls = glm::vec3(1.0f);
	gLight.att = glm::vec3(1.0f, 0.0f, 0.0f);

	gMaterial["Floor"].Ka = glm::vec3(0.2f);
	gMaterial["Floor"].Kd = glm::vec3(0.2f, 0.7f, 1.0f);
	gMaterial["Floor"].Ks = glm::vec3(0.2f, 0.7f, 1.0f);
	gMaterial["Floor"].shininess = 40.0f;

	gMaterial["Cube"].Ka = glm::vec3(0.25f, 0.21f, 0.21f);
	gMaterial["Cube"].Kd = glm::vec3(1.0f, 0.83f, 0.83f);
	gMaterial["Cube"].Ks = glm::vec3(0.3f, 0.3f, 0.3f);
	gMaterial["Cube"].shininess = 11.3f;

	// initialise model matrices
	gModelMatrix["Floor"] = glm::mat4(1.0f);
	gModelMatrix["Cube"] = glm::translate(glm::vec3(-1.0f, 0.5f, 0.0f)) * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
	gModelMatrix["Torus"] = glm::translate(glm::vec3(1.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f)) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	// load model and textures
	gModels["Torus"].loadModel("./models/torus.obj");
	gTexture["Stone"].generate("./images/Fieldstone.bmp");
	gTexture["StoneNormalMap"].generate("./images/FieldstoneBumpDOT3.bmp");

	gTexture["Smile"].generate("./images/smile.bmp");
	gTexture["Check"].generate("./images/check.bmp");

	// load cube environment map texture
	gCubeEnvMap.generate(
		"./images/cm_front.bmp", "./images/cm_back.bmp",
		"./images/cm_left.bmp", "./images/cm_right.bmp",
		"./images/cm_top.bmp", "./images/cm_bottom.bmp");

	gModels["Cube"].loadModel("./models/cube.obj", true);

	// Vertex data for the floor, walls, and cube
	std::vector<GLfloat> floorVertices = {
		-2.5f, 0.0f, 2.5f,	// vertex 0: position Far left
		0.0f, 1.0f, 0.0f,	// vertex 0: normal
		0.0f, 0.0f, 		// vertex 0: texture coordinate
		2.5f, 0.0f, 2.5f,	// vertex 1: position Far right
		0.0f, 1.0f, 0.0f,	// vertex 1: normal
		1.0f, 0.0f,			// vertex 1: texture coordinate
		-2.5f, 0.0f, -2.5f,	// vertex 2: position Near left
		0.0f, 1.0f, 0.0f,	// vertex 2: normal
		0.0f, 1.0f,			// vertex 2: texture coordinate
		2.5f, 0.0f, -2.5f,	// vertex 3: position Near Right
		0.0f, 1.0f, 0.0f,	// vertex 3: normal
		1.0f, 1.0f			// vertex 3: texture coordinate
	};

	std::vector<GLfloat> wallVertices =
	{
		-1.0f, -1.0f, 0.0f,	// vertex 0: position
		0.0f, 0.0f, 1.0f,	// vertex 0: normal
		1.0f, 0.0f, 0.0f,	// vertex 0: tangent
		0.0f, 0.0f,			// vertex 0: texture coordinate
		1.0f, -1.0f, 0.0f,	// vertex 1: position
		0.0f, 0.0f, 1.0f,	// vertex 1: normal
		1.0f, 0.0f, 0.0f,	// vertex 1: tangent
		1.0f, 0.0f,			// vertex 1: texture coordinate
		-1.0f, 1.0f, 0.0f,	// vertex 2: position
		0.0f, 0.0f, 1.0f,	// vertex 2: normal
		1.0f, 0.0f, 0.0f,	// vertex 2: tangent
		0.0f, 1.0f,			// vertex 2: texture coordinate
		1.0f, 1.0f, 0.0f,	// vertex 3: position
		0.0f, 0.0f, 1.0f,	// vertex 3: normal
		1.0f, 0.0f, 0.0f,	// vertex 3: tangent
		1.0f, 1.0f,			// vertex 3: texture coordinate
	};


	// Generate and bind VAO, VBO for Floor
	glGenBuffers(1, &gVBO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * floorVertices.size(), &floorVertices[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &gVAO[0]);
	glBindVertexArray(gVAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[0]);

	GLsizei stride = 8 * sizeof(GLfloat);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);                       // Position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));   // Normal
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(GLfloat)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);  // Unbind VAO

	// Generate and bind VAO, VBO for Walls
	glGenBuffers(1, &gVBO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * wallVertices.size(), &wallVertices[0], GL_STATIC_DRAW);
	glGenVertexArrays(1, &gVAO[1]);
	glBindVertexArray(gVAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, position)));		// specify format of position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, normal)));		// specify format of colour data
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, tangent)));		// specify format of tangent data
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, texCoord)));		// specify format of texture coordinate data
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glBindVertexArray(0);  // Unbind VAO

	// Generate and bind VAO, VBO for Cube
	/*
	
	glGenBuffers(1, &gVBO[2]);
	glGenVertexArrays(1, &gVAO[2]);
	glBindVertexArray(gVAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * cubeVertices.size(), &cubeVertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);  // Unbind VAO */
}


// function used to update the scene
static void update_scene(GLFWwindow* window)
{
	// stores camera forward/back, up/down and left/right movements
	float moveForward = 0.0f;
	float moveRight = 0.0f;

	// update movement variables based on keyboard input
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveForward += gCamMoveSensitivity * gFrameTime;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveForward -= gCamMoveSensitivity * gFrameTime;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveRight -= gCamMoveSensitivity * gFrameTime;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveRight += gCamMoveSensitivity * gFrameTime;

	// update camera position and direction
	gCamera.update(moveForward, moveRight);

	// Rotate Torus on Y-Axis based on Torus Rotation Speed
	gModelMatrix["Torus"] *= glm::rotate(glm::radians(torusRotationSpeed), glm::vec3(0.0f, 0.0f, 1.0f));
	
}

void draw_floor(float alpha, Camera camera)
{
	// use the shaders associated with the shader program
	gShader["Reflection"].use();

	// set light properties
	gShader["Reflection"].setUniform("uLight.pos", gLight.pos);
	gShader["Reflection"].setUniform("uLight.La", gLight.La);
	gShader["Reflection"].setUniform("uLight.Ld", gLight.Ld);
	gShader["Reflection"].setUniform("uLight.Ls", gLight.Ls);
	gShader["Reflection"].setUniform("uLight.att", gLight.att);

	// set viewing position
	gShader["Reflection"].setUniform("uViewpoint", camera.getPosition());

	// set material properties
	gShader["Reflection"].setUniform("uMaterial.Ka", gMaterial["Floor"].Ka);
	gShader["Reflection"].setUniform("uMaterial.Kd", gMaterial["Floor"].Kd);
	gShader["Reflection"].setUniform("uMaterial.Ks", gMaterial["Floor"].Ks);
	gShader["Reflection"].setUniform("uMaterial.shininess", gMaterial["Floor"].shininess);

	// calculate matrices
	glm::mat4 MVP = camera.getProjMatrix() * camera.getViewMatrix() * gModelMatrix["Floor"];
	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["Floor"])));

	// set uniform variables
	gShader["Reflection"].setUniform("uModelViewProjectionMatrix", MVP);
	gShader["Reflection"].setUniform("uModelMatrix", gModelMatrix["Floor"]);
	gShader["Reflection"].setUniform("uNormalMatrix", normalMatrix);

	// set blending amount
	gShader["Reflection"].setUniform("uAlpha", alpha);

	gShader["Reflection"].setUniform("uTextureSampler", 0);
	glActiveTexture(GL_TEXTURE0);
	gTexture["Check"].bind();

	glBindVertexArray(gVAO[0]);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the verticess

	/*
	// use the shaders associated with the shader program
	gShader["Texture"].use();

	// set light properties
	gShader["Texture"].setUniform("uLight.pos", gLight.pos);
	gShader["Texture"].setUniform("uLight.La", gLight.La);
	gShader["Texture"].setUniform("uLight.Ld", gLight.Ld);
	gShader["Texture"].setUniform("uLight.Ls", gLight.Ls);
	gShader["Texture"].setUniform("uLight.att", gLight.att);

	// set viewing position
	gShader["Texture"].setUniform("uViewpoint", camera.getPosition());

	// set material properties
	gShader["Texture"].setUniform("uMaterial.Ka", gMaterial["Floor"].Ka);
	gShader["Texture"].setUniform("uMaterial.Kd", gMaterial["Floor"].Kd);
	gShader["Texture"].setUniform("uMaterial.Ks", gMaterial["Floor"].Ks);
	gShader["Texture"].setUniform("uMaterial.shininess", gMaterial["Floor"].shininess);

	// calculate matrices
	MVP = camera.getProjMatrix() * camera.getViewMatrix() * gModelMatrix["Floor"];
	normalMatrix = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["Floor"])));

	// set uniform variables
	gShader["Texture"].setUniform("uModelViewProjectionMatrix", MVP);
	gShader["Texture"].setUniform("uModelMatrix", gModelMatrix["Floor"]);
	gShader["Texture"].setUniform("uNormalMatrix", normalMatrix);

	// set blending amount
	gShader["Texture"].setUniform("uAlpha", alpha);

	// set textures
	gShader["Texture"].setUniform("uTextureSampler", 0);
	glActiveTexture(GL_TEXTURE0);
	gTexture["Check"].bind();

	glBindVertexArray(gVAO[0]);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertices
	*/
}

void draw_walls(glm::mat4 reflectMatrix, glm::mat4 MVP, glm::mat3 normalMatrix, glm::mat4 modelMatrix, Camera camera) {
	gShader["NormalMap"].use();

	// set light properties
	gShader["NormalMap"].setUniform("uLight.pos", gLight.pos);
	gShader["NormalMap"].setUniform("uLight.La", gLight.La);
	gShader["NormalMap"].setUniform("uLight.Ld", gLight.Ld);
	gShader["NormalMap"].setUniform("uLight.Ls", gLight.Ls);
	gShader["NormalMap"].setUniform("uLight.att", gLight.att);

	// set material properties
	gShader["NormalMap"].setUniform("uMaterial.Ka", gMaterial["Floor"].Ka);
	gShader["NormalMap"].setUniform("uMaterial.Kd", gMaterial["Floor"].Kd);
	gShader["NormalMap"].setUniform("uMaterial.Ks", gMaterial["Floor"].Ks);
	gShader["NormalMap"].setUniform("uMaterial.shininess", gMaterial["Floor"].shininess);

	// set viewing position
	gShader["NormalMap"].setUniform("uViewpoint", camera.getPosition());

	glBindVertexArray(gVAO[1]);
	
	for (int i = 0; i < 4; i++) {

		glm::mat4 wallMatrix = glm::mat4(1.0f);

		if (i == 3) {
			wallMatrix *= glm::translate(glm::vec3(wallOffset, 1.0f, 0.0f));
			wallMatrix *= glm::scale(glm::vec3(1.0f, 1.0f, wallOffset));
		}
		else if (i == 2) {
			wallMatrix *= glm::translate(glm::vec3(0.0f, 1.0f, -wallOffset));
			wallMatrix *= glm::scale(glm::vec3(wallOffset, 1.0f, 1.0f));
		}
		else if (i == 1) {
			wallMatrix *= glm::translate(glm::vec3(-wallOffset, 1.0f, 0.0f));
			wallMatrix *= glm::scale(glm::vec3(1.0f, 1.0f, wallOffset));
		}
		else {
			wallMatrix *= glm::translate(glm::vec3(0.0f, 1.0f, wallOffset));
			wallMatrix *= glm::scale(glm::vec3(wallOffset, 1.0f, 1.0f));
		}

		wallMatrix *= glm::rotate(glm::radians(90.0f * i), glm::vec3(0.0f, 1.0f, 0.0f));

		modelMatrix = reflectMatrix * wallMatrix;
		MVP = camera.getProjMatrix() * camera.getViewMatrix() * modelMatrix;
		normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));

		gShader["NormalMap"].setUniform("uModelViewProjectionMatrix", MVP);
		gShader["NormalMap"].setUniform("uModelMatrix", modelMatrix);
		gShader["NormalMap"].setUniform("uNormalMatrix", normalMatrix);

		gShader["NormalMap"].setUniform("uTextureSampler", 0);
		gShader["NormalMap"].setUniform("uNormalSampler", 1);

		glActiveTexture(GL_TEXTURE0);
		gTexture["Stone"].bind();

		glActiveTexture(GL_TEXTURE1);
		gTexture["StoneNormalMap"].bind();

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	// set textures
	glActiveTexture(GL_TEXTURE0);
	gTexture["Stone"].bind(); 

	glActiveTexture(GL_TEXTURE1);
	gTexture["StoneNormalMap"].bind();

	glBindVertexArray(gVAO[1]);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertices
}

void draw_objects(bool reflection, float torusAlpha, Camera camera)
{
	glm::mat4 MVP = glm::mat4(1.0f);
	glm::mat3 normalMatrix = glm::mat3(1.0f);
	glm::mat4 reflectMatrix = glm::mat4(1.0f);
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::vec3 lightPosition = gLight.pos;

	if (reflection)
	{
		// create reflection matrix about the horizontal plane
		reflectMatrix = glm::scale(glm::vec3(1.0f, -1.0f, 1.0f));
		// reposition the point light when rendering the reflection
		lightPosition = glm::vec3(reflectMatrix * glm::vec4(lightPosition, 1.0f));
	}

	// use the shaders associated with the shader program
	gShader["Texture"].use();
	
	// set light properties
	gShader["Texture"].setUniform("uLight.pos", lightPosition);
	gShader["Texture"].setUniform("uLight.La", gLight.La);
	gShader["Texture"].setUniform("uLight.Ld", gLight.Ld);
	gShader["Texture"].setUniform("uLight.Ls", gLight.Ls);
	gShader["Texture"].setUniform("uLight.att", gLight.att);

	// set viewing position
	gShader["Texture"].setUniform("uViewpoint", camera.getPosition());

	// set material properties
	gShader["Texture"].setUniform("uMaterial.Ka", gMaterial["Cube"].Ka);
	gShader["Texture"].setUniform("uMaterial.Kd", gMaterial["Cube"].Kd);
	gShader["Texture"].setUniform("uMaterial.Ks", gMaterial["Cube"].Ks);
	gShader["Texture"].setUniform("uMaterial.shininess", gMaterial["Cube"].shininess);

	// calculate matrices
	modelMatrix = reflectMatrix * gModelMatrix["Cube"];
	MVP = camera.getProjMatrix() * camera.getViewMatrix() * modelMatrix;
	normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));

	// set uniform variables
	gShader["Texture"].setUniform("uModelViewProjectionMatrix", MVP);
	gShader["Texture"].setUniform("uModelMatrix", modelMatrix);
	gShader["Texture"].setUniform("uNormalMatrix", normalMatrix);

	//gShader["Reflection"].setUniform("uAlpha", 1.0f);

	gShader["Texture"].setUniform("uTextureSampler", 0);
	gShader["Texture"].setUniform("uTextureSampler2", 1);
	gShader["Texture"].setUniform("uFactor", 0.5f);

	//gShader["Texture"].setUniform("uAlpha", gAlphaObject);

	// set textures
	glActiveTexture(GL_TEXTURE0);
	gTexture["Smile"].bind();

	// draw Cube
	gModels["Cube"].drawModel();

	// Change Matrixes for Torus

	gShader["CubeMap"].use();

	// set light properties
	gShader["CubeMap"].setUniform("uLight.dir", gLight.dir);
	gShader["CubeMap"].setUniform("uLight.La", gLight.La);
	gShader["CubeMap"].setUniform("uLight.Ld", gLight.Ld);
	gShader["CubeMap"].setUniform("uLight.Ls", gLight.Ls);

	// set material properties
	gShader["CubeMap"].setUniform("uMaterial.Ka", gMaterial["Floor"].Ka);
	gShader["CubeMap"].setUniform("uMaterial.Kd", gMaterial["Floor"].Kd);
	gShader["CubeMap"].setUniform("uMaterial.Ks", gMaterial["Floor"].Ks);
	gShader["CubeMap"].setUniform("uMaterial.shininess", 40.0f);

	// set viewing position
	gShader["CubeMap"].setUniform("uViewpoint", camera.getPosition());

	modelMatrix = reflectMatrix * gModelMatrix["Torus"];
	MVP = camera.getProjMatrix() * camera.getViewMatrix() * modelMatrix;
	normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));

	// set uniform variables
	gShader["CubeMap"].setUniform("uModelViewProjectionMatrix", MVP);
	gShader["CubeMap"].setUniform("uModelMatrix", modelMatrix);
	gShader["CubeMap"].setUniform("uNormalMatrix", normalMatrix);

	// set blending amount
	gShader["CubeMap"].setUniform("uAlpha", torusAlpha);

	// set cube environment map
	gShader["CubeMap"].setUniform("uEnvironmentMap", 0);

	glActiveTexture(GL_TEXTURE0);
	gCubeEnvMap.bind();

	gModels["Torus"].drawModel();
	draw_walls(reflectMatrix, MVP, normalMatrix, modelMatrix, camera);
}


// function to render the scene
static void render_scene(Camera camera)
{
	/************************************************************************************
	 * Clear colour buffer, depth buffer and stencil buffer
	 ************************************************************************************/
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/************************************************************************************
	 * Disable colour buffer and depth buffer, and draw reflective surface into stencil buffer
	 ************************************************************************************/
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);  // disable any modification to all colour components
	glDepthMask(GL_FALSE);                                // disable any modification to depth value
	glEnable(GL_STENCIL_TEST);                            // enable stencil testing

	// setup the stencil buffer with a reference value
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	// draw the reflective surface into the stencil buffer
	draw_floor(1.0f, camera);

	/************************************************************************************
	 * Enable colour buffer and depth buffer, draw reflected geometry where stencil test passes
	 ************************************************************************************/
	// only render where stencil buffer equals to 1
	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);   // allow all colour components to be modified 
	glDepthMask(GL_TRUE);                              // allow depth value to be modified

	// draw the reflected objects
	draw_objects(true, 1.0f, camera);

	glDisable(GL_STENCIL_TEST);		// disable stencil testing

	/************************************************************************************
	 * Draw the scene
	 ************************************************************************************/
	// draw reflective surface by blending with reflection
	glEnable(GL_BLEND);		//enable blending            
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	// blend reflective surface with reflection
	draw_floor(gAlpha, camera);

	glDisable(GL_BLEND);	//disable blending

	// draw the normal scene
	draw_objects(false, gAlphaObject, camera);

	// flush the graphics pipeline
	glFlush();
}

// key press or release callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// close the window when the ESCAPE key is pressed
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
}

// mouse movement callback function
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// pass cursor position to tweak bar
	TwEventMousePosGLFW(static_cast<int>(xpos), static_cast<int>(ypos));

	// previous cursor coordinates
	static glm::vec2 previousPos = glm::vec2(xpos, ypos);
	static int counter = 0;

	// allow camera rotation when right mouse button held
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		// stablise cursor coordinates for 5 updates
		if (counter < 5)
		{
			// set previous cursor coordinates
			previousPos = glm::vec2(xpos, ypos);
			counter++;
		}

		// change based on previous cursor coordinates
		float deltaYaw = (previousPos.x - xpos) * gCamRotateSensitivity * gFrameTime;
		float deltaPitch = (previousPos.y - ypos) * gCamRotateSensitivity * gFrameTime;

		// update camera's yaw and pitch
		gCamera.updateRotation(deltaYaw, deltaPitch);

		// set previous cursor coordinates
		previousPos = glm::vec2(xpos, ypos);
	}
	else
	{
		counter = 0;
	}
}

// mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// pass mouse button status to tweak bar
	TwEventMouseButtonGLFW(button, action);
}

// error callback function
static void error_callback(int error, const char* description)
{
	std::cerr << description << std::endl;	// output error description
}

// create and populate tweak bar elements
TwBar* create_UI(const std::string name)
{
	// create a tweak bar
	TwBar* twBar = TwNewBar(name.c_str());

	// give tweak bar the size of graphics window
	TwWindowSize(gWindowWidth, gWindowHeight);
	TwDefine(" TW_HELP visible=false ");	// disable help menu
	TwDefine(" GLOBAL fontsize=3 ");		// set large font size

	TwDefine(" Main label='User Interface' refresh=0.02 text=light size='250 300' position='10 10' ");

	// create frame stat entries
	TwAddVarRO(twBar, "Frame Rate", TW_TYPE_FLOAT, &gFrameRate, " group='Frame Stats' precision=2 ");
	TwAddVarRO(twBar, "Frame Time", TW_TYPE_FLOAT, &gFrameTime, " group='Frame Stats' ");

	// scene controls
	TwAddVarRW(twBar, "Wireframe", TW_TYPE_BOOLCPP, &gWireframe, " group='Controls' ");
	TwAddVarRW(twBar, "MultiView", TW_TYPE_BOOLCPP, &gMultiView, " group='Controls' ");

	// light control
	TwAddVarRW(twBar, "Position X", TW_TYPE_FLOAT, &gLight.pos.x, " group='Light' min=-3 max=3 step=0.01 ");
	TwAddVarRW(twBar, "Position Y", TW_TYPE_FLOAT, &gLight.pos.y, " group='Light' min=-3 max=5 step=0.01 ");
	TwAddVarRW(twBar, "Position Z", TW_TYPE_FLOAT, &gLight.pos.z, " group='Light' min=-3 max=3 step=0.01 ");

	// Transformation of Torus Rotation
	TwAddVarRW(twBar, "Rotation Speed", TW_TYPE_FLOAT, &torusRotationSpeed, " group='Transformation' min=-2.0 max=2 step=0.1 ");

	// reflective amount
	TwAddVarRW(twBar, "Floor", TW_TYPE_FLOAT, &gAlpha, " group='Reflection' min=0.2 max=1 step=0.01 ");
	TwAddVarRW(twBar, "Object", TW_TYPE_FLOAT, &gAlphaObject, " group='Reflection' min=0.2 max=1 step=0.01");


	return twBar;
}

int main(void)
{
	GLFWwindow* window = nullptr;	// GLFW window handle

	glfwSetErrorCallback(error_callback);	// set GLFW error callback function

	// initialise GLFW
	if (!glfwInit())
	{
		// if failed to initialise GLFW
		exit(EXIT_FAILURE);
	}

	// minimum OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window and its OpenGL context
	window = glfwCreateWindow(gWindowWidth, gWindowHeight, "OpenGL Project by Declan Greenwell", nullptr, nullptr);

	// check if window created successfully
	if (window == nullptr)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);	// set window context as the current context
	glfwSwapInterval(1);			// swap buffer interval

	// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		std::cerr << "GLEW initialisation failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	// set GLFW callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// initialise scene and render settings
	init(window);

	// initialise AntTweakBar
	TwInit(TW_OPENGL_CORE, nullptr);
	TwBar* tweakBar = create_UI("Main");		// create and populate tweak bar elements

	// timing data
	double lastUpdateTime = glfwGetTime();	// last update time
	double elapsedTime = lastUpdateTime;	// time since last update
	int frameCount = 0;						// number of frames since last update

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		update_scene(window);	// update the scene

		// if wireframe set polygon render mode to wireframe
		if (gWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//render_scene();			// render the scene

		// Multi viewport rendering
		if (gMultiView)
		{
			/************************************************************************************
			* Clear colour buffer, depth buffer and stencil buffer
			 ************************************************************************************/
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			// Top-right viewport
			glViewport(gWindowWidth / 2, gWindowHeight / 2, gWindowWidth / 2, gWindowHeight / 2);
			render_scene(cameraTopRight);

			// Bottom-left viewport
			glViewport(0, 0, gWindowWidth / 2, gWindowHeight / 2);
			render_scene(cameraBottomLeft);

			glViewport(gWindowWidth / 2, 0, gWindowWidth / 2, gWindowHeight / 2);
			render_scene(gCamera);

		}
		else
		{
			/************************************************************************************
			 * Clear colour buffer, depth buffer and stencil buffer
			************************************************************************************/
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			// Fullscreen viewport for normal camera
			glViewport(0, 0, gWindowWidth, gWindowHeight);
			render_scene(gCamera);
			
		}

		// set polygon render mode to fill
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		TwDraw();				// draw tweak bar

		glfwSwapBuffers(window);	// swap buffers
		glfwPollEvents();			// poll for events

		frameCount++;
		elapsedTime = glfwGetTime() - lastUpdateTime;	// time since last update

		// if elapsed time since last update > 1 second
		if (elapsedTime > 1.0)
		{
			gFrameTime = elapsedTime / frameCount;	// average time per frame
			gFrameRate = 1 / gFrameTime;			// frames per second
			lastUpdateTime = glfwGetTime();			// set last update time to current time
			frameCount = 0;							// reset frame counter
		}
	}

	// clean up
	glDeleteBuffers(1, gVBO);
	glDeleteVertexArrays(1, gVAO);

	// uninitialise tweak bar
	TwDeleteBar(tweakBar);
	TwTerminate();

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}