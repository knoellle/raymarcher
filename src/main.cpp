#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "iostream"
#include "SFML/Graphics.hpp"
#include "SFML/OpenGL.hpp"
#include <Eigen/Dense>

#include "scene.h"

#define ERROR_MSG(x) std::cerr << x << std::endl;

char loadShader(sf::Shader &shader)
{
    if (!shader.loadFromFile("../assets/vert.glsl", sf::Shader::Vertex))
    {
        ERROR_MSG("Failed to compile vertex shader");
        return 1;
    }
    if (!shader.loadFromFile("../assets/frag.glsl", sf::Shader::Fragment))
    {
        ERROR_MSG("Failed to compile fragment shader");
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    if (!sf::Shader::isAvailable()) {
        ERROR_MSG("Graphics card does not support shaders");
        return 1;
    }
    sf::Shader shader;
    if (loadShader(shader) != 0)
    {
        return 1;
    }

    sf::ContextSettings settings;
    settings.majorVersion = 2;
    settings.minorVersion = 0;

    sf::VideoMode screen_size = sf::VideoMode(1600, 900, 24);
    sf::Glsl::Vec2 window_res((float)1600, (float)900);
    shader.setUniform("iResolution", window_res);

    sf::RenderWindow window(screen_size, "Marble Marcher", sf::Style::Close, settings);

    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(false);
    window.requestFocus();

    Scene scene;
    scene.addForceToCamera(Eigen::Vector3f(0.0f, 0.0f, -5.0f), 1.f);

    auto lastFileChange = 0;
    struct stat result;
    if(stat("../assets/frag.glsl", &result)==0)
    {
        lastFileChange = result.st_mtime;
    }

    sf::Clock clock;
    double time = 0.0f;
    while (window.isOpen())
    {
        const double dt = clock.restart().asSeconds();
        time += dt;
        if(stat("../assets/frag.glsl", &result)==0)
        {
            if (result.st_mtime > lastFileChange)
            {
                loadShader(shader);
                lastFileChange = result.st_mtime;
            }
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
                break;
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::Q)
                {
                    window.close();
                    break;
                }
                switch (event.key.code) {
                    case sf::Keyboard::W :
                        break;
                    case sf::Keyboard::S :
                        break;
                }
            }
            else if (event.type == sf::Event::Resized)
            {
                sf::Glsl::Vec2 window_res((float)event.size.width, (float)event.size.height);
                shader.setUniform("iResolution", window_res);
            }
        }

        float camera_speed = 10.0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
            camera_speed *= 10.0;
        // Read keyboard input
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            scene.addForceToCamera(Eigen::Vector3f(0.0, 0.0, camera_speed), dt);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            scene.addForceToCamera(Eigen::Vector3f(0.0, 0.0, -camera_speed), dt);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            scene.addForceToCamera(Eigen::Vector3f(-camera_speed, 0.0, 0.0), dt);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            scene.addForceToCamera(Eigen::Vector3f(camera_speed, 0.0, 0.0), dt);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {
            scene.addForceToCamera(Eigen::Vector3f(0.0, camera_speed, 0.0), dt);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::C))
        {
            scene.addForceToCamera(Eigen::Vector3f(0.0, -camera_speed, 0.0), dt);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::J))
        {
            scene.addForceToCamera(Eigen::Vector3f(0.0, 0.0, 0.0), dt, 0, 10);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::K))
        {
            scene.addForceToCamera(Eigen::Vector3f(0.0, 0.0, 0.0), dt, 0, -10);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::H))
        {
            scene.addForceToCamera(Eigen::Vector3f(0.0, 0.0, 0.0), dt, -10, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::L))
        {
            scene.addForceToCamera(Eigen::Vector3f(0.0, 0.0, 0.0), dt, 10, 0);
        }
        for (int key = sf::Keyboard::Num0; key <= sf::Keyboard::Num9; key++)
        {
            if (sf::Keyboard::isKeyPressed((sf::Keyboard::Key) key))
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                    scene.addDebugMomentum(key - sf::Keyboard::Num0, -5.0, dt);
                else
                    scene.addDebugMomentum(key - sf::Keyboard::Num0,  5.0, dt);
            }
        }

        // Update scene
        scene.update(dt);

        // Render scene
        shader.setUniform("iTime", (float)time);
        scene.apply(shader);
        sf::RenderStates states = sf::RenderStates::Default;
        states.shader = &shader;
        sf::RectangleShape rect;
        rect.setSize(window_res);
        rect.setPosition(0, 0);

        window.draw(rect, states);
        window.display();
    }

    return 0;
}
