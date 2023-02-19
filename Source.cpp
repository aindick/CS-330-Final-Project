/*
* Alexis Indick
* Final Project
* CS 330
* 2/19/23
*/


#define GLFW_INCLUDE_NONE
#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"   // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"


using namespace std; // Uses the standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "Final Project -- Alexis Indick"; // Macro for window title

	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 900;


	
	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;
		GLuint vbo; // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nVertices;   // Number of vertices of the mesh
		GLuint nIndices;
		GLuint indexBuffer; // index buffer for indexed rendering data
		GLuint uvBuffer; // buffer for UV data
		GLuint normalBuffer; // buffer for normal data
	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	
	GLMesh gPlaneMesh; // for plane
	GLMesh gCubeAMesh; // Cube A for sticky notes
	GLMesh gCupMesh; // For the Dunkin cup 
	GLMesh gLipbalmMesh; // For lip balm shape
	GLMesh gStrawMesh;  // For the straw in the cup
	GLMesh gBottomLipBalmMesh; // For the bottom torus of the lip balm
	GLMesh gCupLidMesh;
	GLMesh gLotionTube;// For the lotion
	GLMesh gLotionBottom; // Bottom of lotion bottle
	GLMesh gCupBottom; // Silver metal bottom for cup
	GLMesh spotLightMesh;
	GLMesh keyLightMesh;
	

	// Texture id, Desk
	GLuint gTextureDesk;
	//Sticky Note texture
	GLuint gTextureStickyNote;
	//Dunkin Cup Texture
	GLuint gTextureCup;
	//Texture for lip of cup
	GLuint gTextureLid;
	// For Straw Texture
	GLuint gTextureStraw;
	//for lipbalm texture
	GLuint gTextureBalm;
	// Texture for bottom of lip balm
	GLuint gTextureBottomLipBalm;
	//Texture for lotion
	GLuint gTextureLotion;
	// For bottom of lotion bottle
	GLuint gTextureBottomofLotion;
	//Texture for the bottom of the cup
	GLuint gTextureCupBottom;



	// Shader program
	GLuint gProgramId;
	glm::vec2 gUVScale(5.0f, 5.0f);
	GLuint gLightProgramId;
	

	// Subject position and scale
	glm::vec3 gPosition(0.0f, 0.0f, 0.0f);
	glm::vec3 gPositionScale(2.0f);

	// color for light and object
	glm::vec3 gObjectColor(1.0f, 0.2f, 0.0f);
	glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);

	// Light color, position and scale
	glm::vec3 gSpotLightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 gSpotLightPosition(-7.0f, -4.0f, -1.0f);
	glm::vec3 gSpotLightScale(0.3f);

	// Light color, position and scale
	glm::vec3 gKeyLightColor(0.0f, 0.0f, 0.0f);
	glm::vec3 gKeyLightPosition(0.0f, 3.0f, -1.5f);
	glm::vec3 gKeyLightScale(0.2f);

	

	// variable to handle ortho and perspective
	bool perspective = false;
	//cylinder
	int numVerticesSide;		// Vertices to render side of the cylinder
	int numVerticesTopBottom;	// Vertices to render top / bottom of the cylinder
	int numVerticesTotal;		// Sum of both vertices


	// camera
	Camera gCamera(glm::vec3(0.0f, 0.0f,5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -30.0f);
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;


	// timing
	float gDeltaTime = 0.0f; // time between current frame and last frame
	float gLastFrame = 0.0f;

}



/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
	layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f);
	vertexFragmentPos = vec3(model * vec4(position, 1.0f));
	vertexNormal = mat3(transpose(inverse(model))) * normal;
	vertexTextureCoordinate = textureCoordinate;

}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
	in vec3 vertexFragmentPos;
in vec3 vertexNormal;
in vec2 vertexTextureCoordinate; // for texture coordinates, not color

out vec4 fragmentColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 keyLightColor;
uniform vec3 lightPos;
uniform vec3 keyLightPos;
uniform vec3 viewPosition;

uniform sampler2D uTexture;
uniform vec2 uvScale;

void main()
{
	//Calculate Ambient lighting*/
	float spotStrength = 0.7f; // Set ambient or global lighting strength
	float keyStrength = 0.8f; // Set ambient or global lighting strength
	vec3 spot = spotStrength * lightColor; // Generate ambient light color
	vec3 key = keyStrength * keyLightColor;

	//Calculate Diffuse lighting*/
	vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
	vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	vec3 keyLightDirection = normalize(keyLightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube

	float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	float keyImpact = max(dot(norm, keyLightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light

	vec3 diffuse = impact * lightColor; // Generate diffuse light color
	vec3 keyDiffuse = keyImpact * keyLightColor;

	//Calculate Specular lighting*/
	float specularIntensity = 0.5f; // Set specular light strength
	float highlightSize = 16.0f; // Set specular highlight size
	vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
	vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
	vec3 specular = specularIntensity * specularComponent * lightColor;
	vec3 keySpecular = specularIntensity * specularComponent * keyLightColor;

	// Texture holds the color to be used for all three components
	vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

	// Calculate phong result
	vec3 phong = (spot + key + diffuse + keyDiffuse + specular /*+ objectColor*/) * textureColor.xyz;

	fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU

}
);
/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

	layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);

// Light Fragment Shader Source Code
const GLchar* lampFragmentShaderSource = GLSL(440,

	out vec4 fragmentColor; // For outgoing light color to the GPU

void main()
{
	fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
	for (int j = 0; j < height / 2; ++j)
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)
		{
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			++index1;
			++index2;
		}
	}
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreatePlaneMesh(GLMesh& mesh);
void UCreateCubeAMesh(GLMesh& mesh);
void UCreateCylinderDunkinMesh(GLMesh& mesh, float topRadius, float bottomRadius, int numSlices, float height, bool topCircle, bool bottomCircle);
void UCreateLidOfDunkinCup(GLMesh& mesh, int mainSegments, int tubeSegments, float mainRadius, float tubeRadius);
void UCreateLipBalmMesh(GLMesh& mesh, float topRadius2, float bottomRadius2, int numSlices2, float height2, bool topCircle2, bool bottomCircle2);
void UCreateBottomOfLipBalm(GLMesh& mesh,float topRadius, float bottomRadius, int numSlices, float height, bool topCircle, bool bottomCircle);
void UCreateStrawMesh(GLMesh& mesh, float topRadius3, float bottomRadius3, int numSlices3, float height3, bool topCircle3, bool bottomCircle3);
void URender();
void UDestroyMesh(GLMesh& mesh);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void UCreateLightMesh(GLMesh& lightMesh);
void UCreateLotionMesh(GLMesh& mesh);
void UCreateLotionBottomMesh(GLMesh& mesh, int mainSegments, int tubeSegments, float mainRadius, float tubeRadius);
void UCreateCupBottom(GLMesh& mesh, float topRadius4, float bottomRadius4, int numSlices4, float height4, bool topCircle4, bool bottomCircle4);

// main function. Entry point to the OpenGL program
int main(int argc, char* argv[])
{
	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;
	

	// Create the mesh
	UCreatePlaneMesh(gPlaneMesh); // Calls the function to create the Vertex Buffer Object
	UCreateCubeAMesh(gCubeAMesh); // For the sticky notes
	UCreateCylinderDunkinMesh(gCupMesh, 0.5f, 0.5f, 10, 2.8f, true, true); // For dunkin cup
	UCreateLipBalmMesh(gLipbalmMesh, 0.11f, 0.11f, 10, 1.0f, true, true); // For lipbalm
	UCreateBottomOfLipBalm(gBottomLipBalmMesh, 0.12f, 0.12f, 10, 0.2f, true, true);
	UCreateStrawMesh(gStrawMesh, 0.1f, 0.1f, 10, 3.4f, false, true); //For Straw
	UCreateLidOfDunkinCup(gCupLidMesh, 10, 10, 0.5f, 0.05f); // Cup lid mesh
	UCreateLotionBottomMesh(gLotionBottom, 15, 15, 0.05f, 0.05f); // Lotion bottle bottom mesh
	UCreateLotionMesh(gLotionTube); // Lotion bottle mesh
	UCreateCupBottom(gCupBottom, 0.5f, 0.5f, 10, 0.4f, true, true); //Bottom of cup

	// Create the shader program
	if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
		return EXIT_FAILURE;

	// Load texture (relative to project's directory)
	const char* texFilename = "C:\\Users\\alexi\\OneDrive\\Documents\\CS 330 Project Scene Alexis Indick\\desk.jpg";
	if (!UCreateTexture(texFilename, gTextureDesk))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}
	// Load texture (relative to project's directory)
	const char* texFilename2 = "C:\\Users\\alexi\\OneDrive\\Documents\\CS 330 Project Scene Alexis Indick\\blackmarble.jpg";
	if (!UCreateTexture(texFilename2, gTextureCup))
	{
		cout << "Failed to load texture " << texFilename2 << endl;
		return EXIT_FAILURE;
	}
	// Load texture (relative to project's directory)
	const char* texFilename3 = "C:\\Users\\alexi\\OneDrive\\Documents\\CS 330 Project Scene Alexis Indick\\paperyellow.jpg";
	if (!UCreateTexture(texFilename3, gTextureStickyNote))
	{
		cout << "Failed to load texture " << texFilename3 << endl;
		return EXIT_FAILURE;
	}
	// Load texture(relative to project's directory)
		const char* texFilename4 = "C:\\Users\\alexi\\OneDrive\\Documents\\CS 330 Project Scene Alexis Indick\\blueLipBalm.jpg";
	if (!UCreateTexture(texFilename4, gTextureBalm))
	{
		cout << "Failed to load texture " << texFilename4 << endl;
		return EXIT_FAILURE;
	}
	// Load texture(relative to project's directory)
	const char* texFilename5 = "C:\\Users\\alexi\\OneDrive\\Documents\\CS 330 Project Scene Alexis Indick\\straw.jpg";
	if (!UCreateTexture(texFilename5, gTextureStraw))
	{
		cout << "Failed to load texture " << texFilename5 << endl;
		return EXIT_FAILURE;
	}
	// Load texture(relative to project's directory)
	const char* texFilename6 = "C:\\Users\\alexi\\OneDrive\\Documents\\CS 330 Project Scene Alexis Indick\\lipbalm&Lotion.jpg";
	if (!UCreateTexture(texFilename6, gTextureBottomLipBalm))
	{
		cout << "Failed to load texture " << texFilename6 << endl;
		return EXIT_FAILURE;
	}
	// Load texture(relative to project's directory)
	const char* texFilename7 = "C:\\Users\\alexi\\OneDrive\\Documents\\CS 330 Project Scene Alexis Indick\\lid.jpg";
	if (!UCreateTexture(texFilename7, gTextureLid))
	{
		cout << "Failed to load texture " << texFilename7 << endl;
		return EXIT_FAILURE;
	}
	// Load texture(relative to project's directory)
	const char* texFilename8 = "C:\\Users\\alexi\\OneDrive\\Documents\\CS 330 Project Scene Alexis Indick\\lipbalm&Lotion.jpg";
	if (!UCreateTexture(texFilename8, gTextureBottomofLotion))
	{
		cout << "Failed to load texture " << texFilename8 << endl;
		return EXIT_FAILURE;
	}
	// Load texture(relative to project's directory)
	const char* texFilename9 = "C:\\Users\\alexi\\OneDrive\\Documents\\CS 330 Project Scene Alexis Indick\\lotionTexture.jpg";
	if (!UCreateTexture(texFilename9, gTextureLotion))
	{
		cout << "Failed to load texture " << texFilename9 << endl;
		return EXIT_FAILURE;
	}
	// Load texture(relative to project's directory)
	const char* texFilename10 = "C:\\Users\\alexi\\OneDrive\\Documents\\CS 330 Project Scene Alexis Indick\\cupBottom.jpg";
	if (!UCreateTexture(texFilename10, gTextureCupBottom))
	{
		cout << "Failed to load texture " << texFilename10 << endl;
		return EXIT_FAILURE;
	}
	if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLightProgramId))
		return EXIT_FAILURE;


	// Create Light Object
	UCreateLightMesh(spotLightMesh);
	UCreateLightMesh(keyLightMesh);
	
	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	glUseProgram(gProgramId);

	// We set the texture as texture unit 0
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
	
	

	// Sets the background color of the window to black (it will be implicitely used by glClear)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	

	// render loop
	// -----------
	while (!glfwWindowShouldClose(gWindow))
	{
		
		// per-frame timing
	   // --------------------
		float currentFrame = glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;

	

		// input
		// -----
		UProcessInput(gWindow);

		URender();

		glfwPollEvents();
	}

	// Release mesh data
	UDestroyMesh(gPlaneMesh);
	UDestroyMesh(gCubeAMesh);
	UDestroyMesh(gCupMesh);
	UDestroyMesh(gLipbalmMesh);
	UDestroyMesh(gStrawMesh);
	UDestroyMesh(gBottomLipBalmMesh);
	UDestroyMesh(gCupLidMesh);
	UDestroyMesh(gLotionTube);
	UDestroyMesh(gLotionBottom);
	UDestroyMesh(gCupBottom);

	// Release texture
	UDestroyTexture(gTextureDesk);
	UDestroyTexture(gTextureCup);
	UDestroyTexture(gTextureStickyNote);
	UDestroyTexture(gTextureBalm);
	UDestroyTexture(gTextureStraw);
	UDestroyTexture(gTextureBottomLipBalm);
	UDestroyTexture(gTextureLid);
	UDestroyTexture(gTextureLotion);
	UDestroyTexture(gTextureBottomofLotion);
	UDestroyTexture(gTextureCupBottom);
	// Release shader program
	UDestroyShaderProgram(gProgramId);
	UDestroyShaderProgram(gLightProgramId);
	
	exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	// ---------------------
	* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);
	glfwSetCursorPosCallback(*window, UMousePositionCallback);
	glfwSetScrollCallback(*window, UMouseScrollCallback);
	glfwSetMouseButtonCallback(*window, UMouseButtonCallback);
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();
	if (GLEW_OK != GlewInitResult)
	{
		std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
		return false;
	}

	// Displays GPU OpenGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

	return true;
}



// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
	static const float cameraSpeed = 2.5f;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.ProcessKeyboard(UP, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.ProcessKeyboard(DOWN, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		perspective = false;
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		perspective = true;

}

void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (gFirstMouse)
	{
		gLastX = xpos;
		gLastY = ypos;
		gFirstMouse = false;
	}

	float xoffset = xpos - gLastX;
	float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

	gLastX = xpos;
	gLastY = ypos;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
}


void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{

	gCamera.ProcessMouseScroll(yoffset);
}
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
	{
		if (action == GLFW_PRESS)
			cout << "Left mouse button pressed" << endl;
		else
			cout << "Left mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_MIDDLE:
	{
		if (action == GLFW_PRESS)
			cout << "Middle mouse button pressed" << endl;
		else
			cout << "Middle mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_RIGHT:
	{
		if (action == GLFW_PRESS)
			cout << "Right mouse button pressed" << endl;
		else
			cout << "Right mouse button released" << endl;
	}
	break;

	default:
		cout << "Unhandled mouse button event" << endl;
		break;
	}
}



void URender()
{
	GLint modelLoc;
	GLint viewLoc;
	GLint projLoc;
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 translation;
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	// Model matrix: transformations are applied right-to-left order
	glm::mat4 transformation(1.0f);
	// Set the shader to be used
	glUseProgram(gProgramId);
	// Sends transform information to the Vertex shader
	GLuint transformLocation = glGetUniformLocation(gProgramId, "shaderTransform");
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transformation));
	GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

	// z depth
	glEnable(GL_DEPTH_TEST);
	//For transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Clears Background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	// camera/view transformation
     view = gCamera.GetViewMatrix();

	// create perspective projection (fov, aspect ratio, near plane, far plane)

	if (!perspective)
	{
		// p for perspective 
		projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}
	else
		// o for ortho
		projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);

	// Retrieves and passes transform matrices to the shader program
	modelLoc = glGetUniformLocation(gProgramId, "model");
	viewLoc = glGetUniformLocation(gProgramId, "view");
	projLoc = glGetUniformLocation(gProgramId, "projection");
	// Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
	GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
	GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
	GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
	GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

	// Key light
	GLint keyLightColorLoc = glGetUniformLocation(gProgramId, "keyLightColor");
	GLint keyLightPositionLoc = glGetUniformLocation(gProgramId, "keyLightPos");
	// Key Light
	glUniform3f(keyLightColorLoc, gKeyLightColor.r, gKeyLightColor.g, gKeyLightColor.b);
	glUniform3f(keyLightPositionLoc, gKeyLightPosition.x, gKeyLightPosition.y, gKeyLightPosition.z);
	// Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
	glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
	glUniform3f(lightColorLoc, gSpotLightColor.r, gSpotLightColor.g, gSpotLightColor.b);
	glUniform3f(lightPositionLoc, gSpotLightPosition.x, gSpotLightPosition.y, gSpotLightPosition.z);
	const glm::vec3 cameraPosition = gCamera.Position;
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	//Sticky note stack
	// Activate the VBOs contained within the mesh's VAO for the cube
	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureStickyNote);
	glBindVertexArray(gCubeAMesh.vao);

	// 1. Scales the object by 1
	 scale = glm::scale(glm::vec3(1.0f, 0.1f, 1.0f));
	// 2. Rotates shape by 46 degrees in the x axis
	 rotation = glm::rotate(46.0f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Place object at the origin
	 translation = glm::translate(glm::vec3(0.0f, -4.9f, 0.5f));

	 model = translation * rotation * scale; // Creates transform matrix


	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


	glDrawArrays(GL_TRIANGLES, 0, gCubeAMesh.nVertices);


	//Plane/Desk

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureDesk);
	glBindVertexArray(gPlaneMesh.vao);

	// 1. Scales the object by 1
	 scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// 2. Rotates shape by 45 degrees in the x axis
	 rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Place object at the origin
	 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

	 model = translation * rotation * scale; // Creates transform matrix

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


   glDrawArrays(GL_TRIANGLES, 0, gPlaneMesh.nVertices);

	//Cup

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureCup);
	glBindVertexArray(gCupMesh.vao);
	// 1. Scales the object by 1
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// 2. Rotates shape by 45 degrees in the x axis
	rotation = glm::rotate(0.0f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(-1.8f, -3.2f, 0.0f));

	model = translation * rotation * scale; // Creates transform matrix

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	
	// render sides of cylinder
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numVerticesSide);

	// render top circle

	glDrawArrays(GL_TRIANGLE_FAN, numVerticesSide, numVerticesTopBottom);


	// render bottom circle

	glDrawArrays(GL_TRIANGLE_FAN, numVerticesSide + numVerticesTopBottom, numVerticesTopBottom);

	//Cup Bottom

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureCupBottom);
	glBindVertexArray(gCupBottom.vao);
	// 1. Scales the object by 1
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// 2. Rotates shape by 45 degrees in the x axis
	rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(-1.8f, -4.8f, 0.0f));

	model = translation * rotation * scale; // Creates transform matrix

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


	// render sides of cylinder
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numVerticesSide);

	// render top circle

	glDrawArrays(GL_TRIANGLE_FAN, numVerticesSide, numVerticesTopBottom);


	// render bottom circle

	glDrawArrays(GL_TRIANGLE_FAN, numVerticesSide + numVerticesTopBottom, numVerticesTopBottom);

	//Lid of cup
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureLid);
	glBindVertexArray(gCupLidMesh.vao);
	// 1. Scales the object by 1
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// 2. Rotates shape by 180.1 degrees in the x axis
	rotation = glm::rotate(180.1f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(-1.8f, -1.75f, 0.0f));

	model = translation * rotation * scale; // Creates transform matrix

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


	// render sides of cylinder
	glDrawElements(GL_TRIANGLE_STRIP, gCupLidMesh.nIndices, GL_UNSIGNED_INT, 0);



	//Lipbalm tube

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureBalm);

	glBindVertexArray(gLipbalmMesh.vao);
	// 1. Scales the object by 1
	scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
	// 2. Rotates shape by 45 degrees in the x axis
	rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(-0.6f, -4.7f, 1.5f));

	model = translation * rotation * scale; // Creates transform matrix

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


	// render sides of cylinder
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numVerticesSide);

	// render top circle

	glDrawArrays(GL_TRIANGLE_FAN, numVerticesSide, numVerticesTopBottom);


	// render bottom circle

	glDrawArrays(GL_TRIANGLE_FAN, numVerticesSide + numVerticesTopBottom, numVerticesTopBottom);

	//Lip balm bottom
	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureBottomLipBalm);

	glBindVertexArray(gBottomLipBalmMesh.vao);

	// 1. Scales the object by 1
	scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
	// 2. Rotates shape by 45 degrees in the x axis
	rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(-0.6f, -4.9f, 1.5f));

	model = translation * rotation * scale; // Creates transform matrix

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// render sides of cylinder
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numVerticesSide);

	// render top circle

	glDrawArrays(GL_TRIANGLE_FAN, numVerticesSide, numVerticesTopBottom);

	// render bottom circle

	glDrawArrays(GL_TRIANGLE_FAN, numVerticesSide + numVerticesTopBottom, numVerticesTopBottom);


	//Lotion Bottom
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureBottomofLotion);
	glBindVertexArray(gLotionBottom.vao);
	// 1. Scales the object by 1
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// 2. Rotates shape by 45 degrees in the x axis
	rotation = glm::rotate(180.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(1.6f, -4.95f, 0.5f));

	model = translation * rotation * scale; // Creates transform matrix

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	
	
	// render sides of cylinder
	glDrawElements(GL_TRIANGLE_STRIP, gLotionBottom.nIndices, GL_UNSIGNED_INT, 0);
	
	
	//Lotion 
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureLotion);
	glBindVertexArray(gLotionTube.vao);
	// 1. Scales the object by 1
	scale = glm::scale(glm::vec3(-0.9f, -0.9f, 0.9f));
	// 2. Rotates shape by 45 degrees in the x axis
	rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(1.6f, -4.58f, 0.5f));

	model = translation * rotation * scale; // Creates transform matrix

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// render sides of cylinder
	glDrawArrays(GL_TRIANGLES, 0, gLotionTube.nVertices);


	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureStraw);
	//For straw
	glBindVertexArray(gStrawMesh.vao);
	// 1. Scales the object by 1
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// 2. Rotates shape by 45 degrees in the x axis
	rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(-1.8f, -2.5f, 0.0f));

	model = translation * rotation * scale; // Creates transform matrix

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));



	// render sides of cylinder
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numVerticesSide);

	// render top circle

	glDrawArrays(GL_TRIANGLE_FAN, numVerticesSide, numVerticesTopBottom);


	// render bottom circle

	glDrawArrays(GL_TRIANGLE_FAN, numVerticesSide + numVerticesTopBottom, numVerticesTopBottom);
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//Lamps 
	// Draw the Spot Light
	glUseProgram(gLightProgramId);
	glBindVertexArray(spotLightMesh.vao);

	// Light location and Scale
	 model = glm::translate(gSpotLightPosition) * glm::scale(gSpotLightScale);

	// Matrix uniforms from the Light Shader program
	 modelLoc = glGetUniformLocation(gLightProgramId, "model");
	 viewLoc = glGetUniformLocation(gLightProgramId, "view");
	 projLoc = glGetUniformLocation(gLightProgramId, "projection");

	// Matrix data
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Draw the light
	glDrawArrays(GL_TRIANGLES, 0, spotLightMesh.nVertices);



	// Draw the Key Light
	glUseProgram(gLightProgramId);
	glBindVertexArray(keyLightMesh.vao);

	// Light location and Scale
	model = glm::translate(gKeyLightPosition) * glm::scale(gKeyLightScale);

	// Matrix uniforms from the Light Shader program
	modelLoc = glGetUniformLocation(gLightProgramId, "model");
	viewLoc = glGetUniformLocation(gLightProgramId, "view");
	projLoc = glGetUniformLocation(gLightProgramId, "projection");

	// Matrix data
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Draw the light
	glDrawArrays(GL_TRIANGLES, 0, keyLightMesh.nVertices);


	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}




void UDestroyMesh(GLMesh& mesh)
{
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(1, &mesh.vbo);
}
// Template for creating a cube light
void UCreateLightMesh(GLMesh& lightMesh)
{
	// Position and Color data
	GLfloat verts[] = {
		//Positions          //Normals
		// ------------------------------------------------------
		//Back Face          //Negative Z Normal  Texture Coords.
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

	   //Front Face         //Positive Z Normal
	  -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
	   0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
	   0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
	   0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
	  -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
	  -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

	  //Left Face          //Negative X Normal
	 -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	 -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	 -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	 -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	 -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	 -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	 //Right Face         //Positive X Normal
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	 //Bottom Face        //Negative Y Normal
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

	//Top Face           //Positive Y Normal
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal = 3;
	const GLuint floatsPerUV = 2;

	lightMesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

	glGenVertexArrays(1, &lightMesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(lightMesh.vao);

	// Create 2 buffers: first one for the vertex data; second one for the indices
	glGenBuffers(1, &lightMesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, lightMesh.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	// Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);
}
void UCreateLotionMesh(GLMesh& mesh)
{
	// Vertex Data
	GLfloat verts[] = {
		//Positions         //Normals               //Texture Coordinates

		-0.4f, 0.0f, -0.4f,		0.0f, 0.0f, -1.0f,       0.0f, 0.0f,	   // Base 1
		-0.4f, 0.0f,  0.4f,		0.0f, 0.0f, -1.0f,       0.0f, 1.0f,
		 0.4f, 0.0f,  0.4f,		0.0f, 0.0f, -1.0f,       1.0f, 1.0f,

		 0.4f, 0.0f,  0.4f,		0.0f, 0.0f, 1.0f,        1.0f, 1.0f,       // Base 2
		 0.4f, 0.0f, -0.4f,		0.0f, 0.0f, 1.0f,        1.0f, 0.0f,
		-0.4f, 0.0f, -0.4f,		0.0f, 0.0f, 1.0f,        0.0f, 0.0f,

		-0.4f, 0.0f, -0.4f,	    -1.0f, 0.0f, 0.0f,       0.0f, 0.0f,       // Side 1
		-0.4f, 0.0f,  0.4f,		-1.0f, 0.0f, 0.0f,       1.0f, 0.0f,
		 0.0f, 0.4f,  0.0f,		-1.0f, 0.0f, 0.0f,       0.5f, 1.0f,

		-0.4f, 0.0f, -0.4f,		1.0f, 0.0f, 0.0f,        0.0f, 0.0f,       // Side 2
		 0.4f, 0.0f, -0.4f,		1.0f, 0.0f, 0.0f,        1.0f, 0.0f,
		 0.0f, 0.4f,  0.0f,		1.0f, 0.0f, 0.0f,        0.5f, 1.0f,

		 0.4f, 0.0f,  0.4f,		0.0f, -1.0f, 0.0f,       0.0f, 0.0f,       // Side 3
		 0.4f, 0.0f, -0.4f,		0.0f, -1.0f, 0.0f,       1.0f, 0.0f,
		 0.0f, 0.4f,  0.0f,		0.0f, -1.0f, 0.0f,       0.5f, 1.0f,

		-0.4f, 0.0f, 0.4f,		0.0f, 1.0f, 0.0f,        0.0f, 0.0f,       // Side 4
		 0.4f, 0.0f, 0.4f,		0.0f, 1.0f, 0.0f,        1.0f, 0.0f,
		 0.0f, 0.4f, 0.0f,		0.0f, 1.0f, 0.0f,        0.5f, 1.0f


	};
	// Identify how many floats for Position and Texture coordinates
	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal = 3;
	const GLuint floatsPerUV = 2;
	mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

	glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.vao);

	// Create 2 buffers: first one for the vertex data; second one for the indices
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	// Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);
}

// Implements the UCreatePlaneMesh function
void UCreatePlaneMesh(GLMesh& mesh)
{
	
	GLfloat verts[] =
	{
		//Vertex coords		//Normals		       //Texture
		
		//Lower Triangle
	   -5.0f, -5.0f, -5.0f,	 0.0f, 0.0f, -1.0f,	   0.0f, 0.5f,
		5.0f, -5.0f,-5.0f,	 0.0f, 0.0f, -1.0f,	   0.0f, 0.0f,
		5.0f,-5.0f, 5.0f,	 0.0f, 0.0f, -1.0f,	   1.0f, 0.0f,
		//Upper Triangle
		 5.0f,-5.0f, 5.0f,	0.0f,  0.0f,  -1.0f,    0.0f, 0.5f,
		-5.0f,-5.0f,5.0f,   0.0f,  0.0f,  -1.0f,    0.0f, 0.0f,
		-5.0f,-5.0f,-5.0f,	0.0f,  0.0f,  -1.0f,	1.0f, 0.0f
		
	};

	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal = 3;
	const GLuint floatsPerUV = 2;

	mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

	glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.vao);

	// Create 2 buffers: first one for the vertex data; second one for the indices
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	// Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);

}


void UCreateCubeAMesh(GLMesh& mesh) {
	GLfloat verts[] =
	{
		//Positions			//Normals				//Texture Coordinates
	 -0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f,
	  0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 0.0f,
	  0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 1.0f,
	  0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 1.0f,
	 -0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 1.0f,
	 -0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f,

	 -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 0.0f,
	  0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 0.0f,
	  0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
	  0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
	 -0.5f,  0.5f,  0.5f,	 0.0f,  0.0f,  1.0f,    0.0f, 1.0f,
	 -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 0.0f,

	 -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
	 -0.5f,  0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
	 -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
	 -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
	 -0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
	 -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

	  0.5f,  0.5f,  0.5f,      1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
	  0.5f,  0.5f, -0.5f,      1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
	  0.5f, -0.5f, -0.5f,      1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
	  0.5f, -0.5f, -0.5f,      1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
	  0.5f, -0.5f,  0.5f,      1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
	  0.5f,  0.5f,  0.5f,      1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

	 -0.5f, -0.5f, -0.5f,      0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
	  0.5f, -0.5f, -0.5f,      0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
	  0.5f, -0.5f,  0.5f,      0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
	  0.5f, -0.5f,  0.5f,      0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
	 -0.5f, -0.5f,  0.5f,      0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
	 -0.5f, -0.5f, -0.5f,      0.0f, -1.0f,  0.0f,   0.0f, 1.0f,

	 -0.5f,  0.5f, -0.5f,      0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
	  0.5f,  0.5f, -0.5f,      0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
	  0.5f,  0.5f,  0.5f,      0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
	  0.5f,  0.5f,  0.5f,      0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
	 -0.5f,  0.5f,  0.5f,      0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
	 -0.5f,  0.5f, -0.5f,      0.0f,  1.0f,  0.0f,   0.0f, 1.0f

	};
	
	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal = 3;
	const GLuint floatsPerUV = 2;

	mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

	glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.vao);

	// Create 2 buffers: first one for the vertex data; second one for the indices
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	// Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);
}
void UCreateLidOfDunkinCup(GLMesh& mesh, int mainSegments, int tubeSegments, float mainRadius, float tubeRadius)
{
	float currentMainSegmentAngle = 0.0f;
	int index = 0; // current face
	int _mainSegments;
	int _tubeSegments;
	float _mainRadius;
	float _tubeRadius;

	int _numIndices = 0;
	int _primitiveRestartIndex = 0;

	glm::vec3* vertices;
	glm::vec2* uvs;
	glm::vec3* normals;
	GLuint* indices;
	vertices = nullptr;
	normals = nullptr;
	uvs = nullptr;
	indices = nullptr;
	_mainSegments = mainSegments;
	_tubeSegments = tubeSegments;
	_mainRadius = mainRadius;
	_tubeRadius = tubeRadius;

	// Calculate and cache counts of vertices and indices
	mesh.nVertices = (_mainSegments + 1) * (_tubeSegments + 1);
	_primitiveRestartIndex = mesh.nVertices;
	_numIndices = (_mainSegments * 2 * (_tubeSegments + 1)) + _mainSegments - 1; // oh we do have the amount lol
	mesh.nIndices = _numIndices;

	// Allocate data slots:
	vertices = new glm::vec3[mesh.nVertices];
	normals = new glm::vec3[mesh.nVertices];
	uvs = new glm::vec2[mesh.nVertices];
	indices = new GLuint[_numIndices];

	auto mainSegmentAngleStep = glm::radians(360.0f / float(_mainSegments));
	auto tubeSegmentAngleStep = glm::radians(360.0f / float(_tubeSegments));

	// Vertices:
	for (auto i = 0; i <= _mainSegments; i++) {
		// Calculate sine and cosine of main segment angle
		auto sinMainSegment = sin(currentMainSegmentAngle);
		auto cosMainSegment = cos(currentMainSegmentAngle);
		auto currentTubeSegmentAngle = 0.0f;
		for (auto j = 0; j <= _tubeSegments; j++) {
			// Calculate sine and cosine of tube segment angle
			auto sinTubeSegment = sin(currentTubeSegmentAngle);
			auto cosTubeSegment = cos(currentTubeSegmentAngle);

			// Calculate vertex position on the surface of torus
			vertices[index] = glm::vec3(
				(_mainRadius + _tubeRadius * cosTubeSegment) * cosMainSegment,
				(_mainRadius + _tubeRadius * cosTubeSegment) * sinMainSegment,
				_tubeRadius * sinTubeSegment);
			++index;

			// Update current tube angle
			currentTubeSegmentAngle += tubeSegmentAngleStep;
		}

		// Update main segment angle
		currentMainSegmentAngle += mainSegmentAngleStep;
	}

	// Texture coordinates:
	auto mainSegmentTextureStep = 2.0f / float(_mainSegments);
	auto tubeSegmentTextureStep = 1.0f / float(_tubeSegments);

	index = 0; // reset index counter
	auto currentMainSegmentTexCoordV = 0.0f;
	for (auto i = 0; i <= _mainSegments; i++) {
		auto currentTubeSegmentTexCoordU = 0.0f;
		for (auto j = 0; j <= _tubeSegments; j++) {
			uvs[index] = glm::vec2(currentTubeSegmentTexCoordU, currentMainSegmentTexCoordV);
			++index;

			currentTubeSegmentTexCoordU += tubeSegmentTextureStep;
		}

		// Update texture coordinate of main segment
		currentMainSegmentTexCoordV += mainSegmentTextureStep;
	}

	// Normals:
	index = 0; // reset again
	currentMainSegmentAngle = 0.0f;
	for (auto i = 0; i <= _mainSegments; i++) {
		// Calculate sine and cosine of main segment angle
		auto sinMainSegment = sin(currentMainSegmentAngle);
		auto cosMainSegment = cos(currentMainSegmentAngle);
		auto currentTubeSegmentAngle = 0.0f;
		for (auto j = 0; j <= _tubeSegments; j++) {
			// Calculate sine and cosine of tube segment angle
			auto sinTubeSegment = sin(currentTubeSegmentAngle);
			auto cosTubeSegment = cos(currentTubeSegmentAngle);

			normals[index] = glm::vec3(cosMainSegment * cosTubeSegment, sinMainSegment * cosTubeSegment,
				sinTubeSegment);
			++index;

			// Update current tube angle
			currentTubeSegmentAngle += tubeSegmentAngleStep;
		}

		// Update main segment angle
		currentMainSegmentAngle += mainSegmentAngleStep;
	}

	// Indices:
	index = 0; // reset again
	GLuint currentVertexOffset = 0;
	for (auto i = 0; i < _mainSegments; i++) {
		for (auto j = 0; j <= _tubeSegments; j++) {
			GLuint vertexIndexA = currentVertexOffset;
			indices[index] = vertexIndexA;
			++index;

			GLuint vertexIndexB = currentVertexOffset + _tubeSegments + 1;
			indices[index] = vertexIndexB;
			++index;

			currentVertexOffset++;
		}

		// Don't restart primitive, if it's last segment, rendering ends here anyway
		if (i != _mainSegments - 1) {
			indices[index] = _primitiveRestartIndex;
			++index;
		}
	}
	glGenVertexArrays(1, &mesh.vao); // generate a single vao
	glBindVertexArray(mesh.vao);

	// Vertex buffer:
	glGenBuffers(1, &mesh.vbo); // create vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // bind VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.nVertices, vertices,
		GL_STATIC_DRAW); // send vertex data to the GPU

// Create vertex attribute:
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) nullptr); // sizeof(float) * (3)
	glEnableVertexAttribArray(0); // enable vertices

	// Index buffer:
	glGenBuffers(1, &mesh.indexBuffer); // create index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer); // bind index buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.nIndices, indices,
		GL_STATIC_DRAW); // send index data to GPU

// Normal buffer:
	glGenBuffers(1, &mesh.normalBuffer); // create normal buffer
	glBindBuffer(GL_ARRAY_BUFFER, mesh.normalBuffer); // bind normal buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.nVertices, normals,
		GL_STATIC_DRAW); // send normal data to GPU

// Create normal attribute:
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) nullptr); // sizeof(float) * (2)
	glEnableVertexAttribArray(1); // enable normals

	// UV buffer:
	glGenBuffers(1, &mesh.uvBuffer); // create UV buffer
	glBindBuffer(GL_ARRAY_BUFFER, mesh.uvBuffer); // bind UV buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.nVertices, uvs, GL_STATIC_DRAW); // send UV data to GPU

	// Create UV attribute:
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*) nullptr);
	glEnableVertexAttribArray(2); // enable UVs
}
void UCreateBottomOfLipBalm(GLMesh& mesh, float topRadius, float bottomRadius, int numSlices, float height, bool topCircle, bool bottomCircle)
{
	struct Vertex {
		glm::vec3 position;
		glm::vec2 texCoords;
		glm::vec3 normal;
	};


	bool hasTop;
	bool hasBottom;

	hasTop = topCircle;
	hasBottom = bottomCircle;

	// calculate number of vertices
	numVerticesSide = (numSlices + 1) * 2;
	numVerticesTopBottom = numSlices + 2;
	numVerticesTotal = numVerticesSide + numVerticesTopBottom * 2;

	// calculate sines/cosines for number of slices
	const auto sliceAngleStep = 2.0f * glm::pi<float>() / float(numSlices);
	auto currentSliceAngle = 0.0f;
	std::vector<float> sines, cosines;
	for (auto i = 0; i <= numSlices; i++)
	{
		sines.push_back(sin(currentSliceAngle));
		cosines.push_back(cos(currentSliceAngle));

		// update slice angle
		currentSliceAngle += sliceAngleStep;
	}

	// calculate X and Z coordinates for top and bottom circles
	std::vector<float> xTop, xBottom;
	std::vector<float> zTop, zBottom;
	for (auto i = 0; i <= numSlices; i++)
	{
		xTop.push_back(cosines[i] * topRadius);
		zTop.push_back(sines[i] * topRadius);
		xBottom.push_back(cosines[i] * bottomRadius);
		zBottom.push_back(sines[i] * bottomRadius);
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;

	// calculate vertices for sides
	for (auto i = 0; i <= numSlices; i++)
	{
		const auto topPosition = glm::vec3(xTop[i], height / 2.0f, zTop[i]);
		const auto bottomPosition = glm::vec3(xBottom[i], -height / 2.0f, zBottom[i]);
		positions.push_back(topPosition);
		positions.push_back(bottomPosition);
	}

	// add top circle 

	if (hasTop)
	{
		glm::vec3 topCenterPosition(0.0f, height / 2.0f, 0.0f);
		positions.push_back(topCenterPosition);

		for (auto i = 0; i <= numSlices; i++)
		{
			const auto topPosition = glm::vec3(xTop[i], height / 2.0f, zTop[i]);
			positions.push_back(topPosition);
		}
	}

	// add bottom circle
	if (hasBottom)
	{
		glm::vec3 bottomCenterPosition(0.0f, -height / 2.0f, 0.0f);
		positions.push_back(bottomCenterPosition);

		for (auto i = 0; i <= numSlices; i++)
		{
			const auto bottomPosition = glm::vec3(xBottom[i], -height / 2.0f, -zBottom[i]);
			positions.push_back(bottomPosition);
		}
	}


	// calculate texture coord for two wraps around object
	const auto sliceTextureStepU = 2.0f / float(numSlices);
	auto currentSliceTexCoordU = 0.0f;

	for (auto i = 0; i <= numSlices; i++)
	{
		texCoords.push_back(glm::vec2(currentSliceTexCoordU, 1.0f));
		texCoords.push_back(glm::vec2(currentSliceTexCoordU, 0.0f));

		// update texture coordinate of current slice 
		currentSliceTexCoordU += sliceTextureStepU;
	}

	// generate texture coords for top circle
	glm::vec2 topBottomCenterTexCoord(0.5f, 0.5f);
	if (hasTop)
	{
		texCoords.push_back(topBottomCenterTexCoord);
		for (auto i = 0; i <= numSlices; i++)
		{
			texCoords.push_back(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y + cosines[i] * 0.5f));
		}
	}

	// generate texture coords for bottom circle
	if (hasBottom)
	{
		texCoords.push_back(topBottomCenterTexCoord);
		for (auto i = 0; i <= numSlices; i++)
		{
			texCoords.push_back(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y - cosines[i] * 0.5f));
		}
	}
	/*
	* NORMALS
	*/
	for (auto i = 0; i <= numSlices; i++)
	{
		normals.push_back(glm::vec3(cosines[i], 0.0f, sines[i]));
		normals.push_back(glm::vec3(cosines[i], 0.0f, sines[i]));
	}

	// add normals for all vertices in top circle
	if (hasTop)
	{
		for (auto i = 0; i <= numVerticesTopBottom; i++)
		{
			normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	// add normals for all vertices in bottom circle
	if (hasBottom)
	{
		for (auto i = 0; i <= numVerticesTopBottom; i++)
		{
			normals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
		}
	}


	/*
	* CREATE VERTICES
	*/
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++)
	{
		Vertex vertex;

		vertex.position = positions.at(i);
		vertex.texCoords = texCoords.at(i);
		vertex.normal = normals.at(i);

		vertices.push_back(vertex);
	}

	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);

	// vertex positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// vertex texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	// vertex normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}
//For Dunkin cup
void UCreateCylinderDunkinMesh(GLMesh& mesh, float topRadius, float bottomRadius, int numSlices, float height, bool topCircle, bool bottomCircle)
{
	struct Vertex {
		glm::vec3 position;
		glm::vec2 texCoords;
		glm::vec3 normal;
	};

	
	bool hasTop;
	bool hasBottom;

	hasTop = topCircle;
	hasBottom = bottomCircle;

	// calculate number of vertices
	numVerticesSide = (numSlices + 1) * 2;
	numVerticesTopBottom = numSlices + 2;
	numVerticesTotal = numVerticesSide + numVerticesTopBottom * 2;

	// calculate sines/cosines for number of slices
	const auto sliceAngleStep = 2.0f * glm::pi<float>() / float(numSlices);
	auto currentSliceAngle = 0.0f;
	std::vector<float> sines, cosines;
	for (auto i = 0; i <= numSlices; i++)
	{
		sines.push_back(sin(currentSliceAngle));
		cosines.push_back(cos(currentSliceAngle));

		// update slice angle
		currentSliceAngle += sliceAngleStep;
	}

	// calculate X and Z coordinates for top and bottom circles
	std::vector<float> xTop, xBottom;
	std::vector<float> zTop, zBottom;
	for (auto i = 0; i <= numSlices; i++)
	{
		xTop.push_back(cosines[i] * topRadius);
		zTop.push_back(sines[i] * topRadius);
		xBottom.push_back(cosines[i] * bottomRadius);
		zBottom.push_back(sines[i] * bottomRadius);
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;

	// calculate vertices for sides
	for (auto i = 0; i <= numSlices; i++)
	{
		const auto topPosition = glm::vec3(xTop[i], height / 2.0f, zTop[i]);
		const auto bottomPosition = glm::vec3(xBottom[i], -height / 2.0f, zBottom[i]);
		positions.push_back(topPosition);
		positions.push_back(bottomPosition);
	}

	// add top circle 

	if (hasTop)
	{
		glm::vec3 topCenterPosition(0.0f, height / 2.0f, 0.0f);
		positions.push_back(topCenterPosition);

		for (auto i = 0; i <= numSlices; i++)
		{
			const auto topPosition = glm::vec3(xTop[i], height / 2.0f, zTop[i]);
			positions.push_back(topPosition);
		}
	}

	// add bottom circle
	if (hasBottom)
	{
		glm::vec3 bottomCenterPosition(0.0f, -height / 2.0f, 0.0f);
		positions.push_back(bottomCenterPosition);

		for (auto i = 0; i <= numSlices; i++)
		{
			const auto bottomPosition = glm::vec3(xBottom[i], -height / 2.0f, -zBottom[i]);
			positions.push_back(bottomPosition);
		}
	}


	// calculate texture coord for two wraps around object
	const auto sliceTextureStepU = 2.0f / float(numSlices);
	auto currentSliceTexCoordU = 0.0f;

	for (auto i = 0; i <= numSlices; i++)
	{
		texCoords.push_back(glm::vec2(currentSliceTexCoordU, 1.0f));
		texCoords.push_back(glm::vec2(currentSliceTexCoordU, 0.0f));

		// update texture coordinate of current slice 
		currentSliceTexCoordU += sliceTextureStepU;
	}

	// generate texture coords for top circle
	glm::vec2 topBottomCenterTexCoord(0.5f, 0.5f);
	if (hasTop)
	{
		texCoords.push_back(topBottomCenterTexCoord);
		for (auto i = 0; i <= numSlices; i++)
		{
			texCoords.push_back(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y + cosines[i] * 0.5f));
		}
	}

	// generate texture coords for bottom circle
	if (hasBottom)
	{
		texCoords.push_back(topBottomCenterTexCoord);
		for (auto i = 0; i <= numSlices; i++)
		{
			texCoords.push_back(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y - cosines[i] * 0.5f));
		}
	}
	/*
	* NORMALS
	*/
	for (auto i = 0; i <= numSlices; i++)
	{
		normals.push_back(glm::vec3(cosines[i], 0.0f, sines[i]));
		normals.push_back(glm::vec3(cosines[i], 0.0f, sines[i]));
	}

	// add normals for all vertices in top circle
	if (hasTop)
	{
		for (auto i = 0; i <= numVerticesTopBottom; i++)
		{
			normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	// add normals for all vertices in bottom circle
	if (hasBottom)
	{
		for (auto i = 0; i <= numVerticesTopBottom; i++)
		{
			normals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
		}
	}


	/*
	* CREATE VERTICES
	*/
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++)
	{
		Vertex vertex;

		vertex.position = positions.at(i);
		vertex.texCoords = texCoords.at(i);
		vertex.normal = normals.at(i);

		vertices.push_back(vertex);
	}

	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);

	// vertex positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// vertex texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	// vertex normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

}

void UCreateLipBalmMesh(GLMesh& mesh, float topRadius2, float bottomRadius2, int numSlices2, float height2, bool topCircle2, bool bottomCircle2) {
	struct Vertex {
		glm::vec3 position;
		glm::vec2 texCoords;
		glm::vec3 normal;
	};


	bool hasTop;
	bool hasBottom;

	hasTop = topCircle2;
	hasBottom = bottomCircle2;

	// calculate number of vertices
	numVerticesSide = (numSlices2 + 1) * 2;
	numVerticesTopBottom = numSlices2 + 2;
	numVerticesTotal = numVerticesSide + numVerticesTopBottom * 2;

	// calculate sines/cosines for number of slices
	const auto sliceAngleStep = 2.0f * glm::pi<float>() / float(numSlices2);
	auto currentSliceAngle = 0.0f;
	std::vector<float> sines, cosines;
	for (auto i = 0; i <= numSlices2; i++)
	{
		sines.push_back(sin(currentSliceAngle));
		cosines.push_back(cos(currentSliceAngle));

		// update slice angle
		currentSliceAngle += sliceAngleStep;
	}

	// calculate X and Z coordinates for top and bottom circles
	std::vector<float> xTop, xBottom;
	std::vector<float> zTop, zBottom;
	for (auto i = 0; i <= numSlices2; i++)
	{
		xTop.push_back(cosines[i] * topRadius2);
		zTop.push_back(sines[i] * topRadius2);
		xBottom.push_back(cosines[i] * bottomRadius2);
		zBottom.push_back(sines[i] * bottomRadius2);
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;

	// calculate vertices for sides
	for (auto i = 0; i <= numSlices2; i++)
	{
		const auto topPosition = glm::vec3(xTop[i], height2 / 2.0f, zTop[i]);
		const auto bottomPosition = glm::vec3(xBottom[i], -height2 / 2.0f, zBottom[i]);
		positions.push_back(topPosition);
		positions.push_back(bottomPosition);
	}

	// add top circle 

	if (hasTop)
	{
		glm::vec3 topCenterPosition(0.0f, height2 / 2.0f, 0.0f);
		positions.push_back(topCenterPosition);

		for (auto i = 0; i <= numSlices2; i++)
		{
			const auto topPosition = glm::vec3(xTop[i], height2 / 2.0f, zTop[i]);
			positions.push_back(topPosition);
		}
	}

	// add bottom circle
	if (hasBottom)
	{
		glm::vec3 bottomCenterPosition(0.0f, -height2 / 2.0f, 0.0f);
		positions.push_back(bottomCenterPosition);

		for (auto i = 0; i <= numSlices2; i++)
		{
			const auto bottomPosition = glm::vec3(xBottom[i], -height2 / 2.0f, -zBottom[i]);
			positions.push_back(bottomPosition);
		}
	}


	// calculate texture coord for two wraps around object
	const auto sliceTextureStepU = 2.0f / float(numSlices2);
	auto currentSliceTexCoordU = 0.0f;

	for (auto i = 0; i <= numSlices2; i++)
	{
		texCoords.push_back(glm::vec2(currentSliceTexCoordU, 1.0f));
		texCoords.push_back(glm::vec2(currentSliceTexCoordU, 0.0f));

		// update texture coordinate of current slice 
		currentSliceTexCoordU += sliceTextureStepU;
	}

	// generate texture coords for top circle
	glm::vec2 topBottomCenterTexCoord(0.5f, 0.5f);
	if (hasTop)
	{
		texCoords.push_back(topBottomCenterTexCoord);
		for (auto i = 0; i <= numSlices2; i++)
		{
			texCoords.push_back(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y + cosines[i] * 0.5f));
		}
	}

	// generate texture coords for bottom circle
	if (hasBottom)
	{
		texCoords.push_back(topBottomCenterTexCoord);
		for (auto i = 0; i <= numSlices2; i++)
		{
			texCoords.push_back(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y - cosines[i] * 0.5f));
		}
	}
	/*
	* NORMALS
	*/
	for (auto i = 0; i <= numSlices2; i++)
	{
		normals.push_back(glm::vec3(cosines[i], 0.0f, sines[i]));
		normals.push_back(glm::vec3(cosines[i], 0.0f, sines[i]));
	}

	// add normals for all vertices in top circle
	if (hasTop)
	{
		for (auto i = 0; i <= numVerticesTopBottom; i++)
		{
			normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	// add normals for all vertices in bottom circle
	if (hasBottom)
	{
		for (auto i = 0; i <= numVerticesTopBottom; i++)
		{
			normals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
		}
	}


	/*
	* CREATE VERTICES
	*/
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++)
	{
		Vertex vertex;

		vertex.position = positions.at(i);
		vertex.texCoords = texCoords.at(i);
		vertex.normal = normals.at(i);

		vertices.push_back(vertex);
	}

	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);

	// vertex positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// vertex texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	// vertex normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

}
//For bottom of cup
void UCreateCupBottom(GLMesh& mesh, float topRadius4, float bottomRadius4, int numSlices4, float height4, bool topCircle4, bool bottomCircle4) {
	struct Vertex {
		glm::vec3 position;
		glm::vec2 texCoords;
		glm::vec3 normal;
	};


	bool hasTop;
	bool hasBottom;

	hasTop = topCircle4;
	hasBottom = bottomCircle4;

	// calculate number of vertices
	numVerticesSide = (numSlices4 + 1) * 2;
	numVerticesTopBottom = numSlices4 + 2;
	numVerticesTotal = numVerticesSide + numVerticesTopBottom * 2;

	// calculate sines/cosines for number of slices
	const auto sliceAngleStep = 2.0f * glm::pi<float>() / float(numSlices4);
	auto currentSliceAngle = 0.0f;
	std::vector<float> sines, cosines;
	for (auto i = 0; i <= numSlices4; i++)
	{
		sines.push_back(sin(currentSliceAngle));
		cosines.push_back(cos(currentSliceAngle));

		// update slice angle
		currentSliceAngle += sliceAngleStep;
	}

	// calculate X and Z coordinates for top and bottom circles
	std::vector<float> xTop, xBottom;
	std::vector<float> zTop, zBottom;
	for (auto i = 0; i <= numSlices4; i++)
	{
		xTop.push_back(cosines[i] * topRadius4);
		zTop.push_back(sines[i] * topRadius4);
		xBottom.push_back(cosines[i] * bottomRadius4);
		zBottom.push_back(sines[i] * bottomRadius4);
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;

	// calculate vertices for sides
	for (auto i = 0; i <= numSlices4; i++)
	{
		const auto topPosition = glm::vec3(xTop[i], height4 / 2.0f, zTop[i]);
		const auto bottomPosition = glm::vec3(xBottom[i], -height4 / 2.0f, zBottom[i]);
		positions.push_back(topPosition);
		positions.push_back(bottomPosition);
	}

	// add top circle 

	if (hasTop)
	{
		glm::vec3 topCenterPosition(0.0f, height4 / 2.0f, 0.0f);
		positions.push_back(topCenterPosition);

		for (auto i = 0; i <= numSlices4; i++)
		{
			const auto topPosition = glm::vec3(xTop[i], height4 / 2.0f, zTop[i]);
			positions.push_back(topPosition);
		}
	}

	// add bottom circle
	if (hasBottom)
	{
		glm::vec3 bottomCenterPosition(0.0f, -height4 / 2.0f, 0.0f);
		positions.push_back(bottomCenterPosition);

		for (auto i = 0; i <= numSlices4; i++)
		{
			const auto bottomPosition = glm::vec3(xBottom[i], -height4 / 2.0f, -zBottom[i]);
			positions.push_back(bottomPosition);
		}
	}


	// calculate texture coord for two wraps around object
	const auto sliceTextureStepU = 2.0f / float(numSlices4);
	auto currentSliceTexCoordU = 0.0f;

	for (auto i = 0; i <= numSlices4; i++)
	{
		texCoords.push_back(glm::vec2(currentSliceTexCoordU, 1.0f));
		texCoords.push_back(glm::vec2(currentSliceTexCoordU, 0.0f));

		// update texture coordinate of current slice 
		currentSliceTexCoordU += sliceTextureStepU;
	}

	// generate texture coords for top circle
	glm::vec2 topBottomCenterTexCoord(0.5f, 0.5f);
	if (hasTop)
	{
		texCoords.push_back(topBottomCenterTexCoord);
		for (auto i = 0; i <= numSlices4; i++)
		{
			texCoords.push_back(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y + cosines[i] * 0.5f));
		}
	}

	// generate texture coords for bottom circle
	if (hasBottom)
	{
		texCoords.push_back(topBottomCenterTexCoord);
		for (auto i = 0; i <= numSlices4; i++)
		{
			texCoords.push_back(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y - cosines[i] * 0.5f));
		}
	}
	/*
	* NORMALS
	*/
	for (auto i = 0; i <= numSlices4; i++)
	{
		normals.push_back(glm::vec3(cosines[i], 0.0f, sines[i]));
		normals.push_back(glm::vec3(cosines[i], 0.0f, sines[i]));
	}

	// add normals for all vertices in top circle
	if (hasTop)
	{
		for (auto i = 0; i <= numVerticesTopBottom; i++)
		{
			normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	// add normals for all vertices in bottom circle
	if (hasBottom)
	{
		for (auto i = 0; i <= numVerticesTopBottom; i++)
		{
			normals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
		}
	}


	/*
	* CREATE VERTICES
	*/
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++)
	{
		Vertex vertex;

		vertex.position = positions.at(i);
		vertex.texCoords = texCoords.at(i);
		vertex.normal = normals.at(i);

		vertices.push_back(vertex);
	}

	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);

	// vertex positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// vertex texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	// vertex normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}
// For Straw creation
void UCreateStrawMesh(GLMesh& mesh, float topRadius3, float bottomRadius3, int numSlices3, float height3, bool topCircle3, bool bottomCircle3) {
	struct Vertex {
		glm::vec3 position;
		glm::vec2 texCoords;
		glm::vec3 normal;
	};


	bool hasTop;
	bool hasBottom;

	hasTop = topCircle3;
	hasBottom = bottomCircle3;

	// calculate number of vertices
	numVerticesSide = (numSlices3 + 1) * 2;
	numVerticesTopBottom = numSlices3 + 2;
	numVerticesTotal = numVerticesSide + numVerticesTopBottom * 2;

	// calculate sines/cosines for number of slices
	const auto sliceAngleStep = 2.0f * glm::pi<float>() / float(numSlices3);
	auto currentSliceAngle = 0.0f;
	std::vector<float> sines, cosines;
	for (auto i = 0; i <= numSlices3; i++)
	{
		sines.push_back(sin(currentSliceAngle));
		cosines.push_back(cos(currentSliceAngle));

		// update slice angle
		currentSliceAngle += sliceAngleStep;
	}

	// calculate X and Z coordinates for top and bottom circles
	std::vector<float> xTop, xBottom;
	std::vector<float> zTop, zBottom;
	for (auto i = 0; i <= numSlices3; i++)
	{
		xTop.push_back(cosines[i] * topRadius3);
		zTop.push_back(sines[i] * topRadius3);
		xBottom.push_back(cosines[i] * bottomRadius3);
		zBottom.push_back(sines[i] * bottomRadius3);
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;

	// calculate vertices for sides
	for (auto i = 0; i <= numSlices3; i++)
	{
		const auto topPosition = glm::vec3(xTop[i], height3 / 2.0f, zTop[i]);
		const auto bottomPosition = glm::vec3(xBottom[i], -height3 / 2.0f, zBottom[i]);
		positions.push_back(topPosition);
		positions.push_back(bottomPosition);
	}

	// add top circle 

	if (hasTop)
	{
		glm::vec3 topCenterPosition(0.0f, height3 / 2.0f, 0.0f);
		positions.push_back(topCenterPosition);

		for (auto i = 0; i <= numSlices3; i++)
		{
			const auto topPosition = glm::vec3(xTop[i], height3 / 2.0f, zTop[i]);
			positions.push_back(topPosition);
		}
	}

	// add bottom circle
	if (hasBottom)
	{
		glm::vec3 bottomCenterPosition(0.0f, -height3 / 2.0f, 0.0f);
		positions.push_back(bottomCenterPosition);

		for (auto i = 0; i <= numSlices3; i++)
		{
			const auto bottomPosition = glm::vec3(xBottom[i], -height3 / 2.0f, -zBottom[i]);
			positions.push_back(bottomPosition);
		}
	}


	// calculate texture coord for two wraps around object
	const auto sliceTextureStepU = 2.0f / float(numSlices3);
	auto currentSliceTexCoordU = 0.0f;

	for (auto i = 0; i <= numSlices3; i++)
	{
		texCoords.push_back(glm::vec2(currentSliceTexCoordU, 1.0f));
		texCoords.push_back(glm::vec2(currentSliceTexCoordU, 0.0f));

		// update texture coordinate of current slice 
		currentSliceTexCoordU += sliceTextureStepU;
	}

	// generate texture coords for top circle
	glm::vec2 topBottomCenterTexCoord(0.5f, 0.5f);
	if (hasTop)
	{
		texCoords.push_back(topBottomCenterTexCoord);
		for (auto i = 0; i <= numSlices3; i++)
		{
			texCoords.push_back(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y + cosines[i] * 0.5f));
		}
	}

	// generate texture coords for bottom circle
	if (hasBottom)
	{
		texCoords.push_back(topBottomCenterTexCoord);
		for (auto i = 0; i <= numSlices3; i++)
		{
			texCoords.push_back(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y - cosines[i] * 0.5f));
		}
	}
	/*
	* NORMALS
	*/
	for (auto i = 0; i <= numSlices3; i++)
	{
		normals.push_back(glm::vec3(cosines[i], 0.0f, sines[i]));
		normals.push_back(glm::vec3(cosines[i], 0.0f, sines[i]));
	}

	// add normals for all vertices in top circle
	if (hasTop)
	{
		for (auto i = 0; i <= numVerticesTopBottom; i++)
		{
			normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	// add normals for all vertices in bottom circle
	if (hasBottom)
	{
		for (auto i = 0; i <= numVerticesTopBottom; i++)
		{
			normals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
		}
	}


	/*
	* CREATE VERTICES
	*/
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++)
	{
		Vertex vertex;

		vertex.position = positions.at(i);
		vertex.texCoords = texCoords.at(i);
		vertex.normal = normals.at(i);

		vertices.push_back(vertex);
	}

	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);

	// vertex positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// vertex texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	// vertex normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

}

//Bottom of Lotion
void UCreateLotionBottomMesh(GLMesh& mesh, int mainSegments, int tubeSegments, float mainRadius, float tubeRadius) {
	float currentMainSegmentAngle = 0.0f;
	int index = 0; // current face
	int _mainSegments;
	int _tubeSegments;
	float _mainRadius;
	float _tubeRadius;

	int _numIndices = 0;
	int _primitiveRestartIndex = 0;

	glm::vec3* vertices;
	glm::vec2* uvs;
	glm::vec3* normals;
	GLuint* indices;
	vertices = nullptr;
	normals = nullptr;
	uvs = nullptr;
	indices = nullptr;
	_mainSegments = mainSegments;
	_tubeSegments = tubeSegments;
	_mainRadius = mainRadius;
	_tubeRadius = tubeRadius;

	// Calculate and cache counts of vertices and indices
	mesh.nVertices = (_mainSegments + 1) * (_tubeSegments + 1);
	_primitiveRestartIndex = mesh.nVertices;
	_numIndices = (_mainSegments * 2 * (_tubeSegments + 1)) + _mainSegments - 1; // oh we do have the amount lol
	mesh.nIndices = _numIndices;

	// Allocate data slots:
	vertices = new glm::vec3[mesh.nVertices];
	normals = new glm::vec3[mesh.nVertices];
	uvs = new glm::vec2[mesh.nVertices];
	indices = new GLuint[_numIndices];

	auto mainSegmentAngleStep = glm::radians(360.0f / float(_mainSegments));
	auto tubeSegmentAngleStep = glm::radians(360.0f / float(_tubeSegments));

	// Vertices:
	for (auto i = 0; i <= _mainSegments; i++) {
		// Calculate sine and cosine of main segment angle
		auto sinMainSegment = sin(currentMainSegmentAngle);
		auto cosMainSegment = cos(currentMainSegmentAngle);
		auto currentTubeSegmentAngle = 0.0f;
		for (auto j = 0; j <= _tubeSegments; j++) {
			// Calculate sine and cosine of tube segment angle
			auto sinTubeSegment = sin(currentTubeSegmentAngle);
			auto cosTubeSegment = cos(currentTubeSegmentAngle);

			// Calculate vertex position on the surface of torus
			vertices[index] = glm::vec3(
				(_mainRadius + _tubeRadius * cosTubeSegment) * cosMainSegment,
				(_mainRadius + _tubeRadius * cosTubeSegment) * sinMainSegment,
				_tubeRadius * sinTubeSegment);
			++index;

			// Update current tube angle
			currentTubeSegmentAngle += tubeSegmentAngleStep;
		}

		// Update main segment angle
		currentMainSegmentAngle += mainSegmentAngleStep;
	}

	// Texture coordinates:
	auto mainSegmentTextureStep = 2.0f / float(_mainSegments);
	auto tubeSegmentTextureStep = 1.0f / float(_tubeSegments);

	index = 0; // reset index counter
	auto currentMainSegmentTexCoordV = 0.0f;
	for (auto i = 0; i <= _mainSegments; i++) {
		auto currentTubeSegmentTexCoordU = 0.0f;
		for (auto j = 0; j <= _tubeSegments; j++) {
			uvs[index] = glm::vec2(currentTubeSegmentTexCoordU, currentMainSegmentTexCoordV);
			++index;

			currentTubeSegmentTexCoordU += tubeSegmentTextureStep;
		}

		// Update texture coordinate of main segment
		currentMainSegmentTexCoordV += mainSegmentTextureStep;
	}

	// Normals:
	index = 0; // reset again
	currentMainSegmentAngle = 0.0f;
	for (auto i = 0; i <= _mainSegments; i++) {
		// Calculate sine and cosine of main segment angle
		auto sinMainSegment = sin(currentMainSegmentAngle);
		auto cosMainSegment = cos(currentMainSegmentAngle);
		auto currentTubeSegmentAngle = 0.0f;
		for (auto j = 0; j <= _tubeSegments; j++) {
			// Calculate sine and cosine of tube segment angle
			auto sinTubeSegment = sin(currentTubeSegmentAngle);
			auto cosTubeSegment = cos(currentTubeSegmentAngle);

			normals[index] = glm::vec3(cosMainSegment * cosTubeSegment, sinMainSegment * cosTubeSegment,
				sinTubeSegment);
			++index;

			// Update current tube angle
			currentTubeSegmentAngle += tubeSegmentAngleStep;
		}

		// Update main segment angle
		currentMainSegmentAngle += mainSegmentAngleStep;
	}

	// Indices:
	index = 0; // reset again
	GLuint currentVertexOffset = 0;
	for (auto i = 0; i < _mainSegments; i++) {
		for (auto j = 0; j <= _tubeSegments; j++) {
			GLuint vertexIndexA = currentVertexOffset;
			indices[index] = vertexIndexA;
			++index;

			GLuint vertexIndexB = currentVertexOffset + _tubeSegments + 1;
			indices[index] = vertexIndexB;
			++index;

			currentVertexOffset++;
		}

		// Don't restart primitive, if it's last segment, rendering ends here anyway
		if (i != _mainSegments - 1) {
			indices[index] = _primitiveRestartIndex;
			++index;
		}
	}
	glGenVertexArrays(1, &mesh.vao); // generate a single vao
	glBindVertexArray(mesh.vao);

	// Vertex buffer:
	glGenBuffers(1, &mesh.vbo); // create vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // bind VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.nVertices, vertices,
		GL_STATIC_DRAW); // send vertex data to the GPU

// Create vertex attribute:
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) nullptr); // sizeof(float) * (3)
	glEnableVertexAttribArray(0); // enable vertices

	// Index buffer:
	glGenBuffers(1, &mesh.indexBuffer); // create index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer); // bind index buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.nIndices, indices,
		GL_STATIC_DRAW); // send index data to GPU

// Normal buffer:
	glGenBuffers(1, &mesh.normalBuffer); // create normal buffer
	glBindBuffer(GL_ARRAY_BUFFER, mesh.normalBuffer); // bind normal buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.nVertices, normals,
		GL_STATIC_DRAW); // send normal data to GPU

// Create normal attribute:
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) nullptr); // sizeof(float) * (2)
	glEnableVertexAttribArray(1); // enable normals

	// UV buffer:
	glGenBuffers(1, &mesh.uvBuffer); // create UV buffer
	glBindBuffer(GL_ARRAY_BUFFER, mesh.uvBuffer); // bind UV buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.nVertices, uvs, GL_STATIC_DRAW); // send UV data to GPU

	// Create UV attribute:
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*) nullptr); 
	glEnableVertexAttribArray(2); // enable UVs

}
// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	// Compile the vertex shader, and print compilation errors (if any)
	glCompileShader(vertexShaderId); // compile the vertex shader
	// check for shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader
	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	// Attached compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);   // links the shader program
	// check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glUseProgram(programId);    // Uses the shader program

	return true;
}


void UDestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}

bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{
		flipImageVertically(image, width, height, channels);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// Set the texture wrapping parameters.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Set texture filtering parameters.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture.

		return true;
	}

	// Error loading the image
	return false;
}

void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}



