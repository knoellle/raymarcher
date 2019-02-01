#pragma once
#include <Eigen/Dense>

class Scene {
    public:
        Scene();
        void addForceToCamera(const Eigen::Vector3f &v, const double dt, const double x = 0, const double y = 0);
        void addDebugMomentum(const int index, const double value, const double dt);
        void update(const double dt);
        void apply(sf::Shader &shader);
    private:
        Eigen::Matrix4f cam_mat;
        float cam_x;
        float cam_y;
        Eigen::Vector3f cam_pos;
        Eigen::Vector3f cam_momentum;
        Eigen::Vector2f cam_rot_momentum;

        float debug_values[10];
        float debug_momentum[10];
};
