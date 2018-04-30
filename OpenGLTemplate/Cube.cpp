#include "Cube.h"

CCube::CCube()
{
}

CCube::~CCube()
{
	Release();
}

void CCube::Create(string filename, float width, float height, float length)
{
	m_texture.Load(filename);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	m_vbo.Create();
	m_vbo.Bind();
	//Write the code to add interleaved vertex attributes to the VBO
	glm::vec3 p[24];
	
	//front
	p[0] = glm::vec3(-1 * width, -1 * height, 1 * length);
	p[1] = glm::vec3(1 * width, -1 * height, 1 * length);
	p[2] = glm::vec3(-1 * width, 1 * height, 1 * length);
	p[3] = glm::vec3(1 * width, 1 * height, 1 * length);
	
	//back
	p[4] = glm::vec3(1 * width, -1 * height, -1 * length);
	p[5] = glm::vec3(-1 * width, -1 * height, -1 * length);
	p[6] = glm::vec3(1 * width, 1 * height, -1 * length);
	p[7] = glm::vec3(-1 * width, 1 * height, -1 * length);
	
	//right
	p[8] = glm::vec3(1 * width, -1 * height, 1 * length);
	p[9] = glm::vec3(1 * width, -1 * height, -1 * length);
	p[10] = glm::vec3(1 * width, 1 * height, 1 * length);
	p[11] = glm::vec3(1 * width, 1 * height, -1 * length);

	//left
	p[12] = glm::vec3(-1 * width, -1 * height, -1 * length);
	p[13] = glm::vec3(-1 * width, -1 * height, 1 * length);
	p[14] = glm::vec3(-1 * width, 1 * height, -1 * length);
	p[15] = glm::vec3(-1 * width, 1 * height, 1 * length);
	
	//up	
	p[16] = glm::vec3(-1 * width, 1 * height, 1 * length);
	p[17] = glm::vec3(1 * width, 1 * height, 1 * length);
	p[18] = glm::vec3(-1 * width, 1 * height, -1 * length);
	p[19] = glm::vec3(1 * width, 1 * height, -1 * length);
	
	//down
	p[20] = glm::vec3(-1 * width, -1 * height, -1 * length);
	p[21] = glm::vec3(1 * width, -1 * height, -1 * length);
	p[22] = glm::vec3(-1 * width, -1 * height, 1 * length);
	p[23] = glm::vec3(1 * width, -1 * height, 1 * length);

	//texture coords
	glm::vec2 s[4];	
	s[0] = glm::vec2(0, 0);
	s[1] = glm::vec2(1, 0);
	s[2] = glm::vec2(0, 1);
	s[3] = glm::vec2(1, 1);

	//normals
	glm::vec3 n[6];	
	n[0] = glm::vec3(0, 0, 1); //front
	n[1] = glm::vec3(0, 0, -1); //back
	n[2] = glm::vec3(1, 0, 0); //right
	n[3] = glm::vec3(-1, 0, 0); //left
	n[4] = glm::vec3(0, 1, 0); //top
	n[5] = glm::vec3(0, -1, 0); //bottom

	for (unsigned i = 0; i < 24; ++i)
	{
		m_vbo.AddData(&p[i], sizeof(glm::vec3));
		m_vbo.AddData(&s[i % 4], sizeof(glm::vec2));
		m_vbo.AddData(&n[0], sizeof(glm::vec3));
	}

	//Upload data to GPU
	m_vbo.UploadDataToGPU(GL_STATIC_DRAW);
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
	//Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	//Texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	//Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void CCube::Render()
{
	glBindVertexArray(m_vao);
	m_texture.Bind();
	//Call glDrawArrays to render each side
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);

}

void CCube::Release()
{
	m_texture.Release();
	glDeleteVertexArrays(1, &m_vao);
	m_vbo.Release();
}