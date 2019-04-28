#include "Mesh.h"

NAMESPACE_BEGIN(gl_kernel)

CGLMesh::CGLMesh(const std::vector<SMeshVertex>& vMeshVerticesSet, const std::vector<unsigned int> vMeshIndicesSet, const std::vector<SMeshTexture>& vMeshTextureSet)
	: m_MeshVerticesSet(vMeshVerticesSet), m_MeshTextureSet(vMeshTextureSet), m_MeshIndicesSet(vMeshIndicesSet)
{
	__init();
}

CGLMesh::~CGLMesh()
{
	if (m_VertexArrayObject) {
		glDeleteVertexArrays(1, &m_VertexArrayObject);
		m_VertexArrayObject = 0;
	}
}

//***********************************************************************************************
//Function:
void CGLMesh::draw(unsigned int vTextureUnit)
{
	for (size_t i = 0; i < m_MeshTextureSet.size(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + vTextureUnit + i);
		glBindTexture(GL_TEXTURE_2D, m_MeshTextureSet[i].m_TextureID);
	}

	glBindVertexArray(m_VertexArrayObject);
	glDrawElements(GL_TRIANGLES, m_MeshIndicesSet.size(), GL_UNSIGNED_INT, 0);
	
	glBindVertexArray(0);
	for (unsigned int i = 0; i < m_MeshTextureSet.size(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + vTextureUnit + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

//***********************************************************************************************
//Function:
void CGLMesh::__init()
{
	glGenVertexArrays(1, &m_VertexArrayObject);
	glBindVertexArray(m_VertexArrayObject);

	GLuint VBO, EBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, m_MeshVerticesSet.size() * sizeof(SMeshVertex), &m_MeshVerticesSet[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_MeshIndicesSet.size() * sizeof(GLuint), &m_MeshIndicesSet[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SMeshVertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SMeshVertex), (void*)offsetof(SMeshVertex, m_Normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SMeshVertex), (void*)offsetof(SMeshVertex, m_TexCoords));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SMeshVertex), (void*)offsetof(SMeshVertex, m_Tangent));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(SMeshVertex), (void*)offsetof(SMeshVertex, m_Bitangent));

	glBindVertexArray(0);
}

NAMESPACE_END(gl_kernel)