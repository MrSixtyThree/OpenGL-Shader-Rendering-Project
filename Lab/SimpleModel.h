#ifndef SIMPLE_MODEL_H
#define SIMPLE_MODEL_H

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // output data structure
#include <assimp/postprocess.h>     // post processing flags

#include "utilities.h"
#include "ShaderProgram.h"

struct Mesh
{
    // OpenGL buffer objects
    GLuint VBO = 0;
    GLuint IBO = 0;
    GLuint VAO = 0;
    int numOfIndices = 0;
    bool hasTexCoords = false;
};

/*****************************************************************
 * simple model class that loads the first mesh of a model
 *****************************************************************/
class SimpleModel
{
public:
    SimpleModel();
    ~SimpleModel();

    void loadModel(const char *filename, bool texture = false);
    void drawModel();

private:
    bool mIsValid = false;
    Mesh mMesh;
 
    void loadMesh(const aiMesh *mesh);
    void loadMeshWithTexture(const aiMesh* mesh);
};

#endif
