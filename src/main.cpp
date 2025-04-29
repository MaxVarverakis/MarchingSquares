#include <iostream>
#include <math.h>
#include <random>
#include <algorithm>

#include "VertexBuffer/VertexBuffer.hpp"
#include "IndexBuffer/IndexBuffer.hpp"
#include "VertexArray/VertexArray.hpp"
#include "Shader/Shader.hpp"
#include "VertexBufferLayout/VertexBufferLayout.hpp"
#include "Circle/Circle.hpp"

#include "Point/Point.hpp"
#include "Grid/Grid.hpp"
#include "MarchingSquares/MarchingSquares.hpp"
#include "PerlinNoise/PerlinNoise.hpp"

SDL_Window* window;
SDL_GLContext gl_context;

bool is_running;
bool paused { true };
bool showNoise { false };

const float width { 768.0f };
const float height { 768.0f };
const float isolevel { 0.5f };
const bool interp { true };
const unsigned int res { 250 };

// const float FRAME_RATE { 120.0f };
float DELTA_TIME; // { 1.0f / FRAME_RATE};
Uint32 CURRENT_TIME { 0 };
Uint32 LAST_TIME { 0 };
// Uint64 START_TICK_COUNT, END_TICK_COUNT;
// float ELAPSED_MS;

float GLOBAL_TIME { 0.0f };
float DT { 0.025f };

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

void arrowKeyIsolevel(SDL_Event& event, MarchingSquares& msq)
{
    float di { 0.005f };

    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.scancode == SDL_SCANCODE_DOWN)
        {
            msq.setIsolevel(std::clamp(msq.getIsolevel() - di, 0.0f, 1.0f));
            std::cout << msq.getIsolevel() << '\n';
        }
        else if (event.key.keysym.scancode == SDL_SCANCODE_UP)
        {
            msq.setIsolevel(std::clamp(msq.getIsolevel() + di, 0.0f, 1.0f));
            std::cout << msq.getIsolevel() << '\n';
        }
    }
}

void evolveTime(SDL_Event& event)
{
    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.scancode == SDL_SCANCODE_LEFT)
        {
            GLOBAL_TIME < DT ? GLOBAL_TIME = 0.0f : GLOBAL_TIME -= DT;
            std::cout << "Global time: " << GLOBAL_TIME << '\n';
        }
        else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT)
        {
            GLOBAL_TIME += DT;
            std::cout << "Global time: " << GLOBAL_TIME << '\n';
        }
        else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
        {
            paused = paused ? false : true;
            std::cout << "Paused: " << paused << '\n';
        }
    }
}

void renderNoise(SDL_Event& event)
{
    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.scancode == SDL_SCANCODE_RETURN)
        {
            showNoise ? showNoise = false : showNoise = true;
        }
    }
}

void printKey()
{
    std::cout << "###################################" << '\n';
    std::cout << "KEY" << '\n';
    std::cout << "Return/enter" << '\t' << "Show/hide grid values" << '\n';
    std::cout << "Spacebar" << '\t' << "Play/pause" << '\n';
    std::cout << "Right arrow" << '\t' << "Time forward" << '\n';
    std::cout << "Left arrow" << '\t' << "Time reverse" << '\n';
    std::cout << "Up arrow" << '\t' << "Increase iso-level" << '\n';
    std::cout << "Down arrow" << '\t' << "Decrease iso-level" << '\n';
    std::cout << "###################################" << '\n';
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

        printKey();

        // Grid grid(width, height, res, f);
        
        // std::vector<Particle> particles;
        // // initialize particles
        // particles.emplace_back(10.0f, glm::vec2(width/2, height/2), glm::vec2(0.0f, height/20));
        // particles.emplace_back(20.0f, glm::vec2(width/4, height * 3.0f/4.0f), glm::vec2(width/15, height/10));
        // particles.emplace_back(25.0f, glm::vec2(width * 7.0f/8.0f, height/8), glm::vec2(-width/50, height/10));
        // particles.emplace_back(7.0f, glm::vec2(width/8, height/4), glm::vec2(-width/20, height/10));
        // particles.emplace_back(15.0f, glm::vec2(width * 3.0f/4.0f, height/2), glm::vec2(width/30, -height/10));
        
        // Grid grid(width, height, res, true, particles);
        PerlinNoise p(width, height, 10, false);
        Grid grid(width, height, res, p);
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
            // START_TICK_COUNT = SDL_GetPerformanceCounter();
            
            // CURRENT_TIME = SDL_GetTicks();
            // DELTA_TIME = fminf(static_cast<float>(CURRENT_TIME - LAST_TIME) / 1000.0f, 0.025f);
            // evolve particles
            // for (Particle& particle : particles)
            // {
            //     particle.evolve(width, height, DELTA_TIME);
            // }
            // LAST_TIME = CURRENT_TIME;

            while(SDL_PollEvent(&event))
            {
                if( event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) )
                {
                    is_running = false;
                }

                arrowKeyIsolevel(event, MSq);
                evolveTime(event);
                renderNoise(event);
            }

            if (not paused) { GLOBAL_TIME += DT / 5; }
            
            // update z layer if reached the ceiling
            if (GLOBAL_TIME >= 0.99999f)
            {
                GLOBAL_TIME = 0.0f;
                p.nextZGradients();
            }

            // update grid values
            grid.assignValues(p, GLOBAL_TIME);
            // grid.assignValues(particles);
            MSq.march(grid);

            // update circle buffer
            circles.updateColors(values);
            VBO.updateBuffer(circles.m_vertices.data());
            
            // update line buffer (rebuffer because the size of the buffer is non-constant)
            std::vector<float> positions = MSq.positions();
            line_VBO.rebuffer(positions.data(), static_cast<unsigned int>(positions.size() * sizeof(float)), GL_DYNAMIC_DRAW);

            // Render
            renderer.clear();

            // to show grid point values in color
            if (showNoise) { renderer.drawCircles(VAO, IBO, shader); }
            // renderer.drawCircles(line_circ_VAO, line_circ_IBO, shader);
            
            renderer.drawLines(line_VAO, line_VBO, line_shader);
            // renderer.drawLines(line_VAO, colored_line_VBO, line_shader);

            SDL_GL_SwapWindow(window);

            // END_TICK_COUNT = SDL_GetPerformanceCounter();
            // ELAPSED_MS = static_cast<float>((END_TICK_COUNT - START_TICK_COUNT) / SDL_GetPerformanceFrequency()) * 1000.0f;
            // SDL_Delay((Uint32)(DELTA_TIME * 1000.0f - ELAPSED_MS));

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
