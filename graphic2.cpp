#include<graphics.h>
#include<math.h>
#include<windows.h>
#include<iostream>
#include<fstream>
#include<strstream>
#include<string>
#include<vector>
using namespace std;

void MultiplyMatrixVector(float (&i)[9], float (&o)[9] , float (&m)[4][4]);
void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
float area(float x1, float y1, float x2, float y2, float x3, float y3);
bool isInside(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y);
void fillTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
void drawLine(int x1, int y1, int x2, int y2, int c);
bool loadFromObj(string fileName);


DWORD screenWidth = GetSystemMetrics(SM_CXSCREEN);
DWORD screenHeight = GetSystemMetrics(SM_CYSCREEN);

int main(int argc, char const *argv[]){
    
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
        {1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
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

    float pC[12][9] = {0};  //projected cube
    float tC[12][9] = {0};     //translated cube
    float matRotZ[4][4]={0};    //rotation matrix
    float matRotX[4][4] ={0};
    float matRotY[4][4] ={0};
    float rCz[12][9];   //rotated cube
    float rCx[12][9];
    float rCy[12][9];
    float fThetaX = 0.0;    //angle of rotation
    float fThetaY = 0.0;
    float vCamera[3] ={0};     //camera is at origin
    // fTheta+= 1.0f * clock();
    POINT cursorPos;    //mouse cursor position
 
    float prevXM;   //previous x position of mouse
    float prevYM;
    float differenceX;  //difference between current and previous x position of mouse
    float differenceY;
    bool leftButtonHold=false;  //boolean to find out when left mouse button is holded and released
                                                //for making cube visible at start
    //rotation y
    matRotY[0][0]=cosf(fThetaY);
    matRotY[0][2]=-sinf(fThetaY);
    matRotY[1][1]=1;
    matRotY[2][0]=sinf(fThetaY);
    matRotY[2][2]=cosf(fThetaY);
    matRotY[3][3]=1;

    //Rotation x
    matRotX[0][0]=1;
    matRotX[1][1]=cosf(fThetaX);
    matRotX[1][2]=sinf(fThetaX);
    matRotX[2][1]=-sinf(fThetaX);
    matRotX[2][2]=cosf(fThetaX);;
    matRotX[3][3]=1;

    //3d to 2d projection

    float fNear = 0.1f; //near plane
    float fFar = 1000.0f; //far plane
    float fFov = 90.0f; // field of view
    float fAspectRatio = 0.5625f;   // height / width
    float fFovRad = 1.0f/ tanf(fFov * 0.5f / 180.0f * 3.14159f);

    float pM[4][4] = {0};
    
    pM[0][0] = fAspectRatio * fFovRad;
    pM[1][1] = fFovRad;
    pM[2][2] = fFar /(fFar -fNear);
    pM[3][2] = (-fFar * fNear)/ (fFar - fNear);
    pM[2][3] = 1.0f;
    pM[3][3] = 0.0f;
  

    // end of 3d to 2d projection matrix

    int page = 0;
    
    while(1){               //runs always
        setactivepage(page);    // double buffer method
        setvisualpage(1-page);                    
        if((GetKeyState(VK_LBUTTON) & 0x8000) != 0){    //when left click is hold
            GetCursorPos(&cursorPos);
            if(leftButtonHold==false){
                prevXM = cursorPos.x;
                prevYM = cursorPos.y;
                leftButtonHold=true;
            }
            differenceX = prevXM-cursorPos.x;
            differenceY = prevYM-cursorPos.y;
            fThetaX-=differenceY*0.01;          //dragging mouse in y direction give means rotating wrt to x axis and -ve for invertmouseY just like in videogames
            fThetaY+=differenceX*0.01;
            prevXM= cursorPos.x;
            prevYM= cursorPos.y;
            
            // //rotation z
            // matRotZ[0][0]=cosf(fTheta);
            // matRotZ[0][1]=sinf(fTheta);
            // matRotZ[1][0]=-sinf(fTheta);
            // matRotZ[1][1]=cosf(fTheta);
            // matRotZ[2][2]=1;
            // matRotZ[3][3]=1;

            //rotation y
            matRotY[0][0]=cosf(fThetaY);
            matRotY[0][2]=-sinf(fThetaY);
            matRotY[1][1]=1;
            matRotY[2][0]=sinf(fThetaY);
            matRotY[2][2]=cosf(fThetaY);
            matRotY[3][3]=1;

            //Rotation x
            matRotX[0][0]=1;
            matRotX[1][1]=cosf(fThetaX);
            matRotX[1][2]=sinf(fThetaX);
            matRotX[2][1]=-sinf(fThetaX);
            matRotX[2][2]=cosf(fThetaX);;
            matRotX[3][3]=1;
        }else{
                leftButtonHold=false;
        }

        cleardevice();      //when clear device is inside the key press loop, then multiple border ficklering occurs, so, cleardevice should be outside so its being cleared every time
        //draw triangles
        
        for(int i=0;i<12; i++){
                //rotate in y axis
                MultiplyMatrixVector(cube[i],rCy[i],matRotY);
                //rotate in x axis
                MultiplyMatrixVector(rCy[i],rCx[i],matRotX);
                //rotate in z axis
                // MultiplyMatrixVector(rCz[i],rCy[i], matRotZ);

                for(int i=0; i< 12; i++){
                    for(int j=0; j<9; j++){
                        tC[i][j]=rCx[i][j];
                    }
                }   

                // offset into the screen. translating z axis away from camera, otherwise, camera would be inside object
                tC[i][2]=rCx[i][2] + 3.0f;
                tC[i][5]=rCx[i][5] + 3.0f;
                tC[i][8]=rCx[i][8] + 3.0f;

                //visible surface detection
                float normal[3], line1[3], line2[3];

                line1[0]= tC[i][3] - tC[i][0];
                line1[1]= tC[i][4] - tC[i][1];
                line1[2]= tC[i][5] - tC[i][2];

                line2[0]= tC[i][6] - tC[i][0];
                line2[1]= tC[i][7] - tC[i][1];
                line2[2]= tC[i][8] - tC[i][2];
                //cross products
                normal[0] = line1[1] * line2[2] - line1[2]*line2[1];    
                normal[1] = line1[2] * line2[0] - line1[0]*line2[2];
                normal[2] = line1[0] * line2[1] - line1[1]*line2[0];

                float l = sqrt(normal[0]*normal[0] + normal[1]*normal[1]+normal[2]*normal[2]);  
                normal[0]/=l; normal[1]/=l; normal[2]/=l;   // unit vector of normal to surface
                
                // if(normal[2]<0){
                if(normal[0]*(tC[i][0]-vCamera[0]) + normal[1]*(tC[i][1]-vCamera[1]) + normal[2]*(tC[i][2]-vCamera[2])<0.0){    //dot product of camera vector and normal vector of surface of cubes
                //translated cube into projected cube by matrix multiplication with projection matrix
                MultiplyMatrixVector(tC[i],pC[i],pM); 

                //scale into view wrt x and y
                pC[i][0]+=1.0f;pC[i][1]+=1.0f;  //since -1.0 would go out of screen of left side, so +1.0 to bring it to 0, which is inside screen
                pC[i][3]+=1.0f;pC[i][4]+=1.0f;
                pC[i][6]+=1.0f;pC[i][7]+=1.0f;

                //scaling it wrt to screen size         //we scale half the total length to make the object appear on center
                pC[i][0]*=0.5f * (float)screenWidth;    //scaling in x direction is bigger because width is bigger than height, but it's cancelled out by aspect ratio( h/w), so at the end, it is just perfect
                pC[i][1]*=0.5f * (float)screenHeight;
                pC[i][3]*=0.5f * (float)screenWidth;    //
                pC[i][4]*=0.5f * (float)screenHeight;   //
                pC[i][6]*=0.5f * (float)screenWidth;
                pC[i][7]*=0.5f * (float)screenHeight;   //

               
                
                
                //fillTriangle(pC[i][0], pC[i][1], pC[i][3],pC[i][4],pC[i][6],pC[i][7]);
                //setfillstyle(SOLID_FILL,RED);
                drawTriangle(pC[i][0], pC[i][1], pC[i][3],pC[i][4],pC[i][6],pC[i][7]);
                //float midX = (pC[i][0]+pC[i][3]+pC[i][6])/3.0f;
                //float midY = (pC[i][1]+pC[i][4]+pC[i][7])/3.0f;
                //floodfill(midX,midY,RED);
                }
        }
        page = 1-page;  //double buffer method
    }
    closegraph();
    return 0;
}

void MultiplyMatrixVector(float (&i)[9], float (&o)[9] , float (&m)[4][4]){ //pass by reference
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

// bool loadFromObj(string fileName){
//     ifstream f(fileName);
//     if(!f.is_open())
//         return false;
//     vector<float> verts;
//     while(!f.eof()){
//         char line[128];
//         f.getline(line, 128);

//         strstream s;
//         s<<line;
        
//         char junk;

//         if(line[0] == 'v'){

//         }
//     }
// }

void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3){
    // drawLine(x1,y1,x2,y2,WHITE);
    // drawLine(x2,y2,x3,y3,WHITE);
    // drawLine(x3,y3,x1,y1,WHITE);
    line(x1,y1,x2,y2);  //inbuilt line of graphics.h seems to perform better than custom built
    line(x2,y2,x3,y3);
    line(x3,y3,x1,y1);
}

// bresenham line drawing algo
void drawLine(int x1,int y1,int x2,int y2,int c)
{
 int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;
 dx=x2-x1;
 dy=y2-y1;
 dx1=fabs(dx);
 dy1=fabs(dy);
 px=2*dy1-dx1;
 py=2*dx1-dy1;
 if(dy1<=dx1)
 {
  if(dx>=0)
  {
   x=x1;
   y=y1;
   xe=x2;
  }
  else
  {
   x=x2;
   y=y2;
   xe=x1;
  }
  putpixel(x,y,c);
  for(i=0;x<xe;i++)
  {
   x=x+1;
   if(px<0)
   {
    px=px+2*dy1;
   }
   else
   {
    if((dx<0 && dy<0) || (dx>0 && dy>0))
    {
     y=y+1;
    }
    else
    {
     y=y-1;
    }
    px=px+2*(dy1-dx1);
   }
   putpixel(x,y,c);
  }
 }
 else
 {
  if(dy>=0)
  {
   x=x1;
   y=y1;
   ye=y2;
  }
  else
  {
   x=x2;
   y=y2;
   ye=y1;
  }
  putpixel(x,y,c);
  for(i=0;y<ye;i++)
  {
   y=y+1;
   if(py<=0)
   {
    py=py+2*dx1;
   }
   else
   {
    if((dx<0 && dy<0) || (dx>0 && dy>0))
    {
     x=x+1;
    }
    else
    {
     x=x-1;
    }
    py=py+2*(dx1-dy1);
   }
   putpixel(x,y,c);
  }
 }
}

void fillTriangle(float x1, float y1, float x2, float y2, float x3, float y3){
    float ymin;
    float ymax;
    float xmin;
    float xmax;
    if(y1>=y2 && y1>=y3)
        ymax=y1;
    else if(y2>=y1 && y2>=y3)
        ymax=y2;
    else
        ymax=y3;

    if(y1<=y2 && y1<=y3)
        ymin=y1;
    else if(y2<=y1 && y2<=y3)
        ymin=y2;
    else
        ymin=y3;

    if(x1>=x2 && x1>=x3)
        xmax=x1;
    else if(x2>=x1 && x2>=x3)
        xmax=x2;
    else
        xmax=x3;

    if(x1<=x2 && x1<=x3)
        xmin=x1;
    else if(x2<=x1 && x2<=x3)
        xmin=x2;
    else
        xmin=x3;
    
    for(int i=ymin; i<=ymax; i++){
        for(int j=xmin; j<=xmax; j++){
            if(isInside(x1,y1,x2,y2,x3,y3,j,i))
                putpixel(j,i, RED);
        }
    }
}

float area(float x1, float y1, float x2, float y2, float x3, float y3){
 
    return abs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0);
}

bool isInside(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y){

    float A = area (x1, y1, x2, y2, x3, y3);

    float A1 = area (x, y, x2, y2, x3, y3);
     
    float A2 = area (x1, y1, x, y, x3, y3);
   
    float A3 = area (x1, y1, x2, y2, x, y);
     
    if(A >= A1 + A2 + A3)   //this bug <= instead of == took a lot of time to figure out lol. LAMO, again mistake, it should be >=, but still some triangles are filled and some aren't properly
        return true;
    else
        return false;
}

