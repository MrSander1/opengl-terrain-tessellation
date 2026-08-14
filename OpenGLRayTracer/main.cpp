#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <array>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "terrain.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

/* Notes:
* Set up imgui Done!
* Set up textures Done!
* Lighting Done!
* Lighting with textures Done!
* Add point light cube color tie with ambient Done!
* Add rez and subdiv controls (need a function that updates the mesh/rebinds everything tied to vertices array, need a boolean tied to IsItemActive() in Imgui controls, nothing on shaders end) Done!
* Add different shaders i.e texture height map coloring and fog with options to turn em on or off (doing this through one shader and then uniform toogles techniqually not different shaders) Done!
*/

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
unsigned int loadTexture(char const* path);

// Settings
const unsigned int NUM_PATCH_PTS = 4;
const unsigned int height = 800;
const unsigned int width = 600;

int MIN_TESS_LEVEL = 4;
int MAX_TESS_LEVEL = 64;

float MIN_DISTANCE = 20;
float MAX_DISTANCE = 100;

// Terrain Generation 
std::vector<float> vertGen(int rez, int subdivisions);

bool consumed1;
bool consumed2; 

bool fogToogle;
bool wireframeToggle;

// Camera
Camera camera(glm::vec3(0.0f, 10.0f, 3.0f));
float lastX = height / 2.0f;
float lastY = width / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// Skybox Color
float clear_color[4] = { 0.45f, 0.55f, 0.60f, 1.00f };

// Lighting
glm::vec3 dirLightPos(1.2f, 1.0f, 2.0f);
glm::vec3 dirLightColor(0.17f, 0.09f, 0.05f);
glm::vec3 dirDiffuseColor = dirLightColor; // decrease the influence
glm::vec3 dirAmbientColor = dirDiffuseColor; // low influence
glm::vec3 dirSpecular(0.0f, 0.0f, 0.0f);
float dirLightIntensity = 1.0f;

// Material
glm::vec3 matDiffuse(0.4f, 0.25f, 0.15f);
glm::vec3 matAmbient(0.08f, 0.05f, 0.03f);
glm::vec3 matSpecular(0.0f, 0.0f, 0.0f);
float matShininess = 1.0f;

// Fog
float fogMaxDist = 100.0;
float fogMinDist = 0.1;
glm::vec3 fogColor(0.4, 0.4, 0.4);

// Grass & Mud
float peakSharpness = 2.0;

int main() {
    // more rez means more verts
    // more subdivisions means larger grid
    //Terrain                                          sub   rez
    Terrain terrain(0.3f, 0.5f, 0.5f, 2.0f, 8, 10, 10, 100, 68); 

    std::vector vertices = vertGen(terrain.getRez(), terrain.getSubdivisions());

    // glfw: init and configs
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Window
    GLFWwindow* window = glfwCreateWindow(height, width, "Terrain Generator", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD: loads OpenGL Functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark(); // Choose built-in style

    //Initialize Platform/Renderer Backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    GLint maxTessLevel;
    glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxTessLevel);
    std::cout << "Max available tess level: " << maxTessLevel << std::endl;

    // Wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);


    // build and compile shaders
    Shader testShader("test.vert", "test.frag", "test.tesc", "test.tese");
    Shader pointShader("point.vert", "point.frag");

    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };

    // positions of the point lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
    };
    // ambient colors for point lights
    glm::vec3 pointLightAmbients[] = {
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f)
    };
    // diffuse colors for point lights
    glm::vec3 pointLightDiffuses[] = {
        glm::vec3(0.8f, 0.8f, 0.8f),
        glm::vec3(0.8f, 0.8f, 0.8f),
        glm::vec3(0.8f, 0.8f, 0.8f),
        glm::vec3(0.8f, 0.8f, 0.8f)
    };
    // specular colors for point lights
    glm::vec3 pointLightSpeculars[] = {
                glm::vec3(1.0f, 1.0f, 1.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                glm::vec3(1.0f, 1.0f, 1.0f)
    };

    // constants for point lights
    float pointLightConstants[] = {
        1.0f, 1.0f, 1.0f, 1.0f
    };
    // linears for point lights
    float pointLightLinears[] = {
        0.09f, 0.09f, 0.09f, 0.09f
    };
    // quadratics for point lights
    float pointLightQuadratics[] = {
        0.032f, 0.032f, 0.032f, 0.032f
    };

    // register VAO
    unsigned int terrainVAO, terrainVBO;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
   
    glPatchParameteri(GL_PATCH_VERTICES, NUM_PATCH_PTS);


    // cube light source 
    unsigned int lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);

    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindVertexArray(lightVAO);

    // set the vertex attribute 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int texture1 = loadTexture("dirt.jpg");
    unsigned int texture2 = loadTexture("grass.jpg");
    
    testShader.use();
    testShader.setInt("material.diffuseMud", 0);
    testShader.setInt("material.diffuseGrass", 1);


    // Render Loop
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input 
        processInput(window);


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // UI Windows & Widgets
        {
            // Terrain
            ImGui::Begin("Settings Panel | Middle mouse disables/enables cursor");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::ColorEdit4("Background Color", clear_color); // Modifies array values in-place 
            if (ImGui::CollapsingHeader("Terrain Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SliderFloat("Frequency", terrain.getFrequencyPtr(), 0.0f, 1.0f);
                ImGui::SliderFloat("Gain", terrain.getGainPtr(), 0.0f, 1.0f);
                ImGui::SliderFloat("Lacunarity", terrain.getLacunarityPtr(), 0.0f, 4.0f);
                ImGui::SliderInt("Octaves", terrain.getOctavesPtr(), 0, 16);
                ImGui::SliderInt("Max", terrain.getMaxPtr(), 0, 16);
                ImGui::SliderInt("Min", terrain.getMinPtr(), 0, 16);

                // Special binded terrain controls i.e done in buffer/they need a refresh/they are tied to cpu
                ImGui::SliderInt("Subdivisions", terrain.getSubdivisionsPtr(), 1, 500);
                consumed1 = ImGui::IsItemActive();
                ImGui::SliderInt("Resolution", terrain.getRezPtr(), 1, 250);
                consumed2 = ImGui::IsItemActive();

                ImGui::SliderFloat("Grass Quantity", &peakSharpness, 0.0f, 2.0f);

                ImGui::Checkbox("Wireframe Toggle", &wireframeToggle);
            }
            if (ImGui::CollapsingHeader("Tessellation Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SliderInt("Tess Min", &MIN_TESS_LEVEL, 0, 64);
                ImGui::SliderInt("Tess Max", &MAX_TESS_LEVEL, 0, 64);
                ImGui::SliderFloat("Tess Min Distance", &MIN_DISTANCE, 0.0f, 100.0f);
                ImGui::SliderFloat("Tess Max Distance", &MAX_DISTANCE, 0.0f, 100.0f);
            }
            // Lighting
            if (ImGui::CollapsingHeader("Light Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                // Point lights
                for (int i = 0; i < std::size(pointLightPositions); ++i) {
                    if (ImGui::CollapsingHeader(("Point Light " + std::to_string(i) + " Properties").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::SliderFloat3(("Point Light " + std::to_string(i) + " Position").c_str(), &pointLightPositions[i].x, -50.0f, 50.0f);
                        ImGui::SliderFloat3(("Point Light " + std::to_string(i) + " Ambient").c_str(), &pointLightAmbients[i].x, 0.0f, 1.0f);
                        ImGui::SliderFloat3(("Point Light " + std::to_string(i) + " Diffuse").c_str(), &pointLightDiffuses[i].x, 0.0f, 1.0f);
                        ImGui::SliderFloat3(("Point Light " + std::to_string(i) + " Specular").c_str(), &pointLightSpeculars[i].x, 0.0f, 1.0f);
                        ImGui::SliderFloat(("Point Light " + std::to_string(i) + " Constant").c_str(), &pointLightConstants[i], 0.0f, 1.0f);
                        ImGui::SliderFloat(("Point Light " + std::to_string(i) + " Linear").c_str(), &pointLightLinears[i], 0.0f, 1.0f);
                        ImGui::SliderFloat(("Point Light " + std::to_string(i) + " Quadratic").c_str(), &pointLightQuadratics[i], 0.0f, 1.0f);
                    }
                }
                // Directional Lights
                 if (ImGui::CollapsingHeader("Directional Light Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::SliderFloat3("Directional Light Position", &dirLightPos.x, -50.0f, 50.0f);
                        ImGui::SliderFloat3("Directional Diffuse Color", &dirDiffuseColor.x, 0.0f, 1.0f);
                        ImGui::SliderFloat3("Directional Ambient Color", &dirAmbientColor.x, 0.0f, 1.0f);
                        ImGui::SliderFloat("Directional Light Intensity", &dirLightIntensity, 0.0f, 10.0f);
                 }
            }

            // Fog
            if (ImGui::CollapsingHeader("Fog", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Fog Toogle", &fogToogle);
                ImGui::SliderFloat("fogMaxDist", &fogMaxDist, 0.0f, 100.0f);
                ImGui::SliderFloat("fogMinDist", &fogMinDist, 0.0f, 100.0f);
                ImGui::SliderFloat3("FogColor", &fogColor.x, 0.0f, 1.0f);

            }

            ImGui::End();
        }

        ImGui::Render(); // Collects draw data

        // Rendering
        glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (wireframeToggle) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        
        // Texture binding go here btw
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        testShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)height / (float)width, 0.1f, 100000.0f);
        testShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        testShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        testShader.setMat4("model", model);

        // Terrain values: {frequency: 0.3f, amplitude: 0.5f, gain: 0.5f, lacunarity: 2.0f, octaves: 8, max: 10, min: 10}
        testShader.setFloat("terrain.frequency", terrain.getFrequency());
        testShader.setFloat("terrain.amplitude", terrain.getAmplitude());
        testShader.setFloat("terrain.gain", terrain.getGain());
        testShader.setFloat("terrain.lacunarity", terrain.getLacunarity());
        testShader.setInt("terrain.octaves", terrain.getOctaves());
        testShader.setInt("terrain.min", terrain.getMin());
        testShader.setInt("terrain.max", terrain.getMax());
        testShader.setVec3("viewPos", camera.Position);
        testShader.setInt("MIN_TESS_LEVEL", MIN_TESS_LEVEL);
        testShader.setInt("MAX_TESS_LEVEL", MAX_TESS_LEVEL);
        testShader.setFloat("MIN_DISTANCE", MIN_DISTANCE);
        testShader.setFloat("MAX_DISTANCE", MAX_DISTANCE);

        // Light properties
        // 
        // directional light
        testShader.setVec3("dirLight.direction", dirLightPos);
        testShader.setVec3("dirLight.ambient", dirAmbientColor);
        testShader.setVec3("dirLight.diffuse", dirDiffuseColor);
        testShader.setVec3("dirLight.specular", dirSpecular);
        testShader.setFloat("dirLight.intensity", dirLightIntensity);

        for (int i = 0; i < std::size(pointLightPositions); ++i) {
            testShader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLightPositions[i]);
            testShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", pointLightAmbients[i]);
            testShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", pointLightDiffuses[i]);
            testShader.setVec3("pointLights[" + std::to_string(i) + "].specular", pointLightSpeculars[i]);
            testShader.setFloat("pointLights[" + std::to_string(i) + "].constant", pointLightConstants[i]);
            testShader.setFloat("pointLights[" + std::to_string(i) + "].linear", pointLightLinears[i]);
            testShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", pointLightQuadratics[i]);
        }
       

        // material properties
        testShader.setVec3("material.ambient", matAmbient);
        testShader.setVec3("material.diffuse", matDiffuse);
        testShader.setVec3("material.specular", matSpecular); // specular lighting doesn't have full effect on this object's material
        testShader.setFloat("material.shininess", matShininess);

        // Fog & Grass
        testShader.setBool("fogToogle", fogToogle);
        testShader.setFloat("fog.maxDist", fogMaxDist);
        testShader.setFloat("fog.minDist", fogMinDist);
        testShader.setVec3("fog.color", fogColor);

        testShader.setFloat("peakSharpness", peakSharpness);

        

        // Updating buffer for cpu-based generation function
        if (consumed1 || consumed2) {
            vertices = vertGen(terrain.getRez(), terrain.getSubdivisions());
            glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)* vertices.size(), &vertices[0], GL_STATIC_DRAW);
        }

        // Draw mesh
        glBindVertexArray(terrainVAO);
        glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS * terrain.getRez() * terrain.getRez()); // prolly why rez changes stuff


        // Directional Point light cube
        pointShader.use();
        pointShader.setMat4("projection", projection);
        pointShader.setMat4("view", view);

        glBindVertexArray(lightVAO);
        for (unsigned int i = 0; i < std::size(pointLightPositions); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            pointShader.setVec3("objectColor", pointLightAmbients[i]);
            pointShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Check and call events and swap the buffers
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Draws UI over your scene
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // deallocating resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &terrainVAO);
    glDeleteBuffers(1, &terrainVBO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &lightVBO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        int mode = glfwGetInputMode(window, GLFW_CURSOR);

        if (mode == GLFW_CURSOR_DISABLED) {
            camera.setCameraActive(false);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            camera.setCameraActive(true);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// might be smart to move this into terrain
std::vector<float> vertGen(int rez, int subdivisions) {

    std::vector<float> vertices;

    for (unsigned i = 0; i <= rez - 1; i++)
    {
        for (unsigned j = 0; j <= rez - 1; j++)
        {
            vertices.push_back(-subdivisions / 2.0f + subdivisions * i / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-subdivisions / 2.0f + subdivisions * j / (float)rez); // v.z
            vertices.push_back(i / (float)rez); // u
            vertices.push_back(j / (float)rez); // v

            vertices.push_back(-subdivisions / 2.0f + subdivisions * (i + 1) / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-subdivisions / 2.0f + subdivisions * j / (float)rez); // v.z
            vertices.push_back((i + 1) / (float)rez); // u
            vertices.push_back(j / (float)rez); // v

            vertices.push_back(-subdivisions / 2.0f + subdivisions * i / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-subdivisions / 2.0f + subdivisions * (j + 1) / (float)rez); // v.z
            vertices.push_back(i / (float)rez); // u
            vertices.push_back((j + 1) / (float)rez); // v

            vertices.push_back(-subdivisions / 2.0f + subdivisions * (i + 1) / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-subdivisions / 2.0f + subdivisions * (j + 1) / (float)rez); // v.z
            vertices.push_back((i + 1) / (float)rez); // u
            vertices.push_back((j + 1) / (float)rez); // v
        }
    }

    return vertices;

}

unsigned int loadTexture(char const* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


