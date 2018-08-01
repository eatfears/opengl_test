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


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();

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

bool flashlight = false;

// The MAIN function, from here we start the application and run the game loop
int main()
{
    int rc;
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
    glfwSetKeyCallback(window, key_callback);
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
    Shader lightingShader("./shader.vert", "./shader.frag");
    Shader lampShader("./simpleshader.vert", "./lampshader.frag");
    Shader singleColorShader("./scalingshader.vert", "./singlecolorshader.frag");
    Shader rgbaShader("./shader.vert", "./rgbashader.frag");
    Shader screenShader("./postshader.vert", "./postshader.frag");
    Shader skyboxShader("./skyboxshader.vert", "./skyboxshader.frag");

    GLuint cubemapTexture = loadCubemap(faces);
    GLuint boxDiffuseTexture, boxSpecularTexture, vegetationTexture, windowsTexture;
    glGenTextures(1, &boxDiffuseTexture);
    glGenTextures(1, &boxSpecularTexture);
    glGenTextures(1, &vegetationTexture);
    glGenTextures(1, &windowsTexture);

    int img_width, img_height;
    unsigned char* image;

    glBindTexture(GL_TEXTURE_2D, boxDiffuseTexture);
    image = SOIL_load_image("map_diffuse.png", &img_width, &img_height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, boxSpecularTexture);
    image = SOIL_load_image("map_specular.png", &img_width, &img_height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, vegetationTexture);
    image = SOIL_load_image("grass.png", &img_width, &img_height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, windowsTexture);
    image = SOIL_load_image("glass.png", &img_width, &img_height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    /*********************************************************************************/

    // First, set the container's VAO (and VBO)
    GLuint VBO, containerVAO, lightVAO, quadVBO, quadVAO, skyboxVBO, skyboxVAO;
    glGenBuffers(1, &VBO);
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
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6*sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
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

    lightingShader.use();
    lightingShader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
    lightingShader.setVec3("dirLight.diffuse", 2.9f, 2.9f, 2.7f);
    lightingShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);


    for (int i = 0; i < 4; ++i)
    {
        std::string name = "pointLights[" + std::to_string(i) + "]";
        lightingShader.setVec3(name + ".ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3(name + ".diffuse", 0.4f, 0.4f, 1.6f);
        lightingShader.setVec3(name + ".specular", 1.0f, 1.0f, 1.0f);

        lightingShader.setFloat(name + ".constant",  1.0f);
        lightingShader.setFloat(name + ".linear",    0.45f);
        lightingShader.setFloat(name + ".quadratic", 0.075f);
    }

    lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    lightingShader.setVec3("spotLight.diffuse", 0.9f, 0.8f, 0.9f);
    lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("spotLight.constant",  1.0f);
    lightingShader.setFloat("spotLight.linear",    0.045f);
    lightingShader.setFloat("spotLight.quadratic", 0.0075f);
    lightingShader.setFloat("spotLight.cutOff",   glm::cos(glm::radians(12.5f)));
    lightingShader.setFloat("spotLight.outerCutOff",   glm::cos(glm::radians(16.5f)));


    // material properties
    lightingShader.setInt("material.texture_diffuse1", 0);
    lightingShader.setInt("material.texture_specular1", 1);
    lightingShader.setInt("material.texture_ambient1", 2);
    lightingShader.setInt("reflectSample", 3);
    lightingShader.setFloat("material.shininess", 64.0f);

    screenShader.setInt("screenTexture", 0);

    Model nanosuit("./nanosuit/nanosuit.obj");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#define POSTRENDER
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
#endif
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

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        do_movement();

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


        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, boxDiffuseTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, boxSpecularTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        lightingShader.use();
        lightingShader.setVec3("dirLight.direction", glm::vec3(view*glm::vec4(-0.3f, -0.5f, -0.2f, 0.0f)));

        for (int i = 0; i < 4; ++i)
        {
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].position", glm::vec3(view*glm::vec4(pointLightPositions[i], 1.0f)));
        }


        lightingShader.setBool("flashlight",  flashlight);
        lightingShader.setVec3("spotLight.position",  glm::vec3(view*glm::vec4(camera.Position, 1.0f)));
        lightingShader.setVec3("spotLight.direction", glm::vec3(view*glm::vec4(camera.Front, 0.0f)));

        lightingShader.setMat4("viewInv", glm::inverse(view));

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
        glActiveTexture(GL_TEXTURE0);


        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f));
        lightingShader.setMat4("model", model);

//        glStencilFunc(GL_ALWAYS, 1, 0xFF); // каждый фрагмент обновит трафаретный буфер
//        glStencilMask(0xFF);
        nanosuit.Draw(lightingShader);
//        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
//        glStencilMask(0x00); // отключить запись в трафаретный буфер


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
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
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
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
#endif
        /************************************************/

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
        {
            keys[key] = true;
            if (key == GLFW_KEY_Q)
                flashlight = !flashlight;
        }
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void do_movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.processKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.processKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.processKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.processKeyboard(RIGHT, deltaTime);
    if (keys[GLFW_KEY_R])
        camera.processKeyboard(UP, deltaTime);
    if (keys[GLFW_KEY_F])
        camera.processKeyboard(DOWN, deltaTime);
}

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

    camera.processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll(yoffset);
}
