#include "Scene.h"
#include <string>
#include <iostream>

using namespace std;

Material nullMaterial = Material();
Sphere nullSphere;
Plane nullPlane;
HitInfo nullHitInfo = HitInfo(false, FLT_MAX, Material(vec3(0.0, 0.0, 0.0), 0.5, 0.0), vec3(0.0, 0.0, 0.0), 0, 0);

Material defaultMaterial = Material(vec3(1.0, 1.0, 1.0), 1.0, 0.0, 0.0);
Sphere defaultSphere = Sphere(vec3(0.0, 0.0, 0.0), 2.0, defaultMaterial, true);
Plane defaultPlane = Plane(vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), defaultMaterial, true);
Light defaultLight = Light(vec3(0.0, 4.0, 0.0), 1.0, vec3(1.0, 1.0, 1.0), 2.0, true);

vec3 rayPoint(Ray ray, float t)
{
	return ray.origin + ray.direction * t;
}

HitInfo Scene::hitScene(Ray ray)
{
	HitInfo closestHit = nullHitInfo;
	for (int i = 0; i < numSpheres; i++)
	{
		HitInfo hitInfo = hitSphere(ray, spheres[i]);
		if (!hitInfo.hasHit) continue;
		if (hitInfo.t < closestHit.t) closestHit = hitInfo;
	}
	for (int i = 0; i < numPlanes; i++)
	{
		HitInfo hitInfo = hitPlane(ray, planes[i]);
		if (!hitInfo.hasHit) continue;
		if (hitInfo.t < closestHit.t) closestHit = hitInfo;
	}
	return closestHit;
}

HitInfo Scene::hitSphere(Ray ray, Sphere sphere)
{
	if (!sphere.isVisible)
		return nullHitInfo;

	vec3 rayOrigin = ray.origin - sphere.origin;
	vec3 rayDirection = ray.direction;

	float rayDirectionLength = length(rayDirection);
	float rayOriginLength = length(rayOrigin);

	float a = rayDirectionLength * rayDirectionLength;
	float b = 2 * dot(rayDirection, rayOrigin);
	float c = rayOriginLength * rayOriginLength - sphere.radius * sphere.radius;

	float d = b * b - 4.0 * a * c;

	if (d < 0.0f)
		return nullHitInfo;

	float t1 = (-b + sqrt(d)) / (2 * a);
	float t2 = (-b - sqrt(d)) / (2 * a);

	float t = std::min(t1, t2);

	if (t <= -0.001)
		return nullHitInfo;


	return HitInfo(true, t, sphere.material, rayPoint(ray, t) - sphere.origin, sphere.index, 0);
}

HitInfo Scene::hitPlane(Ray ray, Plane plane)
{
	if (!plane.isVisible)
		return nullHitInfo;

	float dn = dot(ray.direction, plane.normal);
	if (dn == 0.0) return nullHitInfo;

	float t = dot(plane.origin - ray.origin, plane.normal) / dn;
	if (t < 0.001)
		return nullHitInfo;

	return HitInfo(true, t, plane.material, plane.normal, plane.index, 1);
}

Scene::Scene(float cameraFov = 50.0f, float cameraAspectRatio = 1920.0/1080.0) : camera(Camera(cameraFov, cameraAspectRatio))
{
	nullMaterial.color = vec3(0.0, 0.0, 0.0);
	nullMaterial.roughness = 0.0;
	nullMaterial.transmission = 0.0;
	nullMaterial.emission = 0.0;

	selectedType = 1;
	selectedIndex = 0;

	nullSphere.isVisible = false;
	nullSphere.origin = vec3(0.0, 0.0, 0.0);
	nullSphere.material = nullMaterial;

	for (int i = 0; i < maxNumSpheres; i++)
		spheres[i] = nullSphere;

	nullPlane.isVisible = false;
	nullPlane.origin = vec3(0.0, 0.0, 0.0);
	nullPlane.normal = vec3(0.0, 0.0, 0.0);
	nullPlane.material = nullMaterial;

	for (int i = 0; i < maxNumPlanes; i++)
		planes[i] = nullPlane;

	Light nullLight = Light();
	nullLight.isVisible = false;
	nullLight.origin = vec3(0.0, 0.0, 0.0);
	nullLight.radius = 0.0;
	nullLight.color = vec3(0.0, 0.0, 0.0);
	nullLight.strength = 0.0;

	for (int i = 0; i < maxNumLights; i++)
		lights[i] = nullLight;

	numSpheres = numPlanes = numLights = 0;

	Sphere sphere1 = Sphere(vec3(0.0, 0.0, 0.0), 1.5, Material(vec3(1.0, 0.3, 0.3), 1.0, 1.0, 0.0), true);
	Sphere sphere2 = Sphere(vec3(-4.0, 0.0, 0.0), 1.5, Material(vec3(0.3, 1.0, 0.3), 1.0, 0.0, 0.0), true);
	Sphere sphere3 = Sphere(vec3(4.0, 0.0, 0.0), 1.5, Material(vec3(0.3, 0.3, 1.0), 1.0, 0.0, 0.0), true);

	Plane plane1 = Plane(vec3(0.0, -1.5, 0.0), vec3(0.0, 1.0, 0.0), Material(vec3(1.0, 1.0, 1.0), 1.0, 0.0, 0.0), true);
	Plane plane2 = Plane(vec3(0.0, -1.5, 5.0), vec3(0.0, 0.0, -1.0), Material(vec3(1.0, 1.0, 1.0), 1.0, 0.0, 0.0), true);
	Plane plane3 = Plane(vec3(-6.0, -1.5, 0.0), vec3(1.0, 0.0, 0.0), Material(vec3(1.0, 0.0, 0.0), 1.0, 0.0, 0.0), true);
	Plane plane4 = Plane(vec3(6.0, -1.5, 0.0), vec3(-1.0, 0.0, 0.0), Material(vec3(0.0, 1.0, 0.0), 1.0, 0.0, 0.0), true);
	Plane plane5 = Plane(vec3(0.0, 10.0, 0.0), vec3(0.0, -1.0, 0.0), Material(vec3(1.0, 1.0, 1.0), 1.0, 0.0, 0.0), true);

	Light light1 = Light(vec3(0.0, 7.0, 0.0), 3.0, vec3(1.0, 1.0, 1.0), 500, true);

	addSphere(sphere1);
	addSphere(sphere2);
	addSphere(sphere3);

	addPlane(plane1);
	addPlane(plane2);
	addPlane(plane3);
	addPlane(plane4);
	addPlane(plane5);

	addLight(light1);
}

void Scene::addSphere(Sphere sphere)
{
	sphere.index = numSpheres;
	spheres[numSpheres] = sphere;
	numSpheres++;
}

void Scene::addPlane(Plane plane)
{
	plane.index = numPlanes;
	planes[numPlanes] = plane;
	numPlanes++;
}

void Scene::addLight(Light light)
{
	lights[numLights] = light;
	numLights++;
}

GLuint cameraOriginLocation;
GLint cameraForwardLocation;
GLint cameraRightLocation;
GLint cameraUpLocation;

GLint numSpheresLocation;
GLint numPlanesLocation;
GLint numLightsLocation;

void Scene::bind(GLuint shaderProgram)
{
	cameraOriginLocation = glGetUniformLocation(shaderProgram, "cameraOrigin");
	cameraForwardLocation = glGetUniformLocation(shaderProgram, "cameraForward");
	cameraRightLocation = glGetUniformLocation(shaderProgram, "cameraRight");
	cameraUpLocation = glGetUniformLocation(shaderProgram, "cameraUp");

	numSpheresLocation = glGetUniformLocation(shaderProgram, "numSpheres");
	numPlanesLocation = glGetUniformLocation(shaderProgram, "numPlanes");
	numLightsLocation = glGetUniformLocation(shaderProgram, "numLights");

	update(shaderProgram);
}

void Scene::update(GLuint shaderProgram)
{
	glUniform3f(cameraOriginLocation, camera.getOrigin().x, camera.getOrigin().y, camera.getOrigin().z);
	glUniform3f(cameraForwardLocation, camera.getForward().x, camera.getForward().y, camera.getForward().z);
	glUniform3f(cameraRightLocation, camera.getRight().x, camera.getRight().y, camera.getRight().z);
	glUniform3f(cameraUpLocation, camera.getUp().x, camera.getUp().y, camera.getUp().z);

	glUniform1i(numSpheresLocation, numSpheres);
	glUniform1i(numPlanesLocation, numPlanes);
	glUniform1i(numLightsLocation, numLights);

	for (int i = 0; i < numSpheres; i++)
	{
		string i_str = to_string(i);
		GLuint sphereOriginLocation = glGetUniformLocation(shaderProgram, string("spheres[").append(i_str).append("].origin").c_str());
		GLuint sphereRadius = glGetUniformLocation(shaderProgram, string("spheres[").append(i_str).append("].radius").c_str());
		GLuint sphereMaterialColorLocation = glGetUniformLocation(shaderProgram, string("spheres[").append(i_str).append("].material.color").c_str());
		GLuint sphereMaterialRoughnessLocation = glGetUniformLocation(shaderProgram, string("spheres[").append(i_str).append("].material.roughness").c_str());
		GLuint sphereMaterialTransmissionLocation = glGetUniformLocation(shaderProgram, string("spheres[").append(i_str).append("].material.transmission").c_str());
		GLuint sphereMaterialEmissionStrengthLocation = glGetUniformLocation(shaderProgram, string("spheres[").append(i_str).append("].material.emission").c_str());
		GLuint sphereIsVisibleLocation = glGetUniformLocation(shaderProgram, string("spheres[").append(i_str).append("].isVisible").c_str());

		glUniform3f(sphereOriginLocation, spheres[i].origin.x, spheres[i].origin.y, spheres[i].origin.z);
		glUniform1f(sphereRadius, spheres[i].radius);
		glUniform3f(sphereMaterialColorLocation, spheres[i].material.color.x, spheres[i].material.color.y, spheres[i].material.color.z);
		glUniform1f(sphereMaterialRoughnessLocation, spheres[i].material.roughness);
		glUniform1f(sphereMaterialTransmissionLocation, spheres[i].material.transmission);
		glUniform1f(sphereMaterialEmissionStrengthLocation, spheres[i].material.emission);
		glUniform1i(sphereIsVisibleLocation, spheres[i].isVisible);
	}
	for (int i = 0; i < numPlanes; i++)
	{
		string i_str = to_string(i);
		GLuint planeOriginLocation = glGetUniformLocation(shaderProgram, string("planes[").append(i_str).append("].origin").c_str());
		GLuint planeNormalLocation = glGetUniformLocation(shaderProgram, string("planes[").append(i_str).append("].normal").c_str());
		GLuint planeMaterialColorLocation = glGetUniformLocation(shaderProgram, string("planes[").append(i_str).append("].material.color").c_str());
		GLuint planeMaterialRoughnessLocation = glGetUniformLocation(shaderProgram, string("planes[").append(i_str).append("].material.roughness").c_str());
		GLuint planeMaterialTransmissionLocation = glGetUniformLocation(shaderProgram, string("planes[").append(i_str).append("].material.transmission").c_str());
		GLuint planeMaterialEmissionStrengthLocation = glGetUniformLocation(shaderProgram, string("planes[").append(i_str).append("].material.emission").c_str());
		GLuint sphereIsVisible = glGetUniformLocation(shaderProgram, string("planes[").append(i_str).append("].isVisible").c_str());

		glUniform3f(planeOriginLocation, planes[i].origin.x, planes[i].origin.y, planes[i].origin.z);
		glUniform3f(planeNormalLocation, planes[i].normal.x, planes[i].normal.y, planes[i].normal.z);
		glUniform3f(planeMaterialColorLocation, planes[i].material.color.x, planes[i].material.color.y, planes[i].material.color.z);
		glUniform1f(planeMaterialRoughnessLocation, planes[i].material.roughness);
		glUniform1f(planeMaterialTransmissionLocation, planes[i].material.transmission);
		glUniform1f(planeMaterialEmissionStrengthLocation, planes[i].material.emission);
		glUniform1i(sphereIsVisible, planes[i].isVisible);
	}
	for (int i = 0; i < numLights; i++)
	{
		string i_str = to_string(i);
		GLuint lightOriginLocation = glGetUniformLocation(shaderProgram, string("lights[").append(i_str).append("].origin").c_str());
		GLuint lightRadiusLocation = glGetUniformLocation(shaderProgram, string("lights[").append(i_str).append("].radius").c_str());
		GLuint lightColorLocation = glGetUniformLocation(shaderProgram, string("lights[").append(i_str).append("].color").c_str());
		GLuint lightStrengthLocation = glGetUniformLocation(shaderProgram, string("lights[").append(i_str).append("].strength").c_str());
		GLuint lightIsVisibleLocation = glGetUniformLocation(shaderProgram, string("lights[").append(i_str).append("].isVisible").c_str());

		glUniform3f(lightOriginLocation, lights[i].origin.x, lights[i].origin.y, lights[i].origin.z);
		glUniform1f(lightRadiusLocation, lights[i].radius);
		glUniform3f(lightColorLocation, lights[i].color.x, lights[i].color.y, lights[i].color.z);
		glUniform1f(lightStrengthLocation, lights[i].strength);
		glUniform1f(lightIsVisibleLocation, lights[i].isVisible);
	}
}

void Scene::gui()
{
	Begin("Object Settings ", nullptr, 0);
	Spacing();

	if (selectedType == 0)
	{
		string index = to_string(selectedIndex);
		Text(string("Sphere ").append(index).append(" is selected").c_str());
		InputFloat3(string("Origin ").append(index).c_str(), &spheres[selectedIndex].origin.x, 0);
		InputFloat(string("Radius ").append(index).c_str(), &spheres[selectedIndex].radius, 0);
		Spacing();
		ColorPicker3(string("Color ").append(index).c_str(), (float*)&spheres[selectedIndex].material.color.x, ImGuiColorEditFlags_Float);
		SliderFloat(string("Roughness ").append(index).c_str(), &spheres[selectedIndex].material.roughness, 0, 1);
		SliderFloat(string("Transmission ").append(index).c_str(), &spheres[selectedIndex].material.transmission, 0, 1);
		InputFloat(string("Emission ").append(index).c_str(), &spheres[selectedIndex].material.emission, 0);
		Checkbox(string("Visibility ").append(index).c_str(), &spheres[selectedIndex].isVisible);
		Spacing();
		if (ImGui::Button("Add new sphere"))
		{
			Sphere sphere = defaultSphere;
			addSphere(sphere);
			selectedIndex = numSpheres - 1;
		}
	}

	if (selectedType == 1)
	{
		string index = to_string(selectedIndex);
		Text(string("Plane ").append(index).append(" is selected").c_str());
		InputFloat3(string("Origin ").append(index).c_str(), &planes[selectedIndex].origin.x, 0);
		InputFloat3(string("Normal ").append(index).c_str(), &planes[selectedIndex].normal.x, 0);
		Spacing();
		ColorPicker3(string("Color ").append(index).c_str(), (float*)&planes[selectedIndex].material.color.x, ImGuiColorEditFlags_Float);
		SliderFloat(string("Roughness ").append(index).c_str(), &planes[selectedIndex].material.roughness, 0, 1);
		SliderFloat(string("Transmission ").append(index).c_str(), &planes[selectedIndex].material.transmission, 0, 1);
		InputFloat(string("Emission ").append(index).c_str(), &planes[selectedIndex].material.emission, 0);
		Checkbox(string("Visibility ").append(index).c_str(), &planes[selectedIndex].isVisible);
		Spacing();
		if (ImGui::Button("Add new plane"))
		{
			Plane plane = defaultPlane;
			addPlane(plane);
			selectedIndex = numPlanes - 1;
		}
	}
	End();

	Begin("Light Settings ", nullptr, 0);
	Spacing();
	for (int i = 0; i < numLights; i++)
	{
		string i_str = to_string(i);
		Text(string("Light ").append(i_str).c_str());
		InputFloat3(string("Light Origin ").append(i_str).c_str(), &lights[i].origin.x, 0);
		InputFloat(string("Light Radius ").append(i_str).c_str(), &lights[i].radius);
		ColorPicker3(string("Light Color ").append(i_str).c_str(), (float*) &lights[i].color.x, ImGuiColorEditFlags_Float);
		InputFloat(string("Light Strength ").append(i_str).c_str(), &lights[i].strength, 0);
	}
	End();
}

void Scene::select(int windowWidth, int windowHeight, double mouseXPosition, double mouseYPosition)
{
	float x = (mouseXPosition - windowWidth / 2.0) / windowWidth;
	float y = -(mouseYPosition - windowHeight / 2.0) / windowHeight;

	vec3 rayDirection = normalize(camera.getForward() + x * camera.getRight() + y * camera.getUp());
	Ray ray = Ray(camera.getOrigin(), rayDirection);

	HitInfo closestHit = hitScene(ray);
	
	selectedType = closestHit.hitType;
	selectedIndex = closestHit.hitIndex;
}

