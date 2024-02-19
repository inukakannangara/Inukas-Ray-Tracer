#pragma once
#include <glm.hpp>
#include "Camera.h"
#include <glad/glad.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace glm;
using namespace ImGui;

const int maxNumSpheres = 64;
const int maxNumPlanes = 64;
const int maxNumLights = 64;

struct Ray
{
    vec3 origin;
    vec3 direction;
    Ray(vec3 origin, vec3 direction) : origin(origin), direction(direction) {};
};

struct Material
{
    vec3 color;
    float roughness;
    float transmission;
    float emission;
    Material(vec3 color = vec3(0.0), float roughness = 0.0, float transmission = 0.0, float emission = 0.0) : color(color), roughness(roughness), transmission(transmission), emission(emission)
    {}
};

struct Plane
{
    vec3 origin;
    vec3 normal;
    Material material;
    bool isVisible;
    int index = INT_MAX;
    Plane(vec3 origin = vec3(0.0), vec3 normal = vec3(0.0), Material material = Material(), bool isVisible = false) : origin(origin), normal(normal), material(material), isVisible(isVisible)
    {}
};
struct Sphere
{
    vec3 origin;
    float radius;
    Material material;
    bool isVisible;
    int index = INT_MAX;
    Sphere(vec3 origin = vec3(0.0), float radius = 0.0, Material material = Material(), bool isVisible = false) : origin(origin), radius(radius), material(material), isVisible(isVisible)
    {}
};
struct Light
{
    vec3 origin;
    float radius;
    vec3 color;
    float strength;
    bool isVisible;
    Light(vec3 origin = vec3(0.0), float radius = 0.0, vec3 color = vec3(0.0), float strength = 0.0, bool isVisible = false) : origin(origin), radius(radius), color(color), strength(strength), isVisible(isVisible)
    {}
};

struct HitInfo
{
    bool hasHit;
    float t;
    Material material;
    vec3 hitNormal;
    int hitIndex;
    int hitType;
    HitInfo(bool hasHit, float t, Material material, vec3 hitNormal, int hitIndex, int hitType) : hasHit(hasHit), t(t), material(material), hitNormal(hitNormal), hitIndex(hitIndex), hitType(hitType) 
    {};
};

class Scene
{
private:
	Sphere spheres[maxNumSpheres];
    Plane planes[maxNumPlanes];
    Light lights[maxNumLights];
    int numSpheres;
    int numPlanes;
    int numLights;
    int selectedIndex;
    int selectedType;
    HitInfo hitScene(Ray ray);
    HitInfo hitSphere(Ray ray, Sphere sphere);
    HitInfo hitPlane(Ray ray, Plane plane);
public:
    Camera camera;
	Scene(float cameraFov, float cameraAspectRatio);
    void addSphere(Sphere sphere);
    void addPlane(Plane plane);
    void addLight(Light light);
    void bind(GLuint shaderProgram);
    void update(GLuint shaderProgram);
    void gui();
    void select(int windowWidth, int windowHeight, double mouseXPosition, double mouseYPosition);
};