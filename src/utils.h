#ifndef UTILS_H_DEF
#define UTILS_H_DEF

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <vector>

#include "Vectors.h"
#include "tiny_obj_loader.h"

enum TransMode {
    GeoTranslation = 0,
    GeoRotation = 1,
    GeoScaling = 2,
    LightEdit = 3,
    ShininessEdit = 4,
};

struct Uniform {
    GLint iLocMVP;
};

struct PhongMaterial {
    Vector3 Ka;
    Vector3 Kd;
    Vector3 Ks;
};

typedef struct
{
    GLuint vao;
    GLuint vbo;
    GLuint vboTex;
    GLuint ebo;
    GLuint p_color;
    int vertex_count;
    GLuint p_normal;
    PhongMaterial material;
    int indexCount;
    GLuint m_texture;
} Shape;

struct model {
    Vector3 position = Vector3(0, 0, 0);
    Vector3 scale = Vector3(1, 1, 1);
    Vector3 rotation = Vector3(0, 0, 0);  // Euler form

    std::vector<Shape> shapes;
};

struct camera {
    Vector3 position;
    Vector3 center;
    Vector3 up_vector;
};

struct project_setting {
    GLfloat nearClip, farClip;
    GLfloat fovy;
    GLfloat aspect;
    GLfloat left, right, top, bottom;
};

static GLvoid Normalize(GLfloat v[3]);
static GLvoid Cross(GLfloat u[3], GLfloat v[3], GLfloat n[3]);
void normalization(tinyobj::attrib_t* attrib, std::vector<GLfloat>& vertices, std::vector<GLfloat>& colors, std::vector<GLfloat>& normals, tinyobj::shape_t* shape);
void loadModel(std::string model_path, model& model);
void glPrintContextInfo(bool printExtension);

#endif  // UTILS_H_DEF