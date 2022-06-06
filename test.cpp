#include <iostream>
#include <cstring>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Windows.h"

using namespace std;
using namespace cv;


Mat *mm;
float lineSize = 3;
int eraseSize = 10;
Vec4b v4b(0, 255, 0, 255);
int curC[4] = {0, 255, 0, 255};
Vec3b v3b(0, 255, 0);
Vec4b clrV(0, 0, 0, 0);

void drawPoint(int xpos, int ypos){
    for(int a = -lineSize; a <= lineSize; a ++){
        int k = lineSize * lineSize - a * a;
        for(int b = 0; b * b < k; b ++){
            if(xpos + b > 0 && xpos + b < mm->cols && ypos + a > 0 && ypos + a < mm->rows){
                mm->at<Vec4b>(ypos + a, xpos + b) = v4b;
            }
        }
    }
}

void ErasePoint(int xpos, int ypos){
    for(int a = -eraseSize; a <= eraseSize; a ++){
        if(xpos + a > 0 && xpos + a < mm->cols)
            for(int b = -eraseSize; b <= eraseSize; b ++){
                if(ypos + b > 0 && ypos + b < mm->rows)
                    mm->at<Vec4b>(ypos + a, xpos + b) = clrV;
            }
    }
};

int main(){
    Mat img, imgf;
    namedWindow("Display Window");
    
    setWindowProperty("Display Window", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);

    VideoCapture cap(0);
    cap.set(CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(CAP_PROP_FRAME_WIDTH, 1280);

    if (!cap.isOpened()){
        cout << "cannot open camera";
    }

    int pointSize = 6;
    lineSize = 2;
    int curColor = 1;
    

    int deltax = 0, deltay = 0;

    bool prev = false;
    bool prevz = false;
    bool prevErase = false;
    int prevx, prevy;

    mm = new Mat(1440, 2560, CV_8UC4, Scalar(0, 0, 0, 0));

    while(true){
        try{
            long long xposs = 0, yposs = 0;
            int tot = 0;
            cap >> imgf;
            flip(imgf, img, 1);

            for(int a = 0; a < img.rows; a ++){
                for(int b = 0; b < img.cols; b ++){
                    Vec3b col = img.at<Vec3b>(a, b);
                    if(((int) col[0]) < col[1] && ((int) col[2]) * 1.5 < col[1] && col[1] > 100){
                        xposs += b;
                        yposs += a;
                        tot ++;
                    }
                    
                    if(a + deltay > 0 && a + deltay < mm->rows && b + deltax > 0 && b + deltax < mm->cols){
                        Vec4b v = mm->at<Vec4b>(a + deltay, b + deltax);
                        if(v[3] > 0){
                            Vec3b &vtemp = img.at<Vec3b>(a, b);
                            for(int fak = 0; fak < 3; fak ++)
                                vtemp[fak] = v[fak];
                        }
                    }
                }
            }

            if(tot > 3){
                xposs /= tot;
                yposs /= tot;

                if(xposs > 0 && xposs < img.cols && yposs > 0 && yposs < img.rows){
                    for(int a = -pointSize; a <= pointSize; a ++){
                        int k = sqrt(pointSize * pointSize - a * a);
                        for(int b = -k; b < k; b ++){
                            if(yposs + a > 0 && yposs + a < img.rows && xposs + b > 0 && xposs + b < img.cols){
                                img.at<Vec3b>(yposs + a, xposs + b) = v3b;
                            }
                        }
                    }

                    if(GetKeyState('Z') < 0){
                        drawPoint(xposs + deltax, yposs + deltay);
                        if(prevz){
                            int dx = abs(prevx - xposs), dy = abs(prevy - yposs);
                            int md = max(dx, dy);
                            for(int a = 0; a <= md; a ++){
                                int px = xposs + a * (prevx - xposs) / md + deltax, py =  yposs + a * (prevy - yposs) / md + deltay;
                                if(px > 0 && px < mm->cols && py > 0 && py < mm->rows){
                                    drawPoint(px, py);
                                }
                            }
                        }
                        prevz = true;
                    }else{
                        prevz = false;
                    }

                    if(GetKeyState('X') < 0){
                        ErasePoint(xposs + deltax, yposs + deltay);
                        if(prevErase){
                            int dx = abs(prevx - xposs), dy = abs(prevy - yposs);
                            int md = max(dx, dy);
                            for(int a = 0; a <= md; a ++){
                                int px = xposs + a * (prevx - xposs) / md + deltax, py =  yposs + a * (prevy - yposs) / md + deltay;
                                if(px > 0 && px < mm->cols && py > 0 && py < mm->rows){
                                    ErasePoint(px, py);
                                }
                            }
                        }
                        prevErase = true;
                    }else
                        prevErase = false;

                    prevx = xposs;
                    prevy = yposs;
                }
            }



            imshow("Display Window", img);
            waitKey(30);

            // if(GetKeyState(VK_ESCAPE) < 0)
            //     break;
                
        }catch(int i){
            
        }
    }

    cout << "terminated\n";
    return 0;
}