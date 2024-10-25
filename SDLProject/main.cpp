/**
* Author: Avyansh Gupta
* Assignment: Pong clone
* Date due: 2024-10-12, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_image.h> // Include SDL_image for loading images
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float PADDLE_SPEED = 0.9f;
const float BALL_SPEED = 0.25f;
const int PADDLE_WIDTH = 10;
const int PADDLE_HEIGHT = 100;
const int BALL_SIZE = 15;

// Game variables
float paddle1_y = (WINDOW_HEIGHT / 2) - (PADDLE_HEIGHT / 2);
float paddle2_y = (WINDOW_HEIGHT / 2) - (PADDLE_HEIGHT / 2);
float ball_x = WINDOW_WIDTH / 2;
float ball_y = WINDOW_HEIGHT / 2;
float ball_dx = BALL_SPEED;
float ball_dy = BALL_SPEED;
bool is_single_player = false;
bool game_over = false;
int score1 = 0;
int score2 = 0;

SDL_Texture* result_texture = nullptr; // Texture for the result image
void draw_circle(SDL_Renderer* renderer, int center_x, int center_y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, center_x + dx, center_y + dy);
            }
        }
    }
}
void handle_input(const Uint8* keys) {
    if (!is_single_player) {
        if (keys[SDL_SCANCODE_W] && paddle1_y > 0) {
            paddle1_y -= PADDLE_SPEED * 10;
        }
        if (keys[SDL_SCANCODE_S] && paddle1_y + PADDLE_HEIGHT < WINDOW_HEIGHT) {
            paddle1_y += PADDLE_SPEED * 10;
        }
    } else {
        paddle1_y += ball_dy * 10;
        if (paddle1_y < 0 || paddle1_y + PADDLE_HEIGHT > WINDOW_HEIGHT) {
            ball_dy = -ball_dy;
        }
    }
    if (keys[SDL_SCANCODE_UP] && paddle2_y > 0) {
        paddle2_y -= PADDLE_SPEED * 10;
    }
    if (keys[SDL_SCANCODE_DOWN] && paddle2_y + PADDLE_HEIGHT < WINDOW_HEIGHT) {
        paddle2_y += PADDLE_SPEED * 10;
    }

    if (keys[SDL_SCANCODE_T]) {
        is_single_player = !is_single_player;
    }
}

void move_ball() {
    ball_x += ball_dx * 10;
    ball_y += ball_dy * 10;

    if (ball_y <= 0 || ball_y + BALL_SIZE >= WINDOW_HEIGHT) {
        ball_dy = -ball_dy;
    }
    if (ball_x <= 20 && ball_y >= paddle1_y && ball_y <= paddle1_y + PADDLE_HEIGHT) {
        ball_dx = -ball_dx;
    }
    if (ball_x + BALL_SIZE >= WINDOW_WIDTH - 20 && ball_y >= paddle2_y && ball_y <= paddle2_y + PADDLE_HEIGHT) {
        ball_dx = -ball_dx;
    }
    if (ball_x <= 0) {
        game_over = true;
        score2++;
    }
    if (ball_x + BALL_SIZE >= WINDOW_WIDTH) {
        game_over = true;
        score1++;
    }
}

void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    if (!game_over) {
        SDL_Rect paddle1 = {10, static_cast<int>(paddle1_y), PADDLE_WIDTH, PADDLE_HEIGHT};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White paddles
        SDL_RenderFillRect(renderer, &paddle1);

        SDL_Rect paddle2 = {WINDOW_WIDTH - 20, static_cast<int>(paddle2_y), PADDLE_WIDTH, PADDLE_HEIGHT};
        SDL_RenderFillRect(renderer, &paddle2);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White ball
                draw_circle(renderer, static_cast<int>(ball_x) + BALL_SIZE / 2, static_cast<int>(ball_y) + BALL_SIZE / 2, BALL_SIZE / 2);
//        SDL_Rect ball = {static_cast<int>(ball_x), static_cast<int>(ball_y), BALL_SIZE, BALL_SIZE};
//        SDL_RenderFillRect(renderer, &ball);
    } else {
        // Show the result image
        SDL_Rect result_rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT}; // Full screen
        SDL_RenderCopy(renderer, result_texture, nullptr, &result_rect);
    }

    SDL_RenderPresent(renderer);
}

// Main game loop
int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Initialize SDL_image
    if (IMG_Init(IMG_INIT_JPG) == 0) {
        std::cout << "SDL_image could not initialize! IMG_Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Pong Clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Load the result image
    result_texture = IMG_LoadTexture(renderer, "gameover.png");
    if (!result_texture) {
        std::cout << "Failed to load texture! IMG_Error: " << IMG_GetError() << std::endl;
        return -1;
    }

    // Main loop flag
    bool quit = false;
    SDL_Event e;

    // Game loop
    while (!quit) {
        // Handle events on the queue
        while (SDL_PollEvent(&e) != 0) {
            // User requests quit
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        handle_input(keys);
        move_ball();

        render(renderer);

        if (game_over) {
            std::cout << "Game Over! Player 1: " << score1 << " | Player 2: " << score2 << std::endl;
            SDL_Delay(2000);
            quit = true;
            //ball_x = WINDOW_WIDTH / 2;
            //ball_y = WINDOW_HEIGHT / 2;
            //ball_dx = BALL_SPEED;
           // ball_dy = BALL_SPEED;
            //game_over = false;
        }

        SDL_Delay(10);
    }

    // Cleanup
    SDL_DestroyTexture(result_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}


//
//#define GL_SILENCE_DEPRECATION
//#define STB_IMAGE_IMPLEMENTATION
//#define LOG(argument) std::cout << argument << '\n'
//#define GL_GLEXT_PROTOTYPES 1
//
//#ifdef _WINDOWS
//#include <GL/glew.h>
//#endif
//
//#include <SDL2/SDL.h>
//#include <SDL_opengl.h>
//#include "glm/mat4x4.hpp"
//#include "glm/gtc/matrix_transform.hpp"
//#include "ShaderProgram.h"
//#include "stb_image.h"
//
//enum AppStatus { RUNNING, TERMINATED };
//
//constexpr int WINDOW_WIDTH  = 640 * 2,
//              WINDOW_HEIGHT = 480 * 2;
//
//constexpr float BG_RED     = 0.9765625f,
//                BG_GREEN   = 0.97265625f,
//                BG_BLUE    = 0.9609375f,
//                BG_OPACITY = 1.0f;
//
//constexpr int VIEWPORT_X      = 0,
//              VIEWPORT_Y      = 0,
//              VIEWPORT_WIDTH  = WINDOW_WIDTH,
//              VIEWPORT_HEIGHT = WINDOW_HEIGHT;
//
//constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
//               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";
//
//constexpr float MILLISECONDS_IN_SECOND = 1000.0;
//
//constexpr GLint NUMBER_OF_TEXTURES = 1, // to be generated, that is
//                LEVEL_OF_DETAIL    = 0, // mipmap reduction image level
//                TEXTURE_BORDER     = 0; // this value MUST be zero
//
//// Make sure the paths are correct on your system
//constexpr char KIMI_SPRITE_FILEPATH[]    = "/Users/avyanshgupta/Desktop/kimi.png",
//               TOTSUKO_SPRITE_FILEPATH[] = "/Users/avyanshgupta/Desktop/totsuko.png";
//
//constexpr glm::vec3 INIT_SCALE       = glm::vec3(5.0f, 5.98f, 0.0f),
//                    INIT_POS_KIMI    = glm::vec3(2.0f, 0.0f, 0.0f),
//                    INIT_POS_TOTSUKO = glm::vec3(-2.0f, 0.0f, 0.0f);
//
//constexpr float ROT_INCREMENT     = 1.0f;
//constexpr float TRAN_VALUE        = 0.025f;
//constexpr float G_GROWTH_FACTOR   = 1.01f;
//constexpr float G_SHRINK_FACTOR   = 0.99f;
//constexpr float ROT_ANGLE         = glm::radians(1.5f); // Smaller rotation angle
//constexpr int   G_MAX_FRAME       = 40;
//
//SDL_Window* g_display_window;
//AppStatus g_app_status = RUNNING;
//ShaderProgram g_shader_program = ShaderProgram();
//
//glm::mat4 g_view_matrix,
//          g_kimi_matrix,
//          g_totsuko_matrix,
//          g_projection_matrix;
//
//float g_previous_ticks = 0.0f;
//int g_frame_counter = 0;
//bool g_is_growing = true;
//
//GLuint g_kimi_texture_id,
//       g_totsuko_texture_id;
//
//constexpr float CIRCLE_RADIUS = 2.0f; // Radius for circular motion
//float g_totsuko_angle = 0.0f; // Angle for circular motion
//
//GLuint load_texture(const char* filepath)
//{
//    int width, height, number_of_components;
//    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
//
//    if (image == NULL)
//    {
//        LOG("Unable to load image. Make sure the path is correct.");
//        assert(false);
//    }
//
//    GLuint textureID;
//    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
//    glBindTexture(GL_TEXTURE_2D, textureID);
//    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
//
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//    stbi_image_free(image);
//
//    return textureID;
//}
//
//void initialise()
//{
//    SDL_Init(SDL_INIT_VIDEO);
//
//    g_display_window = SDL_CreateWindow("Hello, Transformations!",
//                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
//                                      WINDOW_WIDTH, WINDOW_HEIGHT,
//                                      SDL_WINDOW_OPENGL);
//
//    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
//    SDL_GL_MakeCurrent(g_display_window, context);
//
//    if (g_display_window == nullptr)
//    {
//        std::cerr << "Error: SDL window could not be created.\n";
//        SDL_Quit();
//        exit(1);
//    }
//
//#ifdef _WINDOWS
//    glewInit();
//#endif
//
//    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
//
//    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
//
//    g_kimi_matrix       = glm::mat4(1.0f); // Start upright, no initial rotation
//    g_totsuko_matrix    = glm::mat4(1.0f);
//    g_view_matrix       = glm::mat4(1.0f);
//    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
//
//    g_shader_program.set_projection_matrix(g_projection_matrix);
//    g_shader_program.set_view_matrix(g_view_matrix);
//
//    glUseProgram(g_shader_program.get_program_id());
//
//    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
//
//    g_kimi_texture_id   = load_texture(KIMI_SPRITE_FILEPATH);
//    g_totsuko_texture_id = load_texture(TOTSUKO_SPRITE_FILEPATH);
//
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//}
//
//void process_input()
//{
//    SDL_Event event;
//    while (SDL_PollEvent(&event))
//    {
//        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
//        {
//            g_app_status = TERMINATED;
//        }
//    }
//}
//constexpr glm::vec3 TOTSUKO_SCALE = glm::vec3(0.99f, 0.99f, 1.0f);
//constexpr glm::vec3 KIMI_SCALE = glm::vec3(1.0f, 1.0f, 1.0f); // Fixed scale for Kimi
//constexpr float ORBIT_SPEED = 1.0f; // Adjust this for speed of orbit
//constexpr float RADIUS = 2.0f;       // Distance from Kimi to Totsuko
//
//float g_angle = 0.0f; // Angle for Totsuko's orbit
//float g_x_offset = 0.0f; // X offset for Totsuko's position
//float g_y_offset = 0.0f; // Y offset for Totsuko's position
//
//
//
//
//void update()
//{
//    
//    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
//        float delta_time = ticks - g_previous_ticks;
//        g_previous_ticks = ticks;
//
//        // Step 1: Update for Kimi's scaling behavior (pumping effect)
//        g_frame_counter += 1;
//
//        // Update for scaling behavior (pumping effect)
//        if (g_frame_counter >= G_MAX_FRAME)
//        {
//            g_is_growing = !g_is_growing;
//            g_frame_counter = 0;
//        }
//
//        glm::vec3 scale_vector = glm::vec3(
//            g_is_growing ? G_GROWTH_FACTOR : G_SHRINK_FACTOR,
//            g_is_growing ? G_GROWTH_FACTOR : G_SHRINK_FACTOR,
//            1.0f);
//
//        // Create transformation matrix for Kimi (no translation or rotation)
//        g_kimi_matrix = glm::mat4(1.0f); // Reset model matrix for Kimi
//        g_kimi_matrix = glm::scale(g_kimi_matrix, scale_vector * KIMI_SCALE); // Apply scaling effect
//
//        // Step 2: Update Totsuko's angle for orbit
//        g_angle += ORBIT_SPEED * delta_time; // Increment angle for orbit
//
//        // Step 3: Calculate new x, y position using trigonometry for Totsuko's orbit
//        g_x_offset = RADIUS * glm::cos(g_angle);
//        g_y_offset = RADIUS * glm::sin(g_angle);
//
//        // Step 4: Update Totsuko's transformation matrix
//        g_totsuko_matrix = glm::mat4(1.0f); // Reset model matrix for Totsuko
//        g_totsuko_matrix = glm::translate(g_kimi_matrix, glm::vec3(g_x_offset, g_y_offset, 0.0f)); // Orbit around Kimi
//        g_totsuko_matrix = glm::scale(g_totsuko_matrix, TOTSUKO_SCALE); // Scale Totsuko
//    
//    
//
//}
//
//void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
//{
//    g_shader_program.set_model_matrix(object_model_matrix);
//    glBindTexture(GL_TEXTURE_2D, object_texture_id);
//    glDrawArrays(GL_TRIANGLES, 0, 6); // Drawing the two triangles for each object
//}
//
//void render()
//{
//    glClear(GL_COLOR_BUFFER_BIT);
//
//    float vertices[] = {
//        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
//        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
//    };
//
//    // Textures
//    float texture_coordinates[] = {
//        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
//        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
//    };
//
//    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false,
//                          0, vertices);
//    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
//
//    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
//                          false, 0, texture_coordinates);
//    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
//    
//    draw_object(g_kimi_matrix, g_kimi_texture_id);
//    draw_object(g_totsuko_matrix, g_totsuko_texture_id);
//    
//    // We disable two attribute arrays now
//    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
//    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
//
//    SDL_GL_SwapWindow(g_display_window);
//}
//
//int main(int argc, char* argv[])
//{
//    initialise();
//
//    while (g_app_status == RUNNING)
//    {
//        process_input();
//        update();
//        render();
//    }
//
//    SDL_Quit();
//    return 0;
//}
//
//
//


