#include <windows.h>
#include<graphics.h>
#include <iostream>
#include <conio.h>

using namespace std;

int main(){
    // while(1){
    //     if(GetAsyncKeyState(0x01)){
    //         mouse_event(MOUSEEVENTF_LEFTDOWN, 0,0,0,0);
    //         Sleep(100);
    //         mouse_event(MOUSEEVENTF_LEFTUP, 0,0,0,0);
    //     }else if(GetAsyncKeyState(0x02)){
    //         mouse_event(MOUSEEVENTF_RIGHTDOWN, 0,0,0,0);
    //         Sleep(100);
    //         mouse_event(MOUSEEVENTF_RIGHTUP, 0,0,0,0);
    //     }

    int height = GetSystemMetrics(SM_CYSCREEN);
    int width = GetSystemMetrics(SM_CXSCREEN);
    initwindow(width, height, "", -3,-3);
    POINT cursorPos;
    float xco = 0.0;
    float yco = 0.0;
    float prevXM;
    float prevYM;
    float differenceX;
    float differenceY;
    bool leftButtonHold=false;
    while(1){
        if((GetKeyState(VK_LBUTTON) & 0x8000) != 0){
            cleardevice();
            GetCursorPos(&cursorPos);
            if(leftButtonHold==false){
                prevXM = cursorPos.x;
                prevYM = cursorPos.y;
                leftButtonHold=true;
            }
            differenceX = prevXM-cursorPos.x;
            differenceY = prevYM-cursorPos.y;
            xco+=differenceX;
            yco+=differenceY;
            prevXM= cursorPos.x;
            prevYM= cursorPos.y;
            std::cout<<cursorPos.x<<","<< cursorPos.y<<endl;
            std::cout<<"xco, yco"<<xco<<","<< yco<<endl;
            circle(cursorPos.x, cursorPos.y, 50);
        }else{
            leftButtonHold=false;
        }
        delay(50);
    }

    getch();
}