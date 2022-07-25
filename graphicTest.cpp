#include <graphics.h>
#include <math.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <strstream>
#include <string>
#include <vector>
using namespace std;

DWORD screenWidth = GetSystemMetrics(SM_CXSCREEN);
DWORD screenHeight = GetSystemMetrics(SM_CYSCREEN);

struct vec3d
{
    float x, y, z;
};

struct triangle
{
    vec3d p[3];
    short col;
};

struct mesh
{
    vector<triangle> tris;
};

struct mat4x4
{
    float m[4][4] = {0};
};

void MultiplyMatrixVector(vec3d &i, vec3d &o, mat4x4 &m);
void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
float area(float x1, float y1, float x2, float y2, float x3, float y3);
bool isInside(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y);
void fillTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
void drawLine(int x1, int y1, int x2, int y2, int c);
bool loadFromObj(string fileName);
int GetColour(float lum);
void TexturedTriangle(	int x1, int y1, 
							int x2, int y2, 
							int x3, int y3, short col);
mesh meshObj;

int main()
{

    initwindow(screenWidth, screenHeight, "", -3, -3);
    // int gd = DETECT, gm;
    // initgraph(&gd,&gm, (char*)"");

    loadFromObj("cyber.obj");
    // s
    mat4x4 matProj, matRotX, matRotY, matRotZ;

    // meshCube.tris = {
    //     //{x,y,z,x,y,z,x,y,z}
    //     // SOUTH
    //     {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    //     {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f},

    //     // EAST
    //     {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
    //     {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},

    //     // NORTH
    //     {1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
    //     {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},

    //     // WEST
    //     {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    //     {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},

    //     // TOP
    //     {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    //     {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f},

    //     // BOTTOM
    //     {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
    //     {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},

    // };

    float fThetaX = 0.0f; // angle of rotation
    float fThetaY = 0.0f;
    float zoom=0.0f;
    // float fThetaZ = 0.0f;
    vec3d vCamera; // camera is at origin
    vCamera.x=0.0f;
    vCamera.y=0.0f;
    vCamera.z=0.0f;
    POINT cursorPos;     // mouse cursor position

    float prevXM; // previous x position of mouse
    float prevYM;
    float differenceX; // difference between current and previous x position of mouse
    float differenceY;
    bool leftButtonHold = false; // boolean to find out when left mouse button is holded and released
                                 // for making cube visible at start
    // rotation y
    matRotY.m[0][0] = cosf(fThetaY);
    matRotY.m[0][2] = -sinf(fThetaY);
    matRotY.m[1][1] = 1;
    matRotY.m[2][0] = sinf(fThetaY);
    matRotY.m[2][2] = cosf(fThetaY);
    matRotY.m[3][3] = 1;

    // Rotation x
    matRotX.m[0][0] = 1;
    matRotX.m[1][1] = cosf(fThetaX);
    matRotX.m[1][2] = sinf(fThetaX);
    matRotX.m[2][1] = -sinf(fThetaX);
    matRotX.m[2][2] = cosf(fThetaX);
    matRotX.m[3][3] = 1;

    // 3d to 2d projection

    float fNear = 0.1f;                                            // near plane
    float fFar = 1000.0f;                                          // far plane
    float fFov = 90.0f;                                            // field of view
    float fAspectRatio = (float)screenHeight / (float)screenWidth; // height / width
    float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

    matProj.m[0][0] = fAspectRatio * fFovRad;
    matProj.m[1][1] = fFovRad;
    matProj.m[2][2] = fFar / (fFar - fNear);
    matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
    matProj.m[2][3] = 1.0f;
    matProj.m[3][3] = 0.0f;

    
    // end of 3d to 2d projection matrix

    int page = 0;

    while (1)
    {                        // runs always
        setactivepage(page); // double buffer method
        setvisualpage(1 - page);
        if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0)
        { // when left click is hold
            GetCursorPos(&cursorPos);
            if (leftButtonHold == false)
            {
                prevXM = cursorPos.x;
                prevYM = cursorPos.y;
                leftButtonHold = true;
            }
            differenceX = prevXM - cursorPos.x;
            differenceY = prevYM - cursorPos.y;
            fThetaX -= differenceY * 0.01; // dragging mouse in y direction give means rotating wrt to x axis and -ve for invertmouseY just like in videogames
            fThetaY += differenceX * 0.01;
            prevXM = cursorPos.x;
            prevYM = cursorPos.y;

            // //rotation z
            // matRotZ[0][0]=cosf(fTheta);
            // matRotZ[0][1]=sinf(fTheta);
            // matRotZ[1][0]=-sinf(fTheta);
            // matRotZ[1][1]=cosf(fTheta);
            // matRotZ[2][2]=1;
            // matRotZ[3][3]=1;

            // rotation y
            matRotY.m[0][0] = cosf(fThetaY);
            matRotY.m[0][2] = -sinf(fThetaY);
            matRotY.m[1][1] = 1;
            matRotY.m[2][0] = sinf(fThetaY);
            matRotY.m[2][2] = cosf(fThetaY);
            matRotY.m[3][3] = 1;

            // Rotation x
            matRotX.m[0][0] = 1;
            matRotX.m[1][1] = cosf(fThetaX);
            matRotX.m[1][2] = sinf(fThetaX);
            matRotX.m[2][1] = -sinf(fThetaX);
            matRotX.m[2][2] = cosf(fThetaX);
            matRotX.m[3][3] = 1;
        }
        else
        {
            leftButtonHold = false;
        }
        if((GetAsyncKeyState(0x5A) & 0x8000)!=0){   //z key pressed
            zoom--;
            
        }else if((GetAsyncKeyState(0x58) & 0x8000)!=0){ //x key pressed
            zoom++;    
        }

        cleardevice(); // when clear device is inside the key press loop, then multiple border ficklering occurs, so, cleardevice should be outside so its being cleared every time
        // draw triangles

        vector<triangle> vecTrianglesToRaster;

        for (auto tri : meshObj.tris)
        {
            triangle triProjected, triTranslated, triRotatedX, triRotatedY, triRotatedZ;
            vec3d normal, line1, line2;

            MultiplyMatrixVector(tri.p[0], triRotatedY.p[0], matRotY); // tri.p[0] is a vertex
            MultiplyMatrixVector(tri.p[1], triRotatedY.p[1], matRotY);
            MultiplyMatrixVector(tri.p[2], triRotatedY.p[2], matRotY);

            MultiplyMatrixVector(triRotatedY.p[0], triRotatedX.p[0], matRotX);
            MultiplyMatrixVector(triRotatedY.p[1], triRotatedX.p[1], matRotX);
            MultiplyMatrixVector(triRotatedY.p[2], triRotatedX.p[2], matRotX);

            triTranslated = triRotatedX;

            triTranslated.p[0].z = triRotatedX.p[0].z + 20.0f+zoom;
            triTranslated.p[1].z = triRotatedX.p[1].z + 20.0f+zoom;
            triTranslated.p[2].z = triRotatedX.p[2].z + 20.0f+zoom;

            line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
            line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
            line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

            line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
            line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
            line2.z = triTranslated.p[2].z - triTranslated.p[0].z;
            // cross products
            normal.x = line1.y * line2.z - line1.z * line2.y;
            normal.y = line1.z * line2.x - line1.x * line2.z;
            normal.z = line1.x * line2.y - line1.y * line2.x;

            float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            normal.x /= l;
            normal.y /= l;
            normal.z /= l; // unit vector of normal to surface

            if (normal.x * (triTranslated.p[0].x - vCamera.x) + normal.y * (triTranslated.p[0].y - vCamera.y) + normal.z * (triTranslated.p[0].z - vCamera.z) < 0.0f)
            {

                vec3d light_direction = { -1.0f, -1.0f, -1.0f };
				float l = sqrtf(light_direction.x*light_direction.x + light_direction.y*light_direction.y + light_direction.z*light_direction.z);
				light_direction.x /= l; light_direction.y /= l; light_direction.z /= l;

				// How "aligned" are light direction and triangle surface normal?
				float dp = normal.x * light_direction.x + normal.y * light_direction.y + normal.z * light_direction.z;

				// Choose console colours as required (much easier with RGB)
				int c = GetColour(dp);
				triTranslated.col = c;

                MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
                MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
                MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);
                triProjected.col = triTranslated.col;

                triProjected.p[0].x += 1.0f;
                triProjected.p[0].y += 1.0f; // since -1.0 would go out of screen of left side, so +1.0 to bring it to 0, which is inside screen
                triProjected.p[1].x += 1.0f;
                triProjected.p[1].y += 1.0f;
                triProjected.p[2].x += 1.0f;
                triProjected.p[2].y += 1.0f;

                // scaling it wrt to screen size         //we scale half the total length to make the object appear on center
                triProjected.p[0].x *= 0.5f * (float)screenWidth; // scaling in x direction is bigger because width is bigger than height, but it's cancelled out by aspect ratio( h/w), so at the end, it is just perfect
                triProjected.p[0].y *= 0.5f * (float)screenHeight;
                triProjected.p[1].x *= 0.5f * (float)screenWidth;  //
                triProjected.p[1].y *= 0.5f * (float)screenHeight; //
                triProjected.p[2].x *= 0.5f * (float)screenWidth;
                triProjected.p[2].y *= 0.5f * (float)screenHeight;

                //drawTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y, triProjected.p[2].x, triProjected.p[2].y);

                vecTrianglesToRaster.push_back(triProjected);

                
            }
            
        }
        //sort triangles from back to front
        sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle &t1, triangle &t2){
            float z1 = (t1.p[0].z +t1.p[1].z +t1.p[2].z)/3.0f;
            float z2 = (t2.p[0].z +t2.p[1].z +t2.p[2].z)/3.0f;
            return z1>z2;
        });

        for(auto &triProjected: vecTrianglesToRaster){
            // setfillstyle(SOLID_FILL,RED);
            //drawTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y, triProjected.p[2].x, triProjected.p[2].y);
            TexturedTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y, triProjected.p[2].x, triProjected.p[2].y, triProjected.col);
            // float midX = (triProjected.p[0].x+triProjected.p[1].x+triProjected.p[2].x)/3.0f;
            // float midY = (triProjected.p[0].y+triProjected.p[1].y+triProjected.p[2].y)/3.0f;
            // floodfill(midX,midY,RED);
        }
        page = 1 - page; // double buffer method
    }
    closegraph();
    return 0;
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

bool loadFromObj(string fileName){
    ifstream f(fileName);
    if(!f.is_open())
        return false;
    vector<vec3d> verts;
    while(!f.eof()){
        char line[128];
        f.getline(line, 128);

        strstream s;
        s<<line;
        
        char junk;

        if(line[0] == 'v'){
            vec3d v;
            s>>junk>>v.x>>v.y>>v.z;
            verts.push_back(v);
        }
        if(line[0] == 'f'){
            int f[3];
            s>>junk>>f[0]>>f[1]>>f[2];
            meshObj.tris.push_back({verts[f[0]-1], verts[f[1]-1], verts[f[2]-1]});
            
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


int GetColour(float lum)
	{
		int bg_col;
		
		int pixel_bw = (int)(13.0f*lum);
		switch (pixel_bw)
		{
		case 0: bg_col = BLACK; break;

		case 1: bg_col = BLACK;   break;
		case 2: bg_col = BLACK;  break;
		case 3: bg_col =BLACK;  break;
		case 4: bg_col = BLACK;  break;

		case 5: bg_col = DARKGRAY; break;
		case 6: bg_col = DARKGRAY; break;
		case 7: bg_col = DARKGRAY; break;
		case 8: bg_col = DARKGRAY; break;

		case 9:  bg_col = LIGHTGRAY; break;
		case 10: bg_col =LIGHTGRAY; break;
		case 11: bg_col =LIGHTGRAY; break;
		case 12: bg_col =LIGHTGRAY; break;
		default:
			bg_col = BLACK;
		}

		return bg_col;
	}

void TexturedTriangle(	int x1, int y1, 
							int x2, int y2, 
							int x3, int y3, short col)
	{
		if (y2 < y1)
		{
			swap(y1, y2);
			swap(x1, x2);
			// swap(u1, u2);
			// swap(v1, v2);
			// swap(w1, w2);
		}

		if (y3 < y1)
		{
			swap(y1, y3);
			swap(x1, x3);
			// swap(u1, u3);
			// swap(v1, v3);
			// swap(w1, w3);
		}

		if (y3 < y2)
		{
			swap(y2, y3);
			swap(x2, x3);
			// swap(u2, u3);
			// swap(v2, v3);
			// swap(w2, w3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		// float dv1 = v2 - v1;
		// float du1 = u2 - u1;
		// float dw1 = w2 - w1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		// float dv2 = v3 - v1;
		// float du2 = u3 - u1;
		// float dw2 = w3 - w1;

		// float tex_u, tex_v, tex_w;

		float dax_step = 0, dbx_step = 0;
		// 	du1_step = 0, dv1_step = 0,
		// 	du2_step = 0, dv2_step = 0,
		// 	dw1_step=0, dw2_step=0;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		// if (dy1) du1_step = du1 / (float)abs(dy1);
		// if (dy1) dv1_step = dv1 / (float)abs(dy1);
		// if (dy1) dw1_step = dw1 / (float)abs(dy1);

		// if (dy2) du2_step = du2 / (float)abs(dy2);
		// if (dy2) dv2_step = dv2 / (float)abs(dy2);
		// if (dy2) dw2_step = dw2 / (float)abs(dy2);

		if (dy1)
		{
			for (int i = y1; i <= y2; i++)
			{
				int ax = x1 + (float)(i - y1) * dax_step;
				int bx = x1 + (float)(i - y1) * dbx_step;

				// float tex_su = u1 + (float)(i - y1) * du1_step;
				// float tex_sv = v1 + (float)(i - y1) * dv1_step;
				// float tex_sw = w1 + (float)(i - y1) * dw1_step;

				// float tex_eu = u1 + (float)(i - y1) * du2_step;
				// float tex_ev = v1 + (float)(i - y1) * dv2_step;
				// float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					swap(ax, bx);
					// swap(tex_su, tex_eu);
					// swap(tex_sv, tex_ev);
					// swap(tex_sw, tex_ew);
				}

				// tex_u = tex_su;
				// tex_v = tex_sv;
				// tex_w = tex_sw;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					// tex_u = (1.0f - t) * tex_su + t * tex_eu;
					// tex_v = (1.0f - t) * tex_sv + t * tex_ev;
					// tex_w = (1.0f - t) * tex_sw + t * tex_ew;
				
						putpixel(j, i, col);
					
					t += tstep;
				}

			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		// dv1 = v3 - v2;
		// du1 = u3 - u2;
		// dw1 = w3 - w2;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		// du1_step = 0, dv1_step = 0;
		// if (dy1) du1_step = du1 / (float)abs(dy1);
		// if (dy1) dv1_step = dv1 / (float)abs(dy1);
		// if (dy1) dw1_step = dw1 / (float)abs(dy1);

		if (dy1)
		{
			for (int i = y2; i <= y3; i++)
			{
				int ax = x2 + (float)(i - y2) * dax_step;
				int bx = x1 + (float)(i - y1) * dbx_step;

				// float tex_su = u2 + (float)(i - y2) * du1_step;
				// float tex_sv = v2 + (float)(i - y2) * dv1_step;
				// float tex_sw = w2 + (float)(i - y2) * dw1_step;

				// float tex_eu = u1 + (float)(i - y1) * du2_step;
				// float tex_ev = v1 + (float)(i - y1) * dv2_step;
				// float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					swap(ax, bx);
					// swap(tex_su, tex_eu);
					// swap(tex_sv, tex_ev);
					// swap(tex_sw, tex_ew);
				}

				// tex_u = tex_su;
				// tex_v = tex_sv;
				// tex_w = tex_sw;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					// tex_u = (1.0f - t) * tex_su + t * tex_eu;
					// tex_v = (1.0f - t) * tex_sv + t * tex_ev;
					// tex_w = (1.0f - t) * tex_sw + t * tex_ew;

					
						putpixel(j, i, col);
					
					t += tstep;
				}
			}	
		}		
	}
