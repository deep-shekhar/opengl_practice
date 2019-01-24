#include <GL/glew.h>
//#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include<sstream>
#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) GLCLearError();\
	x;\
	GLCheckError(#x,__FILE__,__LINE__);

static void GLCLearError() {
	while (glGetError() != GL_NO_ERROR);
}

static bool GLCheckError(const char* function, const char* file,int line) {
	while (GLenum error = glGetError()) {
		std::cout << "[OpenGL error] :" << std::hex << error << std::dec << function <<" "<<file<<" "<<line<< std::endl;
		return false;
	}
	return true;
}

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& file)
{
	std::ifstream stream(file);
	
	enum class ShaderType {
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1
	};
	
	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;

	while (getline(stream,line)) {
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos) {
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos) {
				type = ShaderType::FRAGMENT;
			}
		}
		else {
			ss[(int)type] << line << '\n';
		}
	}
	return {ss[0].str() , ss[1].str()};
}

static unsigned int CompileShader(const std::string& source, unsigned int type)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1,&src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);

	if (!result) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char *)(alloca(sizeof(char)*length));
		glGetShaderInfoLog(id,length, &length, message);
		std::cout << "Failed to compile " << (type==GL_VERTEX_SHADER? "vertex":"fragment") << " shader!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) 
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(vertexShader,GL_VERTEX_SHADER);
	unsigned int fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	glfwSwapInterval(1);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW INIT ERROR!\n";
	}

	std::cout << glGetString(GL_VERSION) << std::endl;

	float positions[] = { -0.5f,0.5f, 
						   -0.3f, 0.1f,
						    0.0f, 0.7f,
							0.3f, 0.6f,
							};

	unsigned int indices[] = {0,1,2,2,3,0};

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 0);

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	ShaderProgramSource src = ParseShader("Basic.shader");
	std::cout << src.VertexSource << " \n\n " << src.FragmentSource << std::endl;
	unsigned int shader = CreateShader(src.VertexSource, src.FragmentSource);
	glUseProgram(shader);
	
	GLCall(int u_location = glGetUniformLocation(shader, "u_color"));
	ASSERT(u_location != -1)
	GLCall(glUniform4f(u_location, 0.3f,0.0f,0.1f,0.8f));

	unsigned int r;
	unsigned int increment = 0.05f;

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		//GLCLearError();
		GLCall(glUniform4f(u_location, 0.3f, 0.0f, 0.1f, 0.8f));
		GLCall( glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr) );
		//ASSERT(GLCheckError());

		if (r >= 1.0f) {
			increment = -0.05f;
		}
		else if (r <= 0.0f) {
			increment = 0.05f;
		}
		r += increment;
		
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		glfwPollEvents();
	}
	
	//glDeleteProgram(shader);
	glfwTerminate();
	return 0;
}