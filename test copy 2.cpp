#include <iostream>
#include <cstring>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Windows.h"

using namespace std;
using namespace cv;

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

    int pointSize = 8;
    float lineSize = 2;
    int curColor = 1;
    

    int deltax = 0, deltay = 0;

    bool prev = false;
    bool prevz = false;
    int prevx, prevy;

    bool all[361][641][3];
    // bool *fakeAll = (bool*) malloc(722 * 1282 * 3 * sizeof(bool));
    Mat *mm = new Mat(1440, 2560, CV_8UC4, Scalar(0, 0, 0, 0));
    memset(all, 0, sizeof(all));

    while(true){
        long long xposs = 0, yposs = 0;
        int tot = 0;
        cap >> imgf;
        flip(imgf, img, 1);

        for(int a = 0; a < img.rows; a ++){
            for(int b = 0; b < img.cols; b ++){
                Vec3b &col = img.at<Vec3b>(a, b);
                if(((int) col[0]) < col[1] && ((int) col[2]) * 1.5 < col[1] && col[1] > 100){
                    xposs += b;
                    yposs += a;
                    tot ++;
                }
                
                if(a + deltay > 0 && a + deltay < mm->rows && b + deltax > 0 && b + deltax < mm->cols){
                    Vec4b v = mm->at<Vec4b>(a + deltay, b + deltax);
                    if(v[3] > 0){
                        col[0] = v[0];
                        col[1] = v[1];
                        col[2] = v[2];
                    }
                }
            }
        }

        if(tot > 3){
            xposs /= tot;
            yposs /= tot;

            if(xposs > 0 && xposs < 1280 && yposs > 0 && yposs < 720){\
                for(int a = -pointSize; a <= pointSize; a ++){
                    int k = pointSize * pointSize - a * a;
                    for(int b = 0; b * b < k; b ++){
                        Vec3b &v = img.at<Vec3b>(yposs + a, xposs + b);
                        v[0] = 0;
                        v[1] = 255;
                        v[2] = 0;
                        
                        Vec3b &vprime = img.at<Vec3b>(yposs + a, xposs - b);
                        vprime[0] = 0;
                        vprime[1] = 255;
                        vprime[2] = 0;
                    }
                }

                if(GetKeyState('Z') < 0){
                    /* for(int a = -lineSize; a <= lineSize; a ++){
                        int k = sqrt(pow(lineSize, 2) - pow(a, 2));
                        for(int b = -k; b <= k; b ++){
                            all[yposs / 2 + a][xposs / 2 + b][0] = 1;
                        }
                    } */
                    all[yposs / 2][xposs / 2][0] = 1;
                    // Vec4b &v = 
                    if(prevz){
                        int dx = abs(prevx - xposs), dy = abs(prevy - yposs);
                        int sx = prevx > xposs ? 1 : -1;
                        int sy = prevy > yposs ? 1 : -1;
                        int md = max(dx, dy);
                        for(int a = 0; a < md; a ++){
                            all[(yposs + a * (prevy - yposs) / md) / 2][(xposs + a * (prevx - xposs) / md) / 2][0] = true;
                        }
                    }
                    prevz = true;
                }else{
                    prevz = false;
                }

                prevx = xposs;
                prevy = yposs;
            }

            
        }

        for(int a = 0; a < 360; a ++){
            for(int b = 0; b < 640; b ++){
                if(all[a][b][0]){
                    /* Vec3b &v = img.at<Vec3b>(a * 2, b * 2);
                    v[0] = 0;
                    v[1] = 255;
                    v[2] = 0;
                    Vec3b &v1 = img.at<Vec3b>(a * 2 + 1, b * 2);
                    v1[0] = 0;
                    v1[1] = 255;
                    v1[2] = 0;
                    Vec3b &v2 = img.at<Vec3b>(a * 2, b * 2 + 1);
                    v2[0] = 0;
                    v2[1] = 255;
                    v2[2] = 0;
                    Vec3b &v3 = img.at<Vec3b>(a * 2 + 1, b * 2 + 1);
                    v3[0] = 0;
                    v3[1] = 255;
                    v3[2] = 0; */
                    int aa = a * 2, ab = b * 2;
                    for(int k = -lineSize; k <= lineSize; k ++){
                        int mk = sqrt(pow(lineSize, 2) - k * k);
                        for(int i = -mk; i <= mk; i ++){
                            Vec3b &v3 = img.at<Vec3b>(aa + k, ab + i);
                            v3[0] = 0;
                            v3[1] = 255;
                            v3[2] = 0;
                        }
                    }
                }
            }
        }


        imshow("Display Window", img);
        waitKey(20);

        if(getWindowProperty("Display Window", WND_PROP_AUTOSIZE) == -1 || GetKeyState(VK_ESCAPE) < 0)
            break;
    }
    return 0;
}