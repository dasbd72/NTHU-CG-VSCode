#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

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
const int WINDOW_HEIGHT = 600;
// current window size
int screenWidth = WINDOW_WIDTH, screenHeight = WINDOW_HEIGHT;
bool mouse_pressed = false;
int starting_press_x = -1;
int starting_press_y = -1;
vector<string> filenames;  // .obj filename list
vector<Model> models;
Camera main_camera;
ProjectSetting proj;
ProjMode cur_proj_mode = Orthogonal;
TransMode cur_trans_mode = GeoTranslation;
Matrix4 view_matrix;
Matrix4 project_matrix;
Shape m_shpae;

int cur_idx = 0;  // represent which model should be rendered now
vector<string> model_list{"../TextureModels/Fushigidane.obj", "../TextureModels/Mew.obj", "../TextureModels/Nyarth.obj", "../TextureModels/Zenigame.obj", "../TextureModels/laurana500.obj", "../TextureModels/Nala.obj", "../TextureModels/Square.obj"};

// program location
GLuint program;
// uniforms location
GLuint iLocP;
GLuint iLocV;
GLuint iLocM;

Matrix4 translate(Vector3 vec);
Matrix4 scaling(Vector3 vec);
Matrix4 rotateX(GLfloat val);
Matrix4 rotateY(GLfloat val);
Matrix4 rotateZ(GLfloat val);
Matrix4 rotate(Vector3 vec);
void setViewingMatrix();
void setOrthogonal();
void setPerspective();
void changeSize(GLFWwindow* window, int width, int height);
void vector3ToFloat4(Vector3 v, GLfloat res[4]);
void renderScene(int per_vertex_or_per_pixel);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void setShaders();
void initParameter();
void setUniformVariables();
void setupRC();

Matrix4 translate(Vector3 vec) {
    Matrix4 mat;

    mat = Matrix4(
        1.0f, 0.0f, 0.0f, vec.x,
        0.0f, 1.0f, 0.0f, vec.y,
        0.0f, 0.0f, 1.0f, vec.z,
        0.0f, 0.0f, 0.0f, 1.0f);

    return mat;
}

Matrix4 scaling(Vector3 vec) {
    Matrix4 mat;

    mat = Matrix4(
        vec.x, 0.0f, 0.0f, 0.0f,
        0.0f, vec.y, 0.0f, 0.0f,
        0.0f, 0.0f, vec.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return mat;
}

Matrix4 rotateX(GLfloat val) {
    Matrix4 mat;

    mat = Matrix4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cosf(val), -sinf(val), 0.0f,
        0.0f, sinf(val), cosf(val), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return mat;
}

Matrix4 rotateY(GLfloat val) {
    Matrix4 mat;

    mat = Matrix4(
        cosf(val), 0.0f, sinf(val), 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -sinf(val), 0.0f, cosf(val), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return mat;
}

Matrix4 rotateZ(GLfloat val) {
    Matrix4 mat;

    mat = Matrix4(
        cosf(val), -sinf(val), 0.0f, 0.0f,
        sinf(val), cosf(val), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return mat;
}

Matrix4 rotate(Vector3 vec) {
    return rotateX(vec.x) * rotateY(vec.y) * rotateZ(vec.z);
}

void setViewingMatrix() {
    float F[3] = {main_camera.position.x - main_camera.center.x, main_camera.position.y - main_camera.center.y, main_camera.position.z - main_camera.center.z};
    float U[3] = {main_camera.up_vector.x, main_camera.up_vector.y, main_camera.up_vector.z};
    float R[3];
    normalize(F);
    cross(U, F, R);
    normalize(R);
    cross(F, R, U);
    normalize(U);

    view_matrix[0] = R[0];
    view_matrix[1] = R[1];
    view_matrix[2] = R[2];
    view_matrix[3] = 0;
    view_matrix[4] = U[0];
    view_matrix[5] = U[1];
    view_matrix[6] = U[2];
    view_matrix[7] = 0;
    view_matrix[8] = F[0];
    view_matrix[9] = F[1];
    view_matrix[10] = F[2];
    view_matrix[11] = 0;
    view_matrix[12] = 0;
    view_matrix[13] = 0;
    view_matrix[14] = 0;
    view_matrix[15] = 1;

    view_matrix = view_matrix * translate(-main_camera.position);
}

void setOrthogonal() {
    cur_proj_mode = Orthogonal;
    // handle side by side view
    float right = proj.right / 2;
    float left = proj.left / 2;
    project_matrix[0] = 2 / (right - left);
    project_matrix[1] = 0;
    project_matrix[2] = 0;
    project_matrix[3] = -(right + left) / (right - left);
    project_matrix[4] = 0;
    project_matrix[5] = 2 / (proj.top - proj.bottom);
    project_matrix[6] = 0;
    project_matrix[7] = -(proj.top + proj.bottom) / (proj.top - proj.bottom);
    project_matrix[8] = 0;
    project_matrix[9] = 0;
    project_matrix[10] = -2 / (proj.farClip - proj.nearClip);
    project_matrix[11] = -(proj.farClip + proj.nearClip) / (proj.farClip - proj.nearClip);
    project_matrix[12] = 0;
    project_matrix[13] = 0;
    project_matrix[14] = 0;
    project_matrix[15] = 1;
}

void setPerspective() {
    const float tanHalfFOV = tanf((proj.fovy / 2.0) / 180.0 * acosf(-1.0));

    cur_proj_mode = Perspective;
    project_matrix[0] = 1.0f / (tanHalfFOV * proj.aspect);
    project_matrix[1] = 0;
    project_matrix[2] = 0;
    project_matrix[3] = 0;
    project_matrix[4] = 0;
    project_matrix[5] = 1.0f / tanHalfFOV;
    project_matrix[6] = 0;
    project_matrix[7] = 0;
    project_matrix[8] = 0;
    project_matrix[9] = 0;
    project_matrix[10] = -(proj.farClip + proj.nearClip) / (proj.farClip - proj.nearClip);
    project_matrix[11] = -(2 * proj.farClip * proj.nearClip) / (proj.farClip - proj.nearClip);
    project_matrix[12] = 0;
    project_matrix[13] = 0;
    project_matrix[14] = -1;
    project_matrix[15] = 0;
}

// Call back function for window reshape
void changeSize(GLFWwindow* window, int width, int height) {
    // glViewport(0, 0, width, height);
    proj.aspect = (float)(width / 2) / (float)height;
    if (cur_proj_mode == Perspective) {
        setPerspective();
    }

    screenWidth = width;
    screenHeight = height;
}

void vector3ToFloat4(Vector3 v, GLfloat res[4]) {
    res[0] = v.x;
    res[1] = v.y;
    res[2] = v.z;
    res[3] = 1;
}

// Render function for display rendering
void renderScene(int per_vertex_or_per_pixel) {
    Vector3 modelPos = models[cur_idx].position;

    Matrix4 T, R, S;
    T = translate(models[cur_idx].position);
    R = rotate(models[cur_idx].rotation);
    S = scaling(models[cur_idx].scale);

    // render object
    Matrix4 model_matrix = T * R * S;
    glUniformMatrix4fv(iLocM, 1, GL_FALSE, model_matrix.getTranspose());
    glUniformMatrix4fv(iLocV, 1, GL_FALSE, view_matrix.getTranspose());
    glUniformMatrix4fv(iLocP, 1, GL_FALSE, project_matrix.getTranspose());

    for (int i = 0; i < models[cur_idx].shapes.size(); i++) {
        glBindVertexArray(models[cur_idx].shapes[i].vao);

        // [TODO] Bind texture and modify texture filtering & wrapping mode
        // Hint: glActiveTexture, glBindTexture, glTexParameteri

        glDrawArrays(GL_TRIANGLES, 0, models[cur_idx].shapes[i].vertex_count);
    }
}

// Call back function for keyboard
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                exit(0);
                break;
            case GLFW_KEY_Z:
                cur_idx = (cur_idx + 1) % model_list.size();
                break;
            case GLFW_KEY_X:
                cur_idx = (cur_idx - 1 + model_list.size()) % model_list.size();
                break;
            case GLFW_KEY_O:
                if (cur_proj_mode == Perspective) {
                    proj.farClip -= 3.0f;
                    setViewingMatrix();
                    setOrthogonal();
                }
                break;
            case GLFW_KEY_P:
                if (cur_proj_mode == Orthogonal) {
                    proj.farClip += 3.0f;
                    setViewingMatrix();
                    setPerspective();
                }
                break;
            case GLFW_KEY_T:
                cur_trans_mode = GeoTranslation;
                break;
            case GLFW_KEY_S:
                cur_trans_mode = GeoScaling;
                break;
            case GLFW_KEY_R:
                cur_trans_mode = GeoRotation;
                break;
            case GLFW_KEY_E:
                cur_trans_mode = ViewEye;
                break;
            case GLFW_KEY_C:
                cur_trans_mode = ViewCenter;
                break;
            case GLFW_KEY_U:
                cur_trans_mode = ViewUp;
                break;
            case GLFW_KEY_I:
                cout << endl;
                break;
            default:
                break;
        }
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // scroll up positive, otherwise it would be negtive
    switch (cur_trans_mode) {
        case ViewEye:
            main_camera.position.z -= 0.025 * (float)yoffset;
            setViewingMatrix();
            cout << "Camera Position = ( " << main_camera.position.x << " , " << main_camera.position.y << " , " << main_camera.position.z << " )" << endl;
            break;
        case ViewCenter:
            main_camera.center.z += 0.1 * (float)yoffset;
            setViewingMatrix();
            cout << "Camera Viewing Direction = ( " << main_camera.center.x << " , " << main_camera.center.y << " , " << main_camera.center.z << " )" << endl;
            break;
        case ViewUp:
            main_camera.up_vector.z += 0.33 * (float)yoffset;
            setViewingMatrix();
            cout << "Camera Up Vector = ( " << main_camera.up_vector.x << " , " << main_camera.up_vector.y << " , " << main_camera.up_vector.z << " )" << endl;
            break;
        case GeoTranslation:
            models[cur_idx].position.z += 0.1 * (float)yoffset;
            break;
        case GeoScaling:
            models[cur_idx].scale.z += 0.01 * (float)yoffset;
            break;
        case GeoRotation:
            models[cur_idx].rotation.z += (acosf(-1.0f) / 180.0) * 5 * (float)yoffset;
            break;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        mouse_pressed = true;
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouse_pressed = false;
        starting_press_x = -1;
        starting_press_y = -1;
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (mouse_pressed) {
        if (starting_press_x < 0 || starting_press_y < 0) {
            starting_press_x = (int)xpos;
            starting_press_y = (int)ypos;
        } else {
            float diff_x = starting_press_x - (int)xpos;
            float diff_y = starting_press_y - (int)ypos;
            starting_press_x = (int)xpos;
            starting_press_y = (int)ypos;
            switch (cur_trans_mode) {
                case ViewEye:
                    main_camera.position.x += diff_x * (1.0 / 400.0);
                    main_camera.position.y += diff_y * (1.0 / 400.0);
                    setViewingMatrix();
                    cout << "Camera Position = ( " << main_camera.position.x << " , " << main_camera.position.y << " , " << main_camera.position.z << " )" << endl;
                    break;
                case ViewCenter:
                    main_camera.center.x += diff_x * (1.0 / 400.0);
                    main_camera.center.y -= diff_y * (1.0 / 400.0);
                    setViewingMatrix();
                    cout << "Camera Viewing Direction = ( " << main_camera.center.x << " , " << main_camera.center.y << " , " << main_camera.center.z << " )" << endl;
                    break;
                case ViewUp:
                    main_camera.up_vector.x += diff_x * 0.1;
                    main_camera.up_vector.y += diff_y * 0.1;
                    setViewingMatrix();
                    cout << "Camera Up Vector = ( " << main_camera.up_vector.x << " , " << main_camera.up_vector.y << " , " << main_camera.up_vector.z << " )" << endl;
                    break;
                case GeoTranslation:
                    models[cur_idx].position.x += -diff_x * (1.0 / 400.0);
                    models[cur_idx].position.y += diff_y * (1.0 / 400.0);
                    break;
                case GeoScaling:
                    models[cur_idx].scale.x += diff_x * 0.001;
                    models[cur_idx].scale.y += diff_y * 0.001;
                    break;
                case GeoRotation:
                    models[cur_idx].rotation.x += acosf(-1.0f) / 180.0 * diff_y * (45.0 / 400.0);
                    models[cur_idx].rotation.y += acosf(-1.0f) / 180.0 * diff_x * (45.0 / 400.0);
                    break;
            }
        }
    }
}

void setShaders() {
    GLuint v, f, p;
    char* vs = NULL;
    char* fs = NULL;

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);

    vs = textFileRead("shader.vs.glsl");
    fs = textFileRead("shader.fs.glsl");

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

    if (success)
        glUseProgram(p);
    else {
        system("pause");
        exit(123);
    }

    program = p;
}

void initParameter() {
    proj.left = -1;
    proj.right = 1;
    proj.top = 1;
    proj.bottom = -1;
    proj.nearClip = 0.001;
    proj.farClip = 100.0;
    proj.fovy = 80;
    proj.aspect = (float)(WINDOW_WIDTH / 2) / (float)WINDOW_HEIGHT;  // adjust width for side by side view

    main_camera.position = Vector3(0.0f, 0.0f, 2.0f);
    main_camera.center = Vector3(0.0f, 0.0f, 0.0f);
    main_camera.up_vector = Vector3(0.0f, 1.0f, 0.0f);

    setViewingMatrix();
    setPerspective();  // set default projection matrix as perspective matrix
}

void setUniformVariables() {
    iLocP = glGetUniformLocation(program, "um4p");
    iLocV = glGetUniformLocation(program, "um4v");
    iLocM = glGetUniformLocation(program, "um4m");

    // [TODO] Get uniform location of texture
}

void setupRC() {
    // setup shaders
    setShaders();
    initParameter();
    setUniformVariables();

    // OpenGL States and Values
    glClearColor(0.2, 0.2, 0.2, 1.0);

    for (string model_path : model_list) {
        Model tmp_model;
        loadTexturedModels(model_path, tmp_model);
        models.push_back(tmp_model);
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
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, "Student ID HW3", NULL, NULL);
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

    glPrintContextInfo(false);

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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        // render left view
        glViewport(0, 0, screenWidth / 2, screenHeight);
        renderScene(1);
        // render right view
        glViewport(screenWidth / 2, 0, screenWidth / 2, screenHeight);
        renderScene(0);

        // swap buffer from back to front
        glfwSwapBuffers(window);

        // Poll input event
        glfwPollEvents();
    }

    // just for compatibiliy purposes
    return 0;
}
