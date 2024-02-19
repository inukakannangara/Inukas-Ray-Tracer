#version 330 core
out vec4 FragColor;

in vec2 textureCoord;

const float PI = 3.14159265359;
float random(inout uint state) 
{
    uint a = uint(747796405);
    uint b = uint(2891336453);
    uint c = uint(28);
    uint d = uint(4);
    uint e = uint(277803737);

    state = state * a + b;
    uint result = ((state >>((state >> c) + d)) ^ state) * e;
    return result / 4294967295.0;
}
float randomNormalDistribution(inout uint seed) 
{
    float theta = 2 * PI * random(seed);
    float rho = sqrt(-2 * log(random(seed)));
    return rho * cos(theta);
}
vec3 randomDirection(inout uint seed) 
{
    float x = randomNormalDistribution(seed);
    float y = randomNormalDistribution(seed);
    float z = randomNormalDistribution(seed);
    return normalize(vec3(x, y, z));
}

vec2 equirectangularProjection(vec3 p)
{
    float u = 0.5 + atan(p.x/p.z)/(2 * PI);
    float v = 0.5 + asin(p.y)/PI;
    return vec2(u, -v);
}

struct Ray 
{
    vec3 origin;
    vec3 direction;
};
vec3 rayPoint(Ray ray, float t)
{
    return ray.origin + ray.direction * t;
}
struct Material
{
    vec3 color;
    float roughness;
    float emissionStrength;
};
struct HitInfo
{
    bool hasHit;
    float t;
    Material material;
    vec3 hitNormal;
};
struct Plane
{
    vec3 origin;
    vec3 normal;
    Material material;
    bool isVisible;
};
struct Sphere
{
    vec3 origin;
    float radius;
    Material material;
    bool isVisible;
};
struct Light
{
    vec3 origin;
    float radius;
    vec3 color;
    float strength;
    bool isVisible;
};

uniform int screenWidth;
uniform int screenHeight;

uniform int numSamples;
uniform int numLightBounces;
uniform float blurDistance;
uniform float blurStrength;

uniform vec3 cameraOrigin;
uniform vec3 cameraForward;
uniform vec3 cameraRight;
uniform vec3 cameraUp;

const int maxNumSpheres = 64;
const int maxNumPlanes = 64;
const int maxNumLights = 64;

uniform Sphere spheres[maxNumSpheres];
uniform Plane planes[maxNumPlanes];
uniform Light lights[maxNumLights];

uniform int numSpheres;
uniform int numPlanes;
uniform int numLights;

uniform sampler2D hdriTexture;

uint seed = uint(gl_FragCoord.y * screenWidth + gl_FragCoord.x);

HitInfo hitPlane(Ray ray, Plane plane)
{
    if (!plane.isVisible)
        return HitInfo(false, 0.0f, Material(vec3(0.0, 0.0, 0.0), 0.5, 0.0), vec3(0.0, 0.0, 0.0));

    float dn = dot(ray.direction, plane.normal);
    if (dn == 0.0) return HitInfo(false, 0.0f, Material(vec3(0.0, 0.0, 0.0), 0.5, 0.0), vec3(0.0, 0.0, 0.0));

    float t = dot(plane.origin - ray.origin, plane.normal)/dn;
    if (t < 0.001)
        return HitInfo(false, 0.0f, Material(vec3(0.0, 0.0, 0.0), 0.5, 0.0), vec3(0.0, 0.0, 0.0));
    return HitInfo(true, t, plane.material, plane.normal);
}

HitInfo hitSphere(Ray ray, Sphere sphere)
{
    if (!sphere.isVisible)
        return HitInfo(false, 0.0f, Material(vec3(0.0, 0.0, 0.0), 0.5, 0.0), vec3(0.0, 0.0, 0.0));

    vec3 rayOrigin = ray.origin - sphere.origin;
    vec3 rayDirection = ray.direction;

    float rayDirectionLength = length(rayDirection);
    float rayOriginLength = length(rayOrigin);

    float a = rayDirectionLength * rayDirectionLength;
    float b = 2 * dot(rayDirection, rayOrigin);
    float c = rayOriginLength * rayOriginLength - sphere.radius * sphere.radius;

    float d = b*b - 4.0 * a * c;
    
    if (d < 0.0f)
        return HitInfo(false, 0.0f, Material(vec3(0.0, 0.0, 0.0), 0.5, 0.0), vec3(0.0, 0.0, 0.0));

    float t1 = (-b + sqrt(d))/(2*a);
    float t2 = (-b - sqrt(d))/(2*a);

    float t = min(t1, t2);
    
    if (t <= -0.001)
        return HitInfo(false, 0.0f, Material(vec3(0.0, 0.0, 0.0), 0.5, 0.0), vec3(0.0, 0.0, 0.0)); 
   

    return HitInfo(true, t, sphere.material, rayPoint(ray, t) - sphere.origin);
}

HitInfo hitScene(Ray ray)
{
    HitInfo closestHit = HitInfo(false, 100000000.0f, Material(vec3(0.0, 0.0, 0.0), 0.5, 0.0), vec3(0.0, 0.0, 0.0));
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

vec3 calculateDirectLight(vec3 surfaceNormal, vec3 hitPoint, Material material)
{
    vec3 totalDirectLight = vec3(0.0);
    for (int i = 0; i < numLights; i++)
    {
        if (!lights[i].isVisible)
            continue;

        vec3 lightOrigin = lights[i].origin + randomDirection(seed) * lights[i].radius;

        vec3 outGoingRayDirection = normalize(lightOrigin - hitPoint);
        Ray outgoingRay = Ray(hitPoint, outGoingRayDirection);
        HitInfo closestHit = hitScene(outgoingRay);

        if (closestHit.hasHit) return vec3(0.0, 0.0, 0.0);

        vec3 directLight = lights[i].color * lights[i].strength * material.color * dot(surfaceNormal, outGoingRayDirection);
        totalDirectLight += directLight;
    }
    return totalDirectLight;
}

vec3 calculateIndirectLight(Ray incomingRay, vec3 surfaceNormal, vec3 hitPoint, Material hitMaterial, int maxBounces)
{
    vec3 indirectLight = vec3(0.0, 0.0, 0.0);
    vec3 directLight = vec3(0.0, 0.0, 0.0);

    Ray incidentRay = incomingRay;
    vec3 normal = surfaceNormal;
    vec3 incidentPoint = hitPoint;
    Material material = hitMaterial;
    for (int i = 0; i < maxBounces; i++)
    {
        vec3 reflectedRayDirection = incidentRay.direction - 2 * dot(incidentRay.direction, normal) * normal;
        reflectedRayDirection = normalize(reflectedRayDirection + randomDirection(seed) * material.roughness * 2.0); 

        Ray reflectedRay = Ray(incidentPoint, reflectedRayDirection);
        HitInfo closestHit = hitScene(reflectedRay);

        if (!closestHit.hasHit)
        {   
            vec2 textureCoordinate = equirectangularProjection(reflectedRay.direction);
            vec4 texturePixelColor = texture(hdriTexture, textureCoordinate);
            indirectLight += vec3(texturePixelColor) * hitMaterial.color;
        }

        vec3 directLight = closestHit.material.color * calculateDirectLight(closestHit.hitNormal, rayPoint(reflectedRay, closestHit.t), closestHit.material);
        incidentRay = reflectedRay;
        normal = closestHit.hitNormal;
        incidentPoint = rayPoint(reflectedRay, closestHit.t);
        material = closestHit.material;
        indirectLight = indirectLight + directLight * hitMaterial.color;
    }
    return indirectLight/maxBounces;
}

vec3 trace(Ray ray, int maxBounces)
{
    HitInfo closestHit = hitScene(ray);
    if(closestHit.hasHit)
    {
        vec3 hitPoint = rayPoint(ray, closestHit.t);
        vec3 directLight = calculateDirectLight(closestHit.hitNormal, hitPoint, closestHit.material);
        vec3 indirectLight = calculateIndirectLight(ray, closestHit.hitNormal, hitPoint, closestHit.material, maxBounces);
        return  directLight + indirectLight;
    }
    vec2 textureCoordinate = equirectangularProjection(ray.direction);
    vec4 texturePixelColor = texture(hdriTexture, textureCoordinate);
    return vec3(texturePixelColor);
}

void main()
{   
    float x = (gl_FragCoord.x - (screenWidth/2.0f)) / screenWidth;
    float y = (gl_FragCoord.y - (screenHeight/2.0f)) / screenHeight;

    vec3 rayDirection = normalize(cameraForward + x * cameraRight + y * cameraUp);
    Ray ray = Ray(cameraOrigin, rayDirection);

    vec3 rayPoint = rayPoint(ray, max(0.001, blurDistance));

    vec3 averageColor = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < numSamples; i++)
    {
        ray.origin += randomDirection(seed) * blurStrength;
        ray.direction = normalize(rayPoint - ray.origin);
        averageColor = averageColor + trace(ray, numLightBounces);
    }

   FragColor = vec4(averageColor/numSamples, 1.0);
}
