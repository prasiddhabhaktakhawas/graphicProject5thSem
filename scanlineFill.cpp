#include<graphics.h>
#include<conio.h>
#include<math.h>
#include<windows.h>
#include<iostream>
float area(float x1, float y1, float x2, float y2, float x3, float y3);
bool isInside(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y);

void fillTriangle(float x1, float y1, float x2, float y2, float x3, float y3);

int main(int argc, char const *argv[]){
    DWORD screenWidth = GetSystemMetrics(SM_CXSCREEN);
    DWORD screenHeight = GetSystemMetrics(SM_CYSCREEN);
    initwindow(screenWidth, screenHeight, "", -3, -3);\

    fillTriangle(100.0, 100.0, 200.0, 200.0, 50.0, 150.0 );

    getch();
    closegraph();
    return 0;
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
            if(isInside(x1,y1,x2,y2,x3,y3,j,i)){
                putpixel(j,i, WHITE);
            }
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
     
    if(A == A1 + A2 + A3)
        return true;
    else
        return false;
}