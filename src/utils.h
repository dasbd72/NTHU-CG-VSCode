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
    ViewCenter = 3,
    ViewEye = 4,
    ViewUp = 5,
};

typedef struct _Offset {
    GLfloat x;
    GLfloat y;
    /*struct _Offset(GLfloat _x, GLfloat _y) {
            x = _x;
            y = _y;
    };*/
} Offset;

typedef struct
{
    Vector3 Ka;
    Vector3 Kd;
    Vector3 Ks;

    GLuint diffuseTexture;

    // eye texture coordinate
    GLuint isEye;
    std::vector<Offset> offsets;
} PhongMaterial;

typedef struct
{
    GLuint vao;
    GLuint vbo;
    GLuint vboTex;
    GLuint ebo;
    GLuint p_color;
    int vertex_count;
    GLuint p_normal;
    GLuint p_texCoord;
    PhongMaterial material;
    int indexCount;
} Shape;

struct Model {
    Vector3 position = Vector3(0, 0, 0);
    Vector3 scale = Vector3(1, 1, 1);
    Vector3 rotation = Vector3(0, 0, 0);  // Euler form

    std::vector<Shape> shapes;

    bool hasEye;
    GLint max_eye_offset = 7;
    GLint cur_eye_offset_idx = 0;
};

struct Camera {
    Vector3 position;
    Vector3 center;
    Vector3 up_vector;
};

struct ProjectSetting {
    GLfloat nearClip, farClip;
    GLfloat fovy;
    GLfloat aspect;
    GLfloat left, right, top, bottom;
};

enum ProjMode {
    Orthogonal = 0,
    Perspective = 1,
};

GLvoid normalize(GLfloat v[3]);
GLvoid cross(GLfloat u[3], GLfloat v[3], GLfloat n[3]);
void normalization(tinyobj::attrib_t* attrib, std::vector<GLfloat>& vertices, std::vector<GLfloat>& colors, std::vector<GLfloat>& normals, std::vector<GLfloat>& textureCoords, std::vector<int>& material_id, tinyobj::shape_t* shape);
void loadTexturedModels(std::string model_path, Model& model);
void glPrintContextInfo(bool printExtension);

#endif  // UTILS_H_DEF