#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "../Shaders/ShaderCompiler.cpp"
#include <random>
#include <cmath>

double CameraX;
double CameraY;
int WorldSize = 100;
const int  Segments = 50;
int OneClick = 0;
int RightClick = 0;
std::vector<std::vector<float>> CircleVectors;

std::vector<GLuint> CircleVAOs;
std::vector<GLuint> CricleVBOs;
GLuint vao , vbo;

     std::vector<float> CircleData(float Radius,float x,float y)
    {
        float Pi = 3.14159;
        x = (x-CameraX)/(WorldSize);
        y = (y-CameraY)/(WorldSize);
        Radius = Radius/(WorldSize);  

        std::vector<float> Verticies;
        Verticies.push_back(x);
        Verticies.push_back(y);
        Verticies.push_back(0.0f);

        for (int i = 0; i <= Segments; i++) // using basic trig to convert points of a cirlce into a neat segments for the draw thing to run
        {
            float angle = (2.0f * Pi * i) / Segments;
            Verticies.push_back(x+(Radius * cos(angle)));
            Verticies.push_back(y+(Radius * sin(angle)));
            Verticies.push_back(0.0f);
        }
        glBufferSubData(GL_ARRAY_BUFFER,0,Verticies.size()*sizeof(float),Verticies.data());
        return Verticies;
    };

    class Circle 
    {
        public:

        double Xpos;
        double Ypos;
        double Xvel;
        double Yvel;
        double Radius;
        float Red;
        float Green;
        float Blue;

        Circle(double X,double Y,double Xv,double Yv,double Rad,float R,float G,float B)
        {
            Xpos = X;
            Ypos = Y;
            Xvel = Xv;
            Yvel = Yv;
            Radius = Rad;
            Red = R;
            Green = G;
            Blue = B;
        }

        void PosChange(double x,double y)
        {
        Xpos += x;
        Ypos += y;
        CircleData(Radius,Xpos,Ypos);
        }
        void VelocityChange(double x,double y)
        {
            Xvel += x;
            Yvel += y;
        }
        void Movement(double Dt)
        {
            Yvel += -9.8*Dt;

            Xpos += Xvel*Dt;
            Ypos += Yvel*Dt;
            CircleData(Radius,Xpos,Ypos);
        }
    };

    std::vector<Circle> Circles;
    void Reflect(int I)
    {
        if (Circles[I].Xpos+Circles[I].Radius > (WorldSize) || Circles[I].Xpos-Circles[I].Radius < -(WorldSize))
        {
            Circles[I].Xvel *= -1;
        }
        if (Circles[I].Ypos+Circles[I].Radius > (WorldSize) || Circles[I].Ypos-Circles[I].Radius < -(WorldSize))
        {
            Circles[I].Yvel *= -0.9;
        }
        if (Circles[I].Xpos+Circles[I].Radius > (WorldSize))
        {
            Circles[I].Xpos = (WorldSize)-Circles[I].Radius;
        }
        if (Circles[I].Xpos-Circles[I].Radius < -(WorldSize))
        {
            Circles[I].Xpos = -(WorldSize)+Circles[I].Radius;
        }
        if (Circles[I].Ypos+Circles[I].Radius > (WorldSize))
        {
            Circles[I].Ypos = (WorldSize)-Circles[I].Radius;
        }
        if (Circles[I].Ypos-Circles[I].Radius < -(WorldSize))
        {
            Circles[I].Ypos = -(WorldSize)+Circles[I].Radius;
        }
    }

    void Collision(int I)
    {
        int X = Circles[I].Xpos;
        int Y = Circles[I].Ypos;
        for (int i = 0; i < Circles.size(); i++)
        {
           if (I!=i)
           {
            double dx = Circles[i].Xpos-X;
            double dy = Circles[i].Ypos-Y;
            double dis = std::sqrt(dx*dx+dy*dy);
            if (dis != 0)
            { 
                double nx = dx / dis;
                double ny = dy / dis;
                if (dis < Circles[I].Radius+Circles[i].Radius)
                {
                    Circles[i].Xvel += nx;
                    Circles[i].Yvel += ny;
                    Circles[I].Xvel += -nx;
                    Circles[I].Yvel += -ny;
                }
            }
           }
        }
    }
    
    void Cleanup(int I)
    {
    Circles.erase(Circles.begin() + I);
    CircleVectors.erase(CircleVectors.begin() + I);
    CircleVAOs.erase(CircleVAOs.begin() + I);
    CricleVBOs.erase(CricleVBOs.begin() + I);
    }

    void RenderCall(Circle& CMain, GLint FragColor, GLuint CircleVBO, GLuint CircleVAO, double Dt)
    {
        glUniform4f(FragColor, CMain.Red, CMain.Green, CMain.Blue, 0.0f);
        glBindBuffer(GL_ARRAY_BUFFER, CircleVBO);
        glBindVertexArray(CircleVAO);
        CMain.Movement(Dt);
        glDrawArrays(GL_TRIANGLE_FAN, 0, Segments + 2);
    }


    void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
    glViewport(0, 0, width, height);
    }

    void CreateCricle(double x,double y,double xv,double yv,double rad,float r,float g,float b)
    {
        Circle C = Circle(x,y,xv,yv,rad,r,g,b);
        Circles.push_back(C);
         //Verticies Setup

    std::vector<float> vertices = CircleData(C.Radius,C.Xpos,C.Ypos);
    CircleVectors.push_back(vertices);

    // Buffer Setup

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);


        CircleVAOs.push_back(vao);
        CricleVBOs.push_back(vbo);
    }
    
    void processInput(GLFWwindow *window,double Dt) // uses window to process input
    {
        std::random_device rd;

        std::mt19937 gen(rd());

        std::uniform_int_distribution<> intDist(-WorldSize, WorldSize);

        if(glfwGetKey(window,GLFW_KEY_ESCAPE) == GLFW_PRESS) // uses glfw to detect key pressed while user is in window
        {
            glfwSetWindowShouldClose(window, true); // closes
        }
        if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS) // uses glfw to detect key pressed while user is in window
        {
            CameraX += +((WorldSize)*Dt); // Changes x vel
        }
        if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS) // uses glfw to detect key pressed while user is in window
        {
            CameraX += -((WorldSize)*Dt); // Changes x vel
        }
        if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS) // uses glfw to detect key pressed while user is in window
        {
            CameraY += +(WorldSize*Dt); // Changes y vel
        }
        if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS) // uses glfw to detect key pressed while user is in window
        {
            CameraY += -(WorldSize*Dt); // Changes y vel
        }
        if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (OneClick == 0)
            {
                CreateCricle(intDist(gen)+CameraX,intDist(gen)+CameraY,intDist(gen),intDist(gen),10,1.0f,1.0f,1.0f);
                OneClick += 1;
            }
        }
        if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
        {
            if (OneClick >= 1)
            {
                OneClick = 0;
            }
        }
                if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            if (RightClick == 0)
            {
                if (Circles.size() >= 1)
                { 
                    Cleanup(0);
                }
                RightClick += 1;
            }
        }
        if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
        {
            if (RightClick >= 1)
            {
                RightClick = 0;
            }
        }
    }

int main() {
     std::random_device rd;

        std::mt19937 gen(rd());

        std::uniform_int_distribution<> intDist(-WorldSize, WorldSize);

    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* CollisionSim = glfwCreateWindow(800, 800, "CollisionSim", nullptr, nullptr);
    if (!CollisionSim) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(CollisionSim);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to load GLAD\n";
        return -1;
    }

    glfwSetFramebufferSizeCallback(CollisionSim, framebuffer_size_callback);


    int shaderProgram = ShaderCompile();
    GLint FragColor = glGetUniformLocation(shaderProgram,"myCustomColor");
    double LastFrame = 0;

    while (!glfwWindowShouldClose(CollisionSim)) {
        double StartFrameTime = glfwGetTime();
        processInput(CollisionSim,StartFrameTime-LastFrame);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        for (size_t i = 0; i < Circles.size(); i++)
        {
        RenderCall(Circles[i], FragColor, CricleVBOs[i], CircleVAOs[i], StartFrameTime - LastFrame);
        if (Circles[i].Xpos+Circles[i].Radius > (WorldSize) || Circles[i].Xpos-Circles[i].Radius < -(WorldSize) || Circles[i].Ypos+Circles[i].Radius > (WorldSize) || Circles[i].Ypos-Circles[i].Radius < -(WorldSize))
        {
            std::cout<<"Reflect"<<i<<std::endl;
           Reflect(i);
        }
        Collision(i);
        }

        glfwSwapBuffers(CollisionSim);

        glfwPollEvents();
        std::cout<<"Frames Per Second:"<<1/(StartFrameTime-LastFrame)<<std::endl;
        LastFrame = StartFrameTime;
    }

    glfwTerminate();
    return 0;
}
