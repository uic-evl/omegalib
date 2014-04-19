#include <omegaGl.h>

#include "cube.h"

Cube::Cube(float size)
{
    // Initialize cube normals.
    myNormals[0] = Vector3s(-1, 0, 0);
    myNormals[1] = Vector3s(0, 1, 0);
    myNormals[2] = Vector3s(1, 0, 0);
    myNormals[3] = Vector3s(0, -1, 0);
    myNormals[4] = Vector3s(0, 0, 1);
    myNormals[5] = Vector3s(0, 0, -1);

    // Initialize cube face indices.
    myFaces[0] = Vector4i(0, 1, 2, 3);
    myFaces[1] = Vector4i(3, 2, 6, 7);
    myFaces[2] = Vector4i(7, 6, 5, 4);
    myFaces[3] = Vector4i(4, 5, 1, 0);
    myFaces[4] = Vector4i(5, 6, 2, 1);
    myFaces[5] = Vector4i(7, 4, 0, 3);

    // Initialize cube face colors.
    myFaceColors[0] = Color::Aqua;
    myFaceColors[1] = Color::Orange;
    myFaceColors[2] = Color::Olive;
    myFaceColors[3] = Color::Navy;
    myFaceColors[4] = Color::Red;
    myFaceColors[5] = Color::Yellow;

    // Setup cube vertex data
    myVertices[0][0] = myVertices[1][0] = myVertices[2][0] = myVertices[3][0] = -size;
    myVertices[4][0] = myVertices[5][0] = myVertices[6][0] = myVertices[7][0] = size;
    myVertices[0][1] = myVertices[1][1] = myVertices[4][1] = myVertices[5][1] = -size;
    myVertices[2][1] = myVertices[3][1] = myVertices[6][1] = myVertices[7][1] = size;
    myVertices[0][2] = myVertices[3][2] = myVertices[4][2] = myVertices[7][2] = size;
    myVertices[1][2] = myVertices[2][2] = myVertices[5][2] = myVertices[6][2] = -size;
}

void Cube::draw()
{
    // Draw a box
    for(int i = 0; i < 6; i++)
    {
        glBegin(GL_QUADS);
        glColor3fv(myFaceColors[i].data());
        glNormal3fv(myNormals[i].data());
        glVertex3fv(myVertices[myFaces[i][0]].data());
        glVertex3fv(myVertices[myFaces[i][1]].data());
        glVertex3fv(myVertices[myFaces[i][2]].data());
        glVertex3fv(myVertices[myFaces[i][3]].data());
        glEnd();
    }
}
