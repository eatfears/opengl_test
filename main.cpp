#include <iostream>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include <SOIL/SOIL.h>
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Other includes
#include "shader.h"
#include "camera.h"
#include "mesh.h"

#include "arrays.h"
#include "gui/gui.h"

// Function prototypes
void processInput(GLFWwindow *window);
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void do_movement();

// Window dimensions
const GLuint WIDTH = 1300, HEIGHT = 800;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 5.0f));
GLfloat lastX  =  WIDTH  / 2.0;
GLfloat lastY  =  HEIGHT / 2.0;
bool    keys[1024];

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

Gui gui;
bool flashlight_key_pressed = false, blinn_key_pressed = false, normal_key_pressed = false, mousing_key_pressed = false, rotate_key_pressed = false;
bool mousing = true;

// The MAIN function, from here we start the application and run the game loop
int main()
{
    int rc;
    std::cout << std::boolalpha;
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 8);  //MSAA

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    //glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // GLFW Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    // OpenGL options
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//    glEnable(GL_POLYGON_SMOOTH);
//    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    gui.init(window, "#version 330");

//#define REVERSE_Z
#ifdef REVERSE_Z

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if ((major > 4 || (major == 4 && minor >= 5)))
    {
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    }
    else
    {
        std::cerr << "glClipControl required" << std::endl;
        exit(1);
    }
    glDepthFunc(GL_GEQUAL);
    glClearDepth(0.0);

    //Reversed Infinite Projection Matrix
    const float zNear = 0.001f;
    const double viewAngleVertical = 45.0f;
    const float f = 1.0 / tan(viewAngleVertical / 2.0); // 1.0 / tan == cotangent
    const float aspect = float(WIDTH) / float(HEIGHT);
    glm::mat4 projection =
    {
        f/aspect, 0.0f,    0.0f,  0.0f,
        0.0f,    f,    0.0f,  0.0f,
        0.0f, 0.0f,    0.0f, -1.0f,
        0.0f, 0.0f, 2*zNear,  0.0f
    };
#else
    glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
    glDepthFunc(GL_LEQUAL);
#endif
    glm::mat4 view;
    /***********************************************************/

    // Build and compile our shader program
    Shader lightingShader("./shaders/shader.vert", "./shaders/shader.frag");
    Shader lampShader("./shaders/simpleshader.vert", "./shaders/lampshader.frag");
    Shader singleColorShader("./shaders/scalingshader.vert", "./shaders/singlecolorshader.frag");
    Shader rgbaShader("./shaders/shader.vert", "./shaders/rgbashader.frag");
    Shader screenShader("./shaders/postshader.vert", "./shaders/postshader.frag");
    Shader skyboxShader("./shaders/skyboxshader.vert", "./shaders/skyboxshader.frag");

    GLuint boxDiffuseTexture, boxSpecularTexture, vegetationTexture, windowsTexture, skyboxCubemapTexture;
    boxDiffuseTexture = TextureFromFile("resources/textures/container2.png", true);
    boxSpecularTexture = TextureFromFile("resources/textures/container2_specular.png", false);

    vegetationTexture = TextureFromFile("resources/textures/grass.png", true, GL_CLAMP_TO_EDGE);
    windowsTexture = TextureFromFile("resources/textures/window.png", true, GL_CLAMP_TO_EDGE);

    GLuint bricks, bricksDisp, bricksNormal;
    bricks = TextureFromFile("resources/textures/bricks2.jpg", true);
    bricksDisp = TextureFromFile("resources/textures/bricks2_disp.jpg", false);
    bricksNormal = TextureFromFile("resources/textures/bricks2_normal.jpg", false);

    GLuint brickwall, brickwallNormal;
    brickwall = TextureFromFile("resources/textures/brickwall.jpg", true);
    brickwallNormal = TextureFromFile("resources/textures/brickwall_normal.jpg", false);

    GLuint toyDiffuse, toyNormal, toyDispl;
    toyDiffuse = TextureFromFile("resources/textures/toy_box_diffuse.png", true);
    toyNormal = TextureFromFile("resources/textures/toy_box_normal.png", false);
    toyDispl = TextureFromFile("resources/textures/toy_box_disp.png", false);

    skyboxCubemapTexture = loadCubemap(cubemapFaces);

    /*********************************************************************************/

    {
        for ( int i = 0; i < 36; i+=3)
        {
            // Shortcuts for vertices
            glm::vec3 v0(vertices[8*i], vertices[8*i + 1], vertices[8*i + 2]);
            glm::vec3 v1(vertices[8*i+8], vertices[8*i+8 + 1], vertices[8*i+8 + 2]);
            glm::vec3 v2(vertices[8*i+16], vertices[8*i+16 + 1], vertices[8*i+16 + 2]);

            // Shortcuts for UVs
            glm::vec2 uv0(vertices[8*i + 6], vertices[8*i + 7]);
            glm::vec2 uv1(vertices[8*i+8 + 6], vertices[8*i+8 + 7]);
            glm::vec2 uv2(vertices[8*i+16 + 6], vertices[8*i+16 + 7]);

            // Edges of the triangle : position delta
            glm::vec3 deltaPos1 = v1-v0;
            glm::vec3 deltaPos2 = v2-v0;

            // UV delta
            glm::vec2 deltaUV1 = uv1-uv0;
            glm::vec2 deltaUV2 = uv2-uv0;

            float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
            glm::vec3 normal = glm::normalize(glm::cross(deltaPos1, deltaPos2));
            glm::vec3 ltangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
            glm::vec3 lbitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

//            vv0.Normal = normal;
//            vv1.Normal = normal;
//            vv2.Normal = normal;

            tangent[3*i] = ltangent.x; tangent[3*i + 1] = ltangent.y; tangent[3*i + 2] = ltangent.z;
            tangent[3*i+3] = ltangent.x; tangent[3*i+3 + 1] = ltangent.y; tangent[3*i+3 + 2] = ltangent.z;
            tangent[3*i+6] = ltangent.x; tangent[3*i+6 + 1] = ltangent.y; tangent[3*i+6 + 2] = ltangent.z;

            bitangent[3*i] = lbitangent.x; bitangent[3*i + 1] = lbitangent.y; bitangent[3*i + 2] = lbitangent.z;
            bitangent[3*i+3] = lbitangent.x; bitangent[3*i+3 + 1] = lbitangent.y; bitangent[3*i+3 + 2] = lbitangent.z;
            bitangent[3*i+6] = lbitangent.x; bitangent[3*i+6 + 1] = lbitangent.y; bitangent[3*i+6 + 2] = lbitangent.z;
        }
    }
    // First, set the container's VAO (and VBO)
    GLuint VBO, tangentVBO, bitangentVBO, containerVAO, lightVAO, quadVBO, quadVAO, skyboxVBO, skyboxVAO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &tangentVBO);
    glGenBuffers(1, &bitangentVBO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &skyboxVBO);

    glGenVertexArrays(1, &containerVAO);
    glBindVertexArray(containerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6*sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, tangentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tangent), tangent, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, bitangentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bitangent), bitangent, GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(4);
    glBindVertexArray(0);

    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glGenVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glm::vec3 spec(0.3f);
    lightingShader.use();
    lightingShader.setVec3("dirLight.phong.ambient", glm::vec3(0.2f));
    lightingShader.setVec3("dirLight.phong.diffuse", 2.9f, 2.9f, 2.7f);
    lightingShader.setVec3("dirLight.phong.specular", spec);


    for (int i = 0; i < 4; ++i)
    {
        std::string name = "pointLights[" + std::to_string(i) + "]";
        lightingShader.setVec3(name + ".phong.ambient", glm::vec3(0.0f));
        lightingShader.setVec3(name + ".phong.diffuse", 0.4f, 0.5f, 2.8f);
        lightingShader.setVec3(name + ".phong.specular", spec);

        lightingShader.setFloat(name + ".attenuation.constant",  1.0f);
        lightingShader.setFloat(name + ".attenuation.linear",    0.15f);
        lightingShader.setFloat(name + ".attenuation.quadratic", 0.035f);
    }

    lightingShader.setVec3("spotLight.phong.ambient", glm::vec3(0.0f));
    lightingShader.setVec3("spotLight.phong.diffuse", 1.9f, 1.8f, 1.9f);
    lightingShader.setVec3("spotLight.phong.specular", spec);
    lightingShader.setFloat("spotLight.attenuation.constant",  1.0f);
    lightingShader.setFloat("spotLight.attenuation.linear",    0.045f);
    lightingShader.setFloat("spotLight.attenuation.quadratic", 0.0075f);
    lightingShader.setFloat("spotLight.cutOff",   glm::cos(glm::radians(12.5f)));
    lightingShader.setFloat("spotLight.outerCutOff",   glm::cos(glm::radians(16.5f)));


    // material properties
    lightingShader.setInt("material.texture_diffuse1", 0);
    lightingShader.setInt("material.texture_specular1", 1);
    lightingShader.setInt("material.texture_ambient1", 2);
    lightingShader.setInt("material.texture_bump1", 3);
    lightingShader.setInt("material.texture_displ1", 4);
    lightingShader.setInt("reflectSample", 15);
    lightingShader.setFloat("material.shininess", 128.0f);
    lightingShader.setBool("instance", false);

    screenShader.setInt("screenTexture", 0);

    Model nanosuit("resources/objects/nanosuit/nanosuit.obj", {"texture_diffuse", "texture_specular", "texture_ambient", "texture_bump"});
    Model cyborg("resources/objects/cyborg/cyborg.obj");
    Model rock("resources/objects/rock/rock.obj");
    Model planet("resources/objects/planet/planet.obj");

    // generate a large list of semi-random model transformation matrices
    // ------------------------------------------------------------------
    unsigned int amount = 500;
    glm::mat4* modelMatrices;
    modelMatrices = new glm::mat4[amount];
    srand(glfwGetTime()); // initialize random seed
    float radius = 35.0;
    float offset = 7.5f;
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model(1.0f);
//        model = glm::translate(model, glm::vec3(50.0f, 0.0f, -50.0f));
        lightingShader.setMat4("planet_model", glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 0.0f, -50.0f)));
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.03f; // keep height of asteroid field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. scale
        float scale = (rand() % 5) / 100.0f + 0.03;
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = (rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }

    // создаем VBO
    unsigned int rockMatriciesBuffer;
    glGenBuffers(1, &rockMatriciesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, rockMatriciesBuffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    for(unsigned int i = 0; i < rock.meshes.size(); i++)
    {
        unsigned int VAO = rock.meshes[i].VAO;
        glBindVertexArray(VAO);
        // настройка атрибутов
        GLsizei vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);

        glBindVertexArray(0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//#define POSTRENDER
#ifdef POSTRENDER

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER , framebuffer);

    unsigned int screenTexture;
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

#define TEXTURE_MSAA
#   ifdef TEXTURE_MSAA
    unsigned int texColorBuffer;
    glGenTextures(1, &texColorBuffer);
    GLuint samples = 8;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texColorBuffer);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, WIDTH, HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texColorBuffer, 0);
#   else
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);
#   endif


    if ((rc = glCheckFramebufferStatus(GL_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE)
    {
        // все хорошо, можно плясать джигу!
    }
    else
    {
        std::cout << "No frame buffer " << rc << std::endl;
        exit(1);
    }

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
#   ifndef TEXTURE_MSAA
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
#   else
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
#   endif
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

#   ifdef TEXTURE_MSAA
    // configure second post-processing framebuffer
    unsigned int intermediateFBO;
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    // create a color attachment texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	// we only need a color buffer

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << std::endl;
        exit(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#   endif
#else
    glEnable(GL_FRAMEBUFFER_SRGB); // gammacorrection
#endif
    /********************************************************/
    //shadows

    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Shadow framebuffer is not complete!" << std::endl;
        exit(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    /********************************************************/

    unsigned int tmp;
    tmp = glGetUniformBlockIndex(skyboxShader.m_Program, "Matrices");
    glUniformBlockBinding(skyboxShader.m_Program, tmp, 0);
    tmp = glGetUniformBlockIndex(lightingShader.m_Program, "Matrices");
    glUniformBlockBinding(lightingShader.m_Program, tmp, 0);

    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));
    /********************************************************/
    //glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
    glfwSwapInterval(0);
    /********************************************************/

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        //do_movement();

#ifdef POSTRENDER
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
#endif

        // Clear the colorbuffer
        glClearColor(0.1f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);


        // Create camera transformations
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        view = camera.getViewMatrix();
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glm::mat4 model(1.0f);


        glEnable(GL_CULL_FACE);

//        glEnable(GL_STENCIL_TEST);
//        glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
//        glStencilMask(0x00);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, boxDiffuseTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, boxSpecularTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE15);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemapTexture);
        glActiveTexture(GL_TEXTURE0);

        lightingShader.use();
        lightingShader.setVec3("dirLight.direction", glm::vec3(glm::vec4(-0.3f, -0.5f, -0.2f, 0.0f)));

        for (int i = 0; i < 4; ++i)
        {
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].position", glm::vec3(/*view**/glm::vec4(pointLightPositions[i], 1.0f)));
        }

        lightingShader.setVec3("spotLight.position",  glm::vec3(/*view**/glm::vec4(camera.Position, 1.0f) + glm::inverse(view) * glm::vec4(0.2, -0.1, -0.1, 0.0)));
        lightingShader.setVec3("spotLight.direction", glm::vec3(/*view**/glm::vec4(camera.Front, 0.0f)));

        lightingShader.setBool("flashlight", gui.flashlight);
        lightingShader.setBool("blinn",  gui.blinn);
        lightingShader.setBool("normal_mapping",  gui.normal);
        lightingShader.setInt("display_mode",  gui.m_DisplayMode);
        lightingShader.setFloat("refractRatio",  gui.refractRatio);

        lightingShader.setVec3("viewPos", camera.Position);
        //lightingShader.setMat4("viewInv", glm::inverse(view));

        // Draw the container (using container's vertex attributes)
        glBindVertexArray(containerVAO);
        for(unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        // Draw normals
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brickwall);
        lightingShader.setInt("material.texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        lightingShader.setInt("material.texture_specular1", 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        lightingShader.setInt("material.texture_ambient1", 2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, brickwallNormal);
        lightingShader.setInt("material.texture_bump1", 3);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, 0);
        lightingShader.setInt("material.texture_displ1", 4);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(containerVAO);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);


        // Draw displ
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bricks);
        lightingShader.setInt("material.texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        lightingShader.setInt("material.texture_specular1", 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        lightingShader.setInt("material.texture_ambient1", 2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, bricksNormal);
        lightingShader.setInt("material.texture_bump1", 3);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, bricksDisp);
        lightingShader.setInt("material.texture_displ1", 4);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(containerVAO);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(7.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Draw toy
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, toyDiffuse);
        lightingShader.setInt("material.texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        lightingShader.setInt("material.texture_specular1", 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        lightingShader.setInt("material.texture_ambient1", 2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, toyNormal);
        lightingShader.setInt("material.texture_bump1", 3);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, toyDispl);
        lightingShader.setInt("material.texture_displ1", 4);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(containerVAO);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(9.0f, 0.0f, 0.0f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        /*********************************************/

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, 0);

        lightingShader.use();
        model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, -4.0f, -50.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        model = glm::rotate(model, currentFrame/50.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        lightingShader.setMat4("model", model);
        planet.Draw(lightingShader);

        // рендер метеоритов
#define INSTANCE
#ifdef INSTANCE
        lightingShader.setFloat("time", currentFrame);
        lightingShader.setBool("instance", true);
        lightingShader.setInt("material.texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id);
        lightingShader.setInt("material.texture_specular1", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        lightingShader.setInt("material.texture_ambient1", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, 0);

        for (unsigned int i = 0; i < rock.meshes.size(); i++)
        {
            glBindVertexArray(rock.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, rock.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
            glBindVertexArray(0);
        }
        glActiveTexture(GL_TEXTURE0);
        lightingShader.setBool("instance", false);
#else
        for(unsigned int i = 0; i < amount; i++)
        {
            lightingShader.setMat4("model", modelMatrices[i]);
            rock.Draw(lightingShader);
        }
#endif

        lightingShader.use();
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f));
        if (gui.rotate)
        {
            model = glm::rotate(model, currentFrame, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        lightingShader.setMat4("model", model);

//        glStencilFunc(GL_ALWAYS, 1, 0xFF); // каждый фрагмент обновит трафаретный буфер
//        glStencilMask(0xFF);
        nanosuit.Draw(lightingShader);
//        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
//        glStencilMask(0x00); // отключить запись в трафаретный буфер

        model = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.8f));
        if (gui.rotate)
        {
            model = glm::rotate(model, currentFrame, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        lightingShader.setMat4("model", model);
        cyborg.Draw(lightingShader);

        glEnable(GL_CULL_FACE);
        // Also draw the lamp object, again binding the appropriate shader
        lampShader.use();

        glBindVertexArray(lightVAO);
        for(unsigned int i = 0; i < 4; i++)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.1f));
            lampShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);


        /******************************************/

        glDepthMask(GL_FALSE);
        skyboxShader.use();
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        /**********
        glDisable(GL_DEPTH_TEST);
        singleColorShader.use();
        singleColorShader.setMat4("model", model);
        nanosuit.Draw(singleColorShader);
        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF); // каждый фрагмент обновит трафаретный буфер
        /**********/


        glDisable(GL_CULL_FACE);
        rgbaShader.use();
        glBindVertexArray(containerVAO);
        glBindTexture(GL_TEXTURE_2D, vegetationTexture);
        for(unsigned int i = 0; i < vegetation.size(); i++)
        {
            model = glm::translate(glm::mat4(1.0f), vegetation[i]);
            rgbaShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glBindVertexArray(0);


        glBindVertexArray(containerVAO);
        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < windows.size(); i++)
        {
            float distance = glm::length(camera.Position - windows[i]);
            sorted[distance] = windows[i];
        }
        glBindTexture(GL_TEXTURE_2D, windowsTexture);
        for(std::map<float,glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            model = glm::translate(glm::mat4(1.0f), it->second);
            rgbaShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);




        /************************************************************/
#ifdef POSTRENDER
#   ifdef TEXTURE_MSAA
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
        glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
#   endif
        //Render texture quad
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // возвращаем буфер кадра по умолчанию
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        glBindVertexArray(quadVAO);
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screenTexture);
        //glEnable(GL_FRAMEBUFFER_SRGB); // gammacorrection
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //glDisable(GL_FRAMEBUFFER_SRGB);
        glBindVertexArray(0);
#endif
        /************************************************/

        gui.render();

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}


void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        camera.processKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        camera.processKeyboard(DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && !flashlight_key_pressed)
    {
        gui.flashlight = !gui.flashlight;
        flashlight_key_pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
    {
        flashlight_key_pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinn_key_pressed)
    {
        gui.blinn = !gui.blinn;
        blinn_key_pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        blinn_key_pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !normal_key_pressed)
    {
        gui.normal = !gui.normal;
        normal_key_pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE)
    {
        normal_key_pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !rotate_key_pressed)
    {
        gui.rotate = !gui.rotate;
        rotate_key_pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
    {
        rotate_key_pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS && !mousing_key_pressed)
    {
        mousing = !mousing;
        if (!mousing)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        mousing_key_pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_RELEASE)
    {
        mousing_key_pressed = false;
    }
}
// Is called whenever a key is pressed/released via GLFW
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
//{
//    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, GL_TRUE);
//    if (key >= 0 && key < 1024)
//    {
//        if (action == GLFW_PRESS)
//        {
//            keys[key] = true;
//            if (key == GLFW_KEY_Q)
//            {
//                flashlight = !flashlight;
//                std::cout << "Flashlight " << flashlight << std::endl;
//            }
//            if (key == GLFW_KEY_B)
//            {
//                blinn = !blinn;
//                std::cout << "Blinn " << blinn << std::endl;
//            }
//        }
//        else if (action == GLFW_RELEASE)
//            keys[key] = false;
//    }
//}

//void do_movement()
//{
//    // Camera controls
//    if (keys[GLFW_KEY_W])
//        camera.processKeyboard(FORWARD, deltaTime);
//    if (keys[GLFW_KEY_S])
//        camera.processKeyboard(BACKWARD, deltaTime);
//    if (keys[GLFW_KEY_A])
//        camera.processKeyboard(LEFT, deltaTime);
//    if (keys[GLFW_KEY_D])
//        camera.processKeyboard(RIGHT, deltaTime);
//    if (keys[GLFW_KEY_R])
//        camera.processKeyboard(UP, deltaTime);
//    if (keys[GLFW_KEY_F])
//        camera.processKeyboard(DOWN, deltaTime);
//}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

    lastX = xpos;
    lastY = ypos;

    if (mousing)
    {
        camera.processMouseMovement(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll(yoffset);
}
