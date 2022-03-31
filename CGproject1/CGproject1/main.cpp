#include <GL/glew.h>
#include <GL/freeglut.h>

#include "cyTriMesh.h"
#include "cyGL.h"
#include "cyMatrix.h"
#include "res/include/camera.h"
#include "res/include/lodepng.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <assert.h>
#include <iostream>

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
TriMesh cubemap;
GLuint vertexbuffer;
GLuint normalbuffer;
GLuint texturebuffer;
GLuint framebuffer = 0;
GLuint depthbuffer;
GLuint vao;
// plane
GLuint planeVao;
GLuint planVertexbuffer;
GLuint planTexturebuffer;
// geometry
GLuint geometryVao;
GLuint geometryVertexbuffer;
GLuint geometryTexturebuffer;
// cube
GLuint cubemapVertexbuffer;
GLuint cubemapVao;
GLuint texture1;
GLuint spectexture;
GLuint renderedTexture; 
GLuint normalTexture;
GLuint displacementTexture;
GLint originFB;

glm::mat4 transform = glm::mat4(1.0f);
glm::mat4 rotation = glm::mat4(1.0f);
glm::mat4 planetransform = glm::mat4(1.0f);
glm::mat4 planerotation = glm::mat4(1.0f);

bool leftMouseButtonDown = false;
bool rightMouseButtonDown = false;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera cameraPlane(glm::vec3(0.0f, 0.0f, 60.0f));
Camera camera(glm::vec3(0.0f, 0.0f, 60.0f));//60
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
unsigned int normalShader;
unsigned int tessShader;
unsigned int geometryShader;
unsigned int numberOfV = 0;
//unsigned int numberOfVN = 0;

GLuint pos, aNormal, aTexCoord;
GLuint vertexBindingIndex = 0;
GLuint normalBindingIndex = 1;
GLuint textureBindingIndex = 2;

// teapot
std::vector<glm::vec3> vertices;
std::vector<glm::vec3> verticesNormal;
std::vector<glm::vec3> verticesTexture;
// plane
std::vector<glm::vec3> planeVertices;
std::vector<glm::vec2> planVerticesTexture;
// geometry
std::vector<glm::vec3> geometryVertices;

// lighting
glm::vec3 lightPos(20.0f, 20.0f, 10.0f);//1.2f, 1.0f, 2.0f -60.0f, 45.0f, 20.0f
glm::vec3 lightPosOrigin(20.0f, 20.0f, 10.0f);
float degree = 0.0f;
float horDegree = 0.0f;

//texture pixels
std::vector<unsigned char> image; //the raw pixels
std::vector<unsigned char> specimage; //the raw pixels

unsigned width, height;
unsigned int VBO, VAO;

std::string objName;    

bool showTriangle = false;

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

struct ShaderWithGeometryProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
    std::string GeometrySource;
    std::string TessControlSource;
    std::string TessEvaluationSource;
};

static void GLClearError();
static bool GLLogCall(const char* function, const char* file, int line);
std::vector<unsigned char> decodeTwoSteps(const char* filename, std::vector<unsigned char> decodeImage);
char* ReadFromFile(const std::string & fileName);
static ShaderProgramSource ParseShader(const std::string & filepath);
static unsigned int CompileShader(unsigned int type, const std::string& source);
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
static ShaderWithGeometryProgramSource ParseShaderWithGeometry(const std::string& filepath);
static unsigned int CompileShaderWithGeometry(unsigned int type, const std::string& source);
static unsigned int CreateShaderWithGeometry(bool hasGeometry, const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader, const std::string& tessContShader, const std::string& tessEvalShader);
void ReCompileShader();
void myIdle();
void myKeyboard(unsigned char key, int x, int y);
void myMouse(int button, int state, int x, int y);
void mySpecialKeyboard(int key, int x, int y);
void drag2(int x, int y);


void myDisplay()
{
    glClearColor(0, 0, 0, 0);
    //GLCall(glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT));
    glEnable(GL_DEPTH_TEST);
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    
    // draw normal map on the plane
    // ----------------------------
    
    GLCall(glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT));
    glClearColor(0, 0, 0, 0);

    //GLCall(glUseProgram(normalShader));
    //GLCall(glBindVertexArray(planeVao));
    /*MVP into vertex shader*/
    glm::mat4 view = camera.GetViewMatrix();//glm::lookAt(cameraPos, glm::vec3(0, 0, 0), cameraUp); //cameraPos + cameraFront glm::vec3(0, 0, 0)
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
    glm::mat4 model = rotation * glm::mat4(1.0f); //translation*rotation*scale
    ////MVP
    //GLCall(GLuint modelId = glGetUniformLocation(normalShader, "model"));
    //assert(modelId != -1);
    //GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &model[0][0]));

    //GLCall(GLuint viewId = glGetUniformLocation(normalShader, "view"));
    //assert(viewId != -1);
    //GLCall(glUniformMatrix4fv(viewId, 1, GL_FALSE, &view[0][0]));

    //GLCall(GLuint proId = glGetUniformLocation(normalShader, "projection"));
    //assert(proId != -1);
    //GLCall(glUniformMatrix4fv(proId, 1, GL_FALSE, &projection[0][0]));

    //GLCall(GLuint location = glGetUniformLocation(normalShader, "lightPos"));
    //assert(location != -1);
    //GLCall(glUniform3f(location, lightPos.x, lightPos.y, lightPos.z));

    //GLCall(location = glGetUniformLocation(normalShader, "viewPos"));
    //assert(location != -1);
    //GLCall(glUniform3f(location, camera.Position.x, camera.Position.y, camera.Position.z));
    ////texture
    //GLCall(location = glGetUniformLocation(normalShader, "normalMap"));
    //assert(location != -1);
    //GLCall(glUniform1i(location, 0));

    //// bind textures on corresponding texture units
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, normalTexture);
    GLCall(glUseProgram(tessShader));

    GLCall(GLuint modelId = glGetUniformLocation(tessShader, "model"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &model[0][0]));

    GLCall(GLuint viewId = glGetUniformLocation(tessShader, "view"));
    assert(viewId != -1);
    GLCall(glUniformMatrix4fv(viewId, 1, GL_FALSE, &view[0][0]));

    GLCall(GLuint proId = glGetUniformLocation(tessShader, "projection"));
    assert(proId != -1);
    GLCall(glUniformMatrix4fv(proId, 1, GL_FALSE, &projection[0][0]));

    GLCall(GLuint location = glGetUniformLocation(tessShader, "heightMap"));
    assert(location != -1);
    GLCall(glUniform1i(location, 1));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, displacementTexture);


    GLCall(location = glGetUniformLocation(tessShader, "lightPos"));
    assert(location != -1);
    GLCall(glUniform3f(location, lightPos.x, lightPos.y, lightPos.z));

    GLCall(location = glGetUniformLocation(tessShader, "viewPos"));
    assert(location != -1);
    GLCall(glUniform3f(location, camera.Position.x, camera.Position.y, camera.Position.z));
    //texture
    GLCall(location = glGetUniformLocation(tessShader, "normalMap"));
    assert(location != -1);
    GLCall(glUniform1i(location, 0));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, normalTexture);

    GLCall(glBindVertexArray(VAO));
    GLCall(glPatchParameteri(GL_PATCH_VERTICES, 4));
    GLCall(glDrawArrays(GL_PATCHES, 0, 4));


    //GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));

    // use geometry shader
    // -------------------

    if (showTriangle) {
        GLCall(glUseProgram(geometryShader));

        GLCall(modelId = glGetUniformLocation(geometryShader, "model"));
        assert(modelId != -1);
        GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &model[0][0]));

        GLCall(viewId = glGetUniformLocation(geometryShader, "view"));
        assert(viewId != -1);
        GLCall(glUniformMatrix4fv(viewId, 1, GL_FALSE, &view[0][0]));

        GLCall(proId = glGetUniformLocation(geometryShader, "projection"));
        assert(proId != -1);
        GLCall(glUniformMatrix4fv(proId, 1, GL_FALSE, &projection[0][0]));

        GLCall(location = glGetUniformLocation(geometryShader, "heightMap"));
        assert(location != -1);
        GLCall(glUniform1i(location, 1));
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, displacementTexture);


        GLCall(GLuint location = glGetUniformLocation(geometryShader, "lightPos"));
        assert(location != -1);
        GLCall(glUniform3f(location, lightPos.x, lightPos.y, lightPos.z));

        GLCall(location = glGetUniformLocation(geometryShader, "viewPos"));
        assert(location != -1);
        GLCall(glUniform3f(location, camera.Position.x, camera.Position.y, camera.Position.z));
        //texture
        GLCall(location = glGetUniformLocation(geometryShader, "normalMap"));
        assert(location != -1);
        GLCall(glUniform1i(location, 0));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, normalTexture);

        GLCall(glBindVertexArray(VAO));
        GLCall(glPatchParameteri(GL_PATCH_VERTICES, 4));
        GLCall(glDrawArrays(GL_PATCHES, 0, 4));

        //GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));

        //GLCall(glBindVertexArray(geometryVao));
        //GLCall(glDrawArrays(GL_POINTS, 0, 4));
    }
    glutSwapBuffers();

}


static void CreateVertexBuffer()
{

    // teapot vertex array object
    // --------------------------
    std::ostream* outString = nullptr;
    std::string str = "res/texture/" + objName;
    const char* fileLocation = str.c_str();
    std::cout << fileLocation << std::endl;
    bool readSuccess = tm.LoadFromFileObj("res/texture/teapot.obj", true, outString);
    assert(readSuccess);


    for (unsigned int i = 0; i < tm.NF(); i++)
    {
        //vetices
        cy::TriMesh::TriFace face = tm.F(i);
        vertices.push_back(glm::vec3(tm.V(face.v[0]).x, tm.V(face.v[0]).y, tm.V(face.v[0]).z));
        vertices.push_back(glm::vec3(tm.V(face.v[1]).x, tm.V(face.v[1]).y, tm.V(face.v[1]).z));
        vertices.push_back(glm::vec3(tm.V(face.v[2]).x, tm.V(face.v[2]).y, tm.V(face.v[2]).z));

        //normal vertices
        face = tm.FN(i);
        verticesNormal.push_back(glm::vec3(tm.VN(face.v[0]).x, tm.VN(face.v[0]).y, tm.VN(face.v[0]).z));
        verticesNormal.push_back(glm::vec3(tm.VN(face.v[1]).x, tm.VN(face.v[1]).y, tm.VN(face.v[1]).z));
        verticesNormal.push_back(glm::vec3(tm.VN(face.v[2]).x, tm.VN(face.v[2]).y, tm.VN(face.v[2]).z));

        ////texture vertices
        //face = tm.FT(i);
        //verticesTexture.push_back(glm::vec3(tm.VT(face.v[0]).x, tm.VT(face.v[0]).y, tm.VT(face.v[0]).z));
        //verticesTexture.push_back(glm::vec3(tm.VT(face.v[1]).x, tm.VT(face.v[1]).y, tm.VT(face.v[1]).z));
        //verticesTexture.push_back(glm::vec3(tm.VT(face.v[2]).x, tm.VT(face.v[2]).y, tm.VT(face.v[2]).z));
    }

    numberOfV = vertices.size();
    
    //vertex buffer
    GLCall(glCreateBuffers(1, &vertexbuffer)); //in this case only create 1 buffer
    GLCall(glNamedBufferStorage(vertexbuffer, vertices.size() * sizeof(vertices[0]), &vertices[0], 0));

    //normal buffer
    GLCall(glCreateBuffers(1, &normalbuffer));
    GLCall(glNamedBufferStorage(normalbuffer, verticesNormal.size() * sizeof(verticesNormal[0]), &verticesNormal[0], 0));

    ////texture buffer
    //GLCall(glCreateBuffers(1, &texturebuffer));
    //GLCall(glNamedBufferStorage(texturebuffer, verticesTexture.size() * sizeof(verticesTexture[0]), &verticesTexture[0], 0));

    // plane vertices
    // -----------------------
    //float allplaneVertices[] = {
    //    // positions          // texture coords 
    //     20.0f, -0.5f,  20.0f,  1.0f, 0.0f,
    //    -20.0f, -0.5f,  20.0f,  0.0f, 0.0f,
    //    -20.0f, -0.5f, -20.0f,  0.0f, 1.0f,

    //     20.0f, -0.5f,  20.0f,  1.0f, 0.0f,
    //    -20.0f, -0.5f, -20.0f,  0.0f, 1.0f,
    //     20.0f, -0.5f, -20.0f,  1.0f, 1.0f
    //};
    //float allplaneVertices[] = {
    //    // positions          // texture coords 
    //     20.0f, 20.0f,  0.0f,  1.0f, 0.0f,
    //    -20.0f, 20.0f,  0.0f,  0.0f, 0.0f,
    //    -20.0f, -20.0f, 0.0f,  0.0f, 1.0f,

    //     20.0f, 20.0f,  0.0f,  1.0f, 0.0f,
    //    -20.0f, -20.0f, 0.0f,  0.0f, 1.0f,
    //     20.0f, -20.0, 0.0f,  1.0f, 1.0f
    //};
    float allplaneVertices[] = {
        // positions          // texture coords 
        -20.0f, 20.0f,  0.0f,  0.0f, 0.0f,
        -20.0f, -20.0f, 0.0f,  0.0f, 1.0f,
         20.0f, 20.0f,  0.0f,  1.0f, 0.0f,

        -20.0f, -20.0f, 0.0f,  0.0f, 1.0f,
         20.0f, 20.0f,  0.0f,  1.0f, 0.0f,
         20.0f, -20.0, 0.0f,  1.0f, 1.0f
    };

    for (unsigned int i = 0; i < 30; i = i+5)
    {
        planeVertices.push_back(glm::vec3(allplaneVertices[i], allplaneVertices[i+1], allplaneVertices[i+2]));
        planVerticesTexture.push_back(glm::vec2(allplaneVertices[i + 3], allplaneVertices[i + 4]));

        geometryVertices.push_back(glm::vec3(allplaneVertices[i], allplaneVertices[i + 1], allplaneVertices[i + 2]));
    }

    GLCall(glCreateBuffers(1, &planVertexbuffer));
    GLCall(glNamedBufferStorage(planVertexbuffer, planeVertices.size() * sizeof(planeVertices[0]), &planeVertices[0], 0));

    GLCall(glCreateBuffers(1, &planTexturebuffer));
    GLCall(glNamedBufferStorage(planTexturebuffer, planVerticesTexture.size() * sizeof(planVerticesTexture[0]), &planVerticesTexture[0], 0));

    // geometry vertices
    // -----------------------
    //geometryVertices.push_back(glm::vec3(0.0f, 0.0f, 1.0f));

    GLCall(glCreateBuffers(1, &geometryVertexbuffer));
    GLCall(glNamedBufferStorage(geometryVertexbuffer, geometryVertices.size() * sizeof(geometryVertices[0]), &geometryVertices[0], 0));

}


static void CreateVertexArrayObject()
{
    pos = 0; // glGetAttribLocation(shader, "pos");
    aNormal = 1; // glGetAttribLocation(shader, "aNormal");
    aTexCoord = 2; // layout(location = 2) in vec2 aTexCoord;

    // create plan vertex array object
    // -------------------------------
    GLCall(glCreateVertexArrays(1, &planeVao));

    //vertex buffer
    GLCall(glVertexArrayVertexBuffer(planeVao, vertexBindingIndex, planVertexbuffer, 0, sizeof(glm::vec3))); 
    GLCall(glVertexArrayAttribFormat(planeVao, pos, 3, GL_FLOAT, GL_FALSE, 0));                          
    GLCall(glVertexArrayAttribBinding(planeVao, pos, vertexBindingIndex));
    GLCall(glVertexArrayBindingDivisor(planeVao, vertexBindingIndex, 0));
    GLCall(glEnableVertexArrayAttrib(planeVao, pos));

    //texture buffer
    GLCall(glVertexArrayVertexBuffer(planeVao, textureBindingIndex, planTexturebuffer, 0, sizeof(glm::vec2)));
    GLCall(glVertexArrayAttribFormat(planeVao, aTexCoord, 2, GL_FLOAT, GL_FALSE, 0));                   
    GLCall(glVertexArrayAttribBinding(planeVao, aTexCoord, textureBindingIndex));
    GLCall(glVertexArrayBindingDivisor(planeVao, textureBindingIndex, 0));
    GLCall(glEnableVertexArrayAttrib(planeVao, aTexCoord));

    // create quard vertex array object
    // -------------------------------
    GLCall(glCreateVertexArrays(1, &geometryVao));

    //vertex buffer
    GLCall(glVertexArrayVertexBuffer(geometryVao, vertexBindingIndex, geometryVertexbuffer, 0, sizeof(glm::vec3)));
    GLCall(glVertexArrayAttribFormat(geometryVao, pos, 3, GL_FLOAT, GL_FALSE, 0));
    GLCall(glVertexArrayAttribBinding(geometryVao, pos, vertexBindingIndex));
    GLCall(glVertexArrayBindingDivisor(geometryVao, vertexBindingIndex, 0));
    GLCall(glEnableVertexArrayAttrib(geometryVao, pos));

}


static void CreateTexture() {

    // normal texture
    // --------------
    GLCall(glGenTextures(1, &normalTexture));
    GLCall(glBindTexture(GL_TEXTURE_2D, normalTexture));

    // generate specular texture
    specimage = decodeTwoSteps("res/texture/teapot_normal.png", specimage);
    assert(&specimage);
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &specimage[0]));
    GLCall(glGenerateMipmap(GL_TEXTURE_2D));

    // set texture filtering parameters
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // displacement texture
    // --------------
    GLCall(glGenTextures(1, &displacementTexture));
    GLCall(glBindTexture(GL_TEXTURE_2D, displacementTexture));

    // generate specular texture
    image = decodeTwoSteps("res/texture/teapot_disp.png", image);
    assert(&image);
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]));
    GLCall(glGenerateMipmap(GL_TEXTURE_2D));

    // set texture filtering parameters
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}

void Framebuffer() {

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    //unsigned int textureColorbuffer;
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    GLCall(glDrawBuffers(1, drawBuffers));

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

static void LoadShaders() {

    ShaderProgramSource source = ParseShader("res/shaders/Teapot.shader");
    shader = CreateShader(source.VertexSource, source.FragmentSource);
    assert(shader != -1);

    source = ParseShader("res/shaders/NormalMapping.shader");
    normalShader = CreateShader(source.VertexSource, source.FragmentSource);
    assert(normalShader != -1);

    ShaderWithGeometryProgramSource sourceWithG = ParseShaderWithGeometry("res/shaders/Geometry.shader");
    geometryShader = CreateShaderWithGeometry(true, sourceWithG.VertexSource, sourceWithG.FragmentSource, sourceWithG.GeometrySource, sourceWithG.TessControlSource, sourceWithG.TessEvaluationSource);
    assert(geometryShader != -1);

    sourceWithG = ParseShaderWithGeometry("res/shaders/Tessellation.shader");
    tessShader = CreateShaderWithGeometry(false, sourceWithG.VertexSource, sourceWithG.FragmentSource, sourceWithG.GeometrySource, sourceWithG.TessControlSource, sourceWithG.TessEvaluationSource);
    assert(tessShader != -1);
    
}

void CreateBufferTest()
{
    float points[] = {
        //-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
        // 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
        // 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
        //-0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left

         // positions          // texture coords 
        - 20.0f, 20.0f,  0.0f,  0.0f, 1.0f,
        -20.0f, -20.0f, 0.0f,  0.0f, 0.0f,
         20.0f, -20.0, 0.0f,  1.0f, 0.0f,
         20.0f, 20.0f,  0.0f,  1.0f, 1.0f

        //-20.0f, -20.0f, 0.0f,  0.0f, 1.0f,
         //20.0f, 20.0f,  0.0f,  1.0f, 0.0f,
    };
    
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
}

int main(int argc, char** argv)
{
    
    /*for (int i = 0; i < argc; ++i)
        std::cout << argv[i] << "\n";

    objName = argv[1];*/

    glutInit(&argc, argv);
    glutInitContextFlags(GLUT_DEBUG);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("Window Title");

    //CY_GL_REGISTER_DEBUG_CALLBACK;
    glEnable(GL_DEPTH_TEST);

    //Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }


    CreateVertexBuffer();
    CreateBufferTest();

    CreateVertexArrayObject();

    //Framebuffer();
    
    CreateTexture();

    LoadShaders();
    //get origin frame buffer
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originFB);

    glutDisplayFunc(myDisplay);

    glutKeyboardFunc(myKeyboard);
    glutSpecialFunc(mySpecialKeyboard);
    glutMouseFunc(myMouse);
    glutMotionFunc(drag2);
    //glutIdleFunc(myIdle);

    glutMainLoop();
    return 0;
}


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

std::vector<unsigned char> decodeTwoSteps(const char* filename, std::vector<unsigned char> decodeImage) {
    std::vector<unsigned char> png;

    //load and decode
    unsigned error = lodepng::load_file(png, filename);
    if (!error) error = lodepng::decode(decodeImage, width, height, png);

    //if there's an error, display it
    if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    return decodeImage;

    //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
}


char* ReadFromFile(const std::string& fileName)
{
    return nullptr;
}



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

static ShaderWithGeometryProgramSource ParseShaderWithGeometry(const std::string& filepath)
{

    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1,
        GEOMETRY = 2,
        TESSCONTROL = 3,
        TESSEVALUATION = 4
    };

    std::string line;
    std::stringstream ss[5];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {

        if (line.find("#shader") != std::string::npos)
        {

            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
            else if (line.find("geometry") != std::string::npos)
                type = ShaderType::GEOMETRY;
            else if (line.find("tesscontrol") != std::string::npos)
                type = ShaderType::TESSCONTROL;
            else if (line.find("tessevaluation") != std::string::npos)
                type = ShaderType::TESSEVALUATION;
        }
        else
        {

            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str(), ss[2].str(), ss[3].str(), ss[4].str() };
}

static unsigned int CompileShaderWithGeometry(unsigned int type, const std::string& source)
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
        std::string s = "";
        switch (type)
        {
        case GL_VERTEX_SHADER:
            s = "verttex";
            break;
        case GL_FRAGMENT_SHADER:
            s = "fragment";
            break;
        case GL_GEOMETRY_SHADER:
            s = "tess control";
            break;
        case GL_TESS_CONTROL_SHADER:
            s = "tess control";
            break;
        case GL_TESS_EVALUATION_SHADER:
            s = "tess evaluation";
            break;
        default:
            break;
        };

        std::cout << "Failed to compile " <<  s
            
            << " shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static unsigned int CreateShaderWithGeometry(bool hasGeometry, const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader, const std::string& tessContShader, const std::string& tessEvalShader)
{

    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShaderWithGeometry(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShaderWithGeometry(GL_FRAGMENT_SHADER, fragmentShader);
    unsigned int tsc = CompileShaderWithGeometry(GL_TESS_CONTROL_SHADER, tessContShader);
    unsigned int tse = CompileShaderWithGeometry(GL_TESS_EVALUATION_SHADER, tessEvalShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    if (hasGeometry) {
        unsigned int gs = CompileShaderWithGeometry(GL_GEOMETRY_SHADER, geometryShader);
        glAttachShader(program, gs);
        glDeleteShader(gs);
    }
    glAttachShader(program, tsc);
    glAttachShader(program, tse);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
        
    glDeleteShader(tsc);
    glDeleteShader(tse);

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
    case 32: //spacebar
        showTriangle = !showTriangle;
        glutPostRedisplay();
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

        int mods = glutGetModifiers();
        if (mods & GLUT_ACTIVE_ALT) {
            transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, zoom));
            camera.ProcessMouseScroll(static_cast<float>(zoom));
        }
        else {
            planetransform = glm::translate(planetransform, glm::vec3(0.0f, 0.0f, zoom));
            cameraPlane.ProcessMouseScroll(static_cast<float>(zoom));
        }

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

        float sensitivity = 0.01f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        //camera.ProcessMouseMovement(xoffset, yoffset);

        int mods = glutGetModifiers();
        if (mods & GLUT_ACTIVE_CTRL) {

            degree += xoffset;
            glm::vec3 rotate(cos(degree), sin(degree), 1);
            lightPos = rotate * lightPosOrigin;

            horDegree += yoffset;
            glm::vec3 horRotate(1, sin(horDegree), cos(horDegree));
            lightPos = horRotate * lightPosOrigin;
        }
        else {

            planerotation = glm::rotate(planerotation, glm::radians(xoffset), glm::vec3(0.0f, 1.0f, 0.0f));
            planerotation = glm::rotate(planerotation, glm::radians(-yoffset), glm::vec3(1.0f, 0.0f, 1.0f));
        }

        mods = glutGetModifiers();
        if (mods & GLUT_ACTIVE_ALT) {
            rotation = glm::rotate(rotation, glm::radians(xoffset), glm::vec3(0.0f, 1.0f, 0.0f));
            rotation = glm::rotate(rotation, glm::radians(-yoffset), glm::vec3(1.0f, 0.0f, 1.0f));
        }
    }

    glutPostRedisplay();
}
