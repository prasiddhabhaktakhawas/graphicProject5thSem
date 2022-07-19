#include <graphics.h>
#include <math.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <strstream>
#include <list>
#include <string>
#include <vector>

using namespace std;

struct vec3d
{
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1;
};

struct triangle
{
    vec3d p[3];
};

struct mesh
{
    vector<triangle> tris;

    bool LoadFromObject(string sFilename)
    {
        ifstream f(sFilename);
        if (!f.is_open())
            return false;

        // Local cache of verts
        vector<vec3d> verts;

        while (!f.eof())
        {
            char line[128];
            f.getline(line, 128);

            strstream s;
            s << line;

            char junk;

            if (line[0] == 'v')
            {
                vec3d v;
                s >> junk >> v.x >> v.y >> v.z;
                verts.push_back(v);
            }

            if (line[0] == 'f')
            {
                int f[3];
                s >> junk >> f[0] >> f[1] >> f[2];
                tris.push_back({verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1]});
            }
        }

        return true;
    }
};

struct mat4x4
{
    float m[4][4] = {0};
};

class renderer
{
private:
    DWORD screenWidth;
    DWORD screenHeight;
    string objName;
    mat4x4 matProj, matRotX, matRotY;
    mesh meshObj;
    vec3d vLookDir;
    vec3d vCamera;
    POINT cursorPos;
    float fYaw = 0.0f;
    float fThetaX = 0.0f; // angle of rotation
    float fThetaY = 0.0f;
    float zoom = 0.0f;
    int page = 0;

    float prevXM; // previous x position of mouse
    float prevYM;
    float differenceX; // difference between current and previous x position of mouse
    float differenceY;
    bool leftButtonHold = false;

public:
    // constructor
    renderer(string objName, DWORD screenWidth, DWORD screenHeight, int xOffset, int yOffset)
    {
        initwindow(screenWidth, screenHeight, "", -3, -3);
        this->objName = objName;
        this->screenWidth = screenWidth;
        this->screenHeight = screenHeight;

        // Projection Matrix
        matProj = Matrix_MakeProjection(90.0f, (float)screenHeight / (float)screenWidth, 0.1f, 1000.0f);

        meshObj.LoadFromObject(objName);

        vCamera.x = 0.0f;
        vCamera.y = 0.0f;
        vCamera.z = 0.0f;

        // rotation y
        matRotY = Matrix_MakeRotationY(fYaw);
        // rotation x
        matRotX = Matrix_MakeRotationX(fYaw);
    }
    void mainLoop()
    {

        setactivepage(page); // double buffer method
        setvisualpage(1 - page);
        if ((GetAsyncKeyState(0x28) & 0x8000) != 0) // down key held
        {
            vCamera.y += 1.0f;
        }
        if ((GetAsyncKeyState(0x26) & 0x8000) != 0) // up key held
        {
            vCamera.y -= 1.0f;
        }
        if ((GetAsyncKeyState(0x44) & 0x8000) != 0) // d key held
        {
            vCamera.x += 1.0f;
        }
        if ((GetAsyncKeyState(0x41) & 0x8000) != 0) // a key held
        {
            vCamera.x -= 1.0f;
        }

        vec3d vForward = Vector_Mul(vLookDir, 1.0f);

        if ((GetAsyncKeyState(0x57) & 0x8000) != 0) // w key held
        {
            vCamera = Vector_Add(vCamera, vForward);
        }
        if ((GetAsyncKeyState(0x53) & 0x8000) != 0) // s key held
        {
            vCamera = Vector_Sub(vCamera, vForward);
        }

        if ((GetAsyncKeyState(0x27) & 0x8000) != 0) // right key held
        {
            fYaw -= 0.05f;
        }
        if ((GetAsyncKeyState(0x25) & 0x8000) != 0) // left key held
        {
            fYaw += 0.05f;
        }
        // if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0)
        // { // when left click is hold
        //     GetCursorPos(&cursorPos);
        //     if (leftButtonHold == false)
        //     {
        //         prevXM = cursorPos.x;
        //         prevYM = cursorPos.y;
        //         leftButtonHold = true;
        //     }
        //     differenceX = prevXM - cursorPos.x;
        //     differenceY = prevYM - cursorPos.y;
        //     fThetaX -= differenceY * 0.01; // dragging mouse in y direction give means rotating wrt to x axis and -ve for invertmouseY just like in videogames
        //     fThetaY -= differenceX * 0.01;
        //     prevXM = cursorPos.x;
        //     prevYM = cursorPos.y;

        //     // rotation y
        //     matRotY = Matrix_MakeRotationY(fThetaY);
        //     // rotation x
        //     matRotX = Matrix_MakeRotationX(fThetaX);
        // }
        // else
        // {
        //     leftButtonHold = false;
        // }
        // if ((GetAsyncKeyState(0x5A) & 0x8000) != 0)
        // { // z key pressed
        //     zoom--;
        // }
        // else if ((GetAsyncKeyState(0x58) & 0x8000) != 0)
        // { // x key pressed
        //     zoom++;
        // }

        cleardevice(); // when clear device is inside the key press loop, then multiple border ficklering occurs, so, cleardevice should be outside so its being cleared every time
        // draw triangles

        vector<triangle> vecTrianglesToRaster;

        mat4x4 matTrans;
        matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 5.0f);

        mat4x4 matWorld;
        matWorld = Matrix_MakeIdentity();                     // Form World Matrix
        matWorld = Matrix_MultiplyMatrix(matRotY, matRotX);   // Transform by rotation
        matWorld = Matrix_MultiplyMatrix(matWorld, matTrans); // Transform by translation

        // vec3d vLookDir = {0,0,1};
        vec3d vUp = {0, 1, 0};
        // vec3d vTarget = Vector_Add(vCamera, vLookDir);
        vec3d vTarget = {0, 0, 1};
        mat4x4 matCameraRot = Matrix_MakeRotationY(fYaw);
        vLookDir = Matrix_MultiplyVector(matCameraRot, vTarget);
        vTarget = Vector_Add(vCamera, vLookDir);

        mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

        // make view matrix from camera
        mat4x4 matView = Matrix_QuickInverse(matCamera);

        for (auto tri : meshObj.tris)
        {
            triangle triProjected, triTransformed, triViewed;
            vec3d normal, line1, line2;

            triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
            triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
            triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

            // Get lines either side of triangle
            line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
            line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

            // Take cross product of lines to get normal to triangle surface
            normal = Vector_CrossProduct(line1, line2);

            // You normally need to normalise a normal!
            normal = Vector_Normalise(normal);

            // Get Ray from triangle to camera
            vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

            // if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
            // {
            // Convert world space to view space
            triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
            triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
            triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);

            int nClippedTriangles = 0;
            triangle clipped[2];
            nClippedTriangles = Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f}, triViewed, clipped[0], clipped[1]);

            for (int n = 0; n < nClippedTriangles; n++)
            {
                // Project triangles from 3D --> 2D
                triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
                triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
                triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);

                // Scale into view, we moved the normalising into cartesian space
                // out of the matrix.vector function from the previous videos, so
                // do this manually
                triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
                triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
                triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

                vec3d vOffsetView = {1, 1, 0};
                triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
                triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
                triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);

                // scaling it wrt to screen size         //we scale half the total length to make the object appear on center
                triProjected.p[0].x *= 0.5f * (float)screenWidth; // scaling in x direction is bigger because width is bigger than height, but it's cancelled out by aspect ratio( h/w), so at the end, it is just perfect
                triProjected.p[0].y *= 0.5f * (float)screenHeight;
                triProjected.p[1].x *= 0.5f * (float)screenWidth;  //
                triProjected.p[1].y *= 0.5f * (float)screenHeight; //
                triProjected.p[2].x *= 0.5f * (float)screenWidth;
                triProjected.p[2].y *= 0.5f * (float)screenHeight;

                // drawTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y, triProjected.p[2].x, triProjected.p[2].y);

                vecTrianglesToRaster.push_back(triProjected);
                //}
            }
        }
        // sort triangles from back to front
        sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle &t1, triangle &t2)
             {
                float z1 = (t1.p[0].z +t1.p[1].z +t1.p[2].z)/3.0f;
                float z2 = (t2.p[0].z +t2.p[1].z +t2.p[2].z)/3.0f;
                return z1>z2; });

        for (auto &triToRaster : vecTrianglesToRaster)
        {
            // setfillstyle(SOLID_FILL,RED);
            // drawTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y, triProjected.p[2].x, triProjected.p[2].y);
            // float midX = (triProjected.p[0].x+triProjected.p[1].x+triProjected.p[2].x)/3.0f;
            // float midY = (triProjected.p[0].y+triProjected.p[1].y+triProjected.p[2].y)/3.0f;
            // floodfill(midX,midY,RED);
            triangle clipped[2];
            list<triangle> listTriangles;
            listTriangles.push_back(triToRaster);
            int nNewTriangles = 1;

            for (int p = 0; p < 4; p++)
            {
                int nTrisToAdd = 0;
                while (nNewTriangles > 0)
                {
                    // Take triangle from front of queue
                    triangle test = listTriangles.front();
                    listTriangles.pop_front();
                    nNewTriangles--;

                    // Clip it against a plane. We only need to test each
                    // subsequent plane, against subsequent new triangles
                    // as all triangles after a plane clip are guaranteed
                    // to lie on the inside of the plane. I like how this
                    // comment is almost completely and utterly justified
                    switch (p)
                    {
                    case 0:
                        nTrisToAdd = Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, test, clipped[0], clipped[1]);
                        break;
                    case 1:
                        nTrisToAdd = Triangle_ClipAgainstPlane({0.0f, (float)screenHeight - 1, 0.0f}, {0.0f, -1.0f, 0.0f}, test, clipped[0], clipped[1]);
                        break;
                    case 2:
                        nTrisToAdd = Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, test, clipped[0], clipped[1]);
                        break;
                    case 3:
                        nTrisToAdd = Triangle_ClipAgainstPlane({(float)screenWidth - 1, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, test, clipped[0], clipped[1]);
                        break;
                    }

                    // Clipping may yield a variable number of triangles, so
                    // add these new ones to the back of the queue for subsequent
                    // clipping against next planes
                    for (int w = 0; w < nTrisToAdd; w++)
                        listTriangles.push_back(clipped[w]);
                }
                nNewTriangles = listTriangles.size();
            }

            // Draw the transformed, viewed, clipped, projected, sorted, clipped triangles
            for (auto &t : listTriangles)
            {
                drawTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y);
            }
        }
        page = 1 - page; // double buffer method
    }

private:
    vec3d Vector_Add(vec3d &v1, vec3d &v2)
    {
        return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
    }

    vec3d Vector_Sub(vec3d &v1, vec3d &v2)
    {
        return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    }

    vec3d Vector_Mul(vec3d &v1, float k)
    {
        return {v1.x * k, v1.y * k, v1.z * k};
    }

    vec3d Vector_Div(vec3d &v1, float k)
    {
        return {v1.x / k, v1.y / k, v1.z / k};
    }

    float Vector_DotProduct(vec3d &v1, vec3d &v2)
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    float Vector_Length(vec3d &v)
    {
        return sqrtf(Vector_DotProduct(v, v));
    }

    vec3d Vector_Normalise(vec3d &v)
    {
        float l = Vector_Length(v);
        return {v.x / l, v.y / l, v.z / l};
    }

    vec3d Vector_CrossProduct(vec3d &v1, vec3d &v2)
    {
        vec3d v;
        v.x = v1.y * v2.z - v1.z * v2.y;
        v.y = v1.z * v2.x - v1.x * v2.z;
        v.z = v1.x * v2.y - v1.y * v2.x;
        return v;
    }

    vec3d Matrix_MultiplyVector(mat4x4 &m, vec3d &i)
    {
        vec3d v;
        v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
        v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
        v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
        v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
        return v;
    }

    void MultiplyMatrixVector(vec3d &i, vec3d &o, mat4x4 &m)
    { // pass by reference
        o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
        o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
        o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
        float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];
        if (w != 0.0f)
        {
            o.x /= w;
            o.y /= w;
            o.z /= w;
        }
    }

    mat4x4 Matrix_MakeIdentity()
    {
        mat4x4 matrix;
        matrix.m[0][0] = 1.0f;
        matrix.m[1][1] = 1.0f;
        matrix.m[2][2] = 1.0f;
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    mat4x4 Matrix_MakeRotationX(float fAngleRad)
    {
        mat4x4 matrix;
        matrix.m[0][0] = 1.0f;
        matrix.m[1][1] = cosf(fAngleRad);
        matrix.m[1][2] = sinf(fAngleRad);
        matrix.m[2][1] = -sinf(fAngleRad);
        matrix.m[2][2] = cosf(fAngleRad);
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    mat4x4 Matrix_MakeRotationY(float fAngleRad)
    {
        mat4x4 matrix;
        matrix.m[0][0] = cosf(fAngleRad);
        matrix.m[0][2] = sinf(fAngleRad);
        matrix.m[2][0] = -sinf(fAngleRad);
        matrix.m[1][1] = 1.0f;
        matrix.m[2][2] = cosf(fAngleRad);
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    mat4x4 Matrix_MakeRotationZ(float fAngleRad)
    {
        mat4x4 matrix;
        matrix.m[0][0] = cosf(fAngleRad);
        matrix.m[0][1] = sinf(fAngleRad);
        matrix.m[1][0] = -sinf(fAngleRad);
        matrix.m[1][1] = cosf(fAngleRad);
        matrix.m[2][2] = 1.0f;
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    mat4x4 Matrix_MakeTranslation(float x, float y, float z)
    {
        mat4x4 matrix;
        matrix.m[0][0] = 1.0f;
        matrix.m[1][1] = 1.0f;
        matrix.m[2][2] = 1.0f;
        matrix.m[3][3] = 1.0f;
        matrix.m[3][0] = x;
        matrix.m[3][1] = y;
        matrix.m[3][2] = z;
        return matrix;
    }

    mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
    {
        float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
        mat4x4 matrix;
        matrix.m[0][0] = fAspectRatio * fFovRad;
        matrix.m[1][1] = fFovRad;
        matrix.m[2][2] = fFar / (fFar - fNear);
        matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
        matrix.m[2][3] = 1.0f;
        matrix.m[3][3] = 0.0f;
        return matrix;
    }

    mat4x4 Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2)
    {
        mat4x4 matrix;
        for (int c = 0; c < 4; c++)
            for (int r = 0; r < 4; r++)
                matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
        return matrix;
    }

    mat4x4 Matrix_PointAt(vec3d &pos, vec3d &target, vec3d &up)
    {
        // calculate new forward direction
        vec3d newForward = Vector_Sub(target, pos);
        newForward = Vector_Normalise(newForward);

        // calculate new up direction
        vec3d a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
        vec3d newUp = Vector_Sub(up, a);
        newUp = Vector_Normalise(newUp);

        // new right direction
        vec3d newRight = Vector_CrossProduct(newUp, newForward);

        // construct dimensioning and translation matrix
        mat4x4 matrix;
        matrix.m[0][0] = newRight.x;
        matrix.m[0][1] = newRight.y;
        matrix.m[0][2] = newRight.z;
        matrix.m[0][3] = 0.0f;
        matrix.m[1][0] = newUp.x;
        matrix.m[1][1] = newUp.y;
        matrix.m[1][2] = newUp.z;
        matrix.m[1][3] = 0.0f;
        matrix.m[2][0] = newForward.x;
        matrix.m[2][1] = newForward.y;
        matrix.m[2][2] = newForward.z;
        matrix.m[2][3] = 0.0f;
        matrix.m[3][0] = pos.x;
        matrix.m[3][1] = pos.y;
        matrix.m[3][2] = pos.z;
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    mat4x4 Matrix_QuickInverse(mat4x4 &m) // Only for Rotation/Translation Matrices
    {
        mat4x4 matrix;
        matrix.m[0][0] = m.m[0][0];
        matrix.m[0][1] = m.m[1][0];
        matrix.m[0][2] = m.m[2][0];
        matrix.m[0][3] = 0.0f;
        matrix.m[1][0] = m.m[0][1];
        matrix.m[1][1] = m.m[1][1];
        matrix.m[1][2] = m.m[2][1];
        matrix.m[1][3] = 0.0f;
        matrix.m[2][0] = m.m[0][2];
        matrix.m[2][1] = m.m[1][2];
        matrix.m[2][2] = m.m[2][2];
        matrix.m[2][3] = 0.0f;
        matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
        matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
        matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
        matrix.m[3][3] = 1.0f;
        return matrix;
    }
    vec3d Vector_IntersectPlane(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd)
    {
        plane_n = Vector_Normalise(plane_n);
        float plane_d = -Vector_DotProduct(plane_n, plane_p);
        float ad = Vector_DotProduct(lineStart, plane_n);
        float bd = Vector_DotProduct(lineEnd, plane_n);
        float t = (-plane_d - ad) / (bd - ad);
        vec3d lineStartToEnd = Vector_Sub(lineEnd, lineStart);
        vec3d lineToIntersect = Vector_Mul(lineStartToEnd, t);
        return Vector_Add(lineStart, lineToIntersect);
    }

    int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2)
    {
        // Make sure plane normal is indeed normal
        plane_n = Vector_Normalise(plane_n);

        // Return signed shortest distance from point to plane, plane normal must be normalised
        auto dist = [&](vec3d &p)
        {
            vec3d n = Vector_Normalise(p);
            return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vector_DotProduct(plane_n, plane_p));
        };

        // Create two temporary storage arrays to classify points either side of plane
        // If distance sign is positive, point lies on "inside" of plane
        vec3d *inside_points[3];
        int nInsidePointCount = 0;
        vec3d *outside_points[3];
        int nOutsidePointCount = 0;

        // Get signed distance of each point in triangle to plane
        float d0 = dist(in_tri.p[0]);
        float d1 = dist(in_tri.p[1]);
        float d2 = dist(in_tri.p[2]);

        if (d0 >= 0)
        {
            inside_points[nInsidePointCount++] = &in_tri.p[0];
        }
        else
        {
            outside_points[nOutsidePointCount++] = &in_tri.p[0];
        }
        if (d1 >= 0)
        {
            inside_points[nInsidePointCount++] = &in_tri.p[1];
        }
        else
        {
            outside_points[nOutsidePointCount++] = &in_tri.p[1];
        }
        if (d2 >= 0)
        {
            inside_points[nInsidePointCount++] = &in_tri.p[2];
        }
        else
        {
            outside_points[nOutsidePointCount++] = &in_tri.p[2];
        }

        // Now classify triangle points, and break the input triangle into
        // smaller output triangles if required. There are four possible
        // outcomes...

        if (nInsidePointCount == 0)
        {
            // All points lie on the outside of plane, so clip whole triangle
            // It ceases to exist

            return 0; // No returned triangles are valid
        }

        if (nInsidePointCount == 3)
        {
            // All points lie on the inside of plane, so do nothing
            // and allow the triangle to simply pass through
            out_tri1 = in_tri;

            return 1; // Just the one returned original triangle is valid
        }

        if (nInsidePointCount == 1 && nOutsidePointCount == 2)
        {
            // Triangle should be clipped. As two points lie outside
            // the plane, the triangle simply becomes a smaller triangle

            // Copy appearance info to new triangle
            // out_tri1.col =  in_tri.col;
            // out_tri1.sym = in_tri.sym;

            // The inside point is valid, so keep that...
            out_tri1.p[0] = *inside_points[0];

            // but the two new points are at the locations where the
            // original sides of the triangle (lines) intersect with the plane
            out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
            out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

            return 1; // Return the newly formed single triangle
        }

        if (nInsidePointCount == 2 && nOutsidePointCount == 1)
        {
            // Triangle should be clipped. As two points lie inside the plane,
            // the clipped triangle becomes a "quad". Fortunately, we can
            // represent a quad with two new triangles

            // Copy appearance info to new triangles
            // out_tri1.col =  in_tri.col;
            // out_tri1.sym = in_tri.sym;

            // out_tri2.col =  in_tri.col;
            // out_tri2.sym = in_tri.sym;

            // The first triangle consists of the two inside points and a new
            // point determined by the location where one side of the triangle
            // intersects with the plane
            out_tri1.p[0] = *inside_points[0];
            out_tri1.p[1] = *inside_points[1];
            out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

            // The second triangle is composed of one of he inside points, a
            // new point determined by the intersection of the other side of the
            // triangle and the plane, and the newly created point above
            out_tri2.p[0] = *inside_points[1];
            out_tri2.p[1] = out_tri1.p[2];
            out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

            return 2; // Return two newly formed triangles which form a quad
        }
    }

    void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3)
    {
        // drawLine(x1,y1,x2,y2,WHITE);
        // drawLine(x2,y2,x3,y3,WHITE);
        // drawLine(x3,y3,x1,y1,WHITE);
        line(x1, y1, x2, y2); // inbuilt line of graphics.h seems to perform better than custom built
        line(x2, y2, x3, y3);
        line(x3, y3, x1, y1);
    }
};

int main()
{
    DWORD screenWidth = GetSystemMetrics(SM_CXSCREEN);
    DWORD screenHeight = GetSystemMetrics(SM_CYSCREEN);
    renderer car("houseLowPoly.obj", screenWidth, screenHeight, -3, -3);

    while (1)
    {
        car.mainLoop();
    }
    return 0;
}