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
    LightEdit = 6,
    ShininessEdit = 7,
};

struct Uniform {
    struct Transform {
        GLint model;
        GLint view;
        GLint projection;
    } trans;

    struct Material {
        GLint ambient;
        GLint diffuse;
        GLint specular;
        GLint shininess;
    } mat;

    struct DirectionalLight {
        GLint direction;
        GLint ambient;
        GLint diffuse;
        GLint specular;
    } dir_light;

    struct PointLight {
        GLint position;
        GLint ambient;
        GLint diffuse;
        GLint specular;
        GLint attenuation_coefficient[3];
    } point_light;

    struct SpotLight {
        GLint position;
        GLint direction;
        GLint ambient;
        GLint diffuse;
        GLint specular;
        GLint attenuation_coefficient[3];
        GLint exponent;
        GLint cutoff_angle;
    } spot_light;

    GLint camera_pos;
    GLint light_mode;
    GLint per_fragment;
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

enum LightMode {
    DirectionalLight = 0,
    PointLight = 1,
    SpotlightLight = 2,
};

struct Lighting {
    GLfloat shininess;

    // Directional Light
    struct DirectionalLight {
        Vector3 position;
        Vector3 center;
        Vector3 ambient;
        Vector3 diffuse;
        Vector3 specular;
    } dir_light;

    // Point Light
    struct PointLight {
        Vector3 position;
        Vector3 ambient;
        Vector3 diffuse;
        Vector3 specular;
        GLfloat attenuation_coefficient[3];
    } point_light;

    // Spot Light
    struct SpotLight {
        Vector3 position;
        Vector3 direction;
        Vector3 ambient;
        Vector3 diffuse;
        Vector3 specular;
        GLfloat attenuation_coefficient[3];
        GLfloat exponent;
        GLfloat cutoff_angle;
    } spot_light;
};

enum MinFilterMode {
    MinFilterNearest = 0,
    MinFilterLinear = 1,
};

enum MagFilterMode {
    MagFilterNearest = 0,
    MagFilterLinearMipmapLinear = 1,
};

GLvoid normalize(GLfloat v[3]);
GLvoid cross(GLfloat u[3], GLfloat v[3], GLfloat n[3]);
void normalization(tinyobj::attrib_t* attrib, std::vector<GLfloat>& vertices, std::vector<GLfloat>& colors, std::vector<GLfloat>& normals, std::vector<GLfloat>& textureCoords, std::vector<int>& material_id, tinyobj::shape_t* shape);
void loadTexturedModels(std::string model_path, Model& model);
void glPrintContextInfo(bool printExtension);

#endif  // UTILS_H_DEF