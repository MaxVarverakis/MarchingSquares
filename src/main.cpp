#include <iostream>
#include <math.h>
#include <random>

#include "VertexBuffer/VertexBuffer.hpp"
#include "IndexBuffer/IndexBuffer.hpp"
#include "VertexArray/VertexArray.hpp"
#include "Shader/Shader.hpp"
#include "VertexBufferLayout/VertexBufferLayout.hpp"
#include "Circle/Circle.hpp"

#include "Point/Point.hpp"
#include "Grid/Grid.hpp"
#include "MarchingSquares/MarchingSquares.hpp"

SDL_Window* window;
SDL_GLContext gl_context;

bool is_running;

float anglex { 0.0f };
float anglez { 0.0f };

const float width { 768.0f };
const float height { 768.0f };
const float isolevel { 0.5f };
const bool interp { true };
const unsigned int res { 150 };

const float dt { 0.025f };
float sim_time { 0.0f };

float f(const glm::vec2& v, const float t)
{
    const int period { 16 };
    return abs(cosf(period / width * (v.x + 200 + 10 * t)) + sinf(period / height * (v.y - 75))) / 2;
}

void set_sdl_gl_attributes()
{
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
}

void rotateWithArrowKeys(SDL_Event& event)
{
    constexpr float dtheta { 5.0f };

    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.scancode == SDL_SCANCODE_LEFT)
        {
            anglez -= dtheta;
        }
        else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT)
        {
            anglez += dtheta;
        }
        else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN)
        {
            anglex -= dtheta;
        }
        else if (event.key.keysym.scancode == SDL_SCANCODE_UP)
        {
            anglex += dtheta;
        }
    }
}

int main()
{
    if(SDL_Init(SDL_INIT_EVERYTHING)==0)
    {
        std::cout<<"SDL2 initialized successfully."<<std::endl;
        set_sdl_gl_attributes();
        
        window = SDL_CreateWindow("Marching Squares", 0.0f, 0.0f, static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_OPENGL);
        gl_context = SDL_GL_CreateContext(window);
        SDL_GL_SetSwapInterval(1);

        if(glewInit() == GLEW_OK)
        {
            std::cout << "GLEW initialization successful" << std::endl;
        }
        else
        {
            std::cout << "GLEW initialization failed" << std::endl;
            return -1;
        }

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        GLCall(glEnable( GL_LINE_SMOOTH ));
        GLCall(glHint( GL_LINE_SMOOTH_HINT, GL_NICEST ));

        // Grid grid(width, height, res, f);
        std::vector<Particle> particles;
        // initialize particles
        particles.emplace_back(10.0f, glm::vec2(width/2, height/2), glm::vec2(0.0f, height/20));
        particles.emplace_back(20.0f, glm::vec2(width/4, height * 3.0f/4.0f), glm::vec2(width/15, height/10));
        particles.emplace_back(25.0f, glm::vec2(width * 7.0f/8.0f, height/8), glm::vec2(-width/50, height/10));
        particles.emplace_back(7.0f, glm::vec2(width/8, height/4), glm::vec2(-width/20, height/10));
        particles.emplace_back(15.0f, glm::vec2(width * 3.0f/4.0f, height/2), glm::vec2(width/30, -height/10));
        
        
        Grid grid(width, height, res, particles);
        std::vector<Circle> point_circles;
        point_circles.reserve(grid.size());

        const std::vector<Point>& points { grid.points() };
        const std::vector<float>& values { grid.values() };
        for (unsigned int i = 0; i < grid.size(); ++i)
        {
            const Point& point { points[i] };
            float val { values[i] };
            point_circles.emplace_back(Circle(point.position(), 2.5f, glm::vec4(glm::vec3(val), 1.0f)));
            // point_circles.emplace_back(Circle(point.position(), 2.5f, glm::vec4(floor(glm::vec3(val + (1.0f - isolevel))), 1.0f)));
        }

        Circles circles(point_circles);

        VertexArray VAO, line_VAO, line_circ_VAO;

        // constructor automatically binds buffer
        VertexBuffer VBO(circles.m_vertices.data(), static_cast<unsigned int>(circles.m_vertices.size() * sizeof(float)), GL_DYNAMIC_DRAW);
        
        VertexBufferLayout layout;
        for (unsigned int i = 0; i < 5; ++i)
        {
            layout.push<float>(Circle::layout_descriptor[i]);
        }
        VAO.addBuffer(VBO, layout);

        // constructor automatically binds buffer
        IndexBuffer IBO(circles.m_indices.data(), static_cast<unsigned int>(circles.m_indices.size()));

        MarchingSquares MSq(isolevel, interp, grid);
        
        // START HERE
        // std::vector<Circle> edge_circles;
        // for (const Point& point : MSq.points())
        // {
        //     edge_circles.emplace_back(Circle(point.position(), 2.5f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)));
        // }
        // 
        // Circles e_circles(edge_circles);
        // 
        // // constructor automatically binds buffer
        // VertexBuffer line_circ_VBO(e_circles.m_vertices.data(), static_cast<unsigned int>(e_circles.m_vertices.size() * sizeof(float)));
        // 
        // VertexBufferLayout line_circ_layout;
        // for (unsigned int i = 0; i < 5; ++i)
        // {
        //     line_circ_layout.push<float>(Circle::layout_descriptor[i]);
        // }
        // line_circ_VAO.addBuffer(line_circ_VBO, line_circ_layout);
        // 
        // // constructor automatically binds buffer
        // IndexBuffer line_circ_IBO(e_circles.m_indices.data(), static_cast<unsigned int>(e_circles.m_indices.size()));
        // END HERE

        VertexBuffer line_VBO(MSq.positions().data(), static_cast<unsigned int>(MSq.positions().size() * sizeof(float)), GL_DYNAMIC_DRAW);

        // `colored_line` stuff
        // std::vector<float> colored_line_VB;
        // colored_line_VB.reserve(MSq.positions().size());
        // const std::vector<float>& pos { MSq.positions() };
        // unsigned int i { 0 }; 
        // while (i < MSq.positions().size())
        // {
        //     colored_line_VB.emplace_back(pos[i  ]);
        //     colored_line_VB.emplace_back(pos[i+1]);
        //
        //     colored_line_VB.emplace_back(1.0f);
        //     colored_line_VB.emplace_back(0.0f);
        //     colored_line_VB.emplace_back(0.0f);
        //     colored_line_VB.emplace_back(1.0f);
        //
        //     i += 2;
        // }
        // VertexBuffer colored_line_VBO(colored_line_VB.data(), static_cast<unsigned int>(colored_line_VB.size() * sizeof(float)));
        
        VertexBufferLayout line_layout;
        line_layout.push<float>(2); // x,y
        // line_layout.push<float>(4); // r,g,b,a
        line_VAO.addBuffer(line_VBO, line_layout);
        // line_VAO.addBuffer(colored_line_VBO, line_layout);

        // set up MPV matrix
        glm::mat4 proj { glm::ortho(0.0f, width, 0.0f, height, -1.0f, 1.0f) };
        glm::mat4 view { glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)) };
        glm::mat4 model { glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) };
        glm::mat4 MVP { proj * view * model };

        Shader shader("/Users/max/OpenGL_Framework/res/shaders", "circle_");
        shader.bind();
        shader.setUniformMatrix4fv("u_MVP", MVP);
        
        Shader line_shader("res/shaders", "line_");
        line_shader.bind();
        line_shader.setUniformMatrix4fv("u_MVP", MVP);
        line_shader.setUniform4f("u_Color", 1.0f, 0.0f, 0.0f, 1.0f);

        VBO.unbind();
        VAO.unbind();
        IBO.unbind();
        shader.unbind();

        line_VBO.unbind();
        line_VAO.unbind();
        line_shader.unbind();
        // colored_line_VBO.unbind();

        // line_circ_VBO.unbind();
        // line_circ_VAO.unbind();
        // line_circ_IBO.unbind();

        Renderer renderer;

        // Main loop
        SDL_Event event;
        is_running = true;

        while(is_running)
        {
            // Uint64 start = SDL_GetPerformanceCounter();

            while(SDL_PollEvent(&event))
            {
                if( event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) )
                {
                    is_running = false;
                }
                
                // rotateWithArrowKeys(event);
            }

            // grid.assignValues(f, sim_time);
            grid.assignValues(particles);
            MSq.march(grid);

            // update circle buffer
            circles.udpateColors(values);
            VBO.updateBuffer(circles.m_vertices.data());
            
            // update line buffer
            std::vector<float> positions = MSq.positions();
            line_VBO.rebuffer(positions.data(), static_cast<unsigned int>(positions.size() * sizeof(float)), GL_DYNAMIC_DRAW);

            // Render
            renderer.clear();

            // renderer.drawCircles(VAO, IBO, shader);
            // renderer.drawCircles(line_circ_VAO, line_circ_IBO, shader);
            
            renderer.drawLines(line_VAO, line_VBO, line_shader);
            // renderer.drawLines(line_VAO, colored_line_VBO, line_shader);

            SDL_GL_SwapWindow(window);

            // update grid values
            sim_time += dt;
            
            // evolve particles
            for (Particle& particle : particles)
            {
                particle.evolve(width, height, dt);
            }

            // Uint64 end = SDL_GetPerformanceCounter();

            // float elapsed = static_cast<float>(end - start) / (float)SDL_GetPerformanceFrequency();
            // std::cout << "Current FPS: " << std::to_string(1.0f / elapsed) << std::endl;
        }

        SDL_Quit();
    }
    else
    {
        std::cout<<"SDL2 initialization failed."<<std::endl;
        return -1;
    }

    return 0;
}
