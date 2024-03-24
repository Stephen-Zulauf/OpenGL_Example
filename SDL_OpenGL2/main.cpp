#include "config.h"

/*Globals*/
int gScreenWidth = 640;
int gScreenHight = 480;
SDL_Window* gWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;
bool gQuit = false; //true-> quit

// VAO
GLuint gVertexArrayObject = 0;
// VBO
GLuint gVertexBufferObject = 0;
//Program object for the shaders(handle to the pipeline that was set up)
GLuint gPipelineShaderProgram = 0;



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
	const std::vector<GLfloat> vertexPostion{
		//x,  y,  z
		-0.8f, -0.8f, 0.0f, //vertex 1
		0.8f, -0.8f, 0.0f, //vertex 2
		0.0f, 0.8f, 0.0f //vertex 3
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
	glBufferData(GL_ARRAY_BUFFER, vertexPostion.size() * sizeof(GL_FLOAT), vertexPostion.data(), GL_STATIC_DRAW);
	//enable the vertex array that is bound
	glEnableVertexAttribArray(0);
	//specify the location and data format of the array of generic vertex attributes at index 
	//index to use when rendering
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//cleanup
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
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
}

void preDraw() {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glViewport(0, 0, gScreenWidth, gScreenHight);
	glClearColor(0.61f, 0.53f, 0.83f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glUseProgram(gPipelineShaderProgram);
}

void draw() {
	glBindVertexArray(gVertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

	glDrawArrays(GL_TRIANGLES, 0, 3);

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