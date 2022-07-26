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
#include <chrono>

using namespace std;

struct vec2d
{
    float u = 0;
    float v = 0;
    float w = 1;
};

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
    vec2d t[3];

    short col;
};

struct mesh
{
    vector<triangle> tris;

    bool LoadFromObjectFile(string sFilename, bool bHasTexture = false)
    {
        ifstream f(sFilename);
        if (!f.is_open())
            return false;

        // Local cache of verts
        vector<vec3d> verts;
        vector<vec2d> texs;

        while (!f.eof())
        {
            char line[128];
            f.getline(line, 128);

            strstream s;
            s << line;

            char junk;

            if (line[0] == 'v')
            {
                if (line[1] == 't')
                {
                    vec2d v;
                    s >> junk >> junk >> v.u >> v.v;
                    // A little hack for the spyro texture
                    // v.u = 1.0f - v.u;
                    // v.v = 1.0f - v.v;
                    texs.push_back(v);
                }
                else
                {
                    vec3d v;
                    s >> junk >> v.x >> v.y >> v.z;
                    verts.push_back(v);
                }
            }

            if (!bHasTexture)
            {
                if (line[0] == 'f')
                {
                    int f[3];
                    s >> junk >> f[0] >> f[1] >> f[2];
                    tris.push_back({verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1]});
                }
            }
            else
            {
                if (line[0] == 'f')
                {
                    s >> junk;

                    string tokens[6];
                    int nTokenCount = -1;

                    while (!s.eof())
                    {
                        char c = s.get();
                        if (c == ' ' || c == '/')
                            nTokenCount++;
                        else
                            tokens[nTokenCount].append(1, c);
                    }

                    tokens[nTokenCount].pop_back();

                    tris.push_back({verts[stoi(tokens[0]) - 1], verts[stoi(tokens[2]) - 1], verts[stoi(tokens[4]) - 1],
                                    texs[stoi(tokens[1]) - 1], texs[stoi(tokens[3]) - 1], texs[stoi(tokens[5]) - 1]});
                }
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
    mesh meshObj2;
    vec3d vLookDir;
    vec3d vCamera;
    POINT cursorPos;
    float fYaw = 0.0f;
    float fZaw = 0.0f;
    float fXaw = 0.0f;
    float fTheta;
    float fThetaX = 0.0f; // angle of rotation
    float fThetaY = 0.0f;
    float zoom = 0.0f;
    int page = 0;
    float *pDepthBuffer = nullptr;
    mat4x4 matCameraRot;
    std::chrono::time_point<std::chrono::system_clock> m_tp1, m_tp2;

    float prevXM; // previous x position of mouse
    float prevYM;
    float differenceX; // difference between current and previous x position of mouse
    float differenceY;
    bool leftButtonHold = false;

public:
    // constructor
    renderer(string objName, string objName2, DWORD screenWidth, DWORD screenHeight, int xOffset, int yOffset)
    {
        initwindow(screenWidth, screenHeight, "", -3, -3);
        this->objName = objName;
        this->screenWidth = screenWidth;
        this->screenHeight = screenHeight;

        // Projection Matrix
        matProj = Matrix_MakeProjection(90.0f, (float)screenHeight / (float)screenWidth, 0.1f, 1000.0f);

        meshObj.LoadFromObjectFile(objName, true);
        meshObj2.LoadFromObjectFile(objName2, true);

        pDepthBuffer = new float[screenWidth * screenHeight];
        // meshObj.tris = {

        // // SOUTH
        // { 0.0f, 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
        // { 0.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},

        // // EAST
        // { 1.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
        // { 1.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},

        // // NORTH
        // { 1.0f, 0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
        // { 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},

        // // WEST
        // { 0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
        // { 0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},

        // // TOP
        // { 0.0f, 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
        // { 0.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},

        // // BOTTOM
        // { 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
        // { 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},

        // };
        vCamera.x = 0.0f;
        vCamera.y = 0.0f;
        vCamera.z = 0.0f;

        // without this, nothing will appear on screen at first
        //matCameraRot = Matrix_MakeRotationY(fYaw);
        // rotation y
        matRotY = Matrix_MakeRotationY(fYaw);
        // rotation x
        matRotX = Matrix_MakeRotationX(fXaw);
        m_tp1 = std::chrono::system_clock::now();
        m_tp2 = std::chrono::system_clock::now();
    }
    void mainLoop()
    {
        m_tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> fElapsedTime = m_tp2 - m_tp1;
        m_tp1 = m_tp2;
        // std::cout<<elapsedTime.count()<<std::endl;
        float elapsedTime = fElapsedTime.count();
        setactivepage(page); // double buffer method
        setvisualpage(1 - page);
        if ((GetAsyncKeyState(0x26) & 0x8000) != 0) // up key held
        {
            vCamera.y += 8.0f * elapsedTime;
        }
        if ((GetAsyncKeyState(0x28) & 0x8000) != 0) // down key held
        {
            vCamera.y -= 8.0f * elapsedTime;
        }
        // if ((GetAsyncKeyState(0x28) & 0x8000) != 0) // down key held
        // {
        //     fXaw -= 0.05f;

        // }
        // if ((GetAsyncKeyState(0x26) & 0x8000) != 0) // up key held
        // {
        //     fXaw += 0.05f;

        // }
        if ((GetAsyncKeyState(0x41) & 0x8000) != 0) // a key held
        {
            vCamera.x += 8.0f * elapsedTime;
        }
        if ((GetAsyncKeyState(0x44) & 0x8000) != 0) // d key held
        {
            vCamera.x -= 8.0f * elapsedTime;
        }

        vec3d vForward = Vector_Mul(vLookDir, 8.0f * elapsedTime);

        if ((GetAsyncKeyState(0x57) & 0x8000) != 0) // w key held
        {
            vCamera = Vector_Add(vCamera, vForward);
        }
        if ((GetAsyncKeyState(0x53) & 0x8000) != 0) // s key held
        {
            vCamera = Vector_Sub(vCamera, vForward);
        }

        if ((GetAsyncKeyState(0x25) & 0x8000) != 0) // left key held
        {
            fYaw -= 2.0f * elapsedTime;
        }
        if ((GetAsyncKeyState(0x27) & 0x8000) != 0) // right key held
        {
            fYaw += 2.0f * elapsedTime;
        }
        if (GetAsyncKeyState(0x20)  != 0) // space key held
        {
            mesh tempObj = meshObj;
            meshObj = meshObj2;
            meshObj2 = tempObj; 
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
        for (int i = 0; i < screenWidth * screenHeight; i++)
        {
            pDepthBuffer[i] = 0.0f;
        }
        // draw triangles

        vector<triangle> vecTrianglesToRaster;

        // mat4x4 matRotZ, matRotX;
		// fTheta += 1.0f *  elapsedTime; // Uncomment to spin me right round baby right round
		// matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
		// matRotX = Matrix_MakeRotationX(fTheta);

        // maybe for initial condition
        mat4x4 matTrans;
        matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 5.0f);

        mat4x4 matWorld;
        matWorld = Matrix_MakeIdentity();                     // Form World Matrix
        matWorld = Matrix_MultiplyMatrix(matRotY, matRotX);   // Transform by rotation
        //matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
        matWorld = Matrix_MultiplyMatrix(matWorld, matTrans); // Transform by translation
        // maybe for initial condition //ends

        // vec3d vLookDir = {0,0,1};

        // vec3d vTarget = Vector_Add(vCamera, vLookDir);
        vec3d vUp = {0, 1, 0};
        vec3d vTarget = {0, 0, 1};

        // matCameraRot = Matrix_Adder(Matrix_MakeRotationY(fYaw), Matrix_MakeRotationX(fXaw));
        matCameraRot = Matrix_MakeRotationY(fYaw);
        vLookDir = Matrix_MultiplyVector(matCameraRot, vTarget);
        vTarget = Vector_Add(vCamera, vLookDir);

        // matCameraRot = Matrix_MakeRotationX(fXaw);
        // vLookDir = Matrix_MultiplyVector(matCameraRot, vTarget);
        // vTarget = Vector_Add(vCamera, vLookDir);

        mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

        // make view matrix from camera
        mat4x4 matView = Matrix_QuickInverse(matCamera);

        for (auto tri : meshObj.tris)
        {
            triangle triProjected, triTransformed, triViewed;
            vec3d normal, line1, line2;

            // world matrix transform
            triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
            triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
            triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);
            triTransformed.t[0] = tri.t[0];
            triTransformed.t[1] = tri.t[1];
            triTransformed.t[2] = tri.t[2];

            // Get lines either side of triangle
            line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
            line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

            // Take cross product of lines to get normal to triangle surface
            normal = Vector_CrossProduct(line1, line2);

            // You normally need to normalise a normal!
            normal = Vector_Normalise(normal);

            // Get Ray from triangle to camera
            vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

            if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
            {
                // Convert world space to view space
                // Illumination
                vec3d light_direction = {1.0f, 1.0f, -1.0f};
                light_direction = Vector_Normalise(light_direction);

                // How "aligned" are light direction and triangle surface normal?
                float dp = max(0.1f, Vector_DotProduct(light_direction, normal));

                // Choose console colours as required (much easier with RGB)
                int c = GetColour(dp);
                triTransformed.col = c;

                triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
                triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
                triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);

                triViewed.col = triTransformed.col;
                triViewed.t[0] = triTransformed.t[0];
                triViewed.t[1] = triTransformed.t[1];
                triViewed.t[2] = triTransformed.t[2];

                int nClippedTriangles = 0;
                triangle clipped[2];
                nClippedTriangles = Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f}, triViewed, clipped[0], clipped[1]);

                for (int n = 0; n < nClippedTriangles; n++)
                {
                    // Project triangles from 3D --> 2D
                    triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
                    triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
                    triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);
                    triProjected.col = clipped[n].col;

                    triProjected.t[0] = clipped[n].t[0];
                    triProjected.t[1] = clipped[n].t[1];
                    triProjected.t[2] = clipped[n].t[2];

                    triProjected.t[0].u = triProjected.t[0].u / triProjected.p[0].w;
                    triProjected.t[1].u = triProjected.t[1].u / triProjected.p[1].w;
                    triProjected.t[2].u = triProjected.t[2].u / triProjected.p[2].w;

                    triProjected.t[0].v = triProjected.t[0].v / triProjected.p[0].w;
                    triProjected.t[1].v = triProjected.t[1].v / triProjected.p[1].w;
                    triProjected.t[2].v = triProjected.t[2].v / triProjected.p[2].w;

                    triProjected.t[0].w = 1.0f / triProjected.p[0].w;
                    triProjected.t[1].w = 1.0f / triProjected.p[1].w;
                    triProjected.t[2].w = 1.0f / triProjected.p[2].w;

                    // Scale into view, we moved the normalising into cartesian space
                    // out of the matrix.vector function from the previous videos, so
                    // do this manually
                    triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
                    triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
                    triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

                    // X/Y are inverted so put them back
                    triProjected.p[0].x *= -1.0f;
                    triProjected.p[1].x *= -1.0f;
                    triProjected.p[2].x *= -1.0f;
                    triProjected.p[0].y *= -1.0f;
                    triProjected.p[1].y *= -1.0f;
                    triProjected.p[2].y *= -1.0f;

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
                }
            }
            //}
        }
        // sort triangles from back to front
        // sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle &t1, triangle &t2)
        //      {
        //         float z1 = (t1.p[0].z +t1.p[1].z +t1.p[2].z)/3.0f;
        //         float z2 = (t2.p[0].z +t2.p[1].z +t2.p[2].z)/3.0f;
        //         return z1>z2; });

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
                //drawTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y);
                TexturedTriangle(t.p[0].x, t.p[0].y, t.t[0].u, t.t[0].v, t.t[0].w,
                                 t.p[1].x, t.p[1].y, t.t[1].u, t.t[1].v, t.t[1].w,
                                 t.p[2].x, t.p[2].y, t.t[2].u, t.t[2].v, t.t[2].w, t.col);
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

    mat4x4 Matrix_Adder(mat4x4 mat1, mat4x4 mat2)
    {
        mat4x4 mat3;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                mat3.m[i][j] = mat1.m[i][j] + mat2.m[i][j];
            }
        }
        return mat3;
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
    vec3d Vector_IntersectPlane(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd, float &t)
    {
        plane_n = Vector_Normalise(plane_n);
        float plane_d = -Vector_DotProduct(plane_n, plane_p);
        float ad = Vector_DotProduct(lineStart, plane_n);
        float bd = Vector_DotProduct(lineEnd, plane_n);
        t = (-plane_d - ad) / (bd - ad);
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
		vec3d* inside_points[3];  int nInsidePointCount = 0;
		vec3d* outside_points[3]; int nOutsidePointCount = 0;
		vec2d* inside_tex[3]; int nInsideTexCount = 0;
		vec2d* outside_tex[3]; int nOutsideTexCount = 0;


		// Get signed distance of each point in triangle to plane
		float d0 = dist(in_tri.p[0]);
		float d1 = dist(in_tri.p[1]);
		float d2 = dist(in_tri.p[2]);

		if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; inside_tex[nInsideTexCount++] = &in_tri.t[0]; }
		else {
			outside_points[nOutsidePointCount++] = &in_tri.p[0]; outside_tex[nOutsideTexCount++] = &in_tri.t[0];
		}
		if (d1 >= 0) {
			inside_points[nInsidePointCount++] = &in_tri.p[1]; inside_tex[nInsideTexCount++] = &in_tri.t[1];
		}
		else {
			outside_points[nOutsidePointCount++] = &in_tri.p[1];  outside_tex[nOutsideTexCount++] = &in_tri.t[1];
		}
		if (d2 >= 0) {
			inside_points[nInsidePointCount++] = &in_tri.p[2]; inside_tex[nInsideTexCount++] = &in_tri.t[2];
		}
		else {
			outside_points[nOutsidePointCount++] = &in_tri.p[2];  outside_tex[nOutsideTexCount++] = &in_tri.t[2];
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
			out_tri1.col =  in_tri.col;
			

			// The inside point is valid, so keep that...
			out_tri1.p[0] = *inside_points[0];
			out_tri1.t[0] = *inside_tex[0];

			// but the two new points are at the locations where the 
			// original sides of the triangle (lines) intersect with the plane
			float t;
			out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
			out_tri1.t[1].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
			out_tri1.t[1].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
			out_tri1.t[1].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;

			out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1], t);
			out_tri1.t[2].u = t * (outside_tex[1]->u - inside_tex[0]->u) + inside_tex[0]->u;
			out_tri1.t[2].v = t * (outside_tex[1]->v - inside_tex[0]->v) + inside_tex[0]->v;
			out_tri1.t[2].w = t * (outside_tex[1]->w - inside_tex[0]->w) + inside_tex[0]->w;

			return 1; // Return the newly formed single triangle
		}

		if (nInsidePointCount == 2 && nOutsidePointCount == 1)
		{
			// Triangle should be clipped. As two points lie inside the plane,
			// the clipped triangle becomes a "quad". Fortunately, we can
			// represent a quad with two new triangles

			// Copy appearance info to new triangles
			out_tri1.col =  in_tri.col;
			

			out_tri2.col =  in_tri.col;
			

			// The first triangle consists of the two inside points and a new
			// point determined by the location where one side of the triangle
			// intersects with the plane
			out_tri1.p[0] = *inside_points[0];
			out_tri1.p[1] = *inside_points[1];
			out_tri1.t[0] = *inside_tex[0];
			out_tri1.t[1] = *inside_tex[1];

			float t;
			out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
			out_tri1.t[2].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
			out_tri1.t[2].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
			out_tri1.t[2].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;

			// The second triangle is composed of one of he inside points, a
			// new point determined by the intersection of the other side of the 
			// triangle and the plane, and the newly created point above
			out_tri2.p[0] = *inside_points[1];
			out_tri2.t[0] = *inside_tex[1];
			out_tri2.p[1] = out_tri1.p[2];
			out_tri2.t[1] = out_tri1.t[2];
			out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0], t);
			out_tri2.t[2].u = t * (outside_tex[0]->u - inside_tex[1]->u) + inside_tex[1]->u;
			out_tri2.t[2].v = t * (outside_tex[0]->v - inside_tex[1]->v) + inside_tex[1]->v;
			out_tri2.t[2].w = t * (outside_tex[0]->w - inside_tex[1]->w) + inside_tex[1]->w;
			return 2; // Return two newly formed triangles which form a quad
		}
	}

    int GetColour(float lum)
    {
        int bg_col;

        int pixel_bw = (int)(13.0f * lum);
        switch (pixel_bw)
        {
        case 0:
            bg_col = BLACK;
            break;

        case 1:
            bg_col = BLACK;
            break;
        case 2:
            bg_col = BLACK;
            break;
        case 3:
            bg_col = BLACK;
            break;
        case 4:
            bg_col = BLACK;
            break;

        case 5:
            bg_col = DARKGRAY;
            break;
        case 6:
            bg_col = DARKGRAY;
            break;
        case 7:
            bg_col = DARKGRAY;
            break;
        case 8:
            bg_col = DARKGRAY;
            break;

        case 9:
            bg_col = LIGHTGRAY;
            break;
        case 10:
            bg_col = LIGHTGRAY;
            break;
        case 11:
            bg_col = LIGHTGRAY;
            break;
        case 12:
            bg_col = LIGHTGRAY;
            break;
        default:
            bg_col = BLACK;
        }

        return bg_col;
    }

    void TexturedTriangle(int x1, int y1, float u1, float v1, float w1,
                          int x2, int y2, float u2, float v2, float w2,
                          int x3, int y3, float u3, float v3, float w3,
                          short col)
    {
        if (y2 < y1)
        {
            swap(y1, y2);
            swap(x1, x2);
            swap(u1, u2);
            swap(v1, v2);
            swap(w1, w2);
        }

        if (y3 < y1)
        {
            swap(y1, y3);
            swap(x1, x3);
            swap(u1, u3);
            swap(v1, v3);
            swap(w1, w3);
        }

        if (y3 < y2)
        {
            swap(y2, y3);
            swap(x2, x3);
            swap(u2, u3);
            swap(v2, v3);
            swap(w2, w3);
        }

        int dy1 = y2 - y1;
        int dx1 = x2 - x1;
        float dv1 = v2 - v1;
        float du1 = u2 - u1;
        float dw1 = w2 - w1;

        int dy2 = y3 - y1;
        int dx2 = x3 - x1;
        float dv2 = v3 - v1;
        float du2 = u3 - u1;
        float dw2 = w3 - w1;

        float tex_u, tex_v, tex_w;

        float dax_step = 0, dbx_step = 0,
              du1_step = 0, dv1_step = 0,
              du2_step = 0, dv2_step = 0,
              dw1_step = 0, dw2_step = 0;

        if (dy1)
            dax_step = dx1 / (float)abs(dy1);
        if (dy2)
            dbx_step = dx2 / (float)abs(dy2);

        if (dy1)
            du1_step = du1 / (float)abs(dy1);
        if (dy1)
            dv1_step = dv1 / (float)abs(dy1);
        if (dy1)
            dw1_step = dw1 / (float)abs(dy1);

        if (dy2)
            du2_step = du2 / (float)abs(dy2);
        if (dy2)
            dv2_step = dv2 / (float)abs(dy2);
        if (dy2)
            dw2_step = dw2 / (float)abs(dy2);

        if (dy1)
        {
            for (int i = y1; i <= y2; i++)
            {
                int ax = x1 + (float)(i - y1) * dax_step;
                int bx = x1 + (float)(i - y1) * dbx_step;

                float tex_su = u1 + (float)(i - y1) * du1_step;
                float tex_sv = v1 + (float)(i - y1) * dv1_step;
                float tex_sw = w1 + (float)(i - y1) * dw1_step;

                float tex_eu = u1 + (float)(i - y1) * du2_step;
                float tex_ev = v1 + (float)(i - y1) * dv2_step;
                float tex_ew = w1 + (float)(i - y1) * dw2_step;

                if (ax > bx)
                {
                    swap(ax, bx);
                    swap(tex_su, tex_eu);
                    swap(tex_sv, tex_ev);
                    swap(tex_sw, tex_ew);
                }

                tex_u = tex_su;
                tex_v = tex_sv;
                tex_w = tex_sw;

                float tstep = 1.0f / ((float)(bx - ax));
                float t = 0.0f;

                for (int j = ax; j < bx; j++)
                {
                    tex_u = (1.0f - t) * tex_su + t * tex_eu;
                    tex_v = (1.0f - t) * tex_sv + t * tex_ev;
                    tex_w = (1.0f - t) * tex_sw + t * tex_ew;
                    if (tex_w > pDepthBuffer[i * screenWidth + j])
                    {
                        putpixel(j, i, col);
                        pDepthBuffer[i * screenWidth + j] = tex_w;
                    }
                    t += tstep;
                }
            }
        }

        dy1 = y3 - y2;
        dx1 = x3 - x2;
        dv1 = v3 - v2;
        du1 = u3 - u2;
        dw1 = w3 - w2;

        if (dy1)
            dax_step = dx1 / (float)abs(dy1);
        if (dy2)
            dbx_step = dx2 / (float)abs(dy2);

        du1_step = 0, dv1_step = 0;
        if (dy1)
            du1_step = du1 / (float)abs(dy1);
        if (dy1)
            dv1_step = dv1 / (float)abs(dy1);
        if (dy1)
            dw1_step = dw1 / (float)abs(dy1);

        if (dy1)
        {
            for (int i = y2; i <= y3; i++)
            {
                int ax = x2 + (float)(i - y2) * dax_step;
                int bx = x1 + (float)(i - y1) * dbx_step;

                float tex_su = u2 + (float)(i - y2) * du1_step;
                float tex_sv = v2 + (float)(i - y2) * dv1_step;
                float tex_sw = w2 + (float)(i - y2) * dw1_step;

                float tex_eu = u1 + (float)(i - y1) * du2_step;
                float tex_ev = v1 + (float)(i - y1) * dv2_step;
                float tex_ew = w1 + (float)(i - y1) * dw2_step;

                if (ax > bx)
                {
                    swap(ax, bx);
                    swap(tex_su, tex_eu);
                    swap(tex_sv, tex_ev);
                    swap(tex_sw, tex_ew);
                }

                tex_u = tex_su;
                tex_v = tex_sv;
                tex_w = tex_sw;

                float tstep = 1.0f / ((float)(bx - ax));
                float t = 0.0f;

                for (int j = ax; j < bx; j++)
                {
                    tex_u = (1.0f - t) * tex_su + t * tex_eu;
                    tex_v = (1.0f - t) * tex_sv + t * tex_ev;
                    tex_w = (1.0f - t) * tex_sw + t * tex_ew;

                    if (tex_w > pDepthBuffer[i * screenWidth + j])
                    {
                        putpixel(j, i, col);
                        pDepthBuffer[i * screenWidth + j] = tex_w;
                    }
                    t += tstep;
                }
            }
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
    renderer car("exteriorCar.obj","cyberTruckTex.obj", screenWidth, screenHeight, -3, -3);

    while (1)
    {
        car.mainLoop();
    }
    return 0;
}