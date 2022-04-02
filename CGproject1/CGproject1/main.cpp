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
GLuint vertexbuffer;
GLuint normalbuffer;
GLuint texturebuffer;
GLuint framebuffer = 0;
GLuint depthbuffer;
GLuint vao;
// plane
GLuint planeVao;
GLuint planeVAO;
GLuint planeVBO;
GLuint planVertexbuffer;
GLuint planNormalbuffer;
GLuint planTexturebuffer;

GLuint texture1;
GLuint spectexture;
GLuint depthMap; 
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
Camera camera(glm::vec3(0.0f, 15.0f, 60.0f));//camera(glm::vec3(0.0f, 15.0f, 60.0f));//60
Camera pointlight(glm::vec3(40.0f, 20.0f, 20.0f));
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
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;


unsigned int teapotShader;
unsigned int shadowMappingShader;
unsigned int planeShader;
unsigned int shadowShader;
unsigned int debugPlaneShader;
unsigned int numberOfV = 0;
//unsigned int numberOfVN = 0;

GLuint pos, aNormal, aTexCoord;
GLuint vertexBindingIndex = 0;
GLuint normalBindingIndex = 1;
GLuint textureBindingIndex = 2;

// teapot
std::vector<float> verticesFloat;
std::vector<glm::vec3> vertices;
std::vector<glm::vec3> verticesNormal;
std::vector<glm::vec3> verticesTexture;
// plane
std::vector<glm::vec3> planeVertices;
std::vector<glm::vec3> planeVerticesNormal;
std::vector<glm::vec2> planVerticesTexture;

// lighting
//glm::vec3 lightPos(-60.0f, 60.0f, 10.0f);//1.2f, 1.0f, 2.0f -60.0f, 45.0f, 20.0f
glm::vec3 lightPosOrigin(-60.0f, 60.0f, 10.0f);
float degree = 0.0f;
float horDegree = 0.0f;

//texture pixels
std::vector<unsigned char> image; //the raw pixels
std::vector<unsigned char> specimage; //the raw pixels

unsigned width, height;

std::string objName;    

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static void GLClearError();
static bool GLLogCall(const char* function, const char* file, int line);
std::vector<unsigned char> decodeTwoSteps(const char* filename, std::vector<unsigned char> decodeImage);
char* ReadFromFile(const std::string & fileName);
static ShaderProgramSource ParseShader(const std::string & filepath);
static unsigned int CompileShader(unsigned int type, const std::string& source);
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
//void ReCompileShader();
void myIdle();
void myKeyboard(unsigned char key, int x, int y);
void myMouse(int button, int state, int x, int y);
void mySpecialKeyboard(int key, int x, int y);
void drag2(int x, int y);
void renderScene(const unsigned int i_shader);
void renderScene2(const unsigned int i_shader);
void renderTeapot();

unsigned int depthMapFBO;
//unsigned int depthMap;
unsigned int simpleDepthShader;
unsigned int debugDepthQuad;

glm::vec3 lightPos(-10.0f, 15.0f, -10.0f); //lightPos(10.0f, 10.0f, 1.0f);
//lightPos(-2.0f, 4.0f, -1.0f);

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


void myDisplay3() {
    // render
    // ------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. render depth of scene to texture (from light's perspective)
    // --------------------------------------------------------------
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 40.5f;//7.5f;
    lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    //glm::mat4 light_view = pointlight.GetViewMatrix();
    //glm::mat4 light_projection = glm::perspective(glm::radians(pointlight.Zoom), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, 1.0f, 7.5f);//0.1f, 100.0f
    //glm::mat4 matrixMLP = light_projection * light_view;

    // render scene from light's point of view
    GLCall(glUseProgram(shadowShader));
    GLCall(GLuint modelId = glGetUniformLocation(shadowShader, "lightSpaceMatrix"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &lightSpaceMatrix[0][0]));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    renderScene2(shadowShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // reset viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    

    // 2. render scene as normal using the generated depth/shadow map  
    // --------------------------------------------------------------
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLCall(glUseProgram(shadowMappingShader)); //shadowMappingShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    // shader.setMat4("projection", projection);
    GLCall(modelId = glGetUniformLocation(shadowMappingShader, "projection"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &projection[0][0]));
    
    //shader.setMat4("view", view);
    GLCall(modelId = glGetUniformLocation(shadowMappingShader, "view"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &view[0][0]));

    // set light uniforms
    
    //shader.setVec3("viewPos", camera.Position);
    GLCall(GLuint location = glGetUniformLocation(shadowMappingShader, "viewPos"));
    assert(location != -1);
    GLCall(glUniform3f(location, camera.Position.x, camera.Position.y, camera.Position.z));

    //shader.setVec3("lightPos", lightPos);
    GLCall(location = glGetUniformLocation(shadowMappingShader, "lightPos"));
    assert(location != -1);
    GLCall(glUniform3f(location, lightPos.x, lightPos.y, lightPos.z));

    //shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    GLCall(modelId = glGetUniformLocation(shadowMappingShader, "lightSpaceMatrix"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &lightSpaceMatrix[0][0]));

    //texture
    GLCall(location = glGetUniformLocation(shadowMappingShader, "diffuseTexture"));
    assert(location != -1);
    GLCall(glUniform1i(location, 0));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    
    GLCall(location = glGetUniformLocation(shadowMappingShader, "shadowMap"));
    assert(location != -1);
    GLCall(glUniform1i(location, 1));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    renderScene2(shadowMappingShader);

    // render Depth map to quad for visual debugging
    // ---------------------------------------------
    GLCall(glUseProgram(debugDepthQuad));
    /*GLCall(GLuint location = glGetUniformLocation(debugDepthQuad, "near_plane"));
    assert(location != -1);
    GLCall(glUniform1f(location, near_plane));
    GLCall(location = glGetUniformLocation(debugDepthQuad, "far_plane"));
    assert(location != -1);
    GLCall(glUniform1f(location, far_plane));*/
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    //renderQuad();

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------

    glutSwapBuffers();
}

void depthMapSetting() {

    //float planeVertices[] = {
    //    // positions            // normals         // texcoords
    //     25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
    //    -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
    //    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

    //     25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
    //    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
    //     25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    //};
    float planeVertices[] = {
        // positions            // normals         // texcoords
         25.0f, -0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };
    // plane VAO
    unsigned int planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    //unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void myDisplay2() {
    //Set frame buffer target & render
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer));
    GLCall(glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    GLCall(glUseProgram(teapotShader));

    glm::mat4 planeview = cameraPlane.GetViewMatrix();//glm::lookAt(cameraPos, glm::vec3(0, 0, 0), cameraUp); //cameraPos + cameraFront glm::vec3(0, 0, 0)
    glm::mat4 planeprojection = glm::perspective(glm::radians(cameraPlane.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
    glm::mat4 planemodel = planerotation * glm::mat4(1.0f); //translation*rotation*scale

    GLCall(GLuint location = glGetUniformLocation(teapotShader, "ambientColor"));
    assert(location != -1);
    GLCall(glUniform3f(location, tm.M(0).Ka[0], tm.M(0).Ka[1], tm.M(0).Ka[2]));

    GLCall(location = glGetUniformLocation(teapotShader, "diffuseColor"));
    assert(location != -1);
    GLCall(glUniform3f(location, tm.M(0).Kd[0], tm.M(0).Kd[1], tm.M(0).Kd[2]));

    GLCall(location = glGetUniformLocation(teapotShader, "specularColor"));
    assert(location != -1);
    GLCall(glUniform3f(location, tm.M(0).Ks[0], tm.M(0).Ks[1], tm.M(0).Ks[2]));

    GLCall(location = glGetUniformLocation(teapotShader, "lightPos"));
    assert(location != -1);
    GLCall(glUniform3f(location, lightPos.x, lightPos.y, lightPos.z));

    GLCall(location = glGetUniformLocation(teapotShader, "specularExponent"));
    assert(location != -1);
    GLCall(glUniform1f(location, tm.M(0).Ns));

    GLCall(location = glGetUniformLocation(teapotShader, "specularStrength"));
    assert(location != -1);
    GLCall(glUniform1f(location, tm.M(0).illum));
    /*
    GLCall(location = glGetUniformLocation(shader, "viewPos"));
    assert(location != -1);
    GLCall(glUniform3f(location, camera.Position.x, camera.Position.y, camera.Position.z));
    */

    //MVP
    GLCall(GLuint modelId = glGetUniformLocation(teapotShader, "model"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &planemodel[0][0]));

    GLCall(GLuint viewId = glGetUniformLocation(teapotShader, "view"));
    assert(viewId != -1);
    GLCall(glUniformMatrix4fv(viewId, 1, GL_FALSE, &planeview[0][0]));

    GLCall(GLuint proId = glGetUniformLocation(teapotShader, "projection"));
    assert(proId != -1);
    GLCall(glUniformMatrix4fv(proId, 1, GL_FALSE, &planeprojection[0][0]));


    //texture
    GLCall(location = glGetUniformLocation(teapotShader, "tex"));
    assert(location != -1);
    GLCall(glUniform1i(location, 0));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    //specular texture
    GLCall(location = glGetUniformLocation(teapotShader, "specTex"));
    assert(location != -1);
    GLCall(glUniform1i(location, 1));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, spectexture);

    GLCall(glBindVertexArray(vao));
    GLCall(glDrawArrays(GL_TRIANGLES, 0, numberOfV));
    glBindVertexArray(0);

    // bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    // ---------------------------------
    GLCall(glGenerateTextureMipmap(depthMap));
    //Set frame buffer target to the back buffer
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originFB));
    //glDisable(GL_DEPTH_TEST);
    GLCall(glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT));
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLCall(glUseProgram(debugPlaneShader));
    GLCall(glBindVertexArray(planeVao));

    /*MVP into vertex shader*/
    glm::mat4 view = camera.GetViewMatrix();//glm::lookAt(cameraPos, glm::vec3(0, 0, 0), cameraUp); //cameraPos + cameraFront glm::vec3(0, 0, 0)
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
    glm::mat4 model = rotation * glm::mat4(1.0f); //translation*rotation*scale
    //MVP
    GLCall(modelId = glGetUniformLocation(debugPlaneShader, "model"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &model[0][0]));

    GLCall(viewId = glGetUniformLocation(debugPlaneShader, "view"));
    assert(viewId != -1);
    GLCall(glUniformMatrix4fv(viewId, 1, GL_FALSE, &view[0][0]));

    GLCall(proId = glGetUniformLocation(debugPlaneShader, "projection"));
    assert(proId != -1);
    GLCall(glUniformMatrix4fv(proId, 1, GL_FALSE, &projection[0][0]));

    GLCall(location = glGetUniformLocation(debugPlaneShader, "planeColor"));
    assert(location != -1);
    GLCall(glUniform3f(location, 0.5f, 0.5f, 0.5f));//1.0f, 0.5f, 0.31f

    //texture
    GLCall(location = glGetUniformLocation(debugPlaneShader, "teapotTexture"));
    assert(location != -1);
    GLCall(glUniform1i(location, 2));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    //GLCall(glBindTexture(GL_TEXTURE_2D, renderedTexture));
    GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));

    glutSwapBuffers();
}

void myDisplay()
{
    // render
    // ------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Set frame buffer target & render the depth map
    // ----------------------------------------------
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer));
    GLCall(glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT));
    GLCall(glClear(GL_DEPTH_BUFFER_BIT));

    glm::mat4 light_view = pointlight.GetViewMatrix();
    glm::mat4 light_projection = glm::perspective(glm::radians(pointlight.Zoom), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, 1.0f, 7.5f);//0.1f, 100.0f
    //glm::mat4 light_model = glm::mat4(1.0f);
    glm::mat4 matrixMLP = light_projection * light_view;

    GLCall(glUseProgram(shadowShader));

    GLCall(GLuint modelId = glGetUniformLocation(shadowShader, "lightSpaceMatrix"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &matrixMLP[0][0]));
     
    //GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0));
    //GLCall(glDrawBuffer(GL_NONE)); // no color buffer is drawn to
    //GLCall(glReadBuffer(GL_NONE));
    renderScene(shadowShader);
    
    // reset viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. render scene as normal using the generated depth/shadow map  
    // --------------------------------------------------------------
    //GLCall(glGenerateTextureMipmap(depthMap));
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originFB));
    GLCall(glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // draw plane
    // ----------
    GLCall(glUseProgram(debugPlaneShader));
    GLCall(glBindVertexArray(planeVao));

    /*MVP into vertex shader*/
    glm::mat4 plane_view = camera.GetViewMatrix();//glm::lookAt(cameraPos, glm::vec3(0, 0, 0), cameraUp); //cameraPos + cameraFront glm::vec3(0, 0, 0)
    glm::mat4 plane_projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
    glm::mat4 plane_model = rotation * glm::mat4(1.0f); //translation*rotation*scale
    //plane_model = glm::translate(plane_model, glm::vec3(10.0f, -20.0f, -80.0f)); 
    //plane_model = glm::rotate(plane_model, glm::radians(-180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    //plane_model = glm::rotate(plane_model, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //MVP
    GLCall(modelId = glGetUniformLocation(debugPlaneShader, "model"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &plane_model[0][0]));

    GLCall(modelId = glGetUniformLocation(debugPlaneShader, "view"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &plane_view[0][0]));

    GLCall(modelId = glGetUniformLocation(debugPlaneShader, "projection"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &plane_projection[0][0]));

    //texture
    GLCall(modelId = glGetUniformLocation(debugPlaneShader, "shadowMap"));
    assert(modelId != -1);
    GLCall(glUniform1i(modelId, 0));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));

    GLCall(glUseProgram(planeShader));

    glm::mat4 camera_view = camera.GetViewMatrix();
    glm::mat4 camera_projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    GLCall(GLuint viewId = glGetUniformLocation(planeShader, "view"));
    assert(viewId != -1);
    GLCall(glUniformMatrix4fv(viewId, 1, GL_FALSE, &camera_view[0][0]));

    GLCall(GLuint proId = glGetUniformLocation(planeShader, "projection"));
    assert(proId != -1);
    GLCall(glUniformMatrix4fv(proId, 1, GL_FALSE, &camera_projection[0][0]));


    GLCall(GLuint location = glGetUniformLocation(planeShader, "lightPos"));
    assert(location != -1);
    GLCall(glUniform3f(location, pointlight.Position.x, pointlight.Position.y, pointlight.Position.z));

    GLCall(location = glGetUniformLocation(planeShader, "viewPos"));
    assert(location != -1);
    GLCall(glUniform3f(location, camera.Position.x, camera.Position.y, camera.Position.z));
    
    //texture
    GLCall( location = glGetUniformLocation(planeShader, "shadowMap"));
    assert(location != -1);
    GLCall(glUniform1i(location, 0));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    //renderScene(planeShader);

    glutSwapBuffers();
    
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// renders the 3D scene
// --------------------
void renderScene2(const unsigned int i_shader)
{
    // floor
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 plane_model = rotation * glm::mat4(1.0f);
    GLCall(GLuint modelId = glGetUniformLocation(i_shader, "model"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &plane_model[0][0]));
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // cubes
    /*model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    GLCall(modelId = glGetUniformLocation(i_shader, "model"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &model[0][0]));
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.5f));
    GLCall(modelId = glGetUniformLocation(i_shader, "model"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &model[0][0]));
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.25));
    GLCall(modelId = glGetUniformLocation(i_shader, "model"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &model[0][0]));
    renderCube();*/

    glm::mat4 teapot_model = rotation * glm::mat4(1.0f);
    teapot_model = glm::rotate(teapot_model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    GLCall(modelId = glGetUniformLocation(i_shader, "model"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &teapot_model[0][0]));
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, numberOfV);
}

void renderScene(const unsigned int i_shader)
{
    // teapot
    // ------
    //glm::mat4 view = camera.GetViewMatrix();
    //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 teapot_model = rotation * glm::mat4(1.0f);
    teapot_model = glm::rotate(teapot_model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    GLCall(GLuint modelId = glGetUniformLocation(i_shader, "model"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &teapot_model[0][0]));
    GLCall(glBindVertexArray(vao));
    GLCall(glDrawArrays(GL_TRIANGLES, 0, numberOfV));

    // draw plane
    // ----------
    GLCall(glBindVertexArray(planeVao));
    glm::mat4 plane_model = rotation * glm::mat4(1.0f); 
    GLCall(modelId = glGetUniformLocation(i_shader, "model"));
    assert(modelId != -1);
    GLCall(glUniformMatrix4fv(modelId, 1, GL_FALSE, &plane_model[0][0]));
    GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));

}

void renderTeapot() 
{
    // initialize (if necessary)
    if (cubeVAO == 0)
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

            verticesFloat.push_back(tm.V(face.v[0]).x);
            verticesFloat.push_back(tm.V(face.v[0]).y);
            verticesFloat.push_back(tm.V(face.v[0]).z);

            face = tm.FN(i);
            verticesFloat.push_back(tm.VN(face.v[0]).x);
            verticesFloat.push_back(tm.VN(face.v[0]).y);
            verticesFloat.push_back(tm.VN(face.v[0]).z);

            face = tm.FT(i);
            verticesFloat.push_back(tm.VT(face.v[0]).x);
            verticesFloat.push_back(tm.VT(face.v[0]).y);
            verticesFloat.push_back(tm.VT(face.v[0]).z);

            // 1
            face = tm.F(i);
            verticesFloat.push_back(tm.V(face.v[1]).x);
            verticesFloat.push_back(tm.V(face.v[1]).y);
            verticesFloat.push_back(tm.V(face.v[1]).z);

            face = tm.FN(i);
            verticesFloat.push_back(tm.VN(face.v[1]).x);
            verticesFloat.push_back(tm.VN(face.v[1]).y);
            verticesFloat.push_back(tm.VN(face.v[1]).z);

            face = tm.FT(i);
            verticesFloat.push_back(tm.VT(face.v[1]).x);
            verticesFloat.push_back(tm.VT(face.v[1]).y);
            verticesFloat.push_back(tm.VT(face.v[1]).z);

            // 2
            face = tm.F(i);
            verticesFloat.push_back(tm.V(face.v[2]).x);
            verticesFloat.push_back(tm.V(face.v[2]).y);
            verticesFloat.push_back(tm.V(face.v[2]).z);

            face = tm.FN(i);
            verticesFloat.push_back(tm.VN(face.v[2]).x);
            verticesFloat.push_back(tm.VN(face.v[2]).y);
            verticesFloat.push_back(tm.VN(face.v[2]).z);

            face = tm.FT(i);
            verticesFloat.push_back(tm.VT(face.v[2]).x);
            verticesFloat.push_back(tm.VT(face.v[2]).y);
            verticesFloat.push_back(tm.VT(face.v[2]).z);
        }

        numberOfV = vertices.size();

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticesFloat), &verticesFloat, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, numberOfV);
    glBindVertexArray(0);
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

        //texture vertices
        face = tm.FT(i);
        verticesTexture.push_back(glm::vec3(tm.VT(face.v[0]).x, tm.VT(face.v[0]).y, tm.VT(face.v[0]).z));
        verticesTexture.push_back(glm::vec3(tm.VT(face.v[1]).x, tm.VT(face.v[1]).y, tm.VT(face.v[1]).z));
        verticesTexture.push_back(glm::vec3(tm.VT(face.v[2]).x, tm.VT(face.v[2]).y, tm.VT(face.v[2]).z));
    }

    numberOfV = vertices.size();
    
    //vertex buffer
    GLCall(glCreateBuffers(1, &vertexbuffer)); //in this case only create 1 buffer
    GLCall(glNamedBufferStorage(vertexbuffer, vertices.size() * sizeof(vertices[0]), &vertices[0], 0));

    //normal buffer
    GLCall(glCreateBuffers(1, &normalbuffer));
    GLCall(glNamedBufferStorage(normalbuffer, verticesNormal.size() * sizeof(verticesNormal[0]), &verticesNormal[0], 0));

    //texture buffer
    GLCall(glCreateBuffers(1, &texturebuffer));
    GLCall(glNamedBufferStorage(texturebuffer, verticesTexture.size() * sizeof(verticesTexture[0]), &verticesTexture[0], 0));

    //// plane vertices
    //// -----------------------
    //float allplaneVertices[] = {
    //    // positions            // normals         // texcoords
    //     25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
    //    -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
    //    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,

    //     25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
    //    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
    //     25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f
    //};

    //for (unsigned int i = 0; i < 48; i = i+8)
    //{
    //    planeVertices.push_back(glm::vec3(allplaneVertices[i], allplaneVertices[i+1], allplaneVertices[i+2]));
    //    planeVerticesNormal.push_back(glm::vec3(allplaneVertices[i + 3], allplaneVertices[i + 4], allplaneVertices[i + 5]));
    //    planVerticesTexture.push_back(glm::vec2(allplaneVertices[i + 6], allplaneVertices[i + 7]));
    //}

    //GLCall(glCreateBuffers(1, &planVertexbuffer));
    //GLCall(glNamedBufferStorage(planVertexbuffer, planeVertices.size() * sizeof(planeVertices[0]), &planeVertices[0], 0));

    //GLCall(glCreateBuffers(1, &planNormalbuffer));
    //GLCall(glNamedBufferStorage(planNormalbuffer, planeVerticesNormal.size() * sizeof(planeVerticesNormal[0]), &planeVerticesNormal[0], 0));

    //GLCall(glCreateBuffers(1, &planTexturebuffer));
    //GLCall(glNamedBufferStorage(planTexturebuffer, planVerticesTexture.size() * sizeof(planVerticesTexture[0]), &planVerticesTexture[0], 0));

    //

}


static void CreateVertexArrayObject()
{

    // create teapot vertex array object
    // ---------------------------------
    GLCall(glCreateVertexArrays(1, &vao));

    pos = 0; // glGetAttribLocation(shader, "pos");
    aNormal = 1; // glGetAttribLocation(shader, "aNormal");
    aTexCoord = 2; // layout(location = 2) in vec2 aTexCoord;

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

    /*bind texturebuffer to vao*/
    GLCall(glVertexArrayVertexBuffer(vao, textureBindingIndex, texturebuffer, 0, sizeof(glm::vec3))); //sizeof(tm.VN(0))
    GLCall(glVertexArrayAttribFormat(vao, aTexCoord, 2, GL_FLOAT, GL_FALSE, 0));                      //sizeof(tm.VN(0))
    GLCall(glVertexArrayAttribBinding(vao, aTexCoord, textureBindingIndex));
    GLCall(glVertexArrayBindingDivisor(vao, textureBindingIndex, 0));
    GLCall(glEnableVertexArrayAttrib(vao, aTexCoord));

    //// create plane vertex array object
    //// --------------------------------
    //GLCall(glCreateVertexArrays(1, &planeVao));

    ////vertex buffer
    //GLCall(glVertexArrayVertexBuffer(planeVao, vertexBindingIndex, planVertexbuffer, 0, sizeof(glm::vec3))); 
    //GLCall(glVertexArrayAttribFormat(planeVao, pos, 3, GL_FLOAT, GL_FALSE, 0));                          
    //GLCall(glVertexArrayAttribBinding(planeVao, pos, vertexBindingIndex));
    //GLCall(glVertexArrayBindingDivisor(planeVao, vertexBindingIndex, 0));
    //GLCall(glEnableVertexArrayAttrib(planeVao, pos));

    ////normal buffer
    //GLCall(glVertexArrayVertexBuffer(planeVao, normalBindingIndex, planNormalbuffer, 0, sizeof(glm::vec3)));
    //GLCall(glVertexArrayAttribFormat(planeVao, aNormal, 3, GL_FLOAT, GL_FALSE, 0));
    //GLCall(glVertexArrayAttribBinding(planeVao, aNormal, normalBindingIndex));
    //GLCall(glVertexArrayBindingDivisor(planeVao, normalBindingIndex, 0));
    //GLCall(glEnableVertexArrayAttrib(planeVao, aNormal));

    ////texture buffer
    //GLCall(glVertexArrayVertexBuffer(planeVao, textureBindingIndex, planTexturebuffer, 0, sizeof(glm::vec2)));
    //GLCall(glVertexArrayAttribFormat(planeVao, aTexCoord, 2, GL_FLOAT, GL_FALSE, 0));                   
    //GLCall(glVertexArrayAttribBinding(planeVao, aTexCoord, textureBindingIndex));
    //GLCall(glVertexArrayBindingDivisor(planeVao, textureBindingIndex, 0));
    //GLCall(glEnableVertexArrayAttrib(planeVao, aTexCoord));

    //
}


static void CreateTexture() {

    // teapot texture
    // --------------
    // 
    // generate diffuse texture
    GLCall(glGenTextures(1, &texture1));
    GLCall(glBindTexture(GL_TEXTURE_2D, texture1));
    /*std::string file = "res/texture";
    std::string diffusePic = tm.M(0).map_Kd.data;
    std::string str = "res/texture/" + diffusePic;
    const char* fileLocation = str.c_str();*/
    image = decodeTwoSteps("res/texture/brick.png", image);//decodeTwoSteps(fileLocation, image);
    assert(&image);
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]));
    GLCall(glGenerateMipmap(GL_TEXTURE_2D));
    
    // set texture filtering parameters
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLCall(glGenTextures(1, &spectexture));
    GLCall(glBindTexture(GL_TEXTURE_2D, spectexture));

    // generate specular texture
    specimage = decodeTwoSteps("res/texture/brick-specular.png", specimage);
    assert(&specimage);
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &specimage[0]));
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
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthMap, 0);
    /*
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);*/

    //glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);

    //glDrawBuffer(GL_NONE);
    //glReadBuffer(GL_NONE);

    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    GLCall(glDrawBuffers(1, drawBuffers));

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SHADOW_WIDTH, SHADOW_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete! 2" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void FramebufferDepth() {

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    //unsigned int textureColorbuffer;
    /*glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthMap, 0);*/
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    //glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    GLCall(glDrawBuffers(1, drawBuffers));

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SHADOW_WIDTH, SHADOW_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete! 2" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

static void LoadShaders() {

    ShaderProgramSource source = ParseShader("res/shaders/Teapot.shader");
    teapotShader = CreateShader(source.VertexSource, source.FragmentSource);
    assert(teapotShader != -1);

    source = ParseShader("res/shaders/DisplayShadow.shader");
    planeShader = CreateShader(source.VertexSource, source.FragmentSource);
    assert(planeShader != -1);

    source = ParseShader("res/shaders/Shadow.shader");
    shadowShader = CreateShader(source.VertexSource, source.FragmentSource);
    assert(shadowShader != -1);

    source = ParseShader("res/shaders/Plane.shader");
    debugPlaneShader = CreateShader(source.VertexSource, source.FragmentSource);
    assert(debugPlaneShader != -1);

    source = ParseShader("res/shaders/debug_quad_depth.shader");
    debugDepthQuad = CreateShader(source.VertexSource, source.FragmentSource);
    assert(debugDepthQuad != -1);

    source = ParseShader("res/shaders/shadow_mapping.shader");//shadow_mapping
    shadowMappingShader = CreateShader(source.VertexSource, source.FragmentSource);
    assert(shadowMappingShader != -1);
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

    CreateVertexArrayObject();

    //Framebuffer();
    depthMapSetting();
    
    CreateTexture();

    LoadShaders();
    //get origin frame buffer
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originFB);

    glutDisplayFunc(myDisplay3);

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

    //switch (key)
    //{
    //case GLUT_KEY_F6: //F6 keycode
    //    //ReCompileShader();
    //    break;
    //}
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
