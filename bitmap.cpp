#include "image.h"
#include<vector>
#include<iostream>
#include <math.h>
void drawLine(int x1, int y1, int x2, int y2, int c);
void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
const int width = 1920;
const int height = 1080;
Image image(width, height);

struct Co{
    int x, y;
};
std::vector<Co> locus;
int main()
{
    for( int i = 0 ; i<100; i++){
        drawTriangle(100+i,100, 200+i, 200, 100+i, 200);
        image.Export("image.bmp");
    }
    std::cout<<"done"<<std::endl;
    return 0;
}

void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
    drawLine(x1,y1,x2,y2,2);
    drawLine(x2,y2,x3,y3,2);
    drawLine(x3,y3,x1,y1,2);
    // line(x1, y1, x2, y2); // inbuilt line of graphics.h seems to perform better than custom built
    // line(x2, y2, x3, y3);
    // line(x3, y3, x1, y1);
}

void drawLine(int x1, int y1, int x2, int y2, int c)
{
    
    Co xy;
    int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
    dx = x2 - x1;
    dy = y2 - y1;
    dx1 = fabs(dx);
    dy1 = fabs(dy);
    px = 2 * dy1 - dx1;
    py = 2 * dx1 - dy1;
    if (dy1 <= dx1)
    {
        if (dx >= 0)
        {
            x = x1;
            y = y1;
            xe = x2;
        }
        else
        {
            x = x2;
            y = y2;
            xe = x1;
        }
        image.SetColor(Color((float)x / (float)width, 1.0f - ((float)x / (float)width), (float)y / (float)height), x, height-y);
        xy.x=x;
        xy.y=y;
        locus.push_back(xy);
        for (i = 0; x < xe; i++)
        {
            x = x + 1;
            if (px < 0)
            {
                px = px + 2 * dy1;
            }
            else
            {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
                {
                    y = y + 1;
                }
                else
                {
                    y = y - 1;
                }
                px = px + 2 * (dy1 - dx1);
            }
            image.SetColor(Color((float)x / (float)width, 1.0f - ((float)x / (float)width), (float)y / (float)height), x, height-y);
            xy.x=x;
            xy.y=y;
            locus.push_back(xy);
        }
    }
    else
    {
        if (dy >= 0)
        {
            x = x1;
            y = y1;
            ye = y2;
        }
        else
        {
            x = x2;
            y = y2;
            ye = y1;
        }
        image.SetColor(Color((float)x / (float)width, 1.0f - ((float)x / (float)width), (float)y / (float)height), x, height-y);
        xy.x=x;
        xy.y=y;
        locus.push_back(xy);
        for (i = 0; y < ye; i++)
        {
            y = y + 1;
            if (py <= 0)
            {
                py = py + 2 * dx1;
            }
            else
            {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
                {
                    x = x + 1;
                }
                else
                {
                    x = x - 1;
                }
                py = py + 2 * (dx1 - dy1);
            }
            image.SetColor(Color((float)x / (float)width, 1.0f - ((float)x / (float)width), (float)y / (float)height), x, height-y);
            xy.x=x;
            xy.y=y;
            locus.push_back(xy);
        }
    }
}