#pragma once

#include "Common.h"
#include "Texture.h"
#include "VertexBufferObject.h"

//Class for generating a unit cube

class CCube
{
public: 
	CCube();
	~CCube();
	void Create(string filename, float width, float height, float length);
	void Render();
	void Release();
private:
	GLuint m_vao;
	CVertexBufferObject m_vbo;
	CTexture m_texture;
};