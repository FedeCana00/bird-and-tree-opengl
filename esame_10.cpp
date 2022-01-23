#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>

#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "model_renderer.h"
#include "create_shader_program.h"
#include "compute_normals.h"
#include "load_texture.h"
#include "assimp_geometry.h"
#include "cylinder_geometry.h"
#include "sphere_geometry.h"
#include "init_window.h"
#include "cone_geometry.h"

unsigned int scr_width = 600;
unsigned int scr_height = 600;

int shaderProgram;
GLint transformationUniformLocation;
GLint modelviewUniformLocation;
// light properties
GLint lightPositionUniformLocation;
GLint lightAmbientUniformLocation;
GLint lightDiffuseUniformLocation;
GLint lightSpecularUniformLocation;
// material properties
GLint shininessUniformLocation;
GLint colorSpecularUniformLocation;
GLint colorEmittedUniformLocation;
GLint hasTextureUniformLocation;
GLint stateTree;

GLint colorTextureUniformLocation;

ModelRenderer * tree;
ModelRenderer * nest;
ModelRenderer * body;
ModelRenderer * head;
ModelRenderer * mouth;
ModelRenderer * wing;

float bird_angle = 0.0f;
float wing_angle = 0.0f;
int wing_direction = 1.0;
int bird_direction = 1.0;
int state_tree = 0;

bool are_wings_moving = true;
bool is_bird_rotating = true;

glm::mat4 inputModelMatrix = glm::mat4(1.0);

void load_matrices(glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix)
{
  glm::mat4 transf = projection_matrix * view_matrix * model_matrix;
  glm::mat4 modelview = view_matrix * model_matrix;
  glUniformMatrix4fv(transformationUniformLocation, 1, GL_FALSE, glm::value_ptr(transf));
  glUniformMatrix4fv(modelviewUniformLocation, 1, GL_FALSE, glm::value_ptr(modelview));
}

void display_bird(glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 parent_model, GLFWwindow* window)
{
    glm::mat4 bird_matrix = parent_model;
    bird_matrix = glm::rotate(bird_matrix, bird_angle, glm::vec3(0.0f, 0.0f, 1.0f));
    bird_matrix = glm::translate(bird_matrix, glm::vec3(-7.5f, 0.0f, 0.0f));

    //mouth
    glm::mat4 mouth_matrix = bird_matrix;
    mouth_matrix = glm::rotate(mouth_matrix, glm::pi<float>() / 2, glm::vec3(0.0f, 1.0f, 0.0f));
    load_matrices(projection_matrix, view_matrix, mouth_matrix);
    mouth->render();

    //head
    glm::mat4 head_matrix = bird_matrix;
    head_matrix = glm::translate(head_matrix, glm::vec3(-1.0f, 0.0f, 0.0f));
    load_matrices(projection_matrix, view_matrix, head_matrix);
    head->render();

    //body
    glm::mat4 body_matrix = bird_matrix;
    body_matrix = glm::translate(body_matrix, glm::vec3(-3.0f, 0.0f, 0.0f));
    body_matrix = glm::rotate(body_matrix, glm::pi<float>() / 2, glm::vec3(0.0f, 1.0f, 0.0f));
    load_matrices(projection_matrix, view_matrix, body_matrix);
    body->render();

    //wings left
    glm::mat4 wing_left = bird_matrix;
    wing_left = glm::rotate(wing_left, wing_angle, glm::vec3(1.0f, 0.0f, 0.0f));
    wing_left = glm::translate(wing_left, glm::vec3(-3.0f, 1.25f, 0.0f));
    wing_left = glm::scale(wing_left, glm::vec3(1.0f, 1.0f, 0.3f / 2.0f));
    load_matrices(projection_matrix, view_matrix, wing_left);
    wing->render();

    //wings right
    glm::mat4 wing_right = bird_matrix;
    wing_right = glm::rotate(wing_right, -wing_angle, glm::vec3(1.0f, 0.0f, 0.0f));
    wing_right = glm::translate(wing_right, glm::vec3(-3.0f, -1.25f, 0.0f));
    wing_right = glm::scale(wing_right, glm::vec3(1.0f, 1.0f, 0.3f / 2.0f));
    load_matrices(projection_matrix, view_matrix, wing_right);
    wing->render();
}

void display(GLFWwindow* window)
{
    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    const float PI = std::acos(-1.0f);
    glm::mat4 model_matrix = glm::mat4(1.0);
    model_matrix = inputModelMatrix * model_matrix;

    glm::mat4 view_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -35.0f));

    glm::mat4 projection_matrix = glm::perspective(glm::pi<float>() / 4.0f, float(scr_width) / float(scr_height), 1.0f, 75.0f);

    glm::vec3 light_position(0.56f, -0.78f, -0.29f);
    glm::vec3 light_camera_position = glm::vec3(view_matrix * glm::vec4(light_position, 1.0f));
    glm::vec3 light_ambient(0.2f, 0.2f, 0.2f);
    glm::vec3 light_diffuse(0.6f, 0.6f, 0.6f);
    glm::vec3 light_specular(0.4f, 0.4f, 0.4f);
    glUniform3fv(lightPositionUniformLocation, 1, glm::value_ptr(light_camera_position));
    glUniform3fv(lightAmbientUniformLocation, 1, glm::value_ptr(light_ambient));
    glUniform3fv(lightDiffuseUniformLocation, 1, glm::value_ptr(light_diffuse));
    glUniform3fv(lightSpecularUniformLocation, 1, glm::value_ptr(light_specular));

    GLfloat shininess = 25.0f;
    glUniform1f(shininessUniformLocation, shininess);
    glUniform3f(colorSpecularUniformLocation, 0.7f, 0.7f, 0.7f);
    glUniform3f(colorEmittedUniformLocation, 0.0f, 0.0f, 0.0f);

    glUniform1i(colorTextureUniformLocation, 0);

    glUniform1i(stateTree, state_tree);

    load_matrices(projection_matrix, view_matrix, model_matrix);
    tree->render();

    load_matrices(projection_matrix, view_matrix, model_matrix);
    nest->render();

    display_bird(projection_matrix, view_matrix, model_matrix, window);


    glfwSwapBuffers(window);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    scr_width = width;
    scr_height = height;

    glViewport(0, 0, width, height);
    display(window);
}

void window_refresh_callback(GLFWwindow* window)
{
    display(window);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
        are_wings_moving = !are_wings_moving;

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
        is_bird_rotating = !is_bird_rotating;

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
        bird_direction = bird_direction == 1.0 ? -1.0 : 1.0;

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
        state_tree = state_tree == 0.0 ? 1.0 : 2.0;
}

void mouse_cursor_callback(GLFWwindow * window, double xpos, double ypos)
{
    static float prev_x = -1.0; // position at previous iteration (-1 for none)
    static float prev_y = -1.0;
    const float SPEED = 0.005f; // rad/pixel

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (prev_x >= 0.0f && prev_y >= 0.0f)   // if there is a previously stored position
        {
            float xdiff = float(xpos) - prev_x; // compute diff
            float ydiff = float(ypos) - prev_y;
            float delta_y = SPEED * ydiff;
            float delta_x = SPEED * xdiff;

            glm::mat4 rot = glm::mat4(1.0);    // rotate matrix
            rot = glm::rotate(rot, delta_x, glm::vec3(0.0, 1.0, 0.0));
            rot = glm::rotate(rot, delta_y, glm::vec3(1.0, 0.0, 0.0));
            inputModelMatrix = rot * inputModelMatrix;
        }

        prev_x = float(xpos); // store mouse position for next iteration
        prev_y = float(ypos);
    }
    else
    {
        prev_x = -1.0f; // mouse released: reset
        prev_y = -1.0f;
    }
}

void advance(GLFWwindow* window, double time_diff)
{
    float delta_x = 0.0;
    float delta_y = 0.0;
    const float speed = 0.5;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        delta_y = -1.0;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        delta_x = -1.0;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        delta_y = 1.0;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        delta_x = 1.0;
    delta_y *= speed * float(time_diff);
    delta_x *= speed * float(time_diff);

    glm::mat4 rot = glm::mat4(1.0);
    rot = glm::rotate(rot, delta_x, glm::vec3(0.0, 1.0, 0.0));
    rot = glm::rotate(rot, delta_y, glm::vec3(1.0, 0.0, 0.0));
    inputModelMatrix = rot * inputModelMatrix;

    const float bird_speed = 0.40f;
    const float wing_speed = 0.5f;

    if(is_bird_rotating)
        bird_angle += bird_speed * time_diff * bird_direction;
    if (bird_angle > 2 * glm::pi<float>())
        bird_angle = 0;
    if (bird_angle <= 0)
        bird_angle = 2* glm::pi<float>();

    if (are_wings_moving)
        wing_angle += wing_speed * time_diff * wing_direction;
    if (wing_angle >= glm::pi<float>() / 4.0f)
        wing_direction = -1.0;
    if (wing_angle <= -glm::pi<float>() / 4.0f)
        wing_direction = 1.0;
}

class NestGeometry : public IGeometry
{
public:
    NestGeometry()
    {
        const int VERTICES_SIZE = 24;
        const int FACES_SIZE = 12;
        m_vertices_size = VERTICES_SIZE;
        m_size = FACES_SIZE * 3;

        static GLfloat avertices[] = {
            -0.5, -0.5, 7,
            0.5, -0.5, 7,
            0.5, 0.5, 7,
            -0.5, 0.5, 7,

            -1, -1, 6,
            1, -1, 6,
            1, 1, 6,
            -1, 1, 6,
        };
        static GLfloat acolors[] = {
            1, 0.5, 0,
            1, 0.5, 0,
            1, 0.5, 0,
            1, 0.5, 0,

            1, 0.5, 0,
            1, 0.5, 0,
            1, 0.5, 0,
            1, 0.5, 0,
        };

        static GLuint indices[] = {
            0, 1, 2, //top
            0, 2, 3,

            5, 6, 1, //front
            5, 1, 0,

            6, 7, 2, //rigth
            6, 2, 1,

            4, 5, 0, //left
            4, 0, 3,

            7, 4, 3, //back
            7, 3, 2,

            5, 4, 7, //bottom
            5, 7, 6,
        };

        m_vertices = avertices;
        m_faces = indices;

        static GLfloat anormals[VERTICES_SIZE * 3];
        m_normals = anormals;

        for (int i = 0; i < FACES_SIZE; i++)
        {
            int v0 = indices[i * 3 + 0]; // i-th face, first vertex index
            int v1 = indices[i * 3 + 1]; // i-th face, second vertex index
            int v2 = indices[i * 3 + 2]; // i-th face, third vertex index

            float x0 = avertices[v0 * 3 + 0]; // x of first vertex
            float y0 = avertices[v0 * 3 + 1]; // y of first vertex
            float z0 = avertices[v0 * 3 + 2]; // z of first vertex

            float x1 = avertices[v1 * 3 + 0]; // x of second vertex
            float y1 = avertices[v1 * 3 + 1]; // y of second vertex
            float z1 = avertices[v1 * 3 + 2]; // z of second vertex

            float x2 = avertices[v2 * 3 + 0]; // x of third vertex
            float y2 = avertices[v2 * 3 + 1]; // y of third vertex
            float z2 = avertices[v2 * 3 + 2]; // z of third vertex

            glm::vec3 normal = computeNormal(glm::vec3(x0, y0, z0),
                glm::vec3(x1, y1, z1),
                glm::vec3(x2, y2, z2));

            m_normals[v0 * 3 + 0] = normal.x; // set normal for first vertex
            m_normals[v0 * 3 + 1] = normal.y;
            m_normals[v0 * 3 + 2] = normal.z;

            m_normals[v1 * 3 + 0] = normal.x; // set normal for second vertex
            m_normals[v1 * 3 + 1] = normal.y;
            m_normals[v1 * 3 + 2] = normal.z;

            m_normals[v2 * 3 + 0] = normal.x; // set normal for third vertex
            m_normals[v2 * 3 + 1] = normal.y;
            m_normals[v2 * 3 + 2] = normal.z;
        }

    }

    ~NestGeometry()
    {
    }

    const GLfloat* vertices()
    {
        return m_vertices;
    }
    const GLfloat* colors()
    {
        static GLfloat acolors[] = {
            1, 0.5, 0,
            1, 0.5, 0,
            1, 0.5, 0,
            1, 0.5, 0,

            1, 0.5, 0,
            1, 0.5, 0,
            1, 0.5, 0,
            1, 0.5, 0,
        };

        return acolors;
    }

    const GLfloat* normals()
    {
        return m_normals;
    }

    GLsizei verticesSize() { return m_vertices_size; }
    const GLuint* faces()
    {
        return m_faces;
    }

    GLsizei size() { return m_size; }

    GLenum type() { return GL_TRIANGLES; }

private:
    GLfloat* m_vertices;
    GLfloat* m_normals;

    GLuint* m_faces;

    GLsizei m_vertices_size;
    GLsizei m_size;
};

class WingGeometry : public IGeometry
{
public:
    WingGeometry()
    {
        const int VERTICES_SIZE = 24;
        const int FACES_SIZE = 12;
        m_vertices_size = VERTICES_SIZE;
        m_size = FACES_SIZE * 3;

        static GLfloat avertices[] = {
            -1.0,  -1.0,   1.0,
             1.0,  -1.0,   1.0,
             1.0,   1.0,   1.0,
            -1.0,   1.0,   1.0,

             1.0,  -1.0,   1.0,
             1.0,  -1.0,  -1.0,
             1.0,   1.0,  -1.0,
             1.0,   1.0,   1.0,

            -1.0,   1.0,   1.0,
             1.0,   1.0,   1.0,
             1.0,   1.0,  -1.0,
            -1.0,   1.0,  -1.0,

            -1.0,  -1.0,   1.0,
            -1.0,  -1.0,  -1.0,
            -1.0,   1.0,  -1.0,
            -1.0,   1.0,   1.0,

            -1.0,  -1.0,   1.0,
             1.0,  -1.0,   1.0,
             1.0,  -1.0,  -1.0,
            -1.0,  -1.0,  -1.0,

            -1.0,  -1.0,  -1.0,
             1.0,  -1.0,  -1.0,
             1.0,   1.0,  -1.0,
            -1.0,   1.0,  -1.0,
        };

        static GLuint indices[] = {
            0, 1, 2, // front
            0, 2, 3, // front

            4, 5, 6, // right
            4, 6, 7, // right

            8, 9,10, // top
            8,10,11, // top

           12,14,13, // left
           12,15,14, // left

           16,18,17, // bottom
           16,19,18, // bottom

           20,23,21, // back
           21,23,22, // back
        };

        m_vertices = avertices;
        m_faces = indices;

        static GLfloat anormals[VERTICES_SIZE * 3];
        m_normals = anormals;

        for (int i = 0; i < FACES_SIZE; i++)
        {
            int v0 = indices[i * 3 + 0]; // i-th face, first vertex index
            int v1 = indices[i * 3 + 1]; // i-th face, second vertex index
            int v2 = indices[i * 3 + 2]; // i-th face, third vertex index

            float x0 = avertices[v0 * 3 + 0]; // x of first vertex
            float y0 = avertices[v0 * 3 + 1]; // y of first vertex
            float z0 = avertices[v0 * 3 + 2]; // z of first vertex

            float x1 = avertices[v1 * 3 + 0]; // x of second vertex
            float y1 = avertices[v1 * 3 + 1]; // y of second vertex
            float z1 = avertices[v1 * 3 + 2]; // z of second vertex

            float x2 = avertices[v2 * 3 + 0]; // x of third vertex
            float y2 = avertices[v2 * 3 + 1]; // y of third vertex
            float z2 = avertices[v2 * 3 + 2]; // z of third vertex

            glm::vec3 normal = computeNormal(glm::vec3(x0, y0, z0),
                glm::vec3(x1, y1, z1),
                glm::vec3(x2, y2, z2));

            m_normals[v0 * 3 + 0] = normal.x; // set normal for first vertex
            m_normals[v0 * 3 + 1] = normal.y;
            m_normals[v0 * 3 + 2] = normal.z;

            m_normals[v1 * 3 + 0] = normal.x; // set normal for second vertex
            m_normals[v1 * 3 + 1] = normal.y;
            m_normals[v1 * 3 + 2] = normal.z;

            m_normals[v2 * 3 + 0] = normal.x; // set normal for third vertex
            m_normals[v2 * 3 + 1] = normal.y;
            m_normals[v2 * 3 + 2] = normal.z;
        }

    }

    ~WingGeometry()
    {
    }

    const GLfloat* vertices()
    {
        return m_vertices;
    }
    const GLfloat* colors()
    {
        static GLfloat acolors[] = {
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,

            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,

            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,

            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,

            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,

            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
            0.7,   0.7,   0.7,
        };

        return acolors;
    }

    const GLfloat* normals()
    {
        return m_normals;
    }

    GLsizei verticesSize() { return m_vertices_size; }
    const GLuint* faces()
    {
        return m_faces;
    }

    GLsizei size() { return m_size; }

    GLenum type() { return GL_TRIANGLES; }

private:
    GLfloat* m_vertices;
    GLfloat* m_normals;

    GLuint* m_faces;

    GLsizei m_vertices_size;
    GLsizei m_size;
};

int main()
{
    GLFWwindow * window = init_window(scr_width, scr_height, "Test exam 10 Federico Canali");

    // callbacks
    // ---------
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowRefreshCallback(window, window_refresh_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_cursor_callback);

    AssimpGeometry tree_geo("src/p10_tree.ply");
    ModelRenderer tree_geo_renderer(tree_geo);
    tree = &tree_geo_renderer;

    NestGeometry nest_geo;
    ModelRenderer nest_geo_renderer(nest_geo);
    nest = &nest_geo_renderer;

    CylinderGeometry body_geo(0.5f, 3.0f, glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f));
    ModelRenderer body_geo_renderer(body_geo);
    body = &body_geo_renderer;

    SphereGeometry head_geo(0.5f, glm::vec3(0.5f));
    ModelRenderer head_geo_renderer(head_geo);
    head = &head_geo_renderer;

    ConeGeometry mouth_geo(0.25f, 1.0f, glm::vec3(1.0f, 0.7f, 0.0f), glm::vec3(1.0f, 0.7f, 0.0f));
    ModelRenderer mouth_geo_renderer(mouth_geo);
    mouth = &mouth_geo_renderer;

    WingGeometry wing_geo;
    ModelRenderer wing_geo_renderer(wing_geo);
    wing = &wing_geo_renderer;

    // load GLSL shaders
    shaderProgram = createShaderProgram("esame_10.vert", "esame_10.frag");
    transformationUniformLocation = glGetUniformLocation(shaderProgram, "transformation");
    modelviewUniformLocation = glGetUniformLocation(shaderProgram, "modelview");
 
    lightPositionUniformLocation = glGetUniformLocation(shaderProgram, "light_position");
    lightAmbientUniformLocation = glGetUniformLocation(shaderProgram, "light_ambient");
    lightDiffuseUniformLocation = glGetUniformLocation(shaderProgram, "light_diffuse");
    lightSpecularUniformLocation = glGetUniformLocation(shaderProgram, "light_specular");

    shininessUniformLocation = glGetUniformLocation(shaderProgram, "shininess");
    colorSpecularUniformLocation = glGetUniformLocation(shaderProgram, "color_specular");
    colorEmittedUniformLocation = glGetUniformLocation(shaderProgram, "color_emitted");

    colorTextureUniformLocation = glGetUniformLocation(shaderProgram, "color_texture");

    hasTextureUniformLocation = glGetUniformLocation(shaderProgram, "has_texture");
    stateTree = glGetUniformLocation(shaderProgram, "state_tree");

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // enable back face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // enable depth test
    glEnable(GL_DEPTH_TEST);

    // render loop
    // -----------
    double curr_time = glfwGetTime();
    double prev_time;
    while (!glfwWindowShouldClose(window))
    {
        display(window);
        glfwWaitEventsTimeout(0.01);

        prev_time = curr_time;
        curr_time = glfwGetTime();
        advance(window, curr_time - prev_time);
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
