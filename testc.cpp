#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <Windows.h>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
using namespace cv;


GLFWwindow* GLwindow;

uchar allColors[6][4] = {{0, 255, 0, 255}, 
    {255, 0, 0, 255},
    {0, 0, 255, 255},
    {255, 255, 0, 255},
    {255, 0, 255, 255},
    {0, 255, 255, 255},
};

int ac[6][4] = {{0, 255, 0, 255}, 
    {255, 0, 0, 255},
    {0, 0, 255, 255},
    {255, 255, 0, 255},
    {255, 0, 255, 255},
    {0, 255, 255, 255},
};

int currentColorN = 0;
int currentCanvasN = 0;
int cursorColorLoc;
unsigned int crossPro;
unsigned int erasePro;
unsigned int dtexture;
unsigned int dpro;

int imgChannels, nRows, nCols;
int mChannels, mRows, mCols;

float lineSize = 1.5, eraseSize = 20;

int eraseSizeLoc, crossLoc;
int dispLoc;
int deltax = 0, deltay = 0;


Mat* canvases[3];

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods){
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(GLwindow, GLFW_TRUE);
    if(key == GLFW_KEY_D && action == GLFW_PRESS){
        currentColorN = (currentColorN + 1);
        if(currentColorN >= 6)
            currentColorN -= 6;
        glUseProgram(crossPro);
        glUniform4f(cursorColorLoc, ac[currentColorN][0] / 255.0f, ac[currentColorN][1] / 255.0f, ac[currentColorN][2] / 255.0f, ac[currentColorN][3] / 255.0f);
    }
    if(key == GLFW_KEY_A && action == GLFW_PRESS){
        if(eraseSize > 2){
            eraseSize --;
            glUseProgram(erasePro);
            float eraseXLen = eraseSize * 2.0 / nCols, eraseYLen = eraseSize * 2 / nRows;
            glUniform2f(eraseSizeLoc, eraseXLen, eraseYLen);
        }
    }
    if(key == GLFW_KEY_S && action == GLFW_PRESS){
        eraseSize ++;
        glUseProgram(erasePro);
        float eraseXLen = eraseSize * 2.0 / nCols, eraseYLen = eraseSize * 2 / nRows;
        glUniform2f(eraseSizeLoc, eraseXLen, eraseYLen);
    }
    if(key == GLFW_KEY_Q && action == GLFW_PRESS){
        if(lineSize > 0.5){
            lineSize -= 0.5;
            
            glUseProgram(crossPro);
            float cursorXLen = lineSize * 2.0 / nCols, cursorYLen = lineSize * 2 / nRows;
            glUniform2f(crossLoc, cursorXLen, cursorYLen);
        }
    }
    if(key == GLFW_KEY_W && action == GLFW_PRESS){
        lineSize += 0.5;
        glUseProgram(crossPro);
        float cursorXLen = lineSize * 2.0 / nCols, cursorYLen = lineSize * 2 / nRows;
        glUniform2f(crossLoc, cursorXLen, cursorYLen);
    }
    if(key == GLFW_KEY_E && action == GLFW_PRESS){
        currentCanvasN ++;
        if(currentCanvasN > 2)
            currentCanvasN -= 3;
        glBindTexture(GL_TEXTURE_2D, dtexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mCols, mRows, 0, GL_RGBA, GL_UNSIGNED_BYTE, canvases[currentCanvasN]->data);
        glGenerateMipmap(GL_TEXTURE_2D);
        deltax = mCols - nCols;
        deltay = 0;
        glUseProgram(dpro);
        glUniform2f(dispLoc, 1 - (float) nCols / mCols, 0);
    }
}




bool getMousePos(Mat &m, int &px, int &py){
    // int xposs[nCols + 2], yposs[nRows + 2];
    // memset(xposs, 0, sizeof(xposs));
    // memset(yposs, 0, sizeof(yposs));
    long long xposs = 0, yposs = 0;
    int tot = 0;

    int curPos = 0;
    uchar* p = m.data;
    for(int a = 0; a < nRows; a ++){
        for(int b = 0; b < nCols; b ++){
            if(p[0] * 1.3 < (p[1]) && ((int) (p[2])) * 1.5 < (p[1]) && (p[1]) > 100){
                p[0] = 0;
                p[1] = 0;
                p[2] =  255;

                xposs += b;
                yposs += a;
                tot ++;
            }
            p += 3;
        }
    }
    px = -1;
    py = -1;
    if(tot < 3)
        return false;
    px = xposs / tot;
    py = yposs / tot;
    return true;
}


uchar erColor[4] = {255, 255, 255, 0};

void drawPoint(Mat &m, int &px, int &py){
    for(int a = -lineSize; a <= lineSize; a ++){
        if(py + a > 5 && py + a < mRows){
            uchar* d = m.ptr<uchar>(py - a) + px * 4;
            int k = sqrt(lineSize * lineSize - a * a);
            for(int b = -k; b <= k; b ++){
                if(px + b > 5 && px + b < mCols){
                    memcpy(d - b * 4, allColors[currentColorN], 4 * sizeof(uchar));
                }
            }
        }
    }
}


void drawLine(Mat &m, int &px, int &py, int ex, int ey, bool erase){
    if(px != ex || py != ey){
        int wid = lineSize;
        uchar* c = allColors[currentColorN];
        if(erase){
            wid = eraseSize;
            c = erColor;
        }
        int dx = abs(px - ex), dy = abs(py - ey);
        int md = max(dx, dy);
        for(int a = 0; a <= md; a ++){
            int ppx = px + a * (ex - px) / md, ppy =  py + a * (ey - py) / md;
            drawPoint(m, ppx, ppy);
        }
    }
}

int main(){
    if (!glfwInit()){
        printf("\nInitialization of OpenGL Failed\n");
        return -1;
    }
    const GLFWvidmode* vm = glfwGetVideoMode(glfwGetPrimaryMonitor());
    GLwindow = glfwCreateWindow(vm->width, vm->height, "Test Window", glfwGetPrimaryMonitor(), NULL);

    if (!GLwindow){
        std::cout << "\nwindow load failed";
        return -2;
    }
    glfwMakeContextCurrent(GLwindow);
    glfwSwapInterval(1);
    glfwSetKeyCallback(GLwindow, key_callback);
    if(glewInit() != GLEW_OK){
        std::cout << "Glew init failed";
        exit(2);
    }


    Mat imgf;
    bool prevDraw = false, prevMove = false;
    int prevx, prevy;
    int pprevx, pprevy;

    for(int a = 0; a < 3; a ++){   
        canvases[a] = new Mat(1200, 1600, CV_8UC4, Scalar(255, 255, 255, 0));
    }
    mChannels = canvases[0]->channels();
    mRows = canvases[0]->rows;
    mCols = canvases[0]->cols;

    VideoCapture cap(0);
    cap.set(CAP_PROP_FRAME_HEIGHT, 750);
    cap.set(CAP_PROP_FRAME_WIDTH, 1000);

    if (!cap.isOpened()){
        cout << "cannot open camera";
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //the buffer for the image
    float pos[] = {
        -1, -1, 1, 1,
        1, -1, 0, 1,
        -1, 1, 1, 0,
        1, 1, 0, 0
    };
    unsigned int vb;
    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), pos, GL_DYNAMIC_DRAW);
    


    //vertex att array for the image
    unsigned int imVAO;
    glGenVertexArrays(1, &imVAO);
    glBindVertexArray(imVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));


    float crossPos[] = {0, 0};
    unsigned int crossVB, crossVAO;
    glGenVertexArrays(1, &crossVAO);
    glBindVertexArray(crossVAO);
    glGenBuffers(1, &crossVB);
    glBindBuffer(GL_ARRAY_BUFFER, crossVB);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float), crossPos, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);


    //create shader for the drawing
    const char *dvst = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 tPos;\n"
    "uniform vec2 disp;"
    "out vec2 texpos;"
    "void main()\n"
    "{\n"
    "   texpos = tPos + disp;"
    "   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
    "}\0";
    unsigned int dvshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(dvshader, 1, &dvst, nullptr);
    glCompileShader(dvshader);
    int result;
    glGetShaderiv(dvshader, GL_COMPILE_STATUS, &result);
    if(!result){
        int length;
        glGetShaderiv(dvshader, GL_INFO_LOG_LENGTH, &length);
        char* msg = (char*) alloca(length * sizeof(char));
        glGetShaderInfoLog(dvshader, length, &length, msg);
        std::cout << "Compile failed Drawing ";
        std::cout << "V shader";
        std::cout << msg << '\n';
        glDeleteShader(dvshader);
    }

    //create shader for the image
    const char *vst = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 tPos;\n"
    "out vec2 texpos;"
    "void main()\n"
    "{\n"
    "   texpos = tPos;"
    "   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
    "}\0";

    unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vst, nullptr);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &result);
    if(!result){
        int length;
        glGetShaderiv(vshader, GL_INFO_LOG_LENGTH, &length);
        char* msg = (char*) alloca(length * sizeof(char));
        glGetShaderInfoLog(vshader, length, &length, msg);
        std::cout << "Compile failed ";
        std::cout << "V shader";
        std::cout << msg << '\n';
        glDeleteShader(vshader);
    }

    const char* fst = "#version 330 core\n"
    "in vec2 texpos;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D ourTexture;\n"
    "void main(){\n"
    "   FragColor = texture(ourTexture, texpos);\n"
    "}\0";
    unsigned int fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fst, nullptr);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &result);
    if(!result){
        int length;
        glGetShaderiv(fshader, GL_INFO_LOG_LENGTH, &length);
        char* msg = (char*) alloca(length * sizeof(char));
        glGetShaderInfoLog(fshader, length, &length, msg);
        std::cout << "Compile failed ";
        std::cout << "F shader";
        std::cout << msg << '\n';
        glDeleteShader(fshader);
    }

    unsigned int pro = glCreateProgram();
    glAttachShader(pro, fshader);
    glAttachShader(pro, vshader);
    glLinkProgram(pro);
    glUseProgram(pro);

    //drawing pro
    dpro = glCreateProgram();
    glAttachShader(dpro, fshader);
    glAttachShader(dpro, dvshader);
    glLinkProgram(dpro);
    dispLoc = glGetUniformLocation(dpro, "disp");
    glUseProgram(dpro);
    glUniform2f(dispLoc, 0, 0);
    


    //shader for the cross
    const char* crossVS = "#version 330 core\n"
    "layout(location = 0) in vec2 posi;\n"
    "void main(){\n"
    "   gl_Position = vec4(posi, 0.0, 1.0);\n"
    "}\0";
    unsigned int crossV = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(crossV, 1, &crossVS, nullptr);
    glCompileShader(crossV);
    glGetShaderiv(crossV, GL_COMPILE_STATUS, &result);
    if(!result){
        int length;
        glGetShaderiv(crossV, GL_INFO_LOG_LENGTH, &length);
        char* msg = (char*) alloca(length * sizeof(char));
        glGetShaderInfoLog(crossV, length, &length, msg);
        std::cout << "Compile failed ";
        std::cout << "Cross V shader";
        std::cout << msg << '\n';
        glDeleteShader(crossV);
    }

    const char* crossFS = "#version 330 core\n"
    "out vec4 col;\n"
    "uniform vec4 colo;"
    "void main(){\n"
    "   col = colo;\n"
    "}\0";
    unsigned int crossF = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(crossF, 1, &crossFS, nullptr);
    glCompileShader(crossF);
    glGetShaderiv(crossF, GL_COMPILE_STATUS, &result);
    if(!result){
        int length;
        glGetShaderiv(crossF, GL_INFO_LOG_LENGTH, &length);
        char* msg = (char*) alloca(length * sizeof(char));
        glGetShaderInfoLog(crossF, length, &length, msg);
        std::cout << "Compile failed ";
        std::cout << "Cross F shader";
        std::cout << msg << '\n';
        glDeleteShader(crossF);
    }

    
    const char* crossGS = "#version 330 core\n"
    "layout(points) in;\n"
    "layout (triangle_strip, max_vertices = 4) out;\n"
    "uniform vec2 cursorSize;"
    "void main(){\n"
    "   gl_Position = gl_in[0].gl_Position + vec4(-cursorSize, 0, 0);\n"
    "   EmitVertex();\n"
    "   gl_Position = gl_in[0].gl_Position + vec4(cursorSize.x, -cursorSize.y, 0, 0);\n"
    "   EmitVertex();\n"
    "   gl_Position = gl_in[0].gl_Position + vec4(-cursorSize.x, cursorSize.y, 0, 0);\n"
    "   EmitVertex();\n"
    "   gl_Position = gl_in[0].gl_Position + vec4(cursorSize, 0, 0);\n"
    "   EmitVertex();\n"
    "   EndPrimitive();"
    "}\0";
    unsigned int crossG = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(crossG, 1, &crossGS, nullptr);
    glCompileShader(crossG);
    glGetShaderiv(crossG, GL_COMPILE_STATUS, &result);
    if(!result){
        int length;
        glGetShaderiv(crossG, GL_INFO_LOG_LENGTH, &length);
        char* msg = (char*) alloca(length * sizeof(char));
        glGetShaderInfoLog(crossG, length, &length, msg);
        std::cout << "Compile failed ";
        std::cout << "Cross G shader";
        std::cout << msg << '\n';
        glDeleteShader(crossG);
    }

    //erase shaders
    const char* eraseGS = "#version 330 core\n"
    "layout(points) in;\n"
    "layout (line_strip, max_vertices = 5) out;\n"
    "uniform vec2 boxSize;\n"
    "void main(){\n"
    "   gl_Position = gl_in[0].gl_Position + vec4(-boxSize, 0, 0);\n"
    "   EmitVertex();\n"
    "   gl_Position = gl_in[0].gl_Position + vec4(-boxSize.x, boxSize.y, 0, 0);\n"
    "   EmitVertex();\n"
    "   gl_Position = gl_in[0].gl_Position + vec4(boxSize, 0, 0);\n"
    "   EmitVertex();\n"
    "   gl_Position = gl_in[0].gl_Position + vec4(boxSize.x, -boxSize.y, 0, 0);\n"
    "   EmitVertex();\n"
    "   gl_Position = gl_in[0].gl_Position + vec4(-boxSize, 0, 0);\n"
    "   EmitVertex();\n"
    "   EndPrimitive();"
    "}\0";
    unsigned int eraseG = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(eraseG, 1, &eraseGS, nullptr);
    glCompileShader(eraseG);
    glGetShaderiv(eraseG, GL_COMPILE_STATUS, &result);
    if(!result){
        int length;
        glGetShaderiv(eraseG, GL_INFO_LOG_LENGTH, &length);
        char* msg = (char*) alloca(length * sizeof(char));
        glGetShaderInfoLog(eraseG, length, &length, msg);
        std::cout << "Compile failed ";
        std::cout << "erase G shader";
        std::cout << msg << '\n';
        glDeleteShader(eraseG);
    }

    const char* eraseFS = "#version 330 core\n"
    "out vec4 col;\n"
    "void main(){\n"
    "   col = vec4(0, 0, 0.0, 1.0);\n"
    "}\0";
    unsigned int eraseF = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(eraseF, 1, &eraseFS, nullptr);
    glCompileShader(eraseF);
    glGetShaderiv(eraseF, GL_COMPILE_STATUS, &result);
    if(!result){
        int length;
        glGetShaderiv(eraseF, GL_INFO_LOG_LENGTH, &length);
        char* msg = (char*) alloca(length * sizeof(char));
        glGetShaderInfoLog(eraseF, length, &length, msg);
        std::cout << "Compile failed ";
        std::cout << "erase F shader";
        std::cout << msg << '\n';
        glDeleteShader(eraseF);
    }


    crossPro = glCreateProgram();
    glAttachShader(crossPro, crossF);
    glAttachShader(crossPro, crossV);
    glAttachShader(crossPro, crossG);
    glLinkProgram(crossPro);
    cursorColorLoc = glGetUniformLocation(crossPro, "colo");
    crossLoc = glGetUniformLocation(crossPro, "cursorSize");
    glUseProgram(crossPro);
    glUniform4f(cursorColorLoc, ac[currentColorN][0] / 255.0f, ac[currentColorN][1] / 255.0f, ac[currentColorN][2] / 255.0f, ac[currentColorN][3] / 255.0f);


    erasePro = glCreateProgram();
    glAttachShader(erasePro, eraseF);
    glAttachShader(erasePro, crossV);
    glAttachShader(erasePro, eraseG);
    glLinkProgram(erasePro);

    glLineWidth(2);
    


    //Image texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    float borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


    //drawing texture
    dtexture;
    glGenTextures(1, &dtexture);
    glBindTexture(GL_TEXTURE_2D, dtexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mCols, mRows, 0, GL_RGBA, GL_UNSIGNED_BYTE, canvases[currentCanvasN]->data);
    glGenerateMipmap(GL_TEXTURE_2D);


    //initialize the dimensions
    cap >> imgf;
    nRows = imgf.rows;
    nCols = imgf.cols;
    imgChannels = imgf.channels();
    assert(imgf.isContinuous());
    float eraseXLen = eraseSize * 2.0 / nCols, eraseYLen = eraseSize * 2 / nRows;
    eraseSizeLoc = glGetUniformLocation(erasePro, "boxSize");
    glUseProgram(erasePro);
    glUniform2f(eraseSizeLoc, eraseXLen, eraseYLen);

    deltax = mCols - nCols;
    deltay = 0;
    glUseProgram(dpro);
    glUniform2f(dispLoc, 1 - (float) nCols / mCols, 0);

    


    glUseProgram(crossPro);
    float cursorXLen = lineSize * 2.0 / nCols, cursorYLen = lineSize * 2 / nRows;
    glUniform2f(crossLoc, cursorXLen, cursorYLen);

    //the buffer for the image
    float mpos[] = {
        -1, -1, ((float) nCols) / mCols, ((float) nRows) / mRows,
        1, -1, 0, ((float) nRows) / mRows,
        -1, 1, ((float) nCols) / mCols, 0,
        1, 1, 0, 0
    };

    unsigned int mvb;
    glGenBuffers(1, &mvb);
    glBindBuffer(GL_ARRAY_BUFFER, mvb);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), mpos, GL_DYNAMIC_DRAW);
    unsigned int dVAO;
    glGenVertexArrays(1, &dVAO);
    glBindVertexArray(dVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mvb);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));

    namedWindow("w");

    cout << nRows << ' ' << nCols << '\n';

    while(!glfwWindowShouldClose(GLwindow)){
        int mx, my;
        cap >> imgf;


        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(pro);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(imVAO);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nCols, nRows, 0, GL_BGR, GL_UNSIGNED_BYTE, imgf.data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        if(getMousePos(imgf, mx, my)){
            int pmx = mx, pmy = my;
            glBindVertexArray(crossVAO);
            glBindBuffer(GL_ARRAY_BUFFER, crossVB);
            crossPos[0] = -mx * 2.0 / nCols + 1;
            crossPos[1] = -my * 2.0 / nRows + 1;
            glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float), crossPos, GL_DYNAMIC_DRAW);
            
            glUseProgram(crossPro);
            if(GetKeyState('X') < 0)
                glUseProgram(erasePro);
            glDrawArrays(GL_POINTS, 0, 1);

            mx += deltax;
            my += deltay;


            if(GetKeyState('Z') < 0){
                drawPoint(*(canvases[currentCanvasN]), mx, my);
                if(prevDraw){
                    drawLine(*canvases[currentCanvasN], prevx, prevy, mx, my, false);
                }
                prevDraw = true;
                glBindTexture(GL_TEXTURE_2D, dtexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mCols, mRows, 0, GL_RGBA, GL_UNSIGNED_BYTE, canvases[currentCanvasN]->data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }else{
                prevDraw = false;
            }

            if(GetKeyState('X') < 0){
                for(int a = -eraseSize; a <= eraseSize; a ++){
                    if(a + my > 5 && a + my < mRows - 5){
                        uchar* pt = canvases[currentCanvasN]->ptr<uchar>(a + my)+ mx * 4;
                        for(int b = - eraseSize; b <= eraseSize; b ++){
                            if(b + mx > 5 && b + mx < mCols - 5){
                                memcpy(pt + b * 4, erColor, 4 * sizeof(uchar));
                            }
                        }
                    }
                }
                glBindTexture(GL_TEXTURE_2D, dtexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mCols, mRows, 0, GL_RGBA, GL_UNSIGNED_BYTE, canvases[currentCanvasN]->data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }

            if(GetKeyState('C') < 0){
                if(prevMove){
                    deltax += pprevx - pmx;
                    deltay += pprevy - pmy;

                    glUseProgram(dpro);
                    glUniform2f(dispLoc, ((float) deltax) / mCols, ((float) deltay) / mRows);
                }
                prevMove = true;
            }else
                prevMove = false;

            prevx = mx;
            pprevx = pmx;
            prevy = my;
            pprevy = pmy;
        }
        glBindTexture(GL_TEXTURE_2D, dtexture);
        glUseProgram(dpro);
        glBindVertexArray(dVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mvb);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        imshow("w", *canvases[currentCanvasN]);

        
        while(GLenum err = glGetError()){
            std::cout << "[OpenGL Error] " << err << '\n';
        }
        glfwPollEvents();
        glfwSwapBuffers(GLwindow);
    }

    glfwTerminate();
    return 0;
}