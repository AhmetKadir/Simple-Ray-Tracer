#include "tinyxml2.h"
#include <iostream>
#include <string>
#include <sstream>
#include "SceneXmlModel.h"
#include <memory>
#include "ppm.h"
#include "Ray.h"
#include <chrono>
#include <thread>

using namespace tinyxml2;

typedef struct Hit
{
    bool isHit;
    Vector3 surfaceNormal;
    int materialId;
    float t;
    Vector3 pointIntersects;
    int objectId;
} hit;

float findDistance(const Vector3 &a, const Vector3 &b)
{
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

void cameraSetup(Camera &camera)
{
    int width = camera.imageResolution.nx;
    int height = camera.imageResolution.ny;

    // m = e + (gaze* distance)
    Vector3 m = camera.position + (camera.gaze * camera.nearDistance);
    Vector3 w = -1 * camera.gaze;

    // q = m + (left * u) + (top * v)
    camera.v = camera.up;
    camera.u = cross(camera.v, w);
    camera.q = m + camera.nearPlane.left * camera.u + camera.nearPlane.top * camera.v;
}

Ray calculateRay(const Camera &camera, int i, int j)
{
    // S = q + SuU - SvV
    float Su = (camera.nearPlane.right - camera.nearPlane.left) * (i + 0.5) / camera.imageResolution.nx;
    float Sv = (camera.nearPlane.top - camera.nearPlane.bottom) * (j + 0.5) / camera.imageResolution.ny;
    Vector3 SuU = Su * camera.u;
    Vector3 SvV = Sv * camera.v;

    Vector3 S = camera.q + SuU - SvV;

    // r(t) = e + (s-e)t
    Vector3 direction = S - camera.position;
    direction = direction.normalize();

    Ray ray = Ray(camera.position, direction);
    return ray;
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

Hit triangleIntersection(const Ray &ray, const Vector3 &a, const Vector3 &b, const Vector3 &c, int materialId, int objectId)
{
    // a, b, c are vertices of the triangle
    // determine if the ray intersects with the triangle using baricentric coordinates
    Hit hit;
    hit.isHit = false;

    // edge vectors
    Vector3 e1 = b - a;
    Vector3 e2 = c - a;

    Vector3 h = cross(ray.getDirection(), e2);
    float a_ = dot(e1, h);

    if (a_ > -0.00001 && a_ < 0.00001)
    {
        return hit;
    }

    float f = 1.0 / a_;
    Vector3 s = ray.getOrigin() - a;
    float u = f * dot(s, h);

    if (u < 0.0 || u > 1.0)
    {
        return hit;
    }
    Vector3 q = cross(s, e1);
    float v = f * dot(ray.getDirection(), q);

    if (v < 0.0 || u + v > 1.0)
    {
        return hit;
    }

    float t = f * dot(e2, q);

    if (t > 0.00001)
    {
        hit.isHit = true;
        hit.t = t;
        hit.pointIntersects = findIntersectionPoint(ray, t);
        hit.surfaceNormal = cross(e1, e2);
        hit.materialId = materialId;
        hit.objectId = objectId;
    }

    return hit;
}

Hit intersectWithObject(const Scene &scene, const Ray &ray)
{
    Mesh mesh;
    int numberOfMeshes = scene.meshes.size();

    std::vector<Hit> hitV;
    for (int i = 0; i < numberOfMeshes; i++)
    {
        mesh = scene.meshes[i];

        // search every triangle
        for (int j = 0; j < mesh.faces.size(); j++)
        {
            Vector3 vertex1 = scene.vertexData[mesh.faces[j].x - 1];
            Vector3 vertex2 = scene.vertexData[mesh.faces[j].y - 1];
            Vector3 vertex3 = scene.vertexData[mesh.faces[j].z - 1];

            Hit hit = triangleIntersection(ray, vertex1, vertex2, vertex3, mesh.materialId, mesh.id);
            if (hit.isHit && hit.t >= 0)
            {
                hitV.push_back(hit);
            }
        }
    }

    Hit closestHit;
    closestHit.isHit = false;

    if (hitV.size() != 0)
    {
        closestHit = hitV[0];
        for (int i = 1; i < hitV.size(); i++)
        {
            if (hitV[i].t < closestHit.t)
            {
                closestHit = hitV[i];
            }
        }
        closestHit.isHit = true;
    }

    return closestHit;
}

void debugScene(Scene &scene)
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

Vector3 findPixelColor(const Scene &scene, const Hit &hitResult, const Camera &currentCamera, const Ray &ray, int maxDepth)
{
    float pixelX = 0;
    float pixelY = 0;
    float pixelZ = 0;

    Vector3 pixelColor;

    if (hitResult.isHit)
    {
        int materialId = hitResult.materialId;

        // ambient light
        // I = ka * Ia
        // Ia = ambient light
        // ka = ambient coef of the material
        pixelX = scene.materials[materialId - 1].ambient.x * scene.ambientLight.x;
        pixelY = scene.materials[materialId - 1].ambient.y * scene.ambientLight.y;
        pixelZ = scene.materials[materialId - 1].ambient.z * scene.ambientLight.z;

    }
    
    // if no hit
    else 
    {
        pixelX = scene.backgroundColor.x;
        pixelY = scene.backgroundColor.y;
        pixelZ = scene.backgroundColor.z;
    }

    pixelColor.x = pixelX;
    pixelColor.y = pixelY;
    pixelColor.z = pixelZ;

    return pixelColor;
}

void render(Scene *scene, int start, int end, unsigned char *image)
{
    Camera camera = scene->camera;
    int width = camera.imageResolution.nx;

    for (int j = start; j < end; j++)
    {
        for (int i = 0; i < width; i++)
        {
            Ray ray = calculateRay(camera, i, j);

            Hit hit = intersectWithObject(*scene, ray);

            Color3 pixelColor = findPixelColor(*scene, hit, camera, ray, scene->maxRayTraceDepth);

            int pixelNumber = ((j * width) + i) * 3;
            image[pixelNumber] = round(pixelColor.x);
            image[pixelNumber + 1] = round(pixelColor.y);
            image[pixelNumber + 2] = round(pixelColor.z);
        }
    }
}

int main(int argc, char *argv[])
{
    Scene scene = Scene();

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <xml file>" << std::endl;
        return 1;
    }

    auto fileName = std::string(argv[1]);

    if (fileName.empty())
    {
        std::cerr << "File name is empty" << std::endl;
        return 1;
    }

    generateSceneFromXml(fileName, &scene);

    // precalculate some values for the camera
    cameraSetup(scene.camera);

    auto startTime = std::chrono::high_resolution_clock::now();

    std::cout << std::endl << "Rendering has started" << std::endl << std::endl;

    int numThreads = std::thread::hardware_concurrency(); // Get the number of hardware threads
    std::vector<std::thread> threads;

    int height = scene.camera.imageResolution.ny;
    int width = scene.camera.imageResolution.nx;
    unsigned char *image = new unsigned char[width * height * 3];

    int rowsPerThread = height / numThreads;
    int start = 0;
    int end = rowsPerThread;

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back(render, &scene, start, end, image);
        start = end;
        end = (t == numThreads - 2) ? height : std::min(end + rowsPerThread, height);
    }

    // wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

    write_ppm("output.ppm", image, width, height);

    auto endTime = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = endTime - startTime;
    std::cout << std::endl << "Elapsed time: " << elapsed.count() << "s" << std::endl;

    // comment out the following line to see the scene data
    // debugScene(scene);

    return 0;
}