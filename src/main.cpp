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
double starting_press_x = 0.0f;
double starting_press_y = 0.0f;

Uniform uniform;
vector<string> filenames;  // .obj filename list
vector<model> models;
camera main_camera;
project_setting proj;
TransMode cur_trans_mode = GeoTranslation;
Matrix4 view_matrix;
Matrix4 project_matrix;
Lighting lighting;

int cur_idx = 0;  // represent which model should be rendered now
int window_width = WINDOW_WIDTH;
int window_height = WINDOW_HEIGHT;

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
    mat = Matrix4(
        1.0f, 0.0f, 0.0f, vec.x,
        0.0f, 1.0f, 0.0f, vec.y,
        0.0f, 0.0f, 1.0f, vec.z,
        0.0f, 0.0f, 0.0f, 1.0f);
    return mat;
}

// [TODO] given a scaling vector then output a Matrix4 (Scaling Matrix)
Matrix4 scaling(Vector3 vec) {
    Matrix4 mat;
    mat = Matrix4(
        vec.x, 0.0f, 0.0f, 0.0f,
        0.0f, vec.y, 0.0f, 0.0f,
        0.0f, 0.0f, vec.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    return mat;
}

// [TODO] given a float value then ouput a rotation matrix alone axis-X (rotate alone axis-X)
Matrix4 rotateX(GLfloat val) {
    Matrix4 mat;
    mat = Matrix4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cos(val), -sin(val), 0.0f,
        0.0f, sin(val), cos(val), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    return mat;
}

// [TODO] given a float value then ouput a rotation matrix alone axis-Y (rotate alone axis-Y)
Matrix4 rotateY(GLfloat val) {
    Matrix4 mat;
    mat = Matrix4(
        cos(val), 0.0f, sin(val), 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -sin(val), 0.0f, cos(val), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    return mat;
}

// [TODO] given a float value then ouput a rotation matrix alone axis-Z (rotate alone axis-Z)
Matrix4 rotateZ(GLfloat val) {
    Matrix4 mat;
    mat = Matrix4(
        cos(val), -sin(val), 0.0f, 0.0f,
        sin(val), cos(val), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    return mat;
}

Matrix4 rotate(Vector3 vec) {
    return rotateX(vec.x) * rotateY(vec.y) * rotateZ(vec.z);
}

// [TODO] compute viewing matrix accroding to the setting of main_camera
void setViewingMatrix() {
    Vector3 p_c = main_camera.center - main_camera.position;
    Vector3 p_u = main_camera.up_vector;
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

// [TODO] compute persepective projection matrix
void setPerspective() {
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
    // [TODO] change your aspect ratio
    window_width = width;
    window_height = height;
}

// Render function for display rendering
void renderScene(void) {
    // clear canvas
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    GLfloat curr_aspect = (float)window_width / 2.0 / (float)window_height;
    if (proj.aspect != curr_aspect) {
        proj.aspect = curr_aspect;
        setPerspective();
    }

    Matrix4 T, R, S, M;
    // [TODO] update translation, rotation and scaling
    T = translate(models[cur_idx].position);
    R = rotate(models[cur_idx].rotation);
    S = scaling(models[cur_idx].scale);
    M = T * R * S;

    // GLfloat mvp[16];

    // [TODO] multiply all the matrix
    // row-major ---> column-major
    // setGLMatrix(mvp, MVP);

    // use uniform to send mvp to vertex shader
    glUniformMatrix4fv(uniform.trans.model, 1, GL_TRUE, M.get());
    glUniformMatrix4fv(uniform.trans.view, 1, GL_TRUE, view_matrix.get());
    glUniformMatrix4fv(uniform.trans.projection, 1, GL_TRUE, project_matrix.get());

    if (lighting.light_mode == DirectionalLight) {
        Vector3 dir = lighting.dir_light.center - lighting.dir_light.position;
        glUniform3fv(uniform.dir_light.direction, 1, &dir.x);
        glUniform3fv(uniform.dir_light.ambient, 1, &lighting.dir_light.ambient.x);
        glUniform3fv(uniform.dir_light.diffuse, 1, &lighting.dir_light.diffuse.x);
        glUniform3fv(uniform.dir_light.specular, 1, &lighting.dir_light.specular.x);
    }
    if (lighting.light_mode == PointLight) {
        glUniform3fv(uniform.point_light.position, 1, &lighting.point_light.position.x);
        glUniform3fv(uniform.point_light.ambient, 1, &lighting.point_light.ambient.x);
        glUniform3fv(uniform.point_light.diffuse, 1, &lighting.point_light.diffuse.x);
        glUniform3fv(uniform.point_light.specular, 1, &lighting.point_light.specular.x);
        for (int i = 0; i < sizeof(uniform.point_light.attenuation_coefficient) / sizeof(uniform.point_light.attenuation_coefficient[0]); i++) {
            glUniform1f(uniform.point_light.attenuation_coefficient[i], lighting.point_light.attenuation_coefficient[i]);
        }
    } else if (lighting.light_mode == SpotlightLight) {
        glUniform3fv(uniform.spot_light.position, 1, &lighting.spot_light.position.x);
        glUniform3fv(uniform.spot_light.direction, 1, &lighting.spot_light.direction.x);
        glUniform3fv(uniform.spot_light.ambient, 1, &lighting.spot_light.ambient.x);
        glUniform3fv(uniform.spot_light.diffuse, 1, &lighting.spot_light.diffuse.x);
        glUniform3fv(uniform.spot_light.specular, 1, &lighting.spot_light.specular.x);
        for (int i = 0; i < sizeof(uniform.spot_light.attenuation_coefficient) / sizeof(uniform.spot_light.attenuation_coefficient[0]); i++) {
            glUniform1f(uniform.spot_light.attenuation_coefficient[i], lighting.spot_light.attenuation_coefficient[i]);
        }
        glUniform1f(uniform.spot_light.exponent, lighting.spot_light.exponent);
        glUniform1f(uniform.spot_light.cutoff_angle, lighting.spot_light.cutoff_angle);
    }
    glUniform3fv(uniform.camera_pos, 1, &main_camera.position.x);
    glUniform1i(uniform.light_mode, lighting.light_mode);

    for (int side = 0; side < 2; side++) {
        int frame_width, frame_height;
        glfwGetFramebufferSize(glfwGetCurrentContext(), &frame_width, &frame_height);
        glViewport(side * frame_width / 2.0f, 0, frame_width / 2.0f, frame_height);
        glUniform1i(uniform.per_fragment, side);
        for (int i = 0; i < (int)models[cur_idx].shapes.size(); i++) {
            glUniform3fv(uniform.mat.ambient, 1, &models[cur_idx].shapes[i].material.Ka.x);
            glUniform3fv(uniform.mat.diffuse, 1, &models[cur_idx].shapes[i].material.Kd.x);
            glUniform3fv(uniform.mat.specular, 1, &models[cur_idx].shapes[i].material.Ks.x);
            glUniform1f(uniform.mat.shininess, lighting.shininess);

            glBindVertexArray(models[cur_idx].shapes[i].vao);
            glDrawArrays(GL_TRIANGLES, 0, models[cur_idx].shapes[i].vertex_count);
        }
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // [TODO] Call back function for keyboardif (action == GLFW_PRESS) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
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
            case GLFW_KEY_Z:
                cur_idx = (cur_idx + int(models.size()) - 1) % int(models.size());
                break;
            case GLFW_KEY_X:
                cur_idx = (cur_idx + 1) % int(models.size());
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
            case GLFW_KEY_L:
                switch (lighting.light_mode) {
                    case DirectionalLight:
                        lighting.light_mode = PointLight;
                        break;
                    case PointLight:
                        lighting.light_mode = SpotlightLight;
                        break;
                    case SpotlightLight:
                        lighting.light_mode = DirectionalLight;
                        break;
                    default:
                        lighting.light_mode = DirectionalLight;
                        break;
                }
                break;
            case GLFW_KEY_K:
                cur_trans_mode = LightEdit;
                break;
            case GLFW_KEY_J:
                cur_trans_mode = ShininessEdit;
                break;
        }
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // [TODO] scroll up positive, otherwise it would be negtive
    switch (cur_trans_mode) {
        case GeoTranslation:
            models[cur_idx].position.z += yoffset / 20;
            break;
        case GeoScaling:
            models[cur_idx].scale.z += yoffset / 20;
            break;
        case GeoRotation:
            models[cur_idx].rotation.z += yoffset / 20;
            break;
        case LightEdit:
            if (lighting.light_mode == DirectionalLight || lighting.light_mode == PointLight) {
                Vector3 delta = Vector3(yoffset / 20, yoffset / 20, yoffset / 20);
                lighting.dir_light.diffuse += delta;
                lighting.point_light.diffuse += delta;
                lighting.spot_light.diffuse += delta;
            } else if (lighting.light_mode == SpotlightLight) {
                lighting.spot_light.cutoff_angle += yoffset / 5;
            }
            break;
        case ShininessEdit:
            lighting.shininess += yoffset;
            break;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // [TODO] mouse press callback function
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        mouse_pressed = true;
        starting_press_x = xpos;
        starting_press_y = ypos;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouse_pressed = false;
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    // [TODO] cursor position callback function
    switch (cur_trans_mode) {
        case GeoTranslation:
            if (mouse_pressed) {
                models[cur_idx].position.x += (xpos - starting_press_x) / 100;
                models[cur_idx].position.y -= (ypos - starting_press_y) / 100;
            }
            break;
        case GeoScaling:
            if (mouse_pressed) {
                models[cur_idx].scale.x += (xpos - starting_press_x) / 100;
                models[cur_idx].scale.y -= (ypos - starting_press_y) / 100;
            }
            break;
        case GeoRotation:
            if (mouse_pressed) {
                models[cur_idx].rotation.x += (ypos - starting_press_y) / 100;
                models[cur_idx].rotation.y += (xpos - starting_press_x) / 100;
            }
            break;
        case LightEdit:
            if (mouse_pressed) {
                if (lighting.light_mode == DirectionalLight) {
                    lighting.dir_light.position.x += (xpos - starting_press_x) / 100;
                    lighting.dir_light.position.y -= (ypos - starting_press_y) / 100;
                } else if (lighting.light_mode == PointLight) {
                    lighting.point_light.position.x += (xpos - starting_press_x) / 100;
                    lighting.point_light.position.y -= (ypos - starting_press_y) / 100;
                } else if (lighting.light_mode == SpotlightLight) {
                    lighting.spot_light.position.x += (xpos - starting_press_x) / 100;
                    lighting.spot_light.position.y -= (ypos - starting_press_y) / 100;
                }
            }
            break;
        default:
            break;
    }
    starting_press_x = xpos;
    starting_press_y = ypos;
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

    uniform.trans.model = glGetUniformLocation(p, "trans.model");
    uniform.trans.view = glGetUniformLocation(p, "trans.view");
    uniform.trans.projection = glGetUniformLocation(p, "trans.projection");

    uniform.mat.ambient = glGetUniformLocation(p, "mat.ambient");
    uniform.mat.diffuse = glGetUniformLocation(p, "mat.diffuse");
    uniform.mat.specular = glGetUniformLocation(p, "mat.specular");
    uniform.mat.shininess = glGetUniformLocation(p, "mat.shininess");

    uniform.dir_light.direction = glGetUniformLocation(p, "dir_light.direction");
    uniform.dir_light.ambient = glGetUniformLocation(p, "dir_light.ambient");
    uniform.dir_light.diffuse = glGetUniformLocation(p, "dir_light.diffuse");
    uniform.dir_light.specular = glGetUniformLocation(p, "dir_light.specular");

    uniform.point_light.position = glGetUniformLocation(p, "point_light.position");
    uniform.point_light.ambient = glGetUniformLocation(p, "point_light.ambient");
    uniform.point_light.diffuse = glGetUniformLocation(p, "point_light.diffuse");
    uniform.point_light.specular = glGetUniformLocation(p, "point_light.specular");
    for (int i = 0; i < sizeof(uniform.point_light.attenuation_coefficient) / sizeof(uniform.point_light.attenuation_coefficient[0]); i++) {
        uniform.point_light.attenuation_coefficient[i] = glGetUniformLocation(p, ("point_light.attenuation_coefficient[" + to_string(i) + "]").c_str());
    }

    uniform.spot_light.position = glGetUniformLocation(p, "spot_light.position");
    uniform.spot_light.direction = glGetUniformLocation(p, "spot_light.direction");
    uniform.spot_light.ambient = glGetUniformLocation(p, "spot_light.ambient");
    uniform.spot_light.diffuse = glGetUniformLocation(p, "spot_light.diffuse");
    uniform.spot_light.specular = glGetUniformLocation(p, "spot_light.specular");
    for (int i = 0; i < sizeof(uniform.spot_light.attenuation_coefficient) / sizeof(uniform.spot_light.attenuation_coefficient[0]); i++) {
        uniform.spot_light.attenuation_coefficient[i] = glGetUniformLocation(p, ("spot_light.attenuation_coefficient[" + to_string(i) + "]").c_str());
    }
    uniform.spot_light.cutoff_angle = glGetUniformLocation(p, "spot_light.cutoff_angle");
    uniform.spot_light.exponent = glGetUniformLocation(p, "spot_light.exponent");

    uniform.camera_pos = glGetUniformLocation(p, "camera_pos");
    uniform.light_mode = glGetUniformLocation(p, "light_mode");
    uniform.per_fragment = glGetUniformLocation(p, "per_fragment");

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
    proj.aspect = (float)window_width / (float)window_height;

    main_camera.position = Vector3(0.0f, 0.0f, 2.0f);
    main_camera.center = Vector3(0.0f, 0.0f, 0.0f);
    main_camera.up_vector = Vector3(0.0f, 1.0f, 0.0f);

    lighting.shininess = 64.0f;

    lighting.dir_light.position = Vector3(1.0f, 1.0f, 1.0f);
    lighting.dir_light.center = Vector3(0.0f, 0.0f, 0.0f);
    lighting.dir_light.ambient = Vector3(0.15f, 0.15f, 0.15f);
    lighting.dir_light.diffuse = Vector3(1.0f, 1.0f, 1.0f);
    lighting.dir_light.specular = Vector3(1.0f, 1.0f, 1.0f);

    lighting.point_light.position = Vector3(0.0f, 2.0f, 1.0f);
    lighting.point_light.ambient = Vector3(0.15f, 0.15f, 0.15f);
    lighting.point_light.diffuse = Vector3(1.0f, 1.0f, 1.0f);
    lighting.point_light.specular = Vector3(1.0f, 1.0f, 1.0f);
    lighting.point_light.attenuation_coefficient[0] = 0.01f;
    lighting.point_light.attenuation_coefficient[1] = 0.8f;
    lighting.point_light.attenuation_coefficient[2] = 0.1f;

    lighting.spot_light.position = Vector3(0.0f, 0.0f, 0.2f);
    lighting.spot_light.direction = Vector3(0.0f, 0.0f, -1.0f);
    lighting.spot_light.ambient = Vector3(0.15f, 0.15f, 0.15f);
    lighting.spot_light.diffuse = Vector3(1.0f, 1.0f, 1.0f);
    lighting.spot_light.specular = Vector3(1.0f, 1.0f, 1.0f);
    lighting.spot_light.attenuation_coefficient[0] = 0.05f;
    lighting.spot_light.attenuation_coefficient[1] = 0.3f;
    lighting.spot_light.attenuation_coefficient[2] = 0.6f;
    lighting.spot_light.exponent = 50.0f;
    lighting.spot_light.cutoff_angle = 30.0f;

    lighting.light_mode = DirectionalLight;

    setViewingMatrix();
    setPerspective();  // set default projection matrix as perspective matrix

    cur_idx = 0;
}

void setupRC() {
    // setup shaders
    setShaders();
    initParameter();

    // OpenGL States and Values
    glClearColor(0.2, 0.2, 0.2, 1.0);
    vector<string> model_list{"../NormalModels/bunny5KN.obj", "../NormalModels/dragon10KN.obj", "../NormalModels/lucy25KN.obj", "../NormalModels/teapot4KN.obj", "../NormalModels/dolphinN.obj"};
    // [TODO] Load five model at here
    for (int i = 0; i < model_list.size(); i++) {
        model tmp_model;
        loadModel(model_list[i], tmp_model);
        tmp_model.position = Vector3(0, 0, -2);
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
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "109062131 HW2", NULL, NULL);
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
