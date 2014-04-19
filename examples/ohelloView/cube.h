#include <omega.h>

using namespace omega;

class Cube: public ReferenceType
{
public:
    Cube(float size);
    void draw();

private:
    Vector3s myNormals[6];
    Vector4i myFaces[6];
    Vector3s myVertices[8];
    Color myFaceColors[6];
};