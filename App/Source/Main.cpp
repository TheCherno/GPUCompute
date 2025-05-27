#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Renderer.h"

#include <chrono>

class Timer
{
public:
	Timer() { Reset(); }
	void Reset() { m_Start = std::chrono::high_resolution_clock::now(); }
	float Elapsed() const { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f; }
	float ElapsedMillis() const { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f; }
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

class ScopedTimer
{
public:
	ScopedTimer(std::string_view name) : m_Name(name) {}
	~ScopedTimer()
	{
		float time = m_Timer.ElapsedMillis();
		std::cout << m_Name << " - " << time << "ms\n";
	}
private:
	Timer m_Timer;
	std::string m_Name;
};

static bool s_Compute = false;
static uint32_t s_ComputeShader = -1;
static uint32_t s_GraphicsShader = -1;
static const std::filesystem::path s_ComputeShaderPath = "Shaders/Compute.glsl";
static const std::filesystem::path s_VertexShaderPath = "Shaders/Vertex.glsl";
static const std::filesystem::path s_FragmentShaderPath = "Shaders/Fragment.glsl";

static void ErrorCallback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_R)
	{
		s_ComputeShader = ReloadComputeShader(s_ComputeShader, s_ComputeShaderPath);
		s_GraphicsShader = ReloadGraphicsShader(s_GraphicsShader, s_VertexShaderPath, s_FragmentShaderPath);
	}

	if (key == GLFW_KEY_S)
		s_Compute = !s_Compute;
}

int main()
{
	glfwSetErrorCallback(ErrorCallback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	int width = 1280;
	int height = 720;

	GLFWwindow* window = glfwCreateWindow(width, height, "Compute", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, KeyCallback);

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(1);

	s_ComputeShader = CreateComputeShader(s_ComputeShaderPath);
	if (s_ComputeShader == -1)
	{
		std::cerr << "Compute shader failed\n";
		return -1;
	}

	s_GraphicsShader = CreateGraphicsShader(s_VertexShaderPath, s_FragmentShaderPath);
	if (s_GraphicsShader == -1)
	{
		std::cerr << "Graphics shader failed\n";
		return -1;
	}

	Texture computeShaderTexture = CreateTexture(width, height);
	Framebuffer fb = CreateFramebufferWithTexture(computeShaderTexture);

	Texture sky = LoadTexture("Assets/sky1.png");
	Texture ground = LoadTexture("Assets/mariokart.png");

	GLuint vertexArray;
	glCreateVertexArrays(1, &vertexArray);

	GLuint vertexBuffer;
	glCreateBuffers(1, &vertexBuffer);

	bool triangle = true;
	if (!triangle)
	{
		GLuint indexBuffer;
		glCreateBuffers(1, &indexBuffer);

		float vertices[] = {
			-1.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
			 1.0f, -1.0f, 1.0f, 0.0f, // Bottom-right
			-1.0f,  1.0f, 0.0f, 1.0f, // Top-left
			 1.0f,  1.0f, 1.0f, 1.0f, // Top-right
		};

		uint32_t indices[] = { 0, 1, 2, 3 };

		glNamedBufferData(vertexBuffer, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glNamedBufferData(indexBuffer, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexArrayElementBuffer(vertexArray, indexBuffer);
	}
	else
	{
		float vertices[] = {
			-1.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
			 3.0f, -1.0f, 2.0f, 0.0f, // Bottom-right
			-1.0f,  3.0f, 0.0f, 2.0f, // Top-left
		};

		glNamedBufferData(vertexBuffer, sizeof(vertices), vertices, GL_STATIC_DRAW);
	}

	glVertexArrayVertexBuffer(vertexArray, 0, vertexBuffer, 0, sizeof(float) * 4);

	glEnableVertexArrayAttrib(vertexArray, 0);
	glEnableVertexArrayAttrib(vertexArray, 1);

	glVertexArrayAttribFormat(vertexArray, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(vertexArray, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2);

	glVertexArrayAttribBinding(vertexArray, 0, 0);
	glVertexArrayAttribBinding(vertexArray, 1, 0);
	
	float fWorldX = 1000.4f;
	float fWorldY = 1000.39f;
	float fWorldA = 0.77135f;
	float fNear = 0.026492f;
	float fFar = 0.199961f;
	float fFoVHalf = 3.14159f / 4.0f;

	float lastTime = (float)glfwGetTime();

	int fps = 0;
	float secondsTimer = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		//ScopedTimer timer("Main Loop");

		float currentTime = (float)glfwGetTime();
		float dt = currentTime - lastTime;
		lastTime = currentTime;

		secondsTimer += dt;
		if (secondsTimer >= 1.0f)
		{
			std::string title = std::format("{} - {} fps", s_Compute ? "Compute" : "Graphics", fps);
			glfwSetWindowTitle(window, title.c_str());

			secondsTimer = 0.0f;
			fps = 0;
		}

		glfwGetFramebufferSize(window, &width, &height);

		// Resize texture
		if (width != computeShaderTexture.Width || height != computeShaderTexture.Height)
		{
			glDeleteTextures(1, &computeShaderTexture.Handle);
			computeShaderTexture = CreateTexture(width, height);
			AttachTextureToFramebuffer(fb, computeShaderTexture);
		}

		float fFarX1 = fWorldX + glm::cos(fWorldA - fFoVHalf) * fFar;
		float fFarY1 = fWorldY + glm::sin(fWorldA - fFoVHalf) * fFar;

		float fNearX1 = fWorldX + glm::cos(fWorldA - fFoVHalf) * fNear;
		float fNearY1 = fWorldY + glm::sin(fWorldA - fFoVHalf) * fNear;

		float fFarX2 = fWorldX + glm::cos(fWorldA + fFoVHalf) * fFar;
		float fFarY2 = fWorldY + glm::sin(fWorldA + fFoVHalf) * fFar;

		float fNearX2 = fWorldX + glm::cos(fWorldA + fFoVHalf) * fNear;
		float fNearY2 = fWorldY + glm::sin(fWorldA + fFoVHalf) * fNear;

		glm::vec4 fNear = { fNearX1, fNearY1, fNearX2, fNearY2 };
		glm::vec4 fFar = { fFarX1, fFarY1, fFarX2, fFarY2 };

		fWorldX += glm::cos(fWorldA) * 0.2f * dt;
		fWorldY += glm::sin(fWorldA) * 0.2f * dt;

		// Compute
		if (s_Compute)
		{
			glUseProgram(s_ComputeShader);
			glBindImageTexture(0, fb.ColorAttachment.Handle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

			glBindTextureUnit(1, sky.Handle);
			glBindTextureUnit(2, ground.Handle);

			glUniform4fv(0, 1, glm::value_ptr(fNear));
			glUniform4fv(1, 1, glm::value_ptr(fFar));

			const GLuint workGroupSizeX = 16;
			const GLuint workGroupSizeY = 16;

			GLuint numGroupsX = (width + workGroupSizeX - 1) / workGroupSizeX;
			GLuint numGroupsY = (height + workGroupSizeY - 1) / workGroupSizeY;

			glDispatchCompute(numGroupsX, numGroupsY, 1);

			// Ensure all writes to the image are complete
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}
		else
		{
			// Graphics
			glBindFramebuffer(GL_FRAMEBUFFER, fb.Handle);
			glUseProgram(s_GraphicsShader);

			glBindTextureUnit(1, sky.Handle);
			glBindTextureUnit(2, ground.Handle);

			glUniform4fv(0, 1, glm::value_ptr(fNear));
			glUniform4fv(1, 1, glm::value_ptr(fFar));

			glBindVertexArray(vertexArray);
			if (triangle)
				glDrawArrays(GL_TRIANGLES, 0, 3);
			else
				glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, nullptr);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// Blit
		{
			BlitFramebufferToSwapchain(fb);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

		fps++;
	}

	glfwDestroyWindow(window);

	glfwTerminate();
}
