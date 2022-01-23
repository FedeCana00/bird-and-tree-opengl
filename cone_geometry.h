#ifndef CONE_GEOMETRY_H
#define CONE_GEOMETRY_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "model_renderer.h"
#include "compute_normals.h"

class ConeGeometry : public IGeometry
{
public:
    ConeGeometry(float radius, float height, glm::vec3 color_base, glm::vec3 color_side)
    {
        const int SAMPLES = 20;
        m_vertices = new GLfloat[3 * (3 * SAMPLES + 1)];
        m_colors = new GLfloat[3 * (3 * SAMPLES + 1)];
        m_normals = new GLfloat[3 * (3 * SAMPLES + 1)];
        m_faces = new GLuint[3 * (2 * SAMPLES)];

        int face_counter = 0;
        int vertex_counter = 0;

        int zero_vertex;
        // base
        zero_vertex = vertex_counter;

        m_vertices[vertex_counter * 3 + 0] = 0.0f;
        m_vertices[vertex_counter * 3 + 1] = 0.0f;
        m_vertices[vertex_counter * 3 + 2] = -height / 2.0f;

        m_colors[vertex_counter * 3 + 0] = color_base.r;
        m_colors[vertex_counter * 3 + 1] = color_base.g;
        m_colors[vertex_counter * 3 + 2] = color_base.b;

        m_normals[vertex_counter * 3 + 0] = 0.0f;
        m_normals[vertex_counter * 3 + 1] = 0.0f;
        m_normals[vertex_counter * 3 + 2] = -1.0f;
        vertex_counter++;

        for (int i = 0; i < SAMPLES; i++)
        {
            const float angle = float(i) / float(SAMPLES) * glm::pi<float>() * 2.0f;
            const float x = radius * std::cos(angle);
            const float y = radius * std::sin(angle);
            const float z = -height / 2.0f;

            m_vertices[vertex_counter * 3 + 0] = x;
            m_vertices[vertex_counter * 3 + 1] = y;
            m_vertices[vertex_counter * 3 + 2] = z;

            m_colors[vertex_counter * 3 + 0] = color_base.r;
            m_colors[vertex_counter * 3 + 1] = color_base.g;
            m_colors[vertex_counter * 3 + 2] = color_base.b;

            m_normals[vertex_counter * 3 + 0] = 0.0f;
            m_normals[vertex_counter * 3 + 1] = 0.0f;
            m_normals[vertex_counter * 3 + 2] = -1.0f;
            vertex_counter++;
        }

        for (int i = 0; i < SAMPLES; i++)
        {
            m_faces[face_counter * 3 + 0] = zero_vertex;
            m_faces[face_counter * 3 + 1] = zero_vertex + 1 + (i + 1) % SAMPLES;
            m_faces[face_counter * 3 + 2] = zero_vertex + 1 + i;
            face_counter++;
        }

        // apex
        glm::vec3 apex(0.0f, 0.0f, height / 2.0f);

        // lateral surface: vertices
        zero_vertex = vertex_counter;
        int zero_face;
        zero_face = face_counter;
        for (int i = 0; i < SAMPLES; i++)
        {
            const float angle = float(i) / float(SAMPLES) * glm::pi<float>() * 2.0f;
            const float x = radius * std::cos(angle);
            const float y = radius * std::sin(angle);
            const float zb = -height / 2.0f;

            m_vertices[vertex_counter * 3 + 0] = apex.x;
            m_vertices[vertex_counter * 3 + 1] = apex.y;
            m_vertices[vertex_counter * 3 + 2] = apex.z;

            m_colors[vertex_counter * 3 + 0] = color_side.r;
            m_colors[vertex_counter * 3 + 1] = color_side.g;
            m_colors[vertex_counter * 3 + 2] = color_side.b;

            // initialize to 0 for now
            m_normals[vertex_counter * 3 + 0] = 0.0f;
            m_normals[vertex_counter * 3 + 1] = 0.0f;
            m_normals[vertex_counter * 3 + 2] = 0.0f;

            vertex_counter++;

            m_vertices[vertex_counter * 3 + 0] = x;
            m_vertices[vertex_counter * 3 + 1] = y;
            m_vertices[vertex_counter * 3 + 2] = zb;

            m_colors[vertex_counter * 3 + 0] = color_side.r;
            m_colors[vertex_counter * 3 + 1] = color_side.g;
            m_colors[vertex_counter * 3 + 2] = color_side.b;

            m_normals[vertex_counter * 3 + 0] = 0.0f;
            m_normals[vertex_counter * 3 + 1] = 0.0f;
            m_normals[vertex_counter * 3 + 2] = 0.0f;

            vertex_counter++;
        }

        // lateral surface: faces
        for (int i = 0; i < SAMPLES; i++)
        {
            int i0 = zero_vertex + i * 2;  // apex
            int i1 = zero_vertex + i * 2 + 1;
            int i2 = zero_vertex + ((i + 1) % SAMPLES) * 2 + 1;

            m_faces[face_counter * 3 + 0] = i0;
            m_faces[face_counter * 3 + 1] = i1;
            m_faces[face_counter * 3 + 2] = i2;

            face_counter++;

            glm::vec3 v0(m_vertices[i0 * 3 + 0], m_vertices[i0 * 3 + 1], m_vertices[i0 * 3 + 2]);
            glm::vec3 v1(m_vertices[i1 * 3 + 0], m_vertices[i1 * 3 + 1], m_vertices[i1 * 3 + 2]);
            glm::vec3 v2(m_vertices[i2 * 3 + 0], m_vertices[i2 * 3 + 1], m_vertices[i2 * 3 + 2]);
            glm::vec3 normal = computeNormal(v0, v1, v2);

            m_normals[i0 * 3 + 0] += normal.x;
            m_normals[i0 * 3 + 1] += normal.y;
            m_normals[i0 * 3 + 2] += normal.z;

            m_normals[i1 * 3 + 0] += normal.x;
            m_normals[i1 * 3 + 1] += normal.y;
            m_normals[i1 * 3 + 2] += normal.z;

            m_normals[i2 * 3 + 0] += normal.x;
            m_normals[i2 * 3 + 1] += normal.y;
            m_normals[i2 * 3 + 2] += normal.z;
        }

        // normalize normals
        for (int i = zero_vertex; i < vertex_counter; i++)
        {
            glm::vec3 normal(m_normals[i * 3 + 0], m_normals[i * 3 + 1], m_normals[i * 3 + 2]);
            normal = glm::normalize(normal);
            m_normals[i * 3 + 0] = normal.x;
            m_normals[i * 3 + 1] = normal.y;
            m_normals[i * 3 + 2] = normal.z;
        }

        m_faces_size = face_counter;
        m_vertices_size = vertex_counter;
    }

    ~ConeGeometry()
    {
        delete[] m_vertices;
        delete[] m_colors;
        delete[] m_normals;
        delete[] m_faces;
    }

    const GLfloat * vertices() { return m_vertices; }
    const GLfloat * colors() { return m_colors; }
    const GLfloat * normals() { return m_normals; }
    const GLuint * faces() { return m_faces; }
    GLsizei verticesSize() { return m_vertices_size; }
    GLsizei size() { return m_faces_size * 3; }

    GLenum type() { return GL_TRIANGLES; }

private:
    GLfloat * m_vertices;
    GLfloat * m_colors;
    GLfloat * m_normals;
    GLsizei m_vertices_size;

    GLuint * m_faces;
    GLsizei m_faces_size;
};

#endif // CONE_GEOMETRY_H
