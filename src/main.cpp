#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <math.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Matrices.h"
#include "Vectors.h"
#include "textfile.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

using namespace std;

// Default window size
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

bool mouse_pressed = false;
int starting_press_x = -1;
int starting_press_y = -1;

enum TransMode {
    GeoTranslation = 0,
    GeoRotation = 1,
    GeoScaling = 2,
    ViewCenter = 3,
    ViewEye = 4,
    ViewUp = 5,
};

GLint iLocMVP;

vector<string> filenames;  // .obj filename list

struct model {
    Vector3 position = Vector3(0, 0, 0);
    Vector3 scale = Vector3(1, 1, 1);
    Vector3 rotation = Vector3(0, 0, 0);  // Euler form
};
vector<model> models;

struct camera {
    Vector3 position;
    Vector3 center;
    Vector3 up_vector;
};
camera main_camera;

struct project_setting {
    GLfloat nearClip, farClip;
    GLfloat fovy;
    GLfloat aspect;
    GLfloat left, right, top, bottom;
};
project_setting proj;

enum ProjMode {
    Orthogonal = 0,
    Perspective = 1,
};
ProjMode cur_proj_mode = Orthogonal;
TransMode cur_trans_mode = GeoTranslation;

Matrix4 view_matrix;
Matrix4 project_matrix;

typedef struct
{
    GLuint vao;
    GLuint vbo;
    GLuint vboTex;
    GLuint ebo;
    GLuint p_color;
    int vertex_count;
    GLuint p_normal;
    int materialId;
    int indexCount;
    GLuint m_texture;
} Shape;
Shape quad;
Shape m_shpae;
vector<Shape> m_shape_list;
int cur_idx = 0;       // represent which model should be rendered now
int polygon_mode = 0;  // 0:fill 1:line

static GLvoid Normalize(GLfloat v[3]) {
    GLfloat l;

    l = (GLfloat)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
}

static GLvoid Cross(GLfloat u[3], GLfloat v[3], GLfloat n[3]) {
    n[0] = u[1] * v[2] - u[2] * v[1];
    n[1] = u[2] * v[0] - u[0] * v[2];
    n[2] = u[0] * v[1] - u[1] * v[0];
}

// [TODO] given a translation vector then output a Matrix4 (Translation Matrix)
Matrix4 translate(Vector3 vec) {
    Matrix4 mat;

    /*
    mat = Matrix4(
        ...
    );
    */

    return mat;
}

// [TODO] given a scaling vector then output a Matrix4 (Scaling Matrix)
Matrix4 scaling(Vector3 vec) {
    Matrix4 mat;

    /*
    mat = Matrix4(
            ...
    );
    */

    return mat;
}

// [TODO] given a float value then ouput a rotation matrix alone axis-X (rotate alone axis-X)
Matrix4 rotateX(GLfloat val) {
    Matrix4 mat;

    /*
    mat = Matrix4(
            ...
    );
    */

    return mat;
}

// [TODO] given a float value then ouput a rotation matrix alone axis-Y (rotate alone axis-Y)
Matrix4 rotateY(GLfloat val) {
    Matrix4 mat;

    /*
    mat = Matrix4(
            ...
    );
    */

    return mat;
}

// [TODO] given a float value then ouput a rotation matrix alone axis-Z (rotate alone axis-Z)
Matrix4 rotateZ(GLfloat val) {
    Matrix4 mat;

    /*
    mat = Matrix4(
            ...
    );
    */

    return mat;
}

Matrix4 rotate(Vector3 vec) {
    return rotateX(vec.x) * rotateY(vec.y) * rotateZ(vec.z);
}

// [TODO] compute viewing matrix accroding to the setting of main_camera
void setViewingMatrix() {
    Vector3 p_c = main_camera.center - main_camera.position;
    Vector3 p_u = main_camera.up_vector - main_camera.position;
    Vector3 rx = p_c.cross(p_u) / (p_c.cross(p_u)).length();
    Vector3 rz = -p_c / p_c.length();
    Vector3 ry = rz.cross(rx);

    view_matrix =
        Matrix4(
            rx.x, rx.y, rx.z, 0.0f,
            ry.x, ry.y, ry.z, 0.0f,
            rz.x, rz.y, rz.z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f) *
        Matrix4(
            1.0f, 0.0f, 0.0f, -main_camera.position.x,
            0.0f, 1.0f, 0.0f, -main_camera.position.y,
            0.0f, 0.0f, 1.0f, -main_camera.position.z,
            0.0f, 0.0f, 0.0f, 1.0f);
}

// [TODO] compute orthogonal projection matrix
void setOrthogonal() {
    cur_proj_mode = Orthogonal;

    float x_diff = proj.right - proj.left;
    float x_sum = proj.right + proj.left;
    float y_diff = proj.top - proj.bottom;
    float y_sum = proj.top + proj.bottom;
    float z_diff = proj.farClip - proj.nearClip;
    float z_sum = proj.farClip + proj.nearClip;

    project_matrix = Matrix4(
        2.0f / x_diff, 0.0f, 0.0f, -x_sum / x_diff,
        0.0f, 2.0f / y_diff, 0.0f, -y_sum / y_diff,
        0.0f, 0.0f, -2.0f / z_diff, -z_sum / z_diff,
        0.0f, 0.0f, 0.0f, 1.0f);
}

// [TODO] compute persepective projection matrix
void setPerspective() {
    cur_proj_mode = Perspective;

    float fovy = proj.fovy / 180.0 * M_PI / 2.0;
    float half_height = proj.nearClip * tan(fovy);
    float half_width = half_height * proj.aspect;

    float x_min = -half_width;
    float x_max = half_width;
    float y_min = -half_height;
    float y_max = half_height;
    float z_near = proj.nearClip;
    float z_far = proj.farClip;

    float x_diff = x_max - x_min;
    float x_sum = x_max + x_min;
    float y_diff = y_max - y_min;
    float y_sum = y_max + y_min;
    float z_diff = z_far - z_near;
    float z_sum = z_far + z_near;
    float z_mult = z_far * z_near;

    // Derived from
    // project_matrix =
    //     Matrix4(
    //         2.0f / x_diff, 0.0f, 0.0f, -x_sum / x_diff,
    //         0.0f, 2.0f / y_diff, 0.0f, -y_sum / y_diff,
    //         0.0f, 0.0f, 2.0f / z_diff, -z_sum / z_diff,
    //         0.0f, 0.0f, 0.0f, 1.0f) *
    //     Matrix4(
    //         z_near, 0.0f, 0.0f, 0.0f,
    //         0.0f, z_near, 0.0f, 0.0f,
    //         0.0f, 0.0f, -z_sum, -z_mult, // Modded
    //         0.0f, 0.0f, -1.0f, 0.0f); // Modded
    project_matrix = Matrix4(
        2.0f * proj.nearClip / x_diff, 0.0f, -x_sum / x_diff, 0.0f,
        0.0f, 2.0f * proj.nearClip / y_diff, -y_sum / y_diff, 0.0f,
        0.0f, 0.0f, -z_sum / z_diff, -2.0f * z_mult / z_diff,
        0.0f, 0.0f, -1.0f, 0.0f);
}

// Vertex buffers
GLuint VAO, VBO;

// Call back function for window reshape
void ChangeSize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    // [TODO] change your aspect ratio
}

void drawPlane() {
    GLfloat vertices[18]{1.0, -0.9, -1.0,
                         1.0, -0.9, 1.0,
                         -1.0, -0.9, -1.0,
                         1.0, -0.9, 1.0,
                         -1.0, -0.9, 1.0,
                         -1.0, -0.9, -1.0};

    GLfloat colors[18]{0.0, 1.0, 0.0,
                       0.0, 0.5, 0.8,
                       0.0, 1.0, 0.0,
                       0.0, 0.5, 0.8,
                       0.0, 0.5, 0.8,
                       0.0, 1.0, 0.0};

    // [TODO] draw the plane with above vertices and color
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    Matrix4 T, R, S;
    T = translate(Vector3(0, 0, 0));
    R = rotate(Vector3(0, 0, 0));
    S = scaling(Vector3(1, 1, 1));

    Matrix4 MVP = project_matrix * view_matrix * T * R * S;
    MVP.transpose();
    glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, MVP.get());

    Shape plane;
    glGenVertexArrays(1, &plane.vao);
    glBindVertexArray(plane.vao);

    glGenBuffers(1, &plane.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, plane.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    plane.vertex_count = 6;

    glGenBuffers(1, &plane.p_color);
    glBindBuffer(GL_ARRAY_BUFFER, plane.p_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, plane.vertex_count);
}

// Render function for display rendering
void RenderScene(void) {
    // clear canvas
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (polygon_mode == 0) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else if (polygon_mode == 1) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    Matrix4 T, R, S;
    // [TODO] update translation, rotation and scaling
    T = translate(models[cur_idx].position);
    R = rotate(models[cur_idx].rotation);
    S = scaling(models[cur_idx].scale);

    Matrix4 MVP;

    // [TODO] multiply all the matrix
    // [TODO] row-major ---> column-major
    MVP = project_matrix * view_matrix * T * R * S;
    MVP.transpose();

    // use uniform to send mvp to vertex shader
    glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, MVP.get());
    glBindVertexArray(m_shape_list[cur_idx].vao);
    glDrawArrays(GL_TRIANGLES, 0, m_shape_list[cur_idx].vertex_count);
    drawPlane();
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // [TODO] Call back function for keyboard
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_W:
                polygon_mode = 1 - polygon_mode;
                break;
            case GLFW_KEY_Z:
                cur_idx = (cur_idx + 4) % 5;
                break;
            case GLFW_KEY_X:
                cur_idx = (cur_idx + 1) % 5;
                break;
            case GLFW_KEY_O:
                setOrthogonal();
                break;
            case GLFW_KEY_P:
                setPerspective();
                break;
            case GLFW_KEY_I:
                std::cout << "Matrix Value:\n"
                          << "Viewing Matrix:\n"
                          << view_matrix << "\n"
                          << "Projection Matrix:\n"
                          << project_matrix << "\n"
                          << "Translation Matrix:\n"
                          << translate(models[cur_idx].position) << "\n"
                          << "Rotation Matrix:\n"
                          << rotate(models[cur_idx].rotation) << "\n"
                          << "Scaling Matrix:\n"
                          << scaling(models[cur_idx].scale) << "\n";
                break;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // [TODO] scroll up positive, otherwise it would be negtive
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // [TODO] mouse press callback function
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    // [TODO] cursor position callback function
}

void setShaders() {
    GLuint v, f, p;
    char* vs = NULL;
    char* fs = NULL;

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);

    vs = textFileRead("shader.vs");
    fs = textFileRead("shader.fs");

    glShaderSource(v, 1, (const GLchar**)&vs, NULL);
    glShaderSource(f, 1, (const GLchar**)&fs, NULL);

    free(vs);
    free(fs);

    GLint success;
    char infoLog[1000];
    // compile vertex shader
    glCompileShader(v);
    // check for shader compile errors
    glGetShaderiv(v, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(v, 1000, NULL, infoLog);
        std::cout << "ERROR: VERTEX SHADER COMPILATION FAILED\n"
                  << infoLog << std::endl;
    }

    // compile fragment shader
    glCompileShader(f);
    // check for shader compile errors
    glGetShaderiv(f, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(f, 1000, NULL, infoLog);
        std::cout << "ERROR: FRAGMENT SHADER COMPILATION FAILED\n"
                  << infoLog << std::endl;
    }

    // create program object
    p = glCreateProgram();

    // attach shaders to program object
    glAttachShader(p, f);
    glAttachShader(p, v);

    // link program
    glLinkProgram(p);
    // check for linking errors
    glGetProgramiv(p, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(p, 1000, NULL, infoLog);
        std::cout << "ERROR: SHADER PROGRAM LINKING FAILED\n"
                  << infoLog << std::endl;
    }

    glDeleteShader(v);
    glDeleteShader(f);

    iLocMVP = glGetUniformLocation(p, "mvp");

    if (success)
        glUseProgram(p);
    else {
        system("pause");
        exit(123);
    }
}

void normalization(tinyobj::attrib_t* attrib, vector<GLfloat>& vertices, vector<GLfloat>& colors, tinyobj::shape_t* shape) {
    vector<float> xVector, yVector, zVector;
    float minX = 10000, maxX = -10000, minY = 10000, maxY = -10000, minZ = 10000, maxZ = -10000;

    // find out min and max value of X, Y and Z axis
    for (int i = 0; i < attrib->vertices.size(); i++) {
        // maxs = max(maxs, attrib->vertices.at(i));
        if (i % 3 == 0) {
            xVector.push_back(attrib->vertices.at(i));

            if (attrib->vertices.at(i) < minX) {
                minX = attrib->vertices.at(i);
            }

            if (attrib->vertices.at(i) > maxX) {
                maxX = attrib->vertices.at(i);
            }
        } else if (i % 3 == 1) {
            yVector.push_back(attrib->vertices.at(i));

            if (attrib->vertices.at(i) < minY) {
                minY = attrib->vertices.at(i);
            }

            if (attrib->vertices.at(i) > maxY) {
                maxY = attrib->vertices.at(i);
            }
        } else if (i % 3 == 2) {
            zVector.push_back(attrib->vertices.at(i));

            if (attrib->vertices.at(i) < minZ) {
                minZ = attrib->vertices.at(i);
            }

            if (attrib->vertices.at(i) > maxZ) {
                maxZ = attrib->vertices.at(i);
            }
        }
    }

    float offsetX = (maxX + minX) / 2;
    float offsetY = (maxY + minY) / 2;
    float offsetZ = (maxZ + minZ) / 2;

    for (int i = 0; i < attrib->vertices.size(); i++) {
        if (offsetX != 0 && i % 3 == 0) {
            attrib->vertices.at(i) = attrib->vertices.at(i) - offsetX;
        } else if (offsetY != 0 && i % 3 == 1) {
            attrib->vertices.at(i) = attrib->vertices.at(i) - offsetY;
        } else if (offsetZ != 0 && i % 3 == 2) {
            attrib->vertices.at(i) = attrib->vertices.at(i) - offsetZ;
        }
    }

    float greatestAxis = maxX - minX;
    float distanceOfYAxis = maxY - minY;
    float distanceOfZAxis = maxZ - minZ;

    if (distanceOfYAxis > greatestAxis) {
        greatestAxis = distanceOfYAxis;
    }

    if (distanceOfZAxis > greatestAxis) {
        greatestAxis = distanceOfZAxis;
    }

    float scale = greatestAxis / 2;

    for (int i = 0; i < attrib->vertices.size(); i++) {
        // std::cout << i << " = " << (double)(attrib.vertices.at(i) / greatestAxis) << std::endl;
        attrib->vertices.at(i) = attrib->vertices.at(i) / scale;
    }
    size_t index_offset = 0;
    vertices.reserve(shape->mesh.num_face_vertices.size() * 3);
    colors.reserve(shape->mesh.num_face_vertices.size() * 3);
    for (size_t f = 0; f < shape->mesh.num_face_vertices.size(); f++) {
        int fv = shape->mesh.num_face_vertices[f];

        // Loop over vertices in the face.
        for (size_t v = 0; v < fv; v++) {
            // access to vertex
            tinyobj::index_t idx = shape->mesh.indices[index_offset + v];
            vertices.push_back(attrib->vertices[3 * idx.vertex_index + 0]);
            vertices.push_back(attrib->vertices[3 * idx.vertex_index + 1]);
            vertices.push_back(attrib->vertices[3 * idx.vertex_index + 2]);
            // Optional: vertex colors
            colors.push_back(attrib->colors[3 * idx.vertex_index + 0]);
            colors.push_back(attrib->colors[3 * idx.vertex_index + 1]);
            colors.push_back(attrib->colors[3 * idx.vertex_index + 2]);
        }
        index_offset += fv;
    }
}

void LoadModels(string model_path) {
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;
    tinyobj::attrib_t attrib;
    vector<GLfloat> vertices;
    vector<GLfloat> colors;

    string err;
    string warn;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str());

    if (!warn.empty()) {
        cout << warn << std::endl;
    }

    if (!err.empty()) {
        cerr << err << std::endl;
    }

    if (!ret) {
        exit(1);
    }

    std::cout << "Load Model " << model_path
              << " Success! Shapes size " << shapes.size()
              << " Materials size " << materials.size() << "\n";

    normalization(&attrib, vertices, colors, &shapes[0]);

    Shape tmp_shape;
    glGenVertexArrays(1, &tmp_shape.vao);
    glBindVertexArray(tmp_shape.vao);

    glGenBuffers(1, &tmp_shape.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GL_FLOAT), &vertices.at(0), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    tmp_shape.vertex_count = vertices.size() / 3;

    glGenBuffers(1, &tmp_shape.p_color);
    glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.p_color);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GL_FLOAT), &colors.at(0), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    m_shape_list.push_back(tmp_shape);
    model tmp_model;
    models.push_back(tmp_model);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    shapes.clear();
    materials.clear();
}

void initParameter() {
    proj.left = -1;
    proj.right = 1;
    proj.top = 1;
    proj.bottom = -1;
    proj.nearClip = 0.001;
    proj.farClip = 100.0;
    proj.fovy = 80;
    proj.aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    main_camera.position = Vector3(0.0f, 0.0f, 2.0f);
    main_camera.center = Vector3(0.0f, 0.0f, 0.0f);
    main_camera.up_vector = Vector3(0.0f, 1.0f, 0.0f);

    setViewingMatrix();
    setPerspective();  // set default projection matrix as perspective matrix
}

void setupRC() {
    // setup shaders
    setShaders();
    initParameter();

    // OpenGL States and Values
    glClearColor(0.2, 0.2, 0.2, 1.0);
    vector<string> model_list{"../ColorModels/bunny5KC.obj", "../ColorModels/dragon10KC.obj", "../ColorModels/lucy25KC.obj", "../ColorModels/teapot4KC.obj", "../ColorModels/dolphinC.obj"};
    // [TODO] Load five model at here
    for (auto model : model_list)
        LoadModels(model);
}

void glPrintContextInfo(bool printExtension) {
    cout << "GL_VENDOR = " << (const char*)glGetString(GL_VENDOR) << endl;
    cout << "GL_RENDERER = " << (const char*)glGetString(GL_RENDERER) << endl;
    cout << "GL_VERSION = " << (const char*)glGetString(GL_VERSION) << endl;
    cout << "GL_SHADING_LANGUAGE_VERSION = " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    if (printExtension) {
        GLint numExt;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
        cout << "GL_EXTENSIONS =" << endl;
        for (GLint i = 0; i < numExt; i++) {
            cout << "\t" << (const char*)glGetStringi(GL_EXTENSIONS, i) << endl;
        }
    }
}

int main(int argc, char** argv) {
    // initial glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // fix compilation on OS X
#endif

    // create window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "109062131 HW1", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // load OpenGL function pointer
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // register glfw callback functions
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    glfwSetFramebufferSizeCallback(window, ChangeSize);
    glEnable(GL_DEPTH_TEST);
    // Setup render context
    setupRC();

    // main loop
    while (!glfwWindowShouldClose(window)) {
        // render
        RenderScene();

        // swap buffer from back to front
        glfwSwapBuffers(window);

        // Poll input event
        glfwPollEvents();
    }

    // just for compatibiliy purposes
    return 0;
}
