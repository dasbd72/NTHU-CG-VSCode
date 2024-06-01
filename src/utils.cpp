#include "utils.h"

#include <STB/stb_image.h>

GLvoid normalize(GLfloat v[3]) {
    GLfloat l;

    l = (GLfloat)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
}

GLvoid cross(GLfloat u[3], GLfloat v[3], GLfloat n[3]) {
    n[0] = u[1] * v[2] - u[2] * v[1];
    n[1] = u[2] * v[0] - u[0] * v[2];
    n[2] = u[0] * v[1] - u[1] * v[0];
}

void normalization(tinyobj::attrib_t* attrib, std::vector<GLfloat>& vertices, std::vector<GLfloat>& colors, std::vector<GLfloat>& normals, std::vector<GLfloat>& textureCoords, std::vector<int>& material_id, tinyobj::shape_t* shape) {
    std::vector<float> xVector, yVector, zVector;
    float minX = 10000, maxX = -10000, minY = 10000, maxY = -10000, minZ = 10000, maxZ = -10000;

    // find out min and max value of X, Y and Z axis
    for (int i = 0; i < attrib->vertices.size(); i++) {
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
        attrib->vertices.at(i) = attrib->vertices.at(i) / scale;
    }
    size_t index_offset = 0;
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
            // Optional: vertex normals
            normals.push_back(attrib->normals[3 * idx.normal_index + 0]);
            normals.push_back(attrib->normals[3 * idx.normal_index + 1]);
            normals.push_back(attrib->normals[3 * idx.normal_index + 2]);
            // Optional: texture coordinate
            textureCoords.push_back(attrib->texcoords[2 * idx.texcoord_index + 0]);
            textureCoords.push_back(attrib->texcoords[2 * idx.texcoord_index + 1]);
            // The material of this vertex
            material_id.push_back(shape->mesh.material_ids[f]);
        }
        index_offset += fv;
    }
}

static std::string getBaseDir(const std::string& filepath) {
    if (filepath.find_last_of("/\\") != std::string::npos)
        return filepath.substr(0, filepath.find_last_of("/\\"));
    return "";
}

GLuint loadTextureImage(std::string image_path) {
    int channel, width, height;
    int require_channel = 4;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* data = stbi_load(image_path.c_str(), &width, &height, &channel, require_channel);
    if (data != NULL) {
        GLuint tex = 0;

        // [TODO] Bind the image to texture
        // Hint: glGenTextures, glBindTexture, glTexImage2D, glGenerateMipmap
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // free the image from memory after binding to texture
        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);
        return tex;
    } else {
        std::cout << "loadTextureImage: Cannot load image from " << image_path << std::endl;
        return -1;
    }
}

std::vector<Shape> splitShapeByMaterial(std::vector<GLfloat>& vertices, std::vector<GLfloat>& colors, std::vector<GLfloat>& normals, std::vector<GLfloat>& textureCoords, std::vector<int>& material_id, std::vector<PhongMaterial>& materials) {
    std::vector<Shape> res;
    for (int m = 0; m < materials.size(); m++) {
        std::vector<GLfloat> m_vertices, m_colors, m_normals, m_textureCoords;
        for (int v = 0; v < material_id.size(); v++) {
            // extract all vertices with same material id and create a new shape for it.
            if (material_id[v] == m) {
                m_vertices.push_back(vertices[v * 3 + 0]);
                m_vertices.push_back(vertices[v * 3 + 1]);
                m_vertices.push_back(vertices[v * 3 + 2]);

                m_colors.push_back(colors[v * 3 + 0]);
                m_colors.push_back(colors[v * 3 + 1]);
                m_colors.push_back(colors[v * 3 + 2]);

                m_normals.push_back(normals[v * 3 + 0]);
                m_normals.push_back(normals[v * 3 + 1]);
                m_normals.push_back(normals[v * 3 + 2]);

                m_textureCoords.push_back(textureCoords[v * 2 + 0]);
                m_textureCoords.push_back(textureCoords[v * 2 + 1]);
            }
        }

        if (!m_vertices.empty()) {
            Shape tmp_shape;
            glGenVertexArrays(1, &tmp_shape.vao);
            glBindVertexArray(tmp_shape.vao);

            glGenBuffers(1, &tmp_shape.vbo);
            glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.vbo);
            glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(GL_FLOAT), &m_vertices.at(0), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            tmp_shape.vertex_count = m_vertices.size() / 3;

            glGenBuffers(1, &tmp_shape.p_color);
            glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.p_color);
            glBufferData(GL_ARRAY_BUFFER, m_colors.size() * sizeof(GL_FLOAT), &m_colors.at(0), GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glGenBuffers(1, &tmp_shape.p_normal);
            glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.p_normal);
            glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(GL_FLOAT), &m_normals.at(0), GL_STATIC_DRAW);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glGenBuffers(1, &tmp_shape.p_texCoord);
            glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.p_texCoord);
            glBufferData(GL_ARRAY_BUFFER, m_textureCoords.size() * sizeof(GL_FLOAT), &m_textureCoords.at(0), GL_STATIC_DRAW);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);

            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);

            tmp_shape.material = materials[m];
            res.push_back(tmp_shape);
        }
    }

    return res;
}

void loadTexturedModels(std::string model_path, Model& model) {
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    tinyobj::attrib_t attrib;
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> colors;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> textureCoords;
    std::vector<int> material_id;

    std::string err;
    std::string warn;

    std::string base_dir = getBaseDir(model_path);  // handle .mtl with relative path

#ifdef _WIN32
    base_dir += "\\";
#else
    base_dir += "/";
#endif

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str(), base_dir.c_str());

    if (!warn.empty()) {
        std::cout << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        exit(1);
    }

    std::cout << "Load Models Success ! Shapes size " << shapes.size() << " Material size " << materials.size() << std::endl;

    std::vector<PhongMaterial> allMaterial;
    for (int i = 0; i < materials.size(); i++) {
        PhongMaterial material;
        material.Ka = Vector3(materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
        material.Kd = Vector3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
        material.Ks = Vector3(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);

        material.diffuseTexture = loadTextureImage(base_dir + std::string(materials[i].diffuse_texname));
        if (material.diffuseTexture == -1) {
            std::cout << "LoadTexturedModels: Fail to load model's material " << i << std::endl;
            system("pause");
        }

        allMaterial.push_back(material);
    }

    model.shapes.clear();
    for (int i = 0; i < shapes.size(); i++) {
        vertices.clear();
        colors.clear();
        normals.clear();
        textureCoords.clear();
        material_id.clear();

        normalization(&attrib, vertices, colors, normals, textureCoords, material_id, &shapes[i]);

        // split current shape into multiple shapes base on material_id.
        std::vector<Shape> splitedShapeByMaterial = splitShapeByMaterial(vertices, colors, normals, textureCoords, material_id, allMaterial);

        // concatenate splited shape to model's shape list
        model.shapes.insert(model.shapes.end(), splitedShapeByMaterial.begin(), splitedShapeByMaterial.end());
    }
    shapes.clear();
    materials.clear();
}

void glPrintContextInfo(bool printExtension) {
    std::cout << "GL_VENDOR = " << (const char*)glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL_RENDERER = " << (const char*)glGetString(GL_RENDERER) << std::endl;
    std::cout << "GL_VERSION = " << (const char*)glGetString(GL_VERSION) << std::endl;
    std::cout << "GL_SHADING_LANGUAGE_VERSION = " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    if (printExtension) {
        GLint numExt;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
        std::cout << "GL_EXTENSIONS =" << std::endl;
        for (GLint i = 0; i < numExt; i++) {
            std::cout << "\t" << (const char*)glGetStringi(GL_EXTENSIONS, i) << std::endl;
        }
    }
}