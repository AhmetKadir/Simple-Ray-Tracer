#include "tinyxml2.h"
#include <iostream>
#include <string>
#include <sstream>
#include "SceneXmlModel.h"
#include <memory>

using namespace tinyxml2;

void generateSceneFromXml(std::string, Scene *scene);

void printData(Scene &scene);

int main()
{

    Scene scene = Scene();
    generateSceneFromXml("scene.xml", &scene);
    // printData(scene);

    

    return 0;
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
            std::cout << "face " << i++ << ": ";
            std::cout << face.x << " " << face.y << " " << face.z << std::endl;
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