#include "SimpleModel.h"

SimpleModel::SimpleModel()
{}

SimpleModel::~SimpleModel()
{
	// delete mesh buffers
	if (mMesh.VBO != 0)
		glDeleteBuffers(1, &mMesh.VBO);
	if (mMesh.IBO != 0)
		glDeleteBuffers(1, &mMesh.IBO);
	if (mMesh.VAO != 0)
		glDeleteVertexArrays(1, &mMesh.VAO);

	mIsValid = false;
}

void SimpleModel::loadModel(const char *filename, bool texture)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;

	// load model file with assimp 
	const aiScene *scene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices);

	// check whether scene was loaded
	if (!scene)
	{
		// output error message and exit
		std::cerr << "Failed to open: " << filename << std::endl;
		exit(EXIT_FAILURE);
	}

	// only loads first mesh
	if(!texture)
		loadMesh(scene->mMeshes[0]);
	else
		loadMeshWithTexture(scene->mMeshes[0]);

	// importer's destructor will clean up
}

void SimpleModel::drawModel()
{
	if (mIsValid)
	{
		glBindVertexArray(mMesh.VAO);		// make mesh VAO active
		glDrawElements(GL_TRIANGLES, mMesh.numOfIndices, GL_UNSIGNED_INT, 0);	// render vertices
	}
}

void SimpleModel::loadMesh(const aiMesh *mesh)
{
	// mesh data
	std::vector<VertexNormal> vertices;
	std::vector<GLint> indices;

	// check if mesh contains vertex coordinates, normals and faces
	if (!mesh->HasPositions() || !mesh->HasNormals() || !mesh->HasFaces())
	{
		mIsValid = false;
		return;
	}

	// get vertex data
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		VertexNormal vertex;	// for vertex data

		// get vertex position
		vertex.position[0] = mesh->mVertices[i].x;
		vertex.position[1] = mesh->mVertices[i].y;
		vertex.position[2] = mesh->mVertices[i].z;

		// get vertex normal
		vertex.normal[0] = mesh->mNormals[i].x;
		vertex.normal[1] = mesh->mNormals[i].y;
		vertex.normal[2] = mesh->mNormals[i].z;

		// append vertex data
		vertices.push_back(vertex);
	}

	// get face data
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
		{
			// append face index
			indices.push_back(mesh->mFaces[i].mIndices[j]);
		}
	}

	// store total number of indices
	mMesh.numOfIndices = static_cast<int>(indices.size());

	// generate identifier for VBOs and copy data to GPU
	glGenBuffers(1, &mMesh.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, mMesh.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexNormal) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// generate identifier for IBO and copy data to GPU
	glGenBuffers(1, &mMesh.IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mMesh.IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// generate identifiers for VAO and supply information
	glGenVertexArrays(1, &mMesh.VAO);
	glBindVertexArray(mMesh.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, mMesh.VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mMesh.IBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormal), reinterpret_cast<void*>(offsetof(VertexNormal, position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormal), reinterpret_cast<void*>(offsetof(VertexNormal, normal)));

	// enable vertex attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// unbind VAO
	glBindVertexArray(0);

	mIsValid = true;
}

void SimpleModel::loadMeshWithTexture(const aiMesh* mesh)
{
	// mesh data
	std::vector<VertexNormTex> vertices;
	std::vector<GLint> indices;

	// check if mesh contains vertex coordinates, normals and faces
	if (!mesh->HasPositions() || !mesh->HasNormals() || !mesh->HasFaces())
	{
		mIsValid = false;
		return;
	}
	// check if mesh contains texture coordinates (i.e. index 0)
	if (mesh->HasTextureCoords(0))
	{
		mMesh.hasTexCoords = true;
	}

	// get vertex data
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		VertexNormTex vertex;	// for vertex data

		// get vertex position
		vertex.position[0] = mesh->mVertices[i].x;
		vertex.position[1] = mesh->mVertices[i].y;
		vertex.position[2] = mesh->mVertices[i].z;

		// get vertex normal
		vertex.normal[0] = mesh->mNormals[i].x;
		vertex.normal[1] = mesh->mNormals[i].y;
		vertex.normal[2] = mesh->mNormals[i].z;

		// get first vertex texture coordinate (i.e. index 0)
		if (mMesh.hasTexCoords)
		{
			vertex.texCoord[0] = mesh->mTextureCoords[0][i].x;
			vertex.texCoord[1] = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			vertex.texCoord[0] = vertex.texCoord[1] = 0.0f;
		}

		// append vertex data
		vertices.push_back(vertex);
	}

	// get face data
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
		{
			// append face index
			indices.push_back(mesh->mFaces[i].mIndices[j]);
		}
	}

	// store total number of indices
	mMesh.numOfIndices = indices.size();

	// generate identifier for VBOs and copy data to GPU
	glGenBuffers(1, &mMesh.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, mMesh.VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexNormTex), &vertices[0], GL_STATIC_DRAW);

	// generate identifier for IBO and copy data to GPU
	glGenBuffers(1, &mMesh.IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mMesh.IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLint), &indices[0], GL_STATIC_DRAW);

	// generate identifiers for VAO and supply information
	glGenVertexArrays(1, &mMesh.VAO);
	glBindVertexArray(mMesh.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, mMesh.VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mMesh.IBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex), reinterpret_cast<void*>(offsetof(VertexNormTex, position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex), reinterpret_cast<void*>(offsetof(VertexNormTex, normal)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex), reinterpret_cast<void*>(offsetof(VertexNormTex, texCoord)));

	// enable vertex attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// unbind VAO
	glBindVertexArray(0);

	mIsValid = true;
}
