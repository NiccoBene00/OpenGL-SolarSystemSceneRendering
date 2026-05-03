#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "core/shader.h"
#include "core/camera.h"
#include "rendering/sphere.h"
#include "utils/texture.h"
#include "utils/cubemap.h"
#include "rendering/planet.h"

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// input
void processInput(GLFWwindow* window);


int main()
{

    // ---------------------------
    // INIT GLFW
    // ---------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // cattura mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ---------------------------
    // INIT GLAD
    // ---------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    // ---------------------------
    // OPENGL CONFIG
    // ---------------------------
    glEnable(GL_DEPTH_TEST);

    // ---------------------------
    // SHADER 
    // ---------------------------
    Shader shader("../shaders/scene.vert", "../shaders/scene.frag");
    Shader screenShader("../shaders/quad.vert", "../shaders/quad.frag");
    Shader blurShader("../shaders/blur.vert", "../shaders/blur.frag");
    Shader finalShader("../shaders/quad.vert", "../shaders/bloom_final.frag");
    Shader skyboxShader("../shaders/skybox.vert", "../shaders/skybox.frag");

    Sphere sphere;

    //PLANETS DATA
    std::vector<Planet> planets;

    //TEXTURE LOADING
    unsigned int earthTexture = loadTexture("resources/textures/earth.jpg");
    unsigned int sunTexture = loadTexture("resources/textures/sun.jpg");

    planets = {
        {"Mercury", 0.2f, 4.0f, 4.8f, 2.0f, loadTexture("resources/textures/mercury.jpg")},
        {"Venus",   0.3f, 5.0f, 3.5f, 1.8f, loadTexture("resources/textures/venus.jpg")},
        {"Earth",   0.5f, 6.5f, 2.9f, 2.5f, earthTexture},
        {"Mars",    0.4f, 8.0f, 2.4f, 2.2f, loadTexture("resources/textures/mars.jpg")},
        {"Jupiter", 1.2f, 10.0f, 1.3f, 3.0f, loadTexture("resources/textures/jupiter.jpg")},
        {"Saturn",  1.0f, 12.0f,1.0f, 2.8f, loadTexture("resources/textures/saturn.jpg")},
        {"Uranus",  0.8f, 14.0f,0.7f, 2.5f, loadTexture("resources/textures/uranus.jpg")},
        {"Neptune", 0.8f, 16.0f,0.5f, 2.5f, loadTexture("resources/textures/neptune.jpg")}
    };

    std::vector<std::string> faces = {
        "resources/textures/skybox/right8.png",
        "resources/textures/skybox/left8.png",
        "resources/textures/skybox/top8.png",
        "resources/textures/skybox/bottom8.png",
        "resources/textures/skybox/front8.png",
        "resources/textures/skybox/back8.png"
    };

    unsigned int cubemapTexture = loadCubemap(faces);

    

    // ============================
    // HDR FRAMEBUFFER
    // ============================
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // 2 color buffers
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);

    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
            SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D,
            colorBuffers[i],
            0);
    }

    // depth buffer
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // specifica che usiamo 2 color attachments
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int quadVAO = 0;
    unsigned int quadVBO;

    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];

    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);

    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);

        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
            SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
    }
    
    // ---------------------------
    // RENDER LOOP
    // ---------------------------
    while (!glfwWindowShouldClose(window))
    {
        // ---------------------------
        // TIMING
        // ---------------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // =====================================================
        // 1. RENDER SCENE → HDR FRAMEBUFFER
        // =====================================================
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        glm::mat4 view = camera.GetViewMatrix();

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", glm::vec3(0.0f));

        shader.setInt("diffuseMap", 0);
        glActiveTexture(GL_TEXTURE0);

        //  SUN
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(1.5f));

            shader.setMat4("model", model);
            shader.setInt("isEmissive", 1);

            glBindTexture(GL_TEXTURE_2D, sunTexture);
            sphere.Draw();
        }

        /*
        //  EARTH
        {
            float time = glfwGetTime();
            float radius = 3.0f;

            float x = sin(time) * radius;
            float z = cos(time) * radius;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, 0.0f, z));
            model = glm::rotate(model, time * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f));

            shader.setMat4("model", model);
            shader.setInt("isEmissive", 0);

            glBindTexture(GL_TEXTURE_2D, earthTexture);
            sphere.Draw();
        }
        */

        //PLANETS
        {

            float time = glfwGetTime();

            for (auto& planet : planets)
            {
                float angle = time * planet.orbitSpeed;

                float x = sin(angle) * planet.distance;
                float z = cos(angle) * planet.distance;

                glm::mat4 model = glm::mat4(1.0f);

                // orbits
                model = glm::translate(model, glm::vec3(x, 0.0f, z));

                // rotation on its own axis
                model = glm::rotate(model, time * planet.rotationSpeed,
                                    glm::vec3(0.0f, 1.0f, 0.0f));

                // scale
                model = glm::scale(model, glm::vec3(planet.radius));

                shader.setMat4("model", model);
                shader.setInt("isEmissive", 0);

                glBindTexture(GL_TEXTURE_2D, planet.textureID);

                sphere.Draw();
            }

        }

        // =====================================================
        // DRAW SKYBOX (INSIDE HDR PASS)
        // =====================================================
        glDepthFunc(GL_LEQUAL); // importante

        skyboxShader.use();

        // rimuove la traslazione della camera
        glm::mat4 viewSky = glm::mat4(glm::mat3(view));

        skyboxShader.setMat4("view", viewSky);
        skyboxShader.setMat4("projection", projection);

        // bind cubemap
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthFunc(GL_LESS); // ripristina

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // =====================================================
        // 2. BLUR BRIGHT BUFFER (PING-PONG)
        // =====================================================
        bool horizontal = true, first_iteration = true;
        int amount = 10;

        blurShader.use();

        for (int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);

            blurShader.setInt("horizontal", horizontal);

            glBindTexture(GL_TEXTURE_2D,
                first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            horizontal = !horizontal;

            if (first_iteration)
                first_iteration = false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // =====================================================
        // 3. FINAL COMBINE (SCENE + BLOOM)
        // =====================================================
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        finalShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        finalShader.setInt("scene", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        finalShader.setInt("bloomBlur", 1);

        finalShader.setBool("bloom", true);
        finalShader.setFloat("exposure", 1.0f);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// ---------------------------
// INPUT
// ---------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// ---------------------------
// CALLBACKS
// ---------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}