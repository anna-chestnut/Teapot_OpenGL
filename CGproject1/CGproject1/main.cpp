#include <GL/glew.h>
#include <GL/freeglut.h>

#include "cyTriMesh.h"
#include "cyGL.h"
#include "cyMatrix.h"
#include "res/include/camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#include <iostream>;
#include <fstream>;
#include <string>;
#include <sstream>;
#include <assert.h>

using namespace cy;
#define PR_DEBUG

#ifdef PR_DEBUG
#define ASSERT(x) \
  if (!(x))       \
  __debugbreak()
#define GLCall(x) \
  GLClearError(); \
  x;              \
  ASSERT(GLLogCall(#x, __FILE__, __LINE__));
#else
#define GLCall(x) x
#endif

TriMesh tm;
GLuint vertexbuffer;
GLuint normalbuffer;
GLuint vao;
//int width = 1920, height = 1080;
/*
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 60.0f); //glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));

glm::mat4 transformOrigin = glm::mat4(1.0f);*/
//glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
glm::mat4 transform = glm::mat4(1.0f);
glm::mat4 rotation = glm::mat4(1.0f);

bool leftMouseButtonDown = false;
bool rightMouseButtonDown = false;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 60.0f));
//float lastX = SCR_WIDTH / 2.0f;
//float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
//
bool firstLeftMouse = true;
bool firstRightMouse = true;

float leftLastX = SCR_WIDTH / 2.0;
float leftLastY = SCR_HEIGHT / 2.0;
float rightLastX = SCR_WIDTH / 2.0;
float rightLastY = SCR_HEIGHT / 2.0;
float zoom = 0.0f;

unsigned int shader;
unsigned int numberOfV = 0;
//unsigned int numberOfVN = 0;

GLuint pos;
GLuint aNormal;
GLuint vertexBindingIndex = 0;
GLuint normalBindingIndex = 1;

std::vector<glm::vec3> vertices;
std::vector<glm::vec3> verticesNormal;

// lighting
glm::vec3 lightPos(-60.0f, 45.0f, 20.0f);//1.2f, 1.0f, 2.0f

static void GLClearError()
{

    while (glGetError() != GL_NO_ERROR)
        ;
}

static bool GLLogCall(const char* function, const char* file, int line)
{

    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << ")" << function << " " << file << ":" << line << std::endl;
        return false;
    }

    return true;
}

char* ReadFromFile(const std::string& fileName)
{
    return nullptr;
}

struct ShaderProgramSource
{

    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{

    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {

        if (line.find("#shader") != std::string::npos)
        {

            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else
        {

            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{

    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? "verttex" : "fragment")
            << " shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{

    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void ReCompileShader()
{

    std::cout << "Recompile shader" << std::endl;

    GLsizei maxCount = 2;
    GLsizei count;
    GLuint shaders[2];

    glGetAttachedShaders(shader, maxCount,
        &count,
        shaders);

    for (unsigned int i = 0; i < maxCount; i++)
    {
        glDetachShader(shader, shaders[i]);
    }

    ShaderProgramSource source = ParseShader("res/shaders/Teapot.shader");
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, source.VertexSource);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, source.FragmentSource);

    glAttachShader(shader, vs);
    glAttachShader(shader, fs);
    glLinkProgram(shader);
    glValidateProgram(shader);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void myDisplay()
{

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    GLCall(glUseProgram(shader));


    /*rendering*/

    /*MVP into vertex shader*/
    glm::mat4 view = camera.GetViewMatrix();//glm::lookAt(cameraPos, glm::vec3(0, 0, 0), cameraUp); //cameraPos + cameraFront glm::vec3(0, 0, 0)
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
    glm::mat4 model = rotation * glm::mat4(1.0f); //translation*rotation*scale
    //glm::mat4 mvp = projection * transform * view * rotation * model;
    /*
    GLCall(GLuint mvpId = glGetUniformLocation(shader, "MVP"));
    assert(mvpId != -1);
    GLCall(glUniformMatrix4fv(mvpId, 1, GL_FALSE, &mvp[0][0]));
    */
    /*
    glm::mat4 mv = transform * view * rotation * model;
    glm::mat3 mat3_mv = {

        mv[0][0], mv[0][1], mv[0][2],
        mv[1][0], mv[1][1], mv[1][2],
        mv[2][0], mv[2][1], mv[2][2] 
    };*/
    /*
    glm::mat3 inverse(glm::mat3 mat3_mv);
    glm::mat3 transpose(glm::mat3 mat3_mv);
    GLCall(GLuint mvId = glGetUniformLocation(shader, "MV"));
    assert(mvId != -1);
    GLCall(glUniformMatrix3fv(mvId, 1, GL_FALSE, &mat3_mv[0][0]));
    */
    
    //setting light pos & color
    GLCall(GLuint location = glGetUniformLocation(shader, "objectColor"));
    assert(location != -1);
    GLCall(glUniform3f(location, 1.0f, 0.0f, 0.0f));//1.0f, 0.5f, 0.31f

    GLCall(location = glGetUniformLocation(shader, "lightColor"));
    assert(location != -1);
    GLCall(glUniform3f(location, 1.0f, 1.0f, 1.0f));

    GLCall(location = glGetUniformLocation(shader, "lightPos"));
    assert(location != -1);
    GLCall(glUniform3f(location, lightPos.x, lightPos.y, lightPos.z));
    /*
    GLCall(location = glGetUniformLocation(shader, "viewPos"));
    assert(location != -1);
    GLCall(glUniform3f(location, camera.Position.x, camera.Position.y, camera.Position.z));
    */

    //MVP
    GLCall(GLuint modelId = glGetUniformLocation(shader, "model"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &model[0][0]));

    GLCall(GLuint viewId = glGetUniformLocation(shader, "view"));
    assert(viewId != -1);
    GLCall(glUniformMatrix4fv(viewId, 1, GL_FALSE, &view[0][0]));

    GLCall(GLuint proId = glGetUniformLocation(shader, "projection"));
    assert(proId != -1);
    GLCall(glUniformMatrix4fv(proId, 1, GL_FALSE, &projection[0][0]));


    GLCall(glBindVertexArray(vao));
    GLCall(glDrawArrays(GL_TRIANGLES, 0, numberOfV));

    glutSwapBuffers();
}

float red = 0.0f;

void myIdle()
{

    glClearColor(red, 0, 0, 0);
    red += 0.02;
    if (red >= 2)
        red = 0;

    glutPostRedisplay();
}

void myKeyboard(unsigned char key, int x, int y)
{

    switch (key)
    {
    case 27: //ESC keycode
        glutDestroyWindow(glutGetWindow());
        break;
   
    }
}

void mySpecialKeyboard(int key, int x, int y)
{

    switch (key)
    {
    case GLUT_KEY_F6: //F6 keycode
        ReCompileShader();
        break;
    }
}

void myMouse(int button, int state, int x, int y)
{

    // Save the left button state
    if (button == GLUT_LEFT_BUTTON)
    {
        leftMouseButtonDown = (state == GLUT_DOWN);
        if (!leftMouseButtonDown)
        {
            firstLeftMouse = true;
        }
    }

    if (button == GLUT_RIGHT_BUTTON)
    {
        // \/ right MouseButton
        rightMouseButtonDown = (state == GLUT_DOWN);
        if (!rightMouseButtonDown)
        {
            firstRightMouse = true;
        }
    }
}

static void CreateVertexBuffer()
{

    /*vertex buffer object*/
    std::ostream* outString = nullptr;
    bool readSuccess = tm.LoadFromFileObj("teapot.obj", false, outString);
    assert(readSuccess);


    for (unsigned int i = 0; i < tm.NF(); i++)
    {
        unsigned int tmp = i;
        cy::TriMesh::TriFace face = tm.F(i);

        vertices.push_back(glm::vec3(tm.V(face.v[0]).x, tm.V(face.v[0]).y, tm.V(face.v[0]).z));
        vertices.push_back(glm::vec3(tm.V(face.v[1]).x, tm.V(face.v[1]).y, tm.V(face.v[1]).z));
        vertices.push_back(glm::vec3(tm.V(face.v[2]).x, tm.V(face.v[2]).y, tm.V(face.v[2]).z));
    }


    for (unsigned int i = 0; i < tm.NF(); i++)
    {

        unsigned int tmp = i;
        cy::TriMesh::TriFace face = tm.FN(i);

        verticesNormal.push_back(glm::vec3(tm.VN(face.v[0]).x, tm.VN(face.v[0]).y, tm.VN(face.v[0]).z));
        verticesNormal.push_back(glm::vec3(tm.VN(face.v[1]).x, tm.VN(face.v[1]).y, tm.VN(face.v[1]).z));
        verticesNormal.push_back(glm::vec3(tm.VN(face.v[2]).x, tm.VN(face.v[2]).y, tm.VN(face.v[2]).z));
    }

    numberOfV = vertices.size();
    
    /*bind vertex buffer*/
    GLCall(glCreateBuffers(1, &vertexbuffer)); //in this case only create 1 buffer
    GLCall(glNamedBufferStorage(vertexbuffer, vertices.size() * sizeof(vertices[0]), &vertices[0], 0));

    /* normal buffer */
    GLCall(glCreateBuffers(1, &normalbuffer));
    GLCall(glNamedBufferStorage(normalbuffer, verticesNormal.size() * sizeof(verticesNormal[0]), &verticesNormal[0], 0));

}


static void CreateVertexArrayObject()
{

    /*create vertex array object*/

    GLCall(glCreateVertexArrays(1, &vao));

    pos = 0; // glGetAttribLocation(shader, "pos");
    aNormal = 1; // glGetAttribLocation(shader, "aNormal");

    /*bind vertexbuffer to vao*/
    GLCall(glVertexArrayVertexBuffer(vao, vertexBindingIndex, vertexbuffer, 0, sizeof(glm::vec3))); //sizeof(tm.V(0)
    GLCall(glVertexArrayAttribFormat(vao, pos, 3, GL_FLOAT, GL_FALSE, 0));                          //sizeof(tm.V(0)
    GLCall(glVertexArrayAttribBinding(vao, pos, vertexBindingIndex));
    GLCall(glVertexArrayBindingDivisor(vao, vertexBindingIndex, 0));
    GLCall(glEnableVertexArrayAttrib(vao, pos));

    /*bind normalbuffer to vao*/
    GLCall(glVertexArrayVertexBuffer(vao, normalBindingIndex, normalbuffer, 0, sizeof(glm::vec3))); //sizeof(tm.VN(0))
    GLCall(glVertexArrayAttribFormat(vao, aNormal, 3, GL_FLOAT, GL_FALSE, 0));                      //sizeof(tm.VN(0))
    GLCall(glVertexArrayAttribBinding(vao, aNormal, normalBindingIndex));
    GLCall(glVertexArrayBindingDivisor(vao, normalBindingIndex, 0));
    GLCall(glEnableVertexArrayAttrib(vao, aNormal));
}

void drag2(int x, int y)
{

    if (rightMouseButtonDown)
    {

        //left-top is (0, 0)
        float xpos = static_cast<float>(x);
        float ypos = static_cast<float>(y);

        if (firstRightMouse)
        {
            rightLastX = xpos;
            rightLastY = ypos;
            firstRightMouse = false;
        }

        float xoffset = xpos - rightLastX;
        float yoffset = rightLastY - ypos; // reversed since y-coordinates go from bottom to top
        rightLastX = xpos;
        rightLastY = ypos;

        float sensitivity = 0.015f; // change this value to your liking 0.015
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        zoom = xoffset + yoffset;
        //transform = glm::translate(transform, glm::vec3(xoffset, yoffset, 0.0f));
        transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, zoom));
        camera.ProcessMouseScroll(static_cast<float>(zoom));
    }
    if (leftMouseButtonDown)
    {

        float xpos = static_cast<float>(x);
        float ypos = static_cast<float>(y);

        if (firstLeftMouse)
        {
            leftLastX = xpos;
            leftLastY = ypos;
            firstLeftMouse = false;
        }

        float xoffset = xpos - leftLastX;
        float yoffset = leftLastY - ypos; // reversed since y-coordinates go from bottom to top
        leftLastX = xpos;
        leftLastY = ypos;

        float sensitivity = 0.1f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        rotation = glm::rotate(rotation, glm::radians(xoffset), glm::vec3(0.0f, 1.0f, 0.0f));
        rotation = glm::rotate(rotation, glm::radians(-yoffset), glm::vec3(1.0f, 0.0f, 1.0f));

        //camera.ProcessMouseMovement(xoffset, yoffset);
    }

    glutPostRedisplay();
}


int main(int argc, char** argv)
{

    glutInit(&argc, argv);
    glutInitContextFlags(GLUT_DEBUG);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("Window Title");

    //CY_GL_REGISTER_DEBUG_CALLBACK;

    //Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glEnable(GL_DEPTH_TEST);

    CreateVertexBuffer();

    CreateVertexArrayObject();
    //CreateVAO();

    ShaderProgramSource source = ParseShader("res/shaders/Teapot.shader");
    shader = CreateShader(source.VertexSource, source.FragmentSource);
    assert(shader != -1);

    glutDisplayFunc(myDisplay);

    glutKeyboardFunc(myKeyboard);
    glutSpecialFunc(mySpecialKeyboard);
    glutMouseFunc(myMouse);
    glutMotionFunc(drag2);
    //glutIdleFunc(myIdle);

    glutMainLoop();
    return 0;
}