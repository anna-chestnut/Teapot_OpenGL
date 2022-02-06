#include <GL/glew.h>
#include <GL/freeglut.h>

#include "cyTriMesh.h"
#include "cyGL.h"
#include "cyMatrix.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx//transform.hpp"

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
int width = 1920, height = 1080;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 60.0f); //glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
glm::mat4 transform = glm::mat4(1.0f);
glm::mat4 rotation = glm::mat4(1.0f);
glm::mat4 transformOrigin = glm::mat4(1.0f);
//glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

bool leftMouseButtonDown = false;
bool rightMouseButtonDown = false;

bool firstLeftMouse = true;
bool firstRightMouse = true;
float yaw = -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float leftLastX = width / 2.0;
float leftLastY = height / 2.0;
float rightLastX = width / 2.0;
float rightLastY = height / 2.0;
float zoom = 0.0f;
//float xoffset = 0.0f;
//float yoffset = 0.0f;

unsigned int shader;
unsigned int numberOfV = 0;
unsigned int numberOfVN = 0;

GLuint pos;
GLuint aNormal;
GLuint vertexBindingIndex = 0;
GLuint normalBindingIndex = 1;

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

void CreateShader()
{

    /*
      GLuint program = glCreateProgram();

      //compile vertex shader
      const char* vsSource = ReadFromFile("shader.vertex");
      GLuint vs = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vs, 1, &vsSource, nullptr);
      glCompileShader(vs);
      glAttachShader(program, vs);
      delete[] vsSource;

      //compile fragment shader
      const char* fsSource = ReadFromFile("shader.frag");
      GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fs, 1, &fsSource, nullptr);
      glCompileShader(fs);
      glAttachShader(program, fs);
      delete[] fsSource;

      glLinkProgram(program);
      */
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

    /*compile shader*/
    /*
      cy::GLSLProgram prog;
      prog.BuildFiles("shader.vert", "shader.frag");
      prog["mpv"] = mymatrix;
      prog.Bind();
      glDrawArrays(...);*/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ShaderProgramSource source = ParseShader("res/shaders/Teapot.shader");
    shader = CreateShader(source.VertexSource, source.FragmentSource);
    GLCall(glUseProgram(shader));

    /*
      cy::GLSLProgram prog;
      prog.BuildFiles("shader.vert", "shader.frag");

      cy::Matrix3f rot = cy::Matrix3f::RotationY(0);
      //cy::Vec3f transVec = cy::Vec3f(0, 0, 0);
      cy::Matrix4f view = cy::Matrix4f::View(cy::Vec3f(0, 0, 80), cy::Vec3f(0, 0, 0), cy::Vec3f(0, 1, 0));
      //cy::Matrix4f trans = cy::Matrix4f::Translation(transVec);
      cy::Matrix4f persProjection = cy::Matrix4f::Perspective(0.698f, float(width) / float(height), 0.1f, 1000.0f);
      cy::Matrix4f model = cy::Matrix4f(1.0f);
      cy::Matrix4f mvp = persProjection * view * model;

      prog["MVP"] = mvp;
      prog.Bind();*/

      /*tell OpenGL how to interpret data from vertex buffer object*/
      //GLCall(GLuint pos = glGetAttribLocation(shader, "pos"));
      //GLCall(glUseProgram(shader));

      /* shader (location = 0) pos */
      // GLCall(glEnableVertexAttribArray(pos));
      // GLCall(GL_ARRAY_BUFFER, vertexbuffer);
      // GLCall(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0));
    GLCall(glVertexArrayBindingDivisor(vao, vertexBindingIndex, 0));
    GLCall(glEnableVertexArrayAttrib(vao, pos));
    /* shader (location = 1) aNoraml */
    // GLCall(glEnableVertexAttribArray(aNormal));
    // GLCall(GL_ARRAY_BUFFER, normalbuffer);
    // GLCall(glVertexAttribPointer(aNormal, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0));
    GLCall(glVertexArrayBindingDivisor(vao, normalBindingIndex, 0));
    GLCall(glEnableVertexArrayAttrib(vao, aNormal));
    /*rendering*/

    /*MVP into vertex shader*/

    glm::vec3 m_position = glm::vec3(0, 0, 80);
    /*
      const float radius = 80.0f;
      float camX = sin(glutGet(GLUT_ELAPSED_TIME)) * radius;
      float camZ = cos(glutGet(GLUT_ELAPSED_TIME)) * radius;*/

      //glm::mat4 view = glm::lookAt(glm::vec3(1, 2, 3), glm::vec3(0, 0, 0), glm::vec3(0, -1, 0));//(1, 2, 3)
      //cameraPos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, zoom)) * vec4(cameraPos, 1);

    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0, 0, 0), cameraUp); //cameraPos + cameraFront glm::vec3(0, 0, 0)

    //glm::mat4 Projection2 = glm::ortho(-4.0f / 3.0f, 4.0f / 3.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    //glm::mat4 Projection = glm::ortho(-(float)width / (float)height, (float)width / (float)height, -1.0f, 1.0f, 0.0f, 100.0f); // In world coordinates
    glm::mat4 Projection = glm::ortho(-30.0f * (float)width / (float)height, 30.0f * (float)width / (float)height, -30.0f, 30.0f, 0.0f, 100.0f); // In world coordinates

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
    glm::mat4 View = glm::lookAt(
        glm::vec3(0, 0, -80), // Camera is at (0,0,80), in World Space
        glm::vec3(0, 0, 0),   // and looks at the origin
        glm::vec3(0, -1, 0)   // Head is up (set to 0,-1,0 to look upside-down)
    );

    glm::mat4 model = glm::mat4(1.0f); //translation*rotation*scale

    //glm::vec4 myVector(10.0f, 10.0f, 10.0f, 0.0f);
    //glm::vec4 transformedVector = myMatrix * myVector;
    /*
      model = glm::translate(model, glm::vec3(xoffset, yoffset, 0.0f));
      model = glm::rotate(model, glm::radians(xoffset), glm::vec3(0.0, 1.0, 1.0));
      model = glm::rotate(model, glm::radians(-yoffset), glm::vec3(1.0, 0.0, 1.0));
      model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
      */
    glm::mat4 mvp = projection * transform * view * rotation * model;

    GLCall(GLuint mvpId = glGetUniformLocation(shader, "MVP"));
    assert(mvpId != -1);
    GLCall(glUniformMatrix4fv(mvpId, 1, GL_FALSE, &mvp[0][0]));

    glm::mat4 mv = view * rotation * model;
    glm::mat3 mat3_mv = {

        mv[0][0], mv[0][1], mv[0][2],
        mv[1][0], mv[1][1], mv[1][2],
        mv[2][0], mv[2][1], mv[2][2] };
    
    glm::mat3 transpose(glm::mat3 mat3_mv);
    GLCall(GLuint mvId = glGetUniformLocation(shader, "MV"));
    assert(mvId != -1);
    GLCall(glUniformMatrix3fv(mvId, 1, GL_TRUE, &mat3_mv[0][0]));

    GLCall(int location = glGetUniformLocation(shader, "u_Color"));
    ASSERT(location != -1);
    GLCall(glUniform4f(location, 1.0f, 1.0f, 1.0f, 1.0f));

    GLCall(glDrawArrays(GL_TRIANGLES, 0, numberOfV));
    //GLCall(glDrawArrays(GL_POINTS, 0, tm.NV()));
    //GLCall(glDrawElements(GL_TRIANGLES, tm.NV(), GL_UNSIGNED_INT, nullptr));

    glDisableVertexAttribArray(0); //from tutorial "pos"
    glDisableVertexAttribArray(1);

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
            //xoffset = 0.0f;
            //yoffset = 0.0f;
        }
    }

    if (button == GLUT_RIGHT_BUTTON)
    {
        // \/ right MouseButton
        rightMouseButtonDown = (state == GLUT_DOWN);
        if (!rightMouseButtonDown)
        {
            firstRightMouse = true;
            //xoffset = 0.0f;
            //yoffset = 0.0f;
        }
    }
}

static void CreateVertexBuffer()
{

    /*vertex buffer object*/
    std::ostream* outString = nullptr;
    //bool readSuccess = tm.LoadFromFileObj("C:\\Users\\123\\Downloads\\teapot.obj", false, outString);
    bool readSuccess = tm.LoadFromFileObj("teapot.obj", false, outString);
    assert(readSuccess);

    std::vector<glm::vec3> vertices;

    for (unsigned int i = 0; i < tm.NF(); i++)
    {

        unsigned int tmp = i;
        cy::TriMesh::TriFace face = tm.F(i);

        vertices.push_back(glm::vec3(tm.V(face.v[0]).x, tm.V(face.v[0]).y, tm.V(face.v[0]).z));
        vertices.push_back(glm::vec3(tm.V(face.v[1]).x, tm.V(face.v[1]).y, tm.V(face.v[1]).z));
        vertices.push_back(glm::vec3(tm.V(face.v[2]).x, tm.V(face.v[2]).y, tm.V(face.v[2]).z));
    }

    std::vector<glm::vec3> verticesNormal;

    for (unsigned int i = 0; i < tm.NF(); i++)
    {

        unsigned int tmp = i;
        cy::TriMesh::TriFace face = tm.FN(i);

        verticesNormal.push_back(glm::vec3(tm.VN(face.v[0]).x, tm.VN(face.v[0]).y, tm.VN(face.v[0]).z));
        verticesNormal.push_back(glm::vec3(tm.VN(face.v[1]).x, tm.VN(face.v[1]).y, tm.VN(face.v[1]).z));
        verticesNormal.push_back(glm::vec3(tm.VN(face.v[2]).x, tm.VN(face.v[2]).y, tm.VN(face.v[2]).z));
    }

    numberOfV = vertices.size();
    numberOfVN = verticesNormal.size();

    /*bind vertex buffer*/
    GLCall(glGenBuffers(1, &vertexbuffer)); //in this case only create 1 buffer
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
    //GLCall(glBufferData(GL_ARRAY_BUFFER, tm.NV() * sizeof(cy::Vec3f), &tm.V(0), GL_STATIC_DRAW));
    GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW));

    /* normal buffer */
    GLCall(glGenBuffers(1, &normalbuffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, normalbuffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, verticesNormal.size() * sizeof(glm::vec3), &verticesNormal[0], GL_STATIC_DRAW));

    /*calculate bounding box*/
    // Cube 1x1x1, centered on origin
    /*GLfloat vertices[] = {
        -0.5, -0.5, -0.5, 1.0,
         0.5, -0.5, -0.5, 1.0,
         0.5,  0.5, -0.5, 1.0,
        -0.5,  0.5, -0.5, 1.0,
        -0.5, -0.5,  0.5, 1.0,
         0.5, -0.5,  0.5, 1.0,
         0.5,  0.5,  0.5, 1.0,
        -0.5,  0.5,  0.5, 1.0,
      };

      GLfloat
          min_x, max_x,
          min_y, max_y,
          min_z, max_z;
      min_x = max_x = tm.V(0).x;
      min_y = max_y = tm.V(0).y;
      min_z = max_z = tm.V(0).z;
      for (int i = 0; i < tm.NV(); i++) {
          if (tm.V(i).x < min_x) min_x = tm.V(i).x;
          if (tm.V(i).x > max_x) max_x = tm.V(i).x;
          if (tm.V(i).y < min_y) min_y = tm.V(i).y;
          if (tm.V(i).y > max_y) max_y = tm.V(i).y;
          if (tm.V(i).z < min_z) min_z = tm.V(i).z;
          if (tm.V(i).z > max_z) max_z = tm.V(i).z;
      }
      glm::vec3 size = glm::vec3(max_x - min_x, max_y - min_y, max_z - min_z);
      glm::vec3 center = glm::vec3((min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2);
      transformOrigin = glm::translate(glm::mat4(1), center) * glm::scale(glm::mat4(1), size);*/
}

static void CreateVertexArrayObject()
{

    /*create vertex array object*/

    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

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
    }

    glutPostRedisplay();
}


int main(int argc, char** argv)
{

    glutInit(&argc, argv);
    glutInitContextFlags(GLUT_DEBUG);
    glutInitWindowSize(width, height);
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

    glClearColor(0, 0, 0, 0);

    CreateVertexBuffer();

    CreateVertexArrayObject();

    glutDisplayFunc(myDisplay);

    glutKeyboardFunc(myKeyboard);
    glutSpecialFunc(mySpecialKeyboard);
    glutMouseFunc(myMouse);
    glutMotionFunc(drag2);
    //glutIdleFunc(myIdle);

    //OpenGL initialization
    //GLuint program = glCreateProgram();

    glutMainLoop();
    return 0;
}