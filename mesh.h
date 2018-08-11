#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <set>
#include <vector>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <SOIL/SOIL.h>

#include "shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture {
    GLuint id;
    std::string type;
    aiString path;
};

class Mesh {
public:
    /*  Mesh Data  */
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    /*  Functions  */
    Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures)
        : vertices(vertices), indices(indices), textures(textures)
    {
        setupMesh();
    }
    void Draw(const Shader &shader)
    {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int ambientNr = 1;
        unsigned int bumpNr = 1;
        unsigned int displNr = 1;

        shader.setInt("material.texture_diffuse1", 32);
        shader.setInt("material.texture_specular1", 32);
        shader.setInt("material.texture_ambient1", 32);
        shader.setInt("material.texture_bump1", 32);
        shader.setInt("material.texture_displ1", 32);

        for(unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // активируем текстурный блок, до привязки
            // получаем номер текстуры
            std::stringstream ss;
            std::string number;
            std::string name = textures[i].type;
            if(name == "texture_diffuse")
                ss << diffuseNr++;
            else if(name == "texture_specular")
                ss << specularNr++;
            else if(name == "texture_ambient")
                ss << ambientNr++;
            else if(name == "texture_bump")
                ss << bumpNr++;
            else if(name == "texture_displ")
                ss << displNr++;
            number = ss.str();

            shader.setInt(("material." + name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        glActiveTexture(GL_TEXTURE0);

        // отрисовывем полигональную сетку
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
//private:
    /*  Render data  */
    GLuint VAO, VBO, EBO;
private:
    /*  Functions    */
    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                     &indices[0], GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // calculated tangent space
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
    }
};


unsigned int TextureFromFile(const std::string &path, bool gamma = false, GLuint clamp = GL_REPEAT);

class Model
{
public:
    Model(const std::string &path, std::set<std::string> set = {"texture_diffuse"})
    {
        m_SrgbNames = set;
        loadModel(path);
    }
    void Draw(const Shader &shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }
//private:
    std::vector<Mesh> meshes;
    std::vector<Texture> textures_loaded;
private:
    std::string directory;
    std::set<std::string> m_SrgbNames;

    void loadModel(const std::string &path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_FixInfacingNormals | aiProcess_GenUVCoords |  aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);
    }
    void processNode(aiNode *node, const aiScene *scene)
    {
        // обработать все полигональные сетки в узле(если есть)
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // выполнить ту же обработку и для каждого потомка узла
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }
    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        std::vector<Texture> textures;

        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            // обработка координат, нормалей и текстурных координат вершин
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;

            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;

            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;

            if(mesh->mTextureCoords[0]) // сетка обладает набором текстурных координат?
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        if (0)
        {
            for ( int i = 0; i < vertices.size(); i+=3)
            {

                // Shortcuts for vertices
                Vertex & vv0 = vertices[i+0];
                Vertex & vv1 = vertices[i+1];
                Vertex & vv2 = vertices[i+2];

                glm::vec3 & v0 = vv0.Position;
                glm::vec3 & v1 = vv1.Position;
                glm::vec3 & v2 = vv2.Position;

                // Shortcuts for UVs
                glm::vec2 & uv0 = vv0.TexCoords;
                glm::vec2 & uv1 = vv1.TexCoords;
                glm::vec2 & uv2 = vv2.TexCoords;

                // Edges of the triangle : position delta
                glm::vec3 deltaPos1 = v1-v0;
                glm::vec3 deltaPos2 = v2-v0;

                // UV delta
                glm::vec2 deltaUV1 = uv1-uv0;
                glm::vec2 deltaUV2 = uv2-uv0;

                float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
                glm::vec3 normal = glm::normalize(glm::cross(deltaPos1, deltaPos2));
                glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
                glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

                vv0.Normal = normal;
                vv1.Normal = normal;
                vv2.Normal = normal;

                vv0.Tangent = tangent;
                vv1.Tangent = tangent;
                vv2.Tangent = tangent;

                vv0.Bitangent = bitangent;
                vv1.Bitangent = bitangent;
                vv2.Bitangent = bitangent;
            }
        }

        // орбаботка индексов
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // обработка материала
        if(mesh->mMaterialIndex >= 0)
        {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

            std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
                                                                    aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            std::vector<Texture> specularMaps = loadMaterialTextures(material,
                                                                     aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

            std::vector<Texture> ambientMaps = loadMaterialTextures(material,
                                                                    aiTextureType_AMBIENT, "texture_ambient");
            textures.insert(textures.end(), ambientMaps.begin(), ambientMaps.end());

            std::vector<Texture> bumpMaps = loadMaterialTextures(material,
                                                                 aiTextureType_HEIGHT, "texture_bump");
            textures.insert(textures.end(), bumpMaps.begin(), bumpMaps.end());

            std::vector<Texture> displMaps = loadMaterialTextures(material,
                                                                  aiTextureType_DISPLACEMENT, "texture_displ");
            textures.insert(textures.end(), displMaps.begin(), displMaps.end());
        }

        return Mesh(vertices, indices, textures);
    }

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName)
    {
        bool gamma = m_SrgbNames.find(typeName) != m_SrgbNames.end();

        std::vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                // если текстура не была загружена – сделаем это
                Texture texture;
                texture.id = TextureFromFile(directory + "/" + str.C_Str(), gamma);
                texture.type = typeName;
                texture.path = str;
                textures.push_back(texture);
                // занесем текстуру в список уже загруженных
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }
};


unsigned int TextureFromFile(const std::string &path, bool gamma, GLuint clamp)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = SOIL_load_image(path.c_str(), &width, &height, &nrComponents, SOIL_LOAD_AUTO);
    if (data)
    {
        GLenum format, in_format;
        if (nrComponents == 1)
        {
            format = in_format = GL_RED;
        }
        else if (nrComponents == 3)
        {
            format = GL_RGB;
            gamma ? in_format = GL_SRGB : in_format = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            format = GL_RGBA;
            gamma ? in_format = GL_SRGB_ALPHA : in_format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, in_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        SOIL_free_image_data(data);
    }
    else
    {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        SOIL_free_image_data(data);
    }

    return textureID;
}


unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = SOIL_load_image(faces[i].c_str(), &width, &height, &nrChannels, SOIL_LOAD_AUTO);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        }
        SOIL_free_image_data(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

#endif // MESH_H
