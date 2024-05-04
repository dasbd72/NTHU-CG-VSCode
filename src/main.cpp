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
#include "utils.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

using namespace std;

// Default window size
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

bool mouse_pressed = false;
int starting_press_x = -1;
int starting_press_y = -1;

Uniform uniform;
vector<string> filenames;  // .obj filename list
vector<model> models;
camera main_camera;
project_setting proj;
TransMode cur_trans_mode = GeoTranslation;
Matrix4 view_matrix;
Matrix4 project_matrix;

int cur_idx = 0;  // represent which model should be rendered now

Matrix4 translate(Vector3 vec);
Matrix4 scaling(Vector3 vec);
Matrix4 rotateX(GLfloat val);
Matrix4 rotateY(GLfloat val);
Matrix4 rotateZ(GLfloat val);
Matrix4 rotate(Vector3 vec);
void setViewingMatrix();
void setPerspective();
void setGLMatrix(GLfloat* glm, Matrix4& m);
void changeSize(GLFWwindow* window, int width, int height);
void renderScene(void);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void setShaders();
void initParameter();
void setupRC();

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
    // view_matrix[...] = ...
}

// [TODO] compute persepective projection matrix
void setPerspective() {
    // GLfloat f = ...
    // project_matrix [...] = ...
}

void setGLMatrix(GLfloat* glm, Matrix4& m) {
    glm[0] = m[0];
    glm[4] = m[1];
    glm[8] = m[2];
    glm[12] = m[3];
    glm[1] = m[4];
    glm[5] = m[5];
    glm[9] = m[6];
    glm[13] = m[7];
    glm[2] = m[8];
    glm[6] = m[9];
    glm[10] = m[10];
    glm[14] = m[11];
    glm[3] = m[12];
    glm[7] = m[13];
    glm[11] = m[14];
    glm[15] = m[15];
}

// Vertex buffers
GLuint VAO, VBO;

// Call back function for window reshape
void changeSize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    // [TODO] change your aspect ratio
}

// Render function for display rendering
void renderScene(void) {
    // clear canvas
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    Matrix4 T, R, S;
    // [TODO] update translation, rotation and scaling

    Matrix4 MVP;
    GLfloat mvp[16];

    // [TODO] multiply all the matrix
    // row-major ---> column-major
    setGLMatrix(mvp, MVP);

    // use uniform to send mvp to vertex shader
    glUniformMatrix4fv(uniform.iLocMVP, 1, GL_FALSE, mvp);
    for (int i = 0; i < models[cur_idx].shapes.size(); i++) {
        // set glViewport and draw twice ...
        glBindVertexArray(models[cur_idx].shapes[i].vao);
        glDrawArrays(GL_TRIANGLES, 0, models[cur_idx].shapes[i].vertex_count);
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // [TODO] Call back function for keyboard
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // [TODO] scroll up positive, otherwise it would be negtive
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // [TODO] mouse press callback function
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
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

    uniform.iLocMVP = glGetUniformLocation(p, "mvp");

    if (success)
        glUseProgram(p);
    else {
        system("pause");
        exit(123);
    }
}

void initParameter() {
    // [TODO] Setup some parameters if you need
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
    vector<string> model_list{"../NormalModels/bunny5KN.obj", "../NormalModels/dragon10KN.obj", "../NormalModels/lucy25KN.obj", "../NormalModels/teapot4KN.obj", "../NormalModels/dolphinN.obj"};
    // [TODO] Load five model at here
    model tmp_model;
    loadModel(model_list[cur_idx], tmp_model);
    models.push_back(tmp_model);
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
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Student ID HW2", NULL, NULL);
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
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    glfwSetFramebufferSizeCallback(window, changeSize);
    glEnable(GL_DEPTH_TEST);
    // Setup render context
    setupRC();

    // main loop
    while (!glfwWindowShouldClose(window)) {
        // render
        renderScene();

        // swap buffer from back to front
        glfwSwapBuffers(window);

        // Poll input event
        glfwPollEvents();
    }

    // just for compatibiliy purposes
    return 0;
}
