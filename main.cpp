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


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 5.0f));
GLfloat lastX  =  WIDTH  / 2.0;
GLfloat lastY  =  HEIGHT / 2.0;
bool    keys[1024];

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

// The MAIN function, from here we start the application and run the game loop
int main()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

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
#endif
    /***********************************************************/

    // Build and compile our shader program
    Shader lightingShader("./shader.vert", "./shader.frag");
    Shader lampShader("./simpleshader.vert", "./lampshader.frag");
    Shader singleColorShader("./scalingshader.vert", "./singlecolorshader.frag");
    Shader rgbaShader("./shader.vert", "./rgbashader.frag");

    GLuint texture1, texture2, vegetationTexture, windowsTexture;
    glGenTextures(1, &texture1);
    glGenTextures(1, &texture2);
    glGenTextures(1, &vegetationTexture);
    glGenTextures(1, &windowsTexture);

    int img_width, img_height;
    unsigned char* image;

    glBindTexture(GL_TEXTURE_2D, texture1);
    image = SOIL_load_image("map_diffuse.png", &img_width, &img_height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texture2);
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

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    // Set up vertex data (and buffer(s)) and attribute pointers
    GLfloat vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f,  1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
    };

    std::vector<glm::vec3> vegetation;
    vegetation.push_back(glm::vec3(-1.5f,  0.0f, -0.48f));
    vegetation.push_back(glm::vec3( 1.5f,  0.0f,  0.51f));
    vegetation.push_back(glm::vec3( 0.0f,  0.0f,  0.7f));
    vegetation.push_back(glm::vec3(-0.3f,  0.0f, -2.3f));
    vegetation.push_back(glm::vec3( 0.5f,  0.0f, -0.6f));

    std::vector<glm::vec3> windows;
    windows.push_back(glm::vec3(-1.5f,  -1.0f, -0.48f));
    windows.push_back(glm::vec3( 1.5f,  -1.0f,  0.51f));
    windows.push_back(glm::vec3( 0.0f,  -1.0f,  0.7f));
    windows.push_back(glm::vec3(-0.3f,  -1.0f, -2.3f));
    windows.push_back(glm::vec3( 0.5f,  -1.0f, -0.6f));

    glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
    };

    // First, set the container's VAO (and VBO)
    GLuint VBO, containerVAO, lightVAO;
    glGenVertexArrays(1, &containerVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(containerVAO);
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

    lightingShader.use();
    lightingShader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.1f);
    lightingShader.setVec3("dirLight.diffuse", 0.8f, 0.8f, 0.9f);
    lightingShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

    for (int i = 0; i < 4; ++i)
    {
        std::string name = "pointLights[" + std::to_string(i) + "]";
        lightingShader.setVec3(name + ".ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3(name + ".diffuse", 0.4f, 0.4f, 0.9f);
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
    lightingShader.setFloat("material.shininess", 64.0f);

    Model nanosuit("./nanosuit/nanosuit.obj");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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


        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);

        // Clear the colorbuffer
        glClearColor(0.1f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Create camera transformations
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 model(1.0f);

        glStencilMask(0x00);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        lightingShader.use();
        lightingShader.setVec3("dirLight.direction", glm::vec3(view*glm::vec4(0.0f, -1.0f, 0.0f, 0.0f)));

        for (int i = 0; i < 4; ++i)
        {
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].position", glm::vec3(view*glm::vec4(pointLightPositions[i], 1.0f)));
        }

        // Pass the matrices to the shader
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);

        lightingShader.setVec3("spotLight.position",  glm::vec3(view*glm::vec4(camera.Position, 1.0f)));
        lightingShader.setVec3("spotLight.direction", glm::vec3(view*glm::vec4(camera.Front, 0.0f)));

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


        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f));
        lightingShader.setMat4("model", model);

        glStencilFunc(GL_ALWAYS, 1, 0xFF); // каждый фрагмент обновит трафаретный буфер
        glStencilMask(0xFF);
        nanosuit.Draw(lightingShader);


        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00); // отключить запись в трафаретный буфер
        glDisable(GL_DEPTH_TEST);
        singleColorShader.use();
        singleColorShader.setMat4("view", view);
        singleColorShader.setMat4("projection", projection);
        singleColorShader.setMat4("model", model);
        nanosuit.Draw(singleColorShader);
        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF); // каждый фрагмент обновит трафаретный буфер


        rgbaShader.use();
        rgbaShader.setMat4("view", view);
        rgbaShader.setMat4("projection", projection);

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



        // Also draw the lamp object, again binding the appropriate shader
        lampShader.use();
        lampShader.setMat4("view", view);
        lampShader.setMat4("projection", projection);

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
            keys[key] = true;
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
