#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "Shader.h"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#define TINYEXR_IMPLEMENTATION
#include <tinyexr/tinyexr.h>

int main()
{
	glfwInit();

	GLFWwindow* pWindow = glfwCreateWindow(1024, 1024, "Tools_TinyEXR", NULL, NULL);
	if (pWindow == NULL)
	{
		std::cout << "Failed to Create GLFW Window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(pWindow);

	glewInit();

	CShader Shader;
	Shader.addShader("shaders/vertShader.glsl", VERTEX_SHADER);
	Shader.addShader("shaders/fragShader.glsl", FRAGMENT_SHADER);
	Shader.createShaderProgram();

	float vertices[] = {
		 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	GLuint Texture;
	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	float* pTextureData = nullptr;
	const char* Err = nullptr;
	int TextureWidth, TextureHeight;
	int Ret = LoadEXR(&pTextureData, &TextureWidth, &TextureHeight, "textures/cornellbox_bunny_global2.exr", &Err);
	if (Ret != TINYEXR_SUCCESS)
	{
		if (Err)
		{
			fprintf(stderr, "ERR : %s\n", Err);
			FreeEXRErrorMessage(Err); // release memory of error message.
		}
		else
		{
			free(pTextureData);
		}
	}
	if (pTextureData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TextureWidth, TextureHeight, 0, GL_RGBA, GL_FLOAT, pTextureData);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		return -1;
	}
	free(pTextureData);

	Shader.useShaderProgram();
	glUniform1i(glGetUniformLocation(Shader.getShaderProgram(), "sTexture1"), 0);

	while (!glfwWindowShouldClose(pWindow) && !glfwGetKey(pWindow, GLFW_KEY_ESCAPE))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, Texture);

		Shader.useShaderProgram();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwPollEvents();
		glfwSwapBuffers(pWindow);
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();
	return 0;
}