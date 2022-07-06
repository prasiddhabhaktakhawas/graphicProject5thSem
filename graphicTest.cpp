#include<graphics.h>
#include<math.h>
#include<windows.h>

void MultiplyMatrixVector(float (&i)[9], float (&o)[9] , float (&m)[4][4]);
void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);

int main(int argc, char const *argv[]){
    DWORD screenWidth = GetSystemMetrics(SM_CXSCREEN);
    DWORD screenHeight = GetSystemMetrics(SM_CYSCREEN);
    initwindow(screenWidth, screenHeight, "", -3, -3);
    // int gd = DETECT, gm;
    // initgraph(&gd,&gm, (char*)"");

    

    float cube[12][9] = {
        //{x,y,z,x,y,z,x,y,z}
        //south
        {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f},
        //east
        {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        //north
        {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        //west
        {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        //top
        {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
        //bottom
        {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}
    };

    float pC[12][9] = {0};

    cleardevice();

    float tm[4][4] ={0};
    float fNear = 0.1f; //near plane
    float fFar = 1000.0f; //far plane
    float fFov = 90.0f; // field of view
    float fAspectRatio = 0.5625f;
    float fFovRad = 1.0f/ tanf(fFov * 0.5f / 180.0f * 3.14159f);

    float pM[4][4] = {0};
    
    pM[0][0] = fAspectRatio * fFovRad;
    pM[1][1] = fFovRad;
    pM[2][2] = fFar /(fFar -fNear);
    pM[3][2] = (-fFar * fNear)/ (fFar - fNear);
    pM[2][3] = 1.0f;
    pM[3][3] = 0.0f;

    //draw triangles
    for(int i=0;i<12; i++){
            MultiplyMatrixVector(cube[i],pC[i],pM);

            //scale into view
            pC[i][0]+=1.0f;pC[i][1]+=1.0f;
            pC[i][3]+=1.0f;pC[i][4]+=1.0f;
            pC[i][6]+=1.0f;pC[i][7]+=1.0f;

            pC[i][0]*=0.5f * (float)screenWidth;
            pC[i][1]*=0.5f * (float)screenHeight;
            pC[i][3]*=0.5f * (float)screenWidth;
            pC[i][4]*=0.5f * (float)screenHeight;
            pC[i][6]*=0.5f * (float)screenWidth;
            pC[i][7]*=0.5f * (float)screenHeight;

            drawTriangle(pC[i][0], pC[i][1], pC[i][3],pC[i][4],pC[i][6],pC[i][7]);
    } 

    getch();
    closegraph();
    return 0;
}

void MultiplyMatrixVector(float (&i)[9], float (&o)[9] , float (&m)[4][4]){
    o[0] =i[0]*m[0][0] + i[1]*m[1][0] + i[2]*m[2][0]+ m[3][0];
    o[1] =i[0]*m[0][1] + i[1]*m[1][1] + i[2]*m[2][1]+ m[3][1];
    o[2] =i[0]*m[0][2] + i[1]*m[1][2] + i[2]*m[2][2]+ m[3][2];
    float w=i[0]*m[0][3] + i[1]*m[1][3] + i[2]*m[2][3]+ m[3][3];
    if(w!=0.0f){
        o[0]/=w; o[1]/=w; o[2]/=w;
    }
    o[3] =i[3]*m[0][0] + i[4]*m[1][0] + i[5]*m[2][0]+ m[3][0];
    o[4] =i[3]*m[0][1] + i[4]*m[1][1] + i[5]*m[2][1]+ m[3][1];
    o[5] =i[3]*m[0][2] + i[4]*m[1][2] + i[5]*m[2][2]+ m[3][2];
    w=i[3]*m[0][3] + i[4]*m[1][3] + i[5]*m[2][3]+ m[3][3];
    if(w!=0.0f){
        o[3]/=w; o[4]/=w; o[5]/=w;
    }
    o[6] =i[6]*m[0][0] + i[7]*m[1][0] + i[8]*m[2][0]+ m[3][0];
    o[7] =i[6]*m[0][1] + i[7]*m[1][1] + i[8]*m[2][1]+ m[3][1];
    o[8] =i[6]*m[0][2] + i[7]*m[1][2] + i[8]*m[2][2]+ m[3][2];
    w=i[6]*m[0][3] + i[7]*m[1][3] + i[8]*m[2][3]+ m[3][3];
    if(w!=0.0f){
        o[6]/=w; o[7]/=w; o[8]/=w;
    }
}

void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3){
    line(x1,y1,x2,y2);
    line(x2,y2, x3,y3);
    line(x3,y3,x1,y1);
}