#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb/stb_image.h>

#include "Camera.h"
#include "Scene.h"

std::string readShaderFromFile(const std::string& filePath);
static void frameBufferSizeCallback(GLFWwindow* window, int width, int height);

static void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos);
static void mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);         

GLuint createShaderProgram();
GLuint createHdriTexture(std::string path);

std::string vertexShaderCode = readShaderFromFile("vertexshader.glsl");
std::string fragmentShaderCode = readShaderFromFile("fragmentshader.glsl");

const char* vertexShaderSource = vertexShaderCode.c_str();
const char* fragmentShaderSource = fragmentShaderCode.c_str();

int screenWidth = 1920;
int screenHeight = 1080;

const float PI = 3.14159265359f;
float fov = 70.0f;
Scene scene = Scene(fov * PI / 180.0f, 1920.0f / 1080.0f);
float cameraSensitivity = 3.0f;
bool middleMouseButtonHeld = false;
float blurDistance = 5.0;
float blurStrength = 0.01;

GLFWwindow* window;

int main()
{
    if (!glfwInit()) 
    {
        std::cout << "GLFW initialization failed";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(screenWidth, screenHeight, "Ray Tracer", nullptr, nullptr);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    GLuint shaderProgram = createShaderProgram();
    GLuint hdriTexture = createHdriTexture("Outdoors.jpg");
    bool showHdri = true;

    float vertices[] = { 
        -1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f  
    };
    unsigned int indices[] = { 0, 1, 2,2, 3, 0 };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;      
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);

    GLint screenWidthLocation = glGetUniformLocation(shaderProgram, "screenWidth");
    GLint screenHeightLocation = glGetUniformLocation(shaderProgram, "screenHeight");
    int numSamples = 5;
    int numLightBounces = 5;

    GLuint numSamplesLocation = glGetUniformLocation(shaderProgram, "numSamples");
    GLuint numLightBouncesLocation = glGetUniformLocation(shaderProgram, "numLightBounces");
    GLuint blurDistanceLocation = glGetUniformLocation(shaderProgram, "blurDistance");
    GLuint blurStrengthLocation = glGetUniformLocation(shaderProgram, "blurStrength");

    scene.bind(shaderProgram);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) scene.camera.moveForward(cameraSensitivity * 0.01f);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) scene.camera.moveRight(cameraSensitivity * -0.01f);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) scene.camera.moveForward(cameraSensitivity * -0.01f);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) scene.camera.moveRight(cameraSensitivity * 0.01f);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) scene.camera.moveUp(cameraSensitivity * -0.01f);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) scene.camera.moveUp(cameraSensitivity * 0.01f);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        if (showHdri)
            glBindTexture(GL_TEXTURE_2D, hdriTexture);
        else
            glBindTexture(GL_TEXTURE_2D, 0);

        glBindVertexArray(VAO);

        glUniform1i(screenWidthLocation, screenWidth);
        glUniform1i(screenHeightLocation, screenHeight);

        glUniform1f(blurDistanceLocation, blurDistance);
        glUniform1f(blurStrengthLocation, blurStrength);

        glUniform1i(numSamplesLocation, numSamples);
        glUniform1i(numLightBouncesLocation, numLightBounces);

        scene.update(shaderProgram);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        ImGui::Begin("Ray Tracer");     
        ImGui::Text("Render Settings ");
        std::string framerate = std::to_string(int(io.Framerate));
        ImGui::Text(std::string(framerate).append(" fps").c_str());
        ImGui::InputInt("Number of Samples", &numSamples, 1, 500); 
        ImGui::InputInt("Number of Light Bounces", &numLightBounces, 1, 50);
        ImGui::Checkbox("Show Hdri", &showHdri);
        ImGui::Text("Camera Settings");
        ImGui::SliderFloat("Camera Fov", &fov, 5.0f, 175.0f);
        ImGui::SliderFloat("Camera Sensitivity", &cameraSensitivity, 1.0f, 6.0f);
        ImGui::InputFloat("Camera Depth of Field Strength", &blurStrength, 0.0, 0.1);
        scene.gui();
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        scene.camera.setFovAspectRatio(fov * PI / 180.0f, screenWidth / float(screenHeight));

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &hdriTexture);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

std::string readShaderFromFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint createShaderProgram()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
        glfwTerminate();
        return -1;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        std::cout << "Fragment shader failed to compile.";
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), NULL, infoLog);
        std::cout << infoLog;
        glfwTerminate();
        return -1;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
        glfwTerminate();
        return -1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint createHdriTexture(std::string path)
{
    GLuint Texture;
    glGenTextures(1, &Texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int textureWidth, textureHeight, numTextureChannels;
    unsigned char* hdriBytes = stbi_load(path.c_str(), &textureWidth, &textureHeight, &numTextureChannels, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, hdriBytes);

    stbi_image_free(hdriBytes);
    glBindTexture(GL_TEXTURE_2D, 0);

    return Texture;
}

double initialXPos;
double initialYPos;

static void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
    screenWidth = width;
    screenHeight = height;
    glViewport(0, 0, width, height);
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == 1)
        if (action == 1)
        {
            middleMouseButtonHeld = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if (action == 0)
        {
            middleMouseButtonHeld = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    if (button == 0)
    {
        if (action == 0)
        {
            scene.select(screenWidth, screenHeight, initialXPos, initialYPos);
        }
    }
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}

static void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (middleMouseButtonHeld)
    {
        double dx = xPos - initialXPos;
        double dy = yPos - initialYPos;

        float xRot = cameraSensitivity * float(dy) / 10000.0f;
        float yRot = cameraSensitivity * float(dx) / 10000.0f;

        scene.camera.rotateX(xRot);
        scene.camera.rotateY(yRot);
    }
    ImGui_ImplGlfw_CursorPosCallback(window, xPos, yPos);
    initialXPos = xPos;
    initialYPos = yPos;
}

static void mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    blurDistance += 0.5 * yOffset;
    ImGui_ImplGlfw_ScrollCallback(window, xOffset, yOffset);
}
