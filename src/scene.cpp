#include "iostream"
#include "SFML/Graphics.hpp"
#include "SFML/OpenGL.hpp"
#include <Eigen/Dense>
#include <Eigen/Geometry>
using namespace Eigen;
#include "scene.h"

Matrix4f create_affine_matrix(float pitch, float yaw, float roll, Vector3f position)
{
    Transform<float, 3, Affine> t;
    t = Translation<float, 3>(position);
    t.rotate(AngleAxis<float>(yaw, Vector3f::UnitY()));
    t.rotate(AngleAxis<float>(pitch, Vector3f::UnitX()));
    t.rotate(AngleAxis<float>(roll, Vector3f::UnitZ()));
    return t.matrix();
}

Scene::Scene() :
    cam_mat(Eigen::Matrix4f::Identity()),
    cam_x(0.0),
    cam_y(0.0),
    cam_pos(0.0f, 0.0f, 0.0f),
    cam_momentum(0.0f, 0.0f, 0.0f),
    cam_rot_momentum(0.0f, 0.0f),
    debug_values{0.0},
    debug_momentum{0.0}
{
}

void Scene::addForceToCamera(const Eigen::Vector3f &v, const double dt, const double x, const double y)
{
    cam_momentum += v * dt;
    cam_rot_momentum += Vector2f(x, y) * dt;
}

void Scene::addDebugMomentum(const int index, const double value, const double dt)
{
    debug_momentum[index] += value * dt;
    std::cout << index << ":" << debug_values[index] << "\n";
}

void Scene::update(const double dt)
{
    cam_pos += (AngleAxis<float>(cam_x, Vector3f::UnitY()).toRotationMatrix()) * cam_momentum * dt;
    cam_momentum -= cam_momentum * dt * 3.0;

    cam_x += cam_rot_momentum.x() * dt;
    cam_y += cam_rot_momentum.y() * dt;
    cam_rot_momentum -= cam_rot_momentum * dt * 10.0;

    cam_mat = create_affine_matrix(cam_y, cam_x, 0.0f, cam_pos);

    for (int i = 0; i<10; i++)
    {
        debug_values[i] += debug_momentum[i] * dt;
        debug_momentum[i] -= debug_momentum[i] * dt * 3;
        if (abs(debug_momentum[i]) > 0.01)
            std::cout << i << ": " << debug_values[i] << " | " << debug_momentum[i] << "\n";
    }

    return;
    std::cout << cam_mat << "\n\n";
    std::cout << cam_momentum << "\n";
}

void Scene::apply(sf::Shader &shader)
{
    shader.setUniform("iMat", sf::Glsl::Mat4(cam_mat.data()));
    shader.setUniformArray("iDebugValues", debug_values, 10);
}
