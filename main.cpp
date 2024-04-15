#include "tinyxml2.h"
#include <iostream>
#include <string>
#include <sstream>
#include "SceneXmlModel.h"
#include <memory>
#include "ppm.h"
#include "Ray.h"
#include <chrono>

using namespace tinyxml2;

typedef struct Hit
{
    bool hitHappened;
    Vector3 intersectionPoint;
    Vector3 surfaceNormal;
    int material_id;
    float t;
    int obj_id;
} hit;

void generateSceneFromXml(std::string, Scene *scene);
void printData(Scene &scene);
void createPpmImage(Scene &scene);
void render(Scene *scene);
Ray calculateRay(const Camera &camera, int i, int j);
void cameraSetup(Camera *camera);

float findDistance(const Vector3 &a, const Vector3 &b)
{
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

Color3 ray_color(const Ray &r)
{
    return Color3(0, 0, 0);
}

Ray calculateRay(const Camera &camera, int i, int j)
{
    // S = q + SuU - SvV
    float Su = (camera.nearPlane.right - camera.nearPlane.left) * (i + 0.5) / camera.width;
    float Sv = (camera.nearPlane.top - camera.nearPlane.bottom) * (j + 0.5) / camera.height;
    Vector3 SuU = Su * camera.u;
    Vector3 SvV = Sv * camera.v;

    Vector3 S = camera.q + SuU - SvV;

    // r(t) = e + (s-e)t
    Vector3 direction = S - camera.position;

    Ray ray = Ray(camera.position, direction);
    return ray;
}

float determinant(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2)
{
    return v0.x * (v1.y * v2.z - v2.y * v1.z) + v0.y * (v2.x * v1.z - v1.x * v2.z) + v0.z * (v1.x * v2.y - v1.y * v2.x);
}

Vector3 findIntersectionPoint(const Ray &ray, float t)
{
    Vector3 result;
    Vector3 rayOrigin = ray.getOrigin();
    Vector3 rayDirection = ray.getDirection();

    result.x = rayOrigin.x + t * rayDirection.x;
    result.y = rayOrigin.y + t * rayDirection.y;
    result.z = rayOrigin.z + t * rayDirection.z;

    return result;
}

Hit triangleIntersection(const Ray &ray, const Vector3 &a, const Vector3 &b, const Vector3 &c, int material_id, int obj_id)
{
    Hit hit;
    hit.hitHappened = false;

    Vector3 o = ray.getDirection();
    Vector3 d = ray.getOrigin();

    Vector3 a_minus_b = a - b;
    Vector3 a_minus_c = a - c;
    Vector3 a_minus_o = a - d;

    float detA = determinant(a_minus_b, a_minus_c, d);
    if (detA == 0.0)
    {
        return hit;
    }

    float t = (determinant(a_minus_b, a_minus_c, a_minus_o)) / detA;
    if (t <= 0.0)
    {
        return hit;
    }

    float gamma = (determinant(a_minus_b, a_minus_o, d)) / detA;
    std::cout << "gamma: " << gamma << std::endl;
    if (gamma < 0 || gamma > 1)
    {
        return hit;
    }
    // else {
    //     std::cout << "gamma: " << gamma << std::endl;
    // }

    float beta = (determinant(a_minus_o, a_minus_c, d)) / detA;
    if (beta < 0 || beta > (1 - gamma))
    {
        return hit;
    }
    else {
        std::cout << "beta: " << beta << std::endl;
    }

    hit.hitHappened = true;
    hit.obj_id = obj_id;
    hit.material_id = material_id;
    hit.t = t;
    hit.intersectionPoint = findIntersectionPoint(ray, t);
    hit.surfaceNormal = cross(b - a, c - a);
    hit.surfaceNormal = hit.surfaceNormal.normalize();

    if (hit.hitHappened)
    {
        std::cout << "hit happened" << std::endl;
    }

    return hit;
}

Hit findHit(std::vector<Hit> &hitInfoVector)
{
    Hit result;
    result.hitHappened = false;

    if (hitInfoVector.size() != 0)
    {
        result = hitInfoVector[0];

        for (int i = 1; i < hitInfoVector.size(); i++)
        {
            if (hitInfoVector[i].t < result.t)
            {
                result = hitInfoVector[i];
            }
        }
        result.hitHappened = true;
    }

    return result;
}

Hit meshIntersection(const Ray &ray, const Mesh &mesh, const Scene &scene, int material_id, int obj_id)
{
    Hit hit;
    hit.hitHappened = false;
    std::vector<Hit> hitInfoVector;

    /*********FOR EACH TRIANGLE(FACE) IN A MESH**********/
    for (int faceNumber = 0; faceNumber < mesh.faces.size(); faceNumber++)
    {
        Vector3 v0 = scene.vertexData[mesh.faces[faceNumber].x - 1];
        Vector3 v1 = scene.vertexData[mesh.faces[faceNumber].y - 1];
        Vector3 v2 = scene.vertexData[mesh.faces[faceNumber].z - 1];

        hit = triangleIntersection(ray, v0, v1, v2, material_id, obj_id);
        if (hit.hitHappened && hit.t >= 0)
        {
            hit.material_id = material_id;
            hit.obj_id = obj_id;
            hit.intersectionPoint = findIntersectionPoint(ray, hit.t);
            hit.surfaceNormal = cross(v1 - v0, v2 - v0);
            hit.surfaceNormal = hit.surfaceNormal.normalize();

            hitInfoVector.push_back(hit);
        }
    }

    hit = findHit(hitInfoVector);
    return hit;
}

Hit intersectWithObject(const Scene &scene, const Ray &ray)
{

    Mesh currentMesh;
    Vector3 normal, intersectionPoint;

    int numberOfMeshes = scene.meshes.size();

    std::vector<Hit> hitInfoVector;
    for (int meshNumber = 0; meshNumber < numberOfMeshes; meshNumber++)
    {
        currentMesh = scene.meshes[meshNumber];

        Hit hit = meshIntersection(ray, currentMesh, scene, currentMesh.materialId, meshNumber);

        if (hit.hitHappened && hit.t >= 0)
        {
            hitInfoVector.push_back(hit);
        }
    }

    Hit hitResult = findHit(hitInfoVector);

    return hitResult;
}

void createPpmImage(Scene &scene)
{
    // Image
    int image_width = 256;
    int image_height = 256;

    // Render

    std::cout << "P3\n"
              << image_width << ' ' << image_height << "\n255\n";

    for (int j = 0; j < image_height; j++)
    {
        for (int i = 0; i < image_width; i++)
        {
            auto r = double(i) / (image_width - 1);
            auto g = double(j) / (image_height - 1);
            auto b = 0.0;

            if (i > 128)
            {
                r = 1.0;
                g = 0.0;
                b = 0.5;
            }
            else
            {
                r = 0.0;
                g = 0.0;
                b = 0.0;
            }

            int ir = int(255.999 * r);
            int ig = int(255.999 * g);
            int ib = int(255.999 * b);

            std::cout << ir << ' ' << ig << ' ' << ib << '\n';
        }
    }
}

void printData(Scene &scene)
{
    std::cout << std::endl
              << "scene data" << std::endl;

    // print everything to see if it is working
    std::cout << "maxRayTraceDepth: " << scene.maxRayTraceDepth << std::endl;
    std::cout << "backgroundColor: " << scene.backgroundColor.x << " " << scene.backgroundColor.y << " " << scene.backgroundColor.z << std::endl;
    std::cout << "camera position: " << scene.camera.position.x << " " << scene.camera.position.y << " " << scene.camera.position.z << std::endl;
    std::cout << "camera gaze: " << scene.camera.gaze.x << " " << scene.camera.gaze.y << " " << scene.camera.gaze.z << std::endl;
    std::cout << "camera up: " << scene.camera.up.x << " " << scene.camera.up.y << " " << scene.camera.up.z << std::endl;
    std::cout << "camera nearPlane: " << scene.camera.nearPlane.left << " " << scene.camera.nearPlane.right << " " << scene.camera.nearPlane.bottom << " " << scene.camera.nearPlane.top << std::endl;
    std::cout << "camera nearDistance: " << scene.camera.nearDistance << std::endl;
    std::cout << "camera imageResolution: " << scene.camera.imageResolution.nx << " " << scene.camera.imageResolution.ny << std::endl
              << std::endl;

    // print lights
    std::cout << "ambientLight: " << scene.ambientLight.x << " " << scene.ambientLight.y << " " << scene.ambientLight.z << std::endl;
    for (auto pointLight : scene.pointLights)
    {
        std::cout << "pointLight id: " << pointLight.id << std::endl;
        std::cout << "pointLight position: " << pointLight.position.x << " " << pointLight.position.y << " " << pointLight.position.z << std::endl;
        std::cout << "pointLight intensity: " << pointLight.intensity.x << " " << pointLight.intensity.y << " " << pointLight.intensity.z << std::endl;
    }
    for (auto triangularLight : scene.triangularLights)
    {
        std::cout << "triangularLight id: " << triangularLight.id << std::endl;
        std::cout << "triangularLight vertex1: " << triangularLight.vertex1.x << " " << triangularLight.vertex1.y << " " << triangularLight.vertex1.z << std::endl;
        std::cout << "triangularLight vertex2: " << triangularLight.vertex2.x << " " << triangularLight.vertex2.y << " " << triangularLight.vertex2.z << std::endl;
        std::cout << "triangularLight vertex3: " << triangularLight.vertex3.x << " " << triangularLight.vertex3.y << " " << triangularLight.vertex3.z << std::endl;
        std::cout << "triangularLight intensity: " << triangularLight.intensity.x << " " << triangularLight.intensity.y << " " << triangularLight.intensity.z << std::endl;
    }

    // print materials

    for (auto material : scene.materials)
    {
        std::cout << "material id: " << material.id << std::endl;
        std::cout << "material ambient: " << material.ambient.x << " " << material.ambient.y << " " << material.ambient.z << std::endl;
        std::cout << "material diffuse: " << material.diffuse.x << " " << material.diffuse.y << " " << material.diffuse.z << std::endl;
        std::cout << "material specular: " << material.specular.x << " " << material.specular.y << " " << material.specular.z << std::endl;
        std::cout << "material mirrorReflectance: " << material.mirrorReflectance.x << " " << material.mirrorReflectance.y << " " << material.mirrorReflectance.z << std::endl;
        std::cout << "material phongExponent: " << material.phongExponent << std::endl;
    }

    // print vertex data
    for (auto vertex : scene.vertexData)
    {
        std::cout << "vertex: " << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
    }

    // print objects

    for (auto mesh : scene.meshes)
    {
        std::cout << "mesh id: " << mesh.id << std::endl;
        std::cout << "mesh materialId: " << mesh.materialId << std::endl;
        std::cout << "mesh faces: " << std::endl;
        int i = 0;
        for (auto face : mesh.faces)
        {
            if (i < 1)
            {
                std::cout << "face " << i++ << ": ";
                std::cout << face.x << " " << face.y << " " << face.z << std::endl;
            }
        }
    }
}

void generateSceneFromXml(std::string fileName, Scene *scene)
{
    // std::cout << "inside generateSceneFromXml" << std::endl;

    XMLDocument doc;
    doc.LoadFile(fileName.c_str());

    if (doc.Error())
    {
        std::cerr << "Error loading XML file: " << doc.ErrorStr() << std::endl;
        return;
    }

    // Access scene
    XMLElement *sceneElement = doc.FirstChildElement("scene");

    if (sceneElement)
    {
        scene->maxRayTraceDepth = sceneElement->FirstChildElement("maxraytracedepth")->IntText();

        auto bgElement = sceneElement->FirstChildElement("backgroundColor");
        if (bgElement)
        {
            const char *bgText = bgElement->GetText();
            if (bgText)
            {
                std::istringstream iss(bgText);
                iss >> scene->backgroundColor.x >> scene->backgroundColor.y >> scene->backgroundColor.z;
            }
        }
    }

    // Access camera
    XMLElement *cameraElement = sceneElement->FirstChildElement("camera");
    if (cameraElement)
    {

        scene->camera.position = Vector3();
        scene->camera.gaze = Vector3();
        scene->camera.up = Vector3();
        scene->camera.imageResolution = ImageResolution();
        scene->camera.nearPlane = NearPlane();
        auto positionElement = cameraElement->FirstChildElement("position");
        if (positionElement)
        {
            const char *positionText = positionElement->GetText();
            if (positionText)
            {
                std::istringstream iss(positionText);
                iss >> scene->camera.position.x >> scene->camera.position.y >> scene->camera.position.z;
            }
        }

        auto gazeElement = cameraElement->FirstChildElement("gaze");
        if (gazeElement)
        {
            const char *gazeText = gazeElement->GetText();
            if (gazeText)
            {
                std::istringstream iss(gazeText);
                iss >> scene->camera.gaze.x >> scene->camera.gaze.y >> scene->camera.gaze.z;
            }
        }

        auto upElement = cameraElement->FirstChildElement("up");
        if (upElement)
        {
            const char *upText = upElement->GetText();
            if (upText)
            {
                std::istringstream iss(upText);
                iss >> scene->camera.up.x >> scene->camera.up.y >> scene->camera.up.z;
            }
        }

        auto nearPlaneElement = cameraElement->FirstChildElement("nearPlane");
        if (nearPlaneElement)
        {
            const char *nearPlaneText = nearPlaneElement->GetText();
            if (nearPlaneText)
            {
                std::istringstream iss(nearPlaneText);
                iss >> scene->camera.nearPlane.left >>
                    scene->camera.nearPlane.right >>
                    scene->camera.nearPlane.bottom >>
                    scene->camera.nearPlane.top;
            }
        }

        auto nearDistanceElement = cameraElement->FirstChildElement("neardistance");
        if (nearDistanceElement)
        {
            scene->camera.nearDistance = nearDistanceElement->DoubleText();
        }

        auto imageResolutionElement = cameraElement->FirstChildElement("imageresolution");
        if (imageResolutionElement)
        {
            const char *imageResolutionText = imageResolutionElement->GetText();
            if (imageResolutionText)
            {
                std::istringstream iss(imageResolutionText);
                iss >> scene->camera.imageResolution.nx >> scene->camera.imageResolution.ny;
            }
        }
    }

    // Access lights
    XMLElement *lightsElement = sceneElement->FirstChildElement("lights");
    if (lightsElement)
    {
        // ambient light
        auto ambientLightElement = lightsElement->FirstChildElement("ambientlight");
        if (ambientLightElement)
        {
            scene->ambientLight = Vector3();
            const char *ambientLightText = ambientLightElement->GetText();
            if (ambientLightText)
            {
                std::istringstream iss(ambientLightText);
                iss >> scene->ambientLight.x >> scene->ambientLight.y >> scene->ambientLight.z;
            }
        }

        // point lights
        XMLElement *pointLightElement = lightsElement->FirstChildElement("pointlight");
        scene->pointLights = std::vector<PointLight>();
        while (pointLightElement)
        {
            PointLight pointLight = PointLight();
            pointLight.position = Vector3();
            pointLight.intensity = Vector3();
            pointLightElement->QueryIntAttribute("id", &pointLight.id);

            auto positionElement = pointLightElement->FirstChildElement("position");
            if (positionElement)
            {
                const char *positionText = positionElement->GetText();
                if (positionText)
                {
                    std::istringstream iss(positionText);
                    iss >> pointLight.position.x >> pointLight.position.y >> pointLight.position.z;
                }
            }

            auto intensityElement = pointLightElement->FirstChildElement("intensity");
            if (intensityElement)
            {
                const char *intensityText = intensityElement->GetText();
                if (intensityText)
                {
                    std::istringstream iss(intensityText);
                    iss >> pointLight.intensity.x >> pointLight.intensity.y >> pointLight.intensity.z;
                }
            }

            scene->pointLights.push_back(pointLight);
            pointLightElement = pointLightElement->NextSiblingElement("pointlight");
        }

        // triangular lights
        XMLElement *triangularLightElement = lightsElement->FirstChildElement("triangularlight");
        scene->triangularLights = std::vector<TriangularLight>();
        while (triangularLightElement)
        {
            TriangularLight triangularLight = TriangularLight();
            triangularLight.vertex1 = Vector3();
            triangularLight.vertex2 = Vector3();
            triangularLight.vertex3 = Vector3();
            triangularLight.intensity = Vector3();
            triangularLightElement->QueryIntAttribute("id", &triangularLight.id);

            auto vertex1Element = triangularLightElement->FirstChildElement("vertex1");
            if (vertex1Element)
            {
                const char *vertex1Text = vertex1Element->GetText();
                if (vertex1Text)
                {
                    std::istringstream iss(vertex1Text);
                    iss >> triangularLight.vertex1.x >> triangularLight.vertex1.y >> triangularLight.vertex1.z;
                }

                auto vertex2Element = triangularLightElement->FirstChildElement("vertex2");
                if (vertex2Element)
                {
                    const char *vertex2Text = vertex2Element->GetText();
                    if (vertex2Text)
                    {
                        std::istringstream iss(vertex2Text);
                        iss >> triangularLight.vertex2.x >> triangularLight.vertex2.y >> triangularLight.vertex2.z;
                    }
                }

                auto vertex3Element = triangularLightElement->FirstChildElement("vertex3");
                if (vertex3Element)
                {
                    const char *vertex3Text = vertex3Element->GetText();
                    if (vertex3Text)
                    {
                        std::istringstream iss(vertex3Text);
                        iss >> triangularLight.vertex3.x >> triangularLight.vertex3.y >> triangularLight.vertex3.z;
                    }
                }

                auto intensityElement = triangularLightElement->FirstChildElement("intensity");
                if (intensityElement)
                {
                    const char *intensityText = intensityElement->GetText();
                    if (intensityText)
                    {
                        std::istringstream iss(intensityText);
                        iss >> triangularLight.intensity.x >> triangularLight.intensity.y >> triangularLight.intensity.z;
                    }
                }
            }

            scene->triangularLights.push_back(triangularLight);
            triangularLightElement = triangularLightElement->NextSiblingElement("triangularlight");
        }
    }

    // Accessing materials
    XMLElement *materialsElement = sceneElement->FirstChildElement("materials");
    if (materialsElement)
    {
        XMLElement *materialElement = materialsElement->FirstChildElement("material");
        scene->materials = std::vector<Material>();
        while (materialElement)
        {
            Material material = Material();
            material.ambient = Vector3();
            material.diffuse = Vector3();
            material.specular = Vector3();
            material.mirrorReflectance = Vector3();
            materialElement->QueryIntAttribute("id", &material.id);

            auto ambientElement = materialElement->FirstChildElement("ambient");
            if (ambientElement)
            {
                const char *ambientText = ambientElement->GetText();
                if (ambientText)
                {
                    std::istringstream iss(ambientText);
                    iss >> material.ambient.x >> material.ambient.y >> material.ambient.z;
                }
            }

            auto diffuseElement = materialElement->FirstChildElement("diffuse");
            if (diffuseElement)
            {
                const char *diffuseText = diffuseElement->GetText();
                if (diffuseText)
                {
                    std::istringstream iss(diffuseText);
                    iss >> material.diffuse.x >> material.diffuse.y >> material.diffuse.z;
                }
            }

            auto specularElement = materialElement->FirstChildElement("specular");
            if (specularElement)
            {
                const char *specularText = specularElement->GetText();
                if (specularText)
                {
                    std::istringstream iss(specularText);
                    iss >> material.specular.x >> material.specular.y >> material.specular.z;
                }
            }

            auto mirrorReflectanceElement = materialElement->FirstChildElement("mirrorreflectance");
            if (mirrorReflectanceElement)
            {
                const char *mirrorReflectanceText = mirrorReflectanceElement->GetText();
                if (mirrorReflectanceText)
                {
                    std::istringstream iss(mirrorReflectanceText);
                    iss >> material.mirrorReflectance.x >> material.mirrorReflectance.y >> material.mirrorReflectance.z;
                }
            }

            auto phongExponentElement = materialElement->FirstChildElement("phongexponent");
            if (phongExponentElement)
            {
                material.phongExponent = phongExponentElement->IntText();
            }

            scene->materials.push_back(material);
            materialElement = materialElement->NextSiblingElement("material");
        }
    }

    //     <vertexdata>
    //     -0.5 0.5 -2
    //     -0.5 -0.5 -2
    //     0.5 -0.5 -2
    //     0.5 0.5 -2
    //     0.75 0.75 -2
    //     1 0.75 -2
    //     0.875 1 -2
    //     -0.875 1 -2
    // </vertexdata>
    // Access vertex data
    XMLElement *vertexElement = sceneElement->FirstChildElement("vertexdata");
    if (vertexElement)
    {
        std::string vertexText = vertexElement->GetText();
        std::istringstream iss(vertexText);
        scene->vertexData = std::vector<Vector3>();
        while (iss)
        {
            Vector3 vertex = Vector3();
            iss >> vertex.x >> vertex.y >> vertex.z;
            scene->vertexData.push_back(vertex);
        }

        // remove the last element
        scene->vertexData.pop_back();
    }

    // Access objects
    XMLElement *objectsElement = sceneElement->FirstChildElement("objects");
    if (objectsElement)

    {
        XMLElement *meshElement = objectsElement->FirstChildElement("mesh");
        scene->meshes = std::vector<Mesh>();
        while (meshElement)
        {
            Mesh mesh = Mesh();
            mesh.faces = std::vector<Vector3>();
            meshElement->QueryIntAttribute("id", &mesh.id);

            auto materialIdElement = meshElement->FirstChildElement("materialid");
            if (materialIdElement)
            {
                mesh.materialId = materialIdElement->IntText();
            }

            auto facesElement = meshElement->FirstChildElement("faces");
            if (facesElement)
            {
                std::string facesText = facesElement->GetText();
                std::istringstream iss(facesText);
                std::string line;
                while (std::getline(iss, line))
                {
                    // trim white spaces
                    line.erase(line.find_last_not_of(" \n\r\t") + 1);
                    if (line.empty())
                    {
                        continue;
                    }
                    std::istringstream lineIss(line);
                    Vector3 face = Vector3();
                    lineIss >> face.x >> face.y >> face.z;
                    mesh.faces.push_back(face);
                }
            }

            scene->meshes.push_back(mesh);
            meshElement = meshElement->NextSiblingElement("mesh");
        }
    }
}

Vector3 findIrradiance(const PointLight &pointLight, const Vector3 &intersectionPoint)
{
    Vector3 irradiance;
    Vector3 d = pointLight.position - intersectionPoint;
    float d_square = dot(d, d);

    if (d_square != 0.0)
    {
        irradiance.x = pointLight.intensity.x / d_square;
        irradiance.y = pointLight.intensity.y / d_square;
        irradiance.z = pointLight.intensity.z / d_square;
    }
    return irradiance;
}

const Vector3 findDiffuse(const PointLight &currentLight, const Scene &scene, int material_id, const Vector3 &normal, const Vector3 &intersectionPoint)
{
    Vector3 diffuse;

    Vector3 irradiance = findIrradiance(currentLight, intersectionPoint);

    Vector3 l = currentLight.position - intersectionPoint;
    l = l.normalize();

    float dotPro = dot(l, normal);
    if (dotPro < 0)
    {
        dotPro = 0;
    }

    diffuse.x = scene.materials[material_id - 1].diffuse.x * dotPro * irradiance.x;
    diffuse.y = scene.materials[material_id - 1].diffuse.y * dotPro * irradiance.y;
    diffuse.z = scene.materials[material_id - 1].diffuse.z * dotPro * irradiance.z;

    return diffuse;
}

Vector3 findSpecular(const PointLight &currentLight, const Scene &scene, const Ray &ray, int material_id, const Vector3 &normal, const Vector3 &intersectionPoint)
{
    Vector3 specular;

    Material material = scene.materials[material_id - 1];

    Vector3 irradiance = findIrradiance(currentLight, intersectionPoint);

    Vector3 wi = currentLight.position - intersectionPoint;
    wi = wi.normalize();

    Vector3 h = wi - ray.getDirection();
    h = h.normalize();

    float dotPro = dot(normal, h);
    if (dotPro < 0)
    {
        dotPro = 0;
    }

    specular.x = material.specular.x * pow(dotPro, material.phongExponent) * irradiance.x;
    specular.y = material.specular.y * pow(dotPro, material.phongExponent) * irradiance.y;
    specular.z = material.specular.z * pow(dotPro, material.phongExponent) * irradiance.z;

    return specular;
}

Vector3 findPixelColor(const Scene &scene, const Hit &hitResult, const Camera &currentCamera, const Ray &ray, int maxDepth)
{
    int numberOfLights = scene.pointLights.size();
    int numberOfMeshes = scene.meshes.size();

    float pixel1 = 0;
    float pixel2 = 0;
    float pixel3 = 0;

    Vector3 pixelColor;

    if (hitResult.hitHappened)
    {
        std::cout << "Hit happened" << std::endl;
        int material_id = hitResult.material_id;

        pixel1 = scene.materials[material_id - 1].ambient.x * scene.ambientLight.x;
        pixel2 = scene.materials[material_id - 1].ambient.y * scene.ambientLight.y;
        pixel3 = scene.materials[material_id - 1].ambient.z * scene.ambientLight.z;

        for (int lightNumber = 0; lightNumber < numberOfLights; lightNumber++)
        {
            PointLight currentLight = scene.pointLights[lightNumber];
            float lightToCam = findDistance(currentLight.position, currentCamera.position);

            if (lightToCam == 0)
            {
                int material_id = hitResult.material_id;

                Vector3 diffuse = findDiffuse(currentLight, scene, material_id, hitResult.surfaceNormal, hitResult.intersectionPoint);

                Vector3 specular = findSpecular(currentLight, scene, ray, material_id, hitResult.surfaceNormal, hitResult.intersectionPoint);

                pixel1 += diffuse.x + specular.x;
                pixel2 += diffuse.y + specular.y;
                pixel3 += diffuse.z + specular.z;

                std::cout << "pixel color: " << pixel1 << std::endl;
            }
        }
    }
    else // if hitHappened == 0
    {
        pixel1 = scene.backgroundColor.x;
        pixel2 = scene.backgroundColor.y;
        pixel3 = scene.backgroundColor.z;
    }

    pixelColor.x = pixel1;
    pixelColor.y = pixel2;
    pixelColor.z = pixel3;

    return pixelColor;
}

void render(Scene *scene)
{
    Camera camera = scene->camera;
    int width = camera.imageResolution.nx;
    int height = camera.imageResolution.ny;

    unsigned char *image = new unsigned char[width * height * 3];
    int pixelNumber = 0;

    std::cout << "Rendering has started" << std::endl;

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            Ray ray = calculateRay(camera, i, j);

            // std::cout << "ray origin: "
            // << ray.getOrigin().x << " "
            // << ray.getOrigin().y << " "
            // << ray.getOrigin().z << std::endl;

            // std::cout << "ray direction: "
            // << ray.getDirection().x << " "
            // << ray.getDirection().y << " "
            // << ray.getDirection().z << std::endl;

            Hit hitResult = intersectWithObject(*scene, ray);

            Color3 pixelColor = findPixelColor(*scene, hitResult, camera, ray, scene->maxRayTraceDepth);

            // Color3 pixelColor = ray_color(ray);

            image[pixelNumber] = round(pixelColor.x);
            image[pixelNumber + 1] = round(pixelColor.y);
            image[pixelNumber + 2] = round(pixelColor.z);

            if (image[pixelNumber] > 0)
            {
                std::cout << "pixel color: "
                          << image[pixelNumber] << " "
                          << image[pixelNumber + 1] << " "
                          << image[pixelNumber + 2] << std::endl;
            }

            if (image[pixelNumber + 1] > 0)
            {
                std::cout << "pixel color: "
                          << image[pixelNumber] << " "
                          << image[pixelNumber + 1] << " "
                          << image[pixelNumber + 2] << std::endl;
            }

            if (image[pixelNumber + 2] > 0)
            {
                std::cout << "pixel color: "
                          << image[pixelNumber] << " "
                          << image[pixelNumber + 1] << " "
                          << image[pixelNumber + 2] << std::endl;
            }

            pixelNumber += 3;
        }
    }

    write_ppm("output.ppm", image, width, height);
}

int main()
{
    Scene scene = Scene();
    auto fileName = "sample_scenes/bunny.xml";
    generateSceneFromXml(fileName, &scene);
    scene.camera.cameraSetup();
    // calculate run time
    auto start = std::chrono::high_resolution_clock::now();
    render(&scene);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << "s" << std::endl;
    // printData(scene);
    // createPpmImage(scene);

    return 0;
}