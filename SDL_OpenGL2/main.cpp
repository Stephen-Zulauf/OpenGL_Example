#include "config.h"

/*Globals*/
int gScreenWidth = 640;
int gScreenHight = 480;
SDL_Window* gWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;
bool gQuit = false; //true-> quit

// VAO
GLuint gVertexArrayObject = 0;
// VBO1 vectors/colors
GLuint gVertexBufferObject = 0;
// IBO/EBO
GLuint gIndexBufferObject = 0;

//Program object for the shaders(handle to the pipeline that was set up)
GLuint gPipelineShaderProgram = 0;

//uniform modifyer
GLfloat g_uOffset = 0.0f;


/*Error handling*/
static void GLClearAllErrors() {
	while (glGetError() != GL_NO_ERROR) {

	}
}

static bool GLCheckErrorStatus(const char* function, int line) {
	while (GLenum error = glGetError()) {

		std::cout << "OpenGl Error:" << error 
			<< "\tLine: " << line 
			<< "\tFunction: " << function 
			<< std::endl;

		return true;
	}
	return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x,__LINE__);

/*function definitions*/
std::string loadShaderFileAsString(const std::string& nFilename) {
	std::string result = "";

	std::string line = "";
	std::ifstream nFile(nFilename);

	if (nFile.is_open()) {
		while (std::getline(nFile, line)) {
			result += line + '\n';
		}
		nFile.close();
	}

	return result;
}

void vertexSpecification() {

	//lives on the cpu

	const std::vector<GLfloat> vertexData{
		//x,  y,  z
		//r,  g,  b
		
		//0 vertex
		-0.5f, -0.5f, 0.0f, //bottom left vertex
		0.92f, 0.28f, 0.66f,
		//1 vertex
		0.5f, -0.5f, 0.0f, //bottom right vertex
		0.50f, 0.86f, 0.44f,
		//2 vertex
		-0.5f, 0.5f, 0.0f, //top left vertex
		0.77f, 0.00f, 0.97f,
		//3 vertex
		0.5f, 0.5f, 0.0f, //top right vertex
		0.92f, 0.28f, 0.66f,	
		
	};

	//create VAO and bind to it
	glGenVertexArrays(1, &gVertexArrayObject);
	glBindVertexArray(gVertexArrayObject);

	//start generating VBO

	//gen buffer handle in gVertexBufferObject
	glGenBuffers(1, &gVertexBufferObject);
	//bind to the buffer
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
	//fill buffer with vertex data
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

	//indexing buffer object(IBO EBO)
	const std::vector<GLuint> indexBufferData{ 2,0,1,3,2,1 };
	glGenBuffers(1, &gIndexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferData.size() * sizeof(GLuint), indexBufferData.data(), GL_STATIC_DRAW);
	
	//location data
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)0);

	//color info
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (GLvoid*)(sizeof(GLfloat)*3));

	//cleanup
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	
}

GLuint compileShader(GLuint type, const std::string& source) {
	GLuint shaderObject;

	//check shader type (error checking)
	if (type == GL_VERTEX_SHADER) {
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	}
	else if (type == GL_FRAGMENT_SHADER) {
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}
	else {
		std::cout << "shadertype not recognized" << std::endl;
	}

	//compile source
	const char* src = source.c_str();
	glShaderSource(shaderObject, 1, &src, nullptr);
	glCompileShader(shaderObject);

	//error checking
	int result;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		char* errorMessages = new char[length];
		glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

		if (type == GL_VERTEX_SHADER) {
			std::cout << "ERROR GL_VERTEX_SHADER compilation failed\n"
				<< errorMessages << std::endl;
		}
		else if (type == GL_FRAGMENT_SHADER) {
			std::cout << "ERROR GL_FRAGMENT_SHADER compilation failed\n"
				<< errorMessages << std::endl;
		}
		//reclaim memory
		delete[] errorMessages;

		//delete bad shader
		glDeleteShader(shaderObject);

		return 0;
	}

	return shaderObject;
	
}

GLuint createShaderProgram(const std::string& vshaderSource, const std::string& fshaderSource) {

	//create empty pipeline
	GLuint programObject = glCreateProgram();

	//compile the shader sources
	GLuint nVertexShader = compileShader(GL_VERTEX_SHADER, vshaderSource);
	GLuint nFragmentShader = compileShader(GL_FRAGMENT_SHADER, fshaderSource);

	//attach shaders to the empty program object(pipeline)
	glAttachShader(programObject, nVertexShader);
	glAttachShader(programObject, nFragmentShader);
	glLinkProgram(programObject);

	//validate program
	glValidateProgram(programObject);
	//gldetachShader, glDeleteShader TODO

	return programObject;

}

void createGraphicsPipeline() {

	std::string vertexShaderSource = loadShaderFileAsString("vert.glsl");
	std::string fragmentShaderSource = loadShaderFileAsString("frag.glsl");

	gPipelineShaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
}


void initProgrum() {

	//init sdl
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "SDL could not init video subsystem"
			<< std::endl;
		exit(1);
	}

	//set open GL attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);


	//create window
	gWindow = SDL_CreateWindow("OpenGL Progrum", 40, 40, gScreenWidth, gScreenHight, SDL_WINDOW_OPENGL);
	if (!gWindow) {
		std::cout << "SDL_Window failed to be created"
			<< std::endl;
		exit(1);
	}

	//create opengl context
	gOpenGLContext = SDL_GL_CreateContext(gWindow);
	if (!gOpenGLContext) {
		std::cout << "SDL_GL_CreateContext failed to create"
			<< std::endl;
		exit(1);
	}

	//init Glad library
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		std::cout << "Glad was not initialized" << std::endl;
		exit(1);
	}

	//query opengl context (testing info)
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Shading language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

}

void input() {
	//handle input events
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			std::cout << "Closed progrum" << std::endl;
			gQuit = true;
		}
	}

	//retrieve keyboard state
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_UP]) {
		g_uOffset += 0.01f;
		std::cout << "g_uOffset: " << g_uOffset << std::endl;
	}
	if (state[SDL_SCANCODE_DOWN]) {
		g_uOffset -= 0.01f;
		std::cout << "g_uOffset: " << g_uOffset << std::endl;
	}
}

void preDraw() {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glViewport(0, 0, gScreenWidth, gScreenHight);
	glClearColor(0.61f, 0.53f, 0.83f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glUseProgram(gPipelineShaderProgram);

	//setup uniform
	//model transformation by translating object to world space
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_uOffset));

	//retrieve location
	GLint u_ModelMatrixLocation = glGetUniformLocation(gPipelineShaderProgram, "u_ModelMatrix");

	//update uniform value at location
	if (u_ModelMatrixLocation >= 0) {
		glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &translate[0][0]);
	}
	else {
		std::cout << "ERR could not find uniform location" << std::endl;
		exit(EXIT_FAILURE);
	}

	//setup uniform
	//Projection matrix (in perspective)
	glm::mat4 perspective = glm::perspective(glm::radians(45.0f), 
		(float)gScreenWidth/(float)gScreenHight, 0.1f, 10.f);

	//retrieve location
	GLint u_perspectiveLocation = glGetUniformLocation(gPipelineShaderProgram, "u_Perspective");

	//update uniform value at location
	if (u_perspectiveLocation >= 0) {
		glUniformMatrix4fv(u_perspectiveLocation, 1, GL_FALSE, &perspective[0][0]);
	}
	else {
		std::cout << "ERR could not find uniform (perspective) location" << std::endl;
		exit(EXIT_FAILURE);
	}
	
}

void draw() {
	glBindVertexArray(gVertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

	//glDrawArrays(GL_TRIANGLES, 0, 6);
	GLCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));

	glUseProgram(0);
}

void mainLoop() {
	while (!gQuit) {
		input();

		preDraw();

		draw();

		SDL_GL_SwapWindow(gWindow);
	}
}

void cleanUp() {

	SDL_DestroyWindow(gWindow);
	SDL_Quit();
}

/*Main*/
// You must include the command line parameters for your main function to be recognized by SDL
int main(int argc, char** args) {

	initProgrum();

	vertexSpecification();

	createGraphicsPipeline();

	mainLoop();

	cleanUp();

	return 0;
}