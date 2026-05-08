#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <iostream>

#include "core/shader.h"
#include "core/camera.h"
#include "rendering/sphere.h"
#include "utils/texture.h"
#include "utils/cubemap.h"
#include "rendering/planet.h"
#include "rendering/orbit.h"

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

    //mouse capture
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // cattura mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ---------------------------
    // INIT GLAD
    // ---------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    // init backend
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

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
    Shader orbitShader("../shaders/orbit.vert", "../shaders/orbit.frag");

    Sphere sphere;

    //PLANETS DATA
    std::vector<Planet> planets;

    //TEXTURE LOADING
    unsigned int earthTexture = loadTexture("resources/textures/earth.jpg");
    unsigned int sunTexture = loadTexture("resources/textures/sun.jpg");
    unsigned int moonTexture = loadTexture("resources/textures/moon.jpg");
    unsigned int earthNightTexture = loadTexture("resources/textures/earth_nightmap.jpg");

    planets = {
        {"Mercury", 0.6f, 7.0f, 1.5f, 2.0f, loadTexture("resources/textures/mercury.jpg")},
        {"Venus",   0.7f, 10.0f, 0.8f, 1.8f, loadTexture("resources/textures/venus.jpg")},
        {"Earth",   1.1f, 13.5f, 0.82f, 2.5f, earthTexture},
        {"Mars",    0.8f, 16.5f, 0.8f, 2.2f, loadTexture("resources/textures/mars.jpg")},
        {"Jupiter", 1.6f, 22.5f, 0.45f, 3.0f, loadTexture("resources/textures/jupiter.jpg")},
        {"Saturn",  1.4f, 31.0f, 0.4f, 2.8f, loadTexture("resources/textures/saturn.jpg")},
        {"Uranus",  1.2f, 39.0f, 0.35f, 2.5f, loadTexture("resources/textures/uranus.jpg")},
        {"Neptune", 1.2f, 46.0f, 0.25f, 2.5f, loadTexture("resources/textures/neptune.jpg")}
    };

    //ORBITS DATA
    std::vector<Orbit> orbits;

    for (auto& planet : planets)
    {
        orbits.emplace_back(planet.distance);
    }

    //MOON DATA
    float moonDistance = 1.3f;   
    float moonScale    = 0.15f;  
    float moonOrbitSpeed = 2.0f; 
    float moonRotationSpeed = 2.0f;

    //SKYBOX TEXTURES
    
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


    //---------------------------
    //GUI variables
    float timeScale = 1.0f;

    bool bloomEnabled = true;
    float exposure = 1.0f;

    bool showOrbitLines = true;
    //---------------------------

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

        //start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //=====================================
        //GUI controls
        //=====================================
        ImGui::Begin("Solar System Controls");

        // simulation
        ImGui::Text("Simulation");

        ImGui::SliderFloat("Time Scale", &timeScale, 0.0f, 20.0f);

        // bloom
        ImGui::Separator();

        ImGui::Text("Bloom Settings");

        ImGui::Checkbox("Enable Bloom", &bloomEnabled);

        ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f);

        // orbit lines
        ImGui::Separator();

        ImGui::Checkbox("Show Orbit Lines", &showOrbitLines);

        // FPS
        ImGui::Separator();

        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::End();


        // =====================================================
        // 1. RENDER SCENE -> HDR FRAMEBUFFER
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
            model = glm::scale(model, glm::vec3(4.5f));

            shader.setMat4("model", model);
            shader.setInt("isEmissive", 1);

            glBindTexture(GL_TEXTURE_2D, sunTexture);
            sphere.Draw();
        }

        // =====================================================
        // DRAW ORBITS
        // =====================================================
        orbitShader.use();

        orbitShader.setMat4("projection", projection);
        orbitShader.setMat4("view", view);

        // NASA-style color
        orbitShader.setVec3("color", glm::vec3(0.6f, 0.7f, 0.8f));
        orbitShader.setFloat("alpha", 0.25f);

        // line width
        glLineWidth(1.0f);

        if (showOrbitLines)
        {
            for (auto& orbit : orbits)
            {
                orbit.Draw();
            }
        }

        shader.use();

        //PLANETS
        {

            float time = glfwGetTime() * timeScale;

            for (auto& planet : planets)
            {
                float angle = time * planet.orbitSpeed;

                float x = sin(angle) * planet.distance;
                float z = cos(angle) * planet.distance;

                glm::vec3 planetPos = glm::vec3(x, 0.0f, z);

                glm::mat4 model = glm::mat4(1.0f);

                // orbits
                model = glm::translate(model, planetPos);

                // rotation on its own axis
                model = glm::rotate(model, time * planet.rotationSpeed,
                                    glm::vec3(0.0f, 1.0f, 0.0f));

                // scale
                model = glm::scale(model, glm::vec3(planet.radius));

                shader.setMat4("model", model);
                shader.setInt("isEmissive", 0);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, planet.textureID);
                shader.setInt("diffuseMap", 0);

                shader.setInt("hasNightMap", 0);

                // NIGHT MAP SOLO PER LA TERRA
                if (planet.name == "Earth")
                {
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, earthNightTexture);
                    shader.setInt("nightMap", 1);
                    shader.setInt("hasNightMap", 1);
                }
                
                sphere.Draw();

                glActiveTexture(GL_TEXTURE0);

                //MOON
                if (planet.name == "Earth")
                    {
                        float moonAngle = time * moonOrbitSpeed;

                        float mx = sin(moonAngle) * moonDistance;
                        float mz = cos(moonAngle) * moonDistance;

                        glm::vec3 moonPos = planetPos + glm::vec3(mx, 0.0f, mz);

                        glm::mat4 moonModel = glm::mat4(1.0f);

                        // orbits around Earth
                        moonModel = glm::translate(moonModel, moonPos);

                        // rotation on its own axis
                        moonModel = glm::rotate(moonModel, time * moonRotationSpeed,
                                                glm::vec3(0.0f, 1.0f, 0.0f));

                        moonModel = glm::scale(moonModel, glm::vec3(moonScale));

                        shader.setMat4("model", moonModel);
                        //shader.setInt("isEmissive", 0);
                        shader.setInt("hasNightMap", 0);

                        glBindTexture(GL_TEXTURE_2D, moonTexture);
                        sphere.Draw();
                    }
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

        finalShader.setBool("bloom", bloomEnabled);
        finalShader.setFloat("exposure", exposure);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glEnable(GL_DEPTH_TEST);

        // =====================================================
        // RENDER GUI
        // =====================================================
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


bool mouseCaptured = true;
bool tabPressedLastFrame = false;

// ---------------------------
// INPUT
// ---------------------------
void processInput(GLFWwindow* window)
{
    // =====================================================
    // TAB TOGGLE (solo una volta per pressione)
    // =====================================================
    bool tabCurrentlyPressed =
        glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;

    if (tabCurrentlyPressed && !tabPressedLastFrame)
    {
        mouseCaptured = !mouseCaptured;

        if (mouseCaptured)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            // evita salto improvviso camera
            firstMouse = true;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    tabPressedLastFrame = tabCurrentlyPressed;

    // =====================================================
    // ESCAPE
    // =====================================================
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // =====================================================
    // CAMERA MOVEMENT (sempre attivo)
    // =====================================================
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
    // se la UI è attiva, ignora mouse look
    if (!mouseCaptured)
        return;

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