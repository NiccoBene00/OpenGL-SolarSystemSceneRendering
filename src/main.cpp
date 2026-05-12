/*


================================================================================
OPENGL RENDERING PIPELINE OVERVIEW
================================================================================

This project implements an OpenGL Core rendering pipeline using:

- GLFW        -> window creation + input handling
- GLAD        -> OpenGL function loader
- GLM         -> mathematics library (matrices/vectors)
- stb_image   -> texture loading
- ImGui       -> runtime GUI controls

The goal of the project is to simulate a 3D Solar System with:

- physically-inspired lighting
- HDR rendering
- Bloom post-processing
- Cubemap reflections
- Skybox rendering
- Orbit simulation
- Interactive camera system
- ImGui controls for bloom/exposure/time scale

--------------------------------------------------------------------------------
1. GRAPHICS PIPELINE OVERVIEW
--------------------------------------------------------------------------------

OpenGL works as a GPU rendering pipeline.

The pipeline transforms 3D geometry into pixels displayed on screen.

The main stages are:

CPU SIDE (C++ CODE)
    |
    |  Upload vertices/textures/shader uniforms
    v
GPU PIPELINE

Vertex Data --> Vertex Shader --> Primitive Assembly --> Rasterization
                                                    --> Fragment Shader
                                                    --> Framebuffer
                                                    --> Screen

--------------------------------------------------------------------------------
2. VERTEX DATA
--------------------------------------------------------------------------------

Objects in OpenGL are represented using vertices.

A vertex usually contains:

- Position     (vec3)
- Normal        (vec3)
- Texture Coord (vec2)

Example from the Sphere mesh:

    position -> defines where the point exists in 3D space
    normal   -> used for lighting calculations
    UV coords-> used to sample textures

The vertex data is uploaded to GPU memory using:

- VBO (Vertex Buffer Object)
- VAO (Vertex Array Object)

VBO:
    stores raw vertex data inside GPU memory

VAO:
    stores how vertex attributes are interpreted

Example:

    location 0 -> position
    location 1 -> normal
    location 2 -> texture coordinates

--------------------------------------------------------------------------------
3. SHADERS
--------------------------------------------------------------------------------

Shaders are small GPU programs written in GLSL.
They run for each vertex/fragment and compute transformations and colors.

The rendering pipeline executes shaders directly on the GPU.

--------------------------------------------------------------------------------
4. VERTEX SHADER
--------------------------------------------------------------------------------

The Vertex Shader runs ONCE per vertex.

Responsibilities:

- transform vertices from local space to world space
- apply camera transformation
- apply projection transformation
- pass data to fragment shader

Main transformations:

MODEL MATRIX:
    transforms object from local coordinates into world coordinates

VIEW MATRIX:
    represents the camera transformation

PROJECTION MATRIX:
    converts 3D perspective into normalized screen coordinates

Final transformation:

    gl_Position = projection * view * model * vec4(position, 1.0)

This converts a 3D point into clip-space coordinates.

The vertex shader also outputs:

- FragPos
- Normal
- TexCoords

These values are interpolated automatically across the triangle.

--------------------------------------------------------------------------------
5. RASTERIZATION
--------------------------------------------------------------------------------

After the Vertex Shader stage:

- triangles are assembled
- transformed into fragments (potential pixels)

OpenGL interpolates values between vertices.

For instance normals and UV coordinates smoothly interpolate across the surface.

--------------------------------------------------------------------------------
6. FRAGMENT SHADER
--------------------------------------------------------------------------------

The Fragment Shader runs ONCE per fragment/pixel.

Responsibilities:

- compute lighting
- sample textures
- compute final color
- write output into framebuffer

This project uses Phong Lighting:

Ambient:
    constant base light

Diffuse:
    depends on angle between light direction and normal

Specular:
    shiny highlight based on reflection vector and view direction

Formula:

    result = ambient + diffuse + specular

--------------------------------------------------------------------------------
7. TEXTURES
--------------------------------------------------------------------------------

Textures are images loaded into GPU memory.

Used for:

- planet surfaces
- Sun texture
- Earth night lights
- skybox cubemap

Textures are sampled inside fragment shaders using:

    texture(sampler, uv)

OpenGL texture units:

GL_TEXTURE0
GL_TEXTURE1
GL_TEXTURE2
...

Multiple textures can be bound simultaneously.

Example in this project:

TEXTURE0 -> diffuse map
TEXTURE1 -> Earth night map
TEXTURE2 -> cubemap reflection

--------------------------------------------------------------------------------
8. LIGHTING SYSTEM
--------------------------------------------------------------------------------

The Sun acts as the main light source.

The light position is passed as a uniform:

    shader.setVec3("lightPos", glm::vec3(0.0f));

Each planet computes:

- diffuse shading
- specular reflections
- shadowed side
- night illumination

The Earth uses a special night texture:

- visible only on the dark side
- blended using the diffuse lighting factor

--------------------------------------------------------------------------------
9. HDR RENDERING
--------------------------------------------------------------------------------

This project uses HDR (High Dynamic Range) rendering.

Normal rendering stores colors between:

    [0,1]

HDR rendering allows values larger than 1:

    > 1.0

This is essential for:

- realistic bloom
- emissive Sun
- bright highlights

The HDR framebuffer uses floating-point textures:

    GL_RGBA16F

--------------------------------------------------------------------------------
10. FRAMEBUFFERS
--------------------------------------------------------------------------------

A Framebuffer is an off-screen rendering target.

Instead of rendering directly to the monitor,
we first render the scene into textures.

This project uses:

HDR FBO:
    stores full rendered scene

Color Attachment 0:
    normal rendered scene

Color Attachment 1:
    bright fragments only

This enables post-processing effects.

--------------------------------------------------------------------------------
11. BLOOM EFFECT
--------------------------------------------------------------------------------

Bloom simulates light bleeding from extremely bright areas.

Pipeline:

STEP 1:
    render scene into HDR framebuffer

STEP 2:
    extract bright fragments

STEP 3:
    blur bright texture using Gaussian Blur

STEP 4:
    combine blurred image with original scene

This project uses Ping-Pong Blur:

- two framebuffers
- alternating horizontal/vertical blur

This creates smooth glow around:

- Sun
- emissive objects
- Earth night lights

--------------------------------------------------------------------------------
12. GAUSSIAN BLUR
--------------------------------------------------------------------------------

Blur is implemented as a post-processing shader.

The shader samples neighboring pixels using weighted offsets.

Horizontal pass:
    blur across X axis

Vertical pass:
    blur across Y axis

Multiple iterations increase smoothness.

--------------------------------------------------------------------------------
13. TONE MAPPING
--------------------------------------------------------------------------------

HDR values cannot be displayed directly on monitor.

Tone mapping converts HDR into displayable colors.

This project uses exposure-based tone mapping:

    mapped = 1.0 - exp(-hdrColor * exposure)

Exposure can be controlled at runtime using ImGui.

--------------------------------------------------------------------------------
14. SKYBOX + CUBEMAP
--------------------------------------------------------------------------------

The skybox represents deep space surrounding the scene.

Implemented using a Cubemap:

- 6 textures
- one for each cube face

The skybox is rendered using:

    GL_TEXTURE_CUBE_MAP

The camera translation is removed from the skybox view matrix:

    glm::mat4(glm::mat3(view))

This creates the illusion of infinite distance.

--------------------------------------------------------------------------------
15. REFLECTIONS
--------------------------------------------------------------------------------

Reflection is implemented using environment mapping.

The fragment shader computes:

- incident vector
- reflection vector

using:

    reflect(I, N)

The reflection vector samples the cubemap texture.

This creates fake real-time reflections on planets.

--------------------------------------------------------------------------------
16. CAMERA SYSTEM
--------------------------------------------------------------------------------

The project uses a free-fly FPS camera.

Controls:

- WASD movement
- mouse look
- scroll zoom

The View Matrix is generated using:

    camera.GetViewMatrix()

This simulates movement through 3D space.

--------------------------------------------------------------------------------
17. IMGUI USER INTERFACE
--------------------------------------------------------------------------------

ImGui provides runtime GUI controls.

Used for:

- bloom toggle
- exposure slider
- time scale control
- orbit visibility

The GUI is rendered after the 3D scene.

--------------------------------------------------------------------------------
18. DEPTH TESTING
--------------------------------------------------------------------------------

Depth testing ensures correct visibility.

Each fragment stores a depth value.

OpenGL keeps the closest fragment.

Enabled using:

    glEnable(GL_DEPTH_TEST)

Without depth testing:

- objects would render on top of each other incorrectly

--------------------------------------------------------------------------------
19. SOLAR SYSTEM SIMULATION
--------------------------------------------------------------------------------

Planet motion is simulated mathematically.

Orbit equations:

    x = sin(angle) * radius
    z = cos(angle) * radius

Each planet has:

- orbit speed
- rotation speed
- distance from Sun
- scale factor

The Moon uses hierarchical transformations:

    moonPosition = earthPosition + orbitOffset

--------------------------------------------------------------------------------
20. FINAL RENDERING FLOW OF THE PROJECT
--------------------------------------------------------------------------------

FRAME LOOP:

1. Process input
2. Start ImGui frame
3. Render scene into HDR framebuffer
4. Render planets + lighting
5. Render skybox
6. Extract bright fragments
7. Apply Gaussian blur
8. Combine scene + bloom
9. Render ImGui
10. Swap buffers

--------------------------------------------------------------------------------
21. IMPORTANT OPENGL CONCEPTS USED
--------------------------------------------------------------------------------

VAO:
    stores vertex attribute configuration

VBO:
    stores vertex data inside GPU memory

EBO:
    stores index data

Shader:
    GPU program executed per vertex/fragment

Framebuffer:
    off-screen render target

Cubemap:
    6-sided environment texture

Uniform:
    CPU -> GPU variable

Sampler:
    texture access inside shader

Depth Buffer:
    stores fragment depth information

Post-Processing:
    image-space effect applied after rendering

================================================================================
END OF OPENGL OVERVIEW
================================================================================
*/



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
#include "rendering/model.h"

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


struct AsteroidInstance
{
    glm::vec3 position;

    float scale;

    float rotationSpeed;

    glm::vec3 rotationAxis;
};


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


    // ---------------------------
    // INIT GLAD
    // ---------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }


    //GUI SETUP
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
    Shader modelShader("../shaders/model.vert", "../shaders/model.frag");

    Sphere sphere;

    //PLANETS DATA
    std::vector<Planet> planets;

    //Asteroid
    std::vector<AsteroidInstance> asteroids;

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
    float moonDistance = 1.5f;   
    float moonScale    = 0.20f;  
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
    
    Model asteroid("resources/models/asteroid/10464_Asteroid_v1_Iterations-2.obj");

    srand((unsigned int)time(0));

    for (int i = 0; i < 40; i++)
    {
        AsteroidInstance asteroidData;

        // random space around the Sun
        float x = ((rand() % 200) - 100);
        float y = ((rand() % 80) - 40);
        float z = ((rand() % 200) - 100);

        asteroidData.position = glm::vec3(x, y, z);

        // random scale
        asteroidData.scale =
            0.0004f +
            static_cast<float>(rand()) / RAND_MAX * 0.0008f;

        // rotation
        asteroidData.rotationSpeed =
            0.2f +
            static_cast<float>(rand()) / RAND_MAX * 2.0f;

        asteroidData.rotationAxis = glm::normalize(glm::vec3(
            rand() % 10,
            rand() % 10,
            rand() % 10
        ));

        asteroids.push_back(asteroidData);
    }

    // ============================
    // HDR FRAMEBUFFER
    // ============================
    //framebuffer := rendering destination (instead of default framebuffer which is the screen)
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);//now all rendering commands will write to this framebuffer instead of the default one (the screen)

    //color buffers
    unsigned int colorBuffers[2]; //two differennt render textures: one for normal rendering, one for bright fragments only (for bloom)
    glGenTextures(2, colorBuffers); //allcoate two GPU texture objects

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
            //so we have in color attachment 0 the normal rendered scene, and in color attachment 1 only the bright fragments 
            //(using a brightness threshold in the shader)
    }

    // depth buffer
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    //specify which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int quadVAO = 0;
    unsigned int quadVBO;


    //the shader to process a texture basically render rectangle over screen
    //this rectangle is a fullscreen quad
    //indeed GPU renders triangles only and we know that two triangles can form a rectangle
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

    //we define a cube surrounding the scene, this will be our skybox
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
    
    //these two framebuffers will be used for the ping-pong blur step (gaussian blur) of the bloom effect
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
        ImGui::Begin("Controls");

        // simulation
        ImGui::Text("Simulation");

        ImGui::SliderFloat("Time", &timeScale, 0.0f, 20.0f);

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

        shader.setInt("skybox", 2);

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

        //bind skybox cubemap to texture unit 2
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

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

                shader.setInt("useReflection", 0);

                // NIGHT MAP SOLO PER LA TERRA
                if (planet.name == "Earth")
                {
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, earthNightTexture);
                    shader.setInt("nightMap", 1);
                    shader.setInt("hasNightMap", 1);
                    shader.setInt("useReflection", 1);
                }
                
                sphere.Draw();

                glActiveTexture(GL_TEXTURE0);

                shader.setInt("useReflection", 1);

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
        // ASTEROID FIELD
        // =====================================================
        float time = glfwGetTime() * timeScale;

        for (auto& a : asteroids)
        {
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, a.position);

            model = glm::rotate(
                model,
                time * a.rotationSpeed,
                a.rotationAxis
            );

            model = glm::scale(
                model,
                glm::vec3(a.scale)
            );

            shader.setMat4("model", model);

            shader.setInt("isEmissive", 0);
            shader.setInt("hasNightMap", 0);
            shader.setInt("useReflection", 0);

            asteroid.Draw();
        }


        // =====================================================
        // DRAW SKYBOX (INSIDE HDR PASS)
        // =====================================================
        glDepthFunc(GL_LEQUAL); // important for the skybox to be rendered behing everything else

        skyboxShader.use();

        //remove camera translation
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
    // TAB TOGGLE (just one time click)
    // =====================================================
    bool tabCurrentlyPressed =
        glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;

    if (tabCurrentlyPressed && !tabPressedLastFrame)
    {
        mouseCaptured = !mouseCaptured;

        if (mouseCaptured)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            // avoid out of the blue camera jump
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
    // CAMERA MOVEMENT (always active, even when UI is active)
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
    //if the UI is active, ignore mouse look
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