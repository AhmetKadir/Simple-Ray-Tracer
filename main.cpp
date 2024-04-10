#include "tinyxml2.h"
#include <iostream>
#include <string>
#include <sstream>
#include "SceneXmlModel.h"

using namespace tinyxml2;

void readXml(std::string);

int main() {

    Scene scene;

    readXml("scene.xml", scene);

    return 0;
}

void readXml(std::string, Scene &scene) {
    std::cout << "Hello, World!" << std::endl;
    XMLDocument doc;
    doc.LoadFile("scene.xml");

    if (doc.Error()) {
        std::cerr << "Error loading XML file: " << doc.ErrorStr() << std::endl;
        return;
    }

    // Accessing scene properties
    XMLElement* sceneElement = doc.FirstChildElement("scene");

    if (sceneElement) {
        scene.maxRayTraceDepth = sceneElement->FirstChildElement("maxraytracedepth")->IntText();

        auto bgElement = sceneElement->FirstChildElement("backgroundColor");
        if (bgElement) {
        const char* bgText = bgElement->GetText();
        if (bgText) {
            std::istringstream iss(bgText);
            iss >> scene.backgroundColor.x >> scene.backgroundColor.y >> scene.backgroundColor.z;
        }
    }
    }

    // Accessing camera properties
    XMLElement* cameraElement = sceneElement->FirstChildElement("camera");
    // Access camera properties and attributes similarly
    //     <camera>
    //     <position>0 0 0</position>
    //     <gaze>0 0 -1</gaze>
    //     <up>0 1 0</up>
    //     <nearPlane>-1 1 -1 1</nearPlane>
    //     <neardistance>1</neardistance>
    //     <imageresolution>800 800</imageresolution>
    // </camera>
    Camera *camera = new Camera();
    if (cameraElement) {
        auto positionElement = cameraElement->FirstChildElement("position");
        if (positionElement) {
            const char* positionText = positionElement->GetText();
            if (positionText) {
                std::istringstream iss(positionText);
                iss >> camera->position.x >> camera->position.y >> camera->position.z;
            }
        }

        auto gazeElement = cameraElement->FirstChildElement("gaze");
        if (gazeElement) {
            const char* gazeText = gazeElement->GetText();
            if (gazeText) {
                std::istringstream iss(gazeText);
                iss >> camera->gaze.x >> camera->gaze.y >> camera->gaze.z;
            }
        }

        auto upElement = cameraElement->FirstChildElement("up");
        if (upElement) {
            const char* upText = upElement->GetText();
            if (upText) {
                std::istringstream iss(upText);
                iss >> camera->up.x >> camera->up.y >> camera->up.z;
            }
        }

        auto nearPlaneElement = cameraElement->FirstChildElement("nearPlane");
        if (nearPlaneElement) {
            const char* nearPlaneText = nearPlaneElement->GetText();
            if (nearPlaneText) {
                std::istringstream iss(nearPlaneText);
                iss >> camera->nearPlane[0] >> camera->nearPlane[1] >> camera->nearPlane[2] >> camera->nearPlane[3];
            }
        }

        auto nearDistanceElement = cameraElement->FirstChildElement("nearDistance");
        if (nearDistanceElement) {
            camera->nearDistance = nearDistanceElement->DoubleText();
        }

        auto imageResolutionElement = cameraElement->FirstChildElement("imageResolution");
        if (imageResolutionElement) {
            const char* imageResolutionText = imageResolutionElement->GetText();
            if (imageResolutionText) {
                std::istringstream iss(imageResolutionText);
                iss >> camera->imageResolution[0] >> camera->imageResolution[1];
            }
        }
    }
    

    // Accessing lights
    XMLElement* lightsElement = sceneElement->FirstChildElement("lights");
    // Access light properties and attributes similarly

    // Accessing materials
    XMLElement* materialsElement = sceneElement->FirstChildElement("materials");
    // Access material properties and attributes similarly

    // Accessing vertex data
    XMLElement* vertexDataElement = sceneElement->FirstChildElement("vertexdata");
    // Access vertex data similarly

    // Accessing objects
    XMLElement* objectsElement = sceneElement->FirstChildElement("objects");
    // Access object properties and attributes similarly

    // Example: Iterating over vertices
    if (vertexDataElement) {
        XMLElement* vertexElement = vertexDataElement->FirstChildElement();
        while (vertexElement) {
            double x, y, z;
            vertexElement->QueryDoubleAttribute("x", &x);
            vertexElement->QueryDoubleAttribute("y", &y);
            vertexElement->QueryDoubleAttribute("z", &z);
            // Process vertex data here
            std::cout << "Vertex: (" << x << ", " << y << ", " << z << ")" << std::endl;
            vertexElement = vertexElement->NextSiblingElement();
        }
    }
}
