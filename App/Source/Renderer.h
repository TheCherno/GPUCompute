#pragma once

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <filesystem>

struct Texture
{
	GLuint Handle = 0;
	uint32_t Width = 0;
	uint32_t Height = 0;
};

struct Framebuffer
{
	GLuint Handle = 0;
	Texture ColorAttachment;
};

Texture CreateTexture(int width, int height);
Texture LoadTexture(const std::filesystem::path& path);
Framebuffer CreateFramebufferWithTexture(const Texture texture);
bool AttachTextureToFramebuffer(Framebuffer& framebuffer, const Texture texture);
void BlitFramebufferToSwapchain(const Framebuffer framebuffer);