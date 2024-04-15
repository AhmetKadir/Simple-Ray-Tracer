#ifndef SCENEXMLMODEL_H
#define SCENEXMLMODEL_H

#include <vector>
#include <string>
#include "Vector3.h"

class NearPlane {
public:
    double left;
    double right;
    double bottom;
    double top;
};

class ImageResolution {
public:
    double nx;
    double ny;
};

class Camera {
public:
    Vector3 position;
    Vector3 gaze;
    Vector3 up;
    NearPlane nearPlane;
    double nearDistance;
    ImageResolution imageResolution;

    // top left corner of the near plane
    Vector3 q;
    // u and v are the basis vectors of the near plane
    Vector3 v;
    Vector3 u;
    int width;
    int height;

    void cameraSetup(){
        width = imageResolution.nx;
        height = imageResolution.ny;

        // m = e + (gaze* distance)
        Vector3 m = position + gaze * nearDistance;
        Vector3 w = -gaze;

        // q = m + (left * u) + (top * v)
        v = up;
        u = cross(v, w);
        q = m + nearPlane.left * u + nearPlane.top * v;
    }
};

class PointLight {
public:
    int id;
    Vector3 position;
    Vector3 intensity;
};

class TriangularLight {
public:
    int id;
    Vector3 vertex1;
    Vector3 vertex2;
    Vector3 vertex3;
    Vector3 intensity;
};

class Material {
public:
    int id;
    Vector3 ambient;
    Vector3 diffuse;
    Vector3 specular;
    Vector3 mirrorReflectance;
    int phongExponent;
};

class Mesh {
public:
    int id;
    int materialId;
    std::vector<Vector3> faces;
};

class Scene {
public:
    int maxRayTraceDepth;
    Color3 backgroundColor;
    Camera camera;
    std::vector<PointLight> pointLights;
    std::vector<TriangularLight> triangularLights;
    Vector3 ambientLight;
    std::vector<Material> materials;
    std::vector<Vector3> vertexData;
    std::vector<Mesh> meshes;
};

#endif // SCENEXMLMODEL_H
