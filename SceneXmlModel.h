#include <vector>
#include <string>

class NearPlane {
public:
    double left;
    double right;
    double bottom;
    double top;
};

class Vector3 {
public:
    double x;
    double y;
    double z;
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
    std::vector<std::vector<int>> faces;
};

class Scene {
public:
    int maxRayTraceDepth;
    Vector3 backgroundColor;
    Camera camera;
    std::vector<PointLight> pointLights;
    std::vector<TriangularLight> triangularLights;
    Vector3 ambientLight;
    std::vector<Material> materials;
    std::vector<Vector3> vertexData;
    std::vector<Mesh> meshes;
};
