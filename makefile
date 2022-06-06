INC = -Idependencies/# -IC:/Personal/coding/libraries/glew/glew-2.1.0/include/ -IC:/Personal/coding/libraries/opengl/glfw-3.3.6.bin.WIN64/include/
LIB = -LC:/Personal/Soft/opencv/opencv/build/mingw/install/x64/mingw/lib -lopencv_core455.dll -lopencv_highgui455.dll -lopencv_videoio455.dll# -LC:/Personal/coding/libraries/glew/glew-2.1.0/lib/Release/x64 -lglew32 -LC:/Personal/coding/libraries/opengl/glfw-3.3.6.bin.WIN64/lib-mingw-w64 -lglfw3 -lgdi32 -lopengl32
INCT = -Idependencies/ -IC:/Personal/coding/libraries/glew/glew-2.1.0/include/ -IC:/Personal/coding/libraries/opengl/glfw-3.3.6.bin.WIN64/include/
LIBT = -LC:/Personal/Soft/opencv/opencv/build/mingw/install/x64/mingw/lib -lopencv_core455.dll -lopencv_highgui455.dll -lopencv_videoio455.dll -LC:/Personal/coding/libraries/glew/glew-2.1.0/lib/Release/x64 -lglew32 -LC:/Personal/coding/libraries/opengl/glfw-3.3.6.bin.WIN64/lib-mingw-w64 -lglfw3 -lgdi32 -lopengl32

all: test


test: build/test.o
	g++ build/test.o -o build/test.exe $(LIB)

build/test.o: test.cpp
	g++ test.cpp -c -o build/test.o $(INC)

fake: fake.cpp
	g++ -I"C:/Personal/Soft/opencv/opencv/build/mingw/install/include" -L"C:/Personal/Soft/opencv/opencv/build/mingw/install/x64/mingw/bin" -L"C:/Personal/Soft/opencv/opencv/build/mingw/install/x64/mingw/lib" -lopencv_core455 -lopencv_highgui455 -lopencv_imgproc455 -lopencv_imgcodecs455 fake.cpp -o f.exe

testc: testc.cpp
	g++ testc.cpp -o build/testc.exe $(LIBT) $(INCT)

clean:
	rm *.o
	rm *.exe