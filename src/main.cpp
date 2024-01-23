#include <iostream>
#include <array>
#include <cstdlib>
#include <algorithm>
#include <vector>

#include "raylib.h"
#include "raymath.h"

#define PRINT_VECTOR(v1) std::cout << "x: " << (v1).x << ", y: " << (v1).y << "\n";

enum class Mode {
    MENU,
    GAME,
    OPTIONS
};

int score = 0;
float dt = 0;
float fps = 60;
Vector2 screen_size = Vector2{ 800, 600 };
bool pause = true;
bool debug = false;
Mode mode = Mode::MENU;
bool sound_on = true;

Sound fx_shoot;
Sound fx_die;
Sound fx_explotion;

Font font;
int size_font = 30;
int spacing = 1;

Texture2D player_texture;
Vector2 player_position = Vector2{ 400, 300 };
float player_speed = 400.f;
float player_rotation = 0;
float rotation_speed = 200.f;

int bullet_max = 5;
int bullet_avaible = bullet_max;
float time_since_last_reload = 0;
float reload_threshold = 0.5f;
bool first_bullet = true;

Texture2D asteroid_texture;
float time_since_last_asteroid = 0;
float asteroid_appear_threshold = 0.6f;

Texture2D logo_texture;
Vector2 logo_position = Vector2{ 400, 100 };

Texture2D bg_texture;

struct Asteroid {
    Vector2 position;
    Vector2 size;
    float rotation;
    float speed{100.f};
};

std::vector<Asteroid> asteroids;

struct Bullet {
    Vector2 position;
    float rotation;
    float speed{600.f};

    bool operator==(const Bullet& other) const {
        return position.x == other.position.x && position.y == other.position.y && rotation == other.rotation;
    }
};

std::vector<Bullet> bullets;

void load();
void unload();
void draw();
void update();
void start_again();
void update_collisions();
void update_ui();

int main()
{
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(screen_size.x, screen_size.y, "Space Odyssey");
    InitAudioDevice();

    SetExitKey(KEY_NULL);
    load();

    SetTargetFPS(fps);

    while (!WindowShouldClose())
    {
        dt = GetFrameTime();

        update();

        BeginDrawing();
            ClearBackground(BLACK);
            draw();
        EndDrawing();
    }

    unload();

    CloseWindow();

    return 0;
}

void load() {
    player_texture = LoadTexture("assets/player.png");
    asteroid_texture = LoadTexture("assets/asteroid.png");
    logo_texture = LoadTexture("assets/logo.png");
    bg_texture = LoadTexture("assets/bg.png");

    font = LoadFont("assets/font.ttf");
    fx_shoot = LoadSound("assets/shooting.mp3");
    fx_die = LoadSound("assets/die.mp3");
    fx_explotion = LoadSound("assets/explotion.mp3");
}

void unload() {
    UnloadTexture(player_texture);
    UnloadTexture(asteroid_texture);
    UnloadTexture(logo_texture);
    UnloadTexture(bg_texture);

    UnloadFont(font);
    UnloadSound(fx_shoot);
    UnloadSound(fx_die);
    UnloadSound(fx_explotion);

    CloseAudioDevice();
}

void make_bullet() {
    if(GetTime() - time_since_last_reload < reload_threshold) {
        return;
    }

    bullet_avaible--;

    if(bullet_avaible<=0) {
        bullet_avaible = bullet_max;
        time_since_last_reload = GetTime();
    }

    if(sound_on) PlaySound(fx_shoot);

    Bullet bullet = Bullet{player_position,player_rotation-90.f};
    bullets.push_back(bullet);
}

void make_asteroid() {
    int random_val = GetRandomValue(32,static_cast<int>(screen_size.x/4));
    Vector2 size = Vector2{static_cast<float>(random_val),static_cast<float>(random_val)};

    // Generate x-coordinate outside of the window
    int x = (rand() % 2 == 0) ? (-size.x) : (screen_size.y + size.x+50);

    // Generate y-coordinate outside of the window
    int y = (rand() % 2 == 0) ? (-size.y) : (screen_size.y + size.y+50);

    Vector2 position = Vector2{static_cast<float>(x), static_cast<float>(y)};

    Vector2 random_point = Vector2{static_cast<float>(GetRandomValue(100,screen_size.x-100)),static_cast<float>(GetRandomValue(100,screen_size.y-100))};
    float angle = Vector2Angle(random_point,Vector2Scale(screen_size,0.5f));
    angle = (angle > 1.f || angle < -1.f) ? std::min(std::max(360-angle*360,-360.f),360.f) : angle*360.f;

    Asteroid asteroid = Asteroid{position,size,angle};
    asteroids.push_back(asteroid);
}

void update_player() {
    if(pause) return;
    Vector2 direction = Vector2Zero();
    if (IsKeyDown(KEY_RIGHT)) {
        player_rotation += rotation_speed * dt;
    }
    if (IsKeyDown(KEY_LEFT)) {
        player_rotation -= rotation_speed * dt;
    }
    if (IsKeyDown(KEY_UP)) {
        // the direction have to take account of the rotation
        direction.y -= cos(player_rotation * DEG2RAD);
        direction.x += sin(player_rotation * DEG2RAD);
    }
    if (IsKeyDown(KEY_DOWN)) {
        // the direction have to take account of the rotation
        direction.y += cos(player_rotation * DEG2RAD);
        direction.x -= sin(player_rotation * DEG2RAD);
    }

    direction = Vector2Normalize(direction);
    direction = Vector2Scale(direction, player_speed * dt);

    player_position = Vector2Add(player_position,direction);
}

void update_bullets() {
    if(pause) return;
    
    if(IsKeyPressed(KEY_SPACE)) {
        make_bullet();
    }

    if(bullets.empty()) return;

    for (auto& bullet : bullets) {
        Vector2 rotation_position = Vector2{cos(bullet.rotation * DEG2RAD),
                                            sin(bullet.rotation * DEG2RAD)};
        Vector2 direction = Vector2Scale(rotation_position, bullet.speed * dt);
        bullet.position = Vector2Add(bullet.position, direction);
    }

}

void update_collisions() {
    for (auto& asteroid : asteroids) {
        // check player collision
        if(CheckCollisionCircles(Vector2{asteroid.position.x+asteroid.size.x/2,asteroid.position.y+asteroid.size.y/2}, asteroid.size.x/2.5, player_position, 16)) {
            if(sound_on) PlaySound(fx_die);
            start_again();
            return;
        }
    }

    if(bullets.empty()) return;

    for (std::vector<Bullet>::iterator it = bullets.begin(); it != bullets.end();) {
        if(it->position.x < 0 || it->position.x > screen_size.x || it->position.y < 0 || it->position.y > screen_size.y) {
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }

    for (std::vector<Asteroid>::iterator it1 = asteroids.begin(); it1 != asteroids.end();) {
        for (std::vector<Bullet>::iterator it = bullets.begin(); it != bullets.end();) {
            if(CheckCollisionCircles(Vector2{it1->position.x+it1->size.x/2,it1->position.y+it1->size.y/2}, it1->size.x/2.5, it->position, 10)) {
                if(sound_on) PlaySound(fx_explotion);
                it = bullets.erase(it);
                it1 = asteroids.erase(it1);
                score++;
            } else {
                ++it;
            }
        }
        ++it1;
    }
}

void update_asteroids() {
    if(pause) return;
    
    if(GetTime() - time_since_last_asteroid >= asteroid_appear_threshold) {
        make_asteroid();
        time_since_last_asteroid = GetTime();
    }

    if(asteroids.empty()) return;

    for (auto& asteroid : asteroids) {
        Vector2 rotation_position = Vector2{cos(asteroid.rotation * DEG2RAD),
                                            sin(asteroid.rotation * DEG2RAD)};
        Vector2 direction = Vector2Scale(rotation_position, asteroid.speed * dt);
        asteroid.position = Vector2Add(asteroid.position, direction);
    }
}

void start_again() {
    asteroids.clear();
    bullets.clear();
    score = 0;
    bullet_avaible = bullet_max;
    asteroid_appear_threshold = 0.6f;
    player_position = Vector2{ 400, 300 };
}

void update_ui() {
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 position = GetMousePosition();

        Vector2 size_play = MeasureTextEx(font, "PLAY", size_font, spacing);
        Vector2 size_options = MeasureTextEx(font, "OPTIONS", size_font, spacing);
        Vector2 size_exit = MeasureTextEx(font, "EXIT", size_font, spacing);

        std::string sound_text = std::string("Sound: ") + (sound_on ? std::string("True") : std::string("false"));
        Vector2 size_sound = MeasureTextEx(font, sound_text.c_str(), 40, spacing);

        switch(mode) {
            case Mode::MENU:
                if(CheckCollisionPointRec(position, Rectangle{screen_size.x*0.5f - size_play.x*0.5f,screen_size.y*0.5f - size_play.y*0.5f,size_play.x,size_play.y})) {
                    mode = Mode::GAME;
                    pause = false;
                } else if(CheckCollisionPointRec(position, Rectangle{screen_size.x*0.5f - size_options.x*0.5f,screen_size.y*0.6f - size_options.y*0.5f,size_options.x,size_options.y})) {
                    mode = Mode::OPTIONS;
                    pause = true;
                } else if(CheckCollisionPointRec(position, Rectangle{screen_size.x*0.5f - size_exit.x*0.5f,screen_size.y*0.7f - size_exit.y*0.5f,size_exit.x,size_exit.y})) {
                    unload();
                    CloseWindow();
                    exit(0);
                }
                break;

            case Mode::OPTIONS:
                if(CheckCollisionPointRec(position, Rectangle{screen_size.x*0.3f - size_sound.x*0.5f,screen_size.y*0.3f - size_sound.y*0.5f,size_sound.x,size_sound.y})) {
                    sound_on = !sound_on;
                }
                break;

            default:
                break;
        }
    }
}

void update_difficulty() {
    if(score % 10 == 0 && score != 0) {
        asteroid_appear_threshold = -1.f/200*score+0.5f;
    }
}

void update() {
    if(IsKeyPressed(KEY_P)) {
        pause = !pause;
    }

    if(IsKeyPressed(KEY_ESCAPE)) {
        mode = Mode::MENU;
        pause = true;
    }

    update_player();
    update_bullets();
    update_asteroids();
    update_collisions();
    update_difficulty();

    update_ui();
}

void draw() {
    std::string score_text = std::string("Score: ")+std::to_string(score);
    Vector2 size = MeasureTextEx(font, score_text.c_str(), size_font, spacing);
    Vector2 score_position = Vector2{screen_size.x - size.x - size.x*0.1f,screen_size.y*0.05f};
    
    std::string bullets_text = std::string("Bullets: ")+(GetTime() - time_since_last_reload < reload_threshold ? "0" : std::to_string(bullet_avaible));
    Vector2 size_bullets_font = MeasureTextEx(font, bullets_text.c_str(), size_font, spacing);
    Vector2 bullets_text_position = Vector2{screen_size.x*0.1f,screen_size.y*0.1f};

    std::string play_text = std::string("PLAY");
    Vector2 size_play = MeasureTextEx(font, play_text.c_str(), size_font, spacing);
    Vector2 play_text_position = Vector2{screen_size.x*0.5f - size_play.x*0.5f,screen_size.y*0.5f - size_play.y*0.5f};

    std::string options_text = std::string("OPTIONS");
    Vector2 size_options = MeasureTextEx(font, options_text.c_str(), size_font, spacing);
    Vector2 options_text_position = Vector2{screen_size.x*0.5f - size_options.x*0.5f,screen_size.y*0.6f - size_options.y*0.5f};

    std::string exit_text = std::string("EXIT");
    Vector2 size_exit = MeasureTextEx(font, exit_text.c_str(), size_font, spacing);
    Vector2 exit_text_position = Vector2{screen_size.x*0.5f - size_exit.x*0.5f,screen_size.y*0.7f - size_exit.y*0.5f};

    std::string sound_text = std::string("Sound: ") + (sound_on ? std::string("True") : std::string("False"));
    Vector2 size_sound = MeasureTextEx(font, sound_text.c_str(), 40, spacing);
    Vector2 sound_text_position = Vector2{screen_size.x*0.3f - size_sound.x*0.5f,screen_size.y*0.3f - size_sound.y*0.5f};


    DrawTextureEx(bg_texture, Vector2{0,0}, 0, 13, WHITE);

    switch (mode)
    {
    case Mode::MENU:
        DrawTextureEx(logo_texture, Vector2{logo_position.x - (logo_texture.width*2.5f)/2,logo_position.y - logo_texture.height/2}, 0, 2.5f, WHITE);
        DrawTextEx(font, "PLAY", play_text_position, size_font, spacing, WHITE);
        DrawTextEx(font, "OPTIONS", options_text_position, size_font, spacing, WHITE);
        DrawTextEx(font, "EXIT", exit_text_position, size_font, spacing, WHITE);

        break;
    case Mode::GAME:
        for (auto& bullet : bullets) {
            DrawRectanglePro(Rectangle{bullet.position.x+2.5f,bullet.position.y,10,5}, Vector2{5,5}, bullet.rotation, WHITE);
            if(debug) DrawCircle(bullet.position.x,bullet.position.y,10,WHITE);
        }

        for (auto& asteroid : asteroids) {
            Vector2 size = asteroid.size;
            Vector2 position = asteroid.position;
            if(debug) DrawCircle(position.x+size.x/2,position.y+size.y/2,size.x/2.5,WHITE);
            DrawTexturePro(asteroid_texture, Rectangle{0,0,64,64}, 
                            Rectangle{position.x+size.x/2,position.y+size.y/2,size.x,size.y},Vector2{size.x/2,size.y/2}, asteroid.rotation, WHITE);
        } 

        DrawTexturePro(player_texture, Rectangle{0,0,32,32}, Rectangle{player_position.x,player_position.y,32,32}, Vector2{16,16}, player_rotation, WHITE);
        if(debug) DrawCircleV(player_position, 16, RED);

        DrawTextEx(font, score_text.c_str(), score_position, size_font, spacing, WHITE);
        DrawTextEx(font, bullets_text.c_str(), bullets_text_position, size_font, spacing, WHITE);
        break;

    case Mode::OPTIONS:
        Color color;
        if(sound_on) {
            color = GREEN;
        } else {
            color = RED;
        }
        DrawTextEx(font, sound_text.c_str(), sound_text_position, size_font, spacing, color);
        break;

    default:
        break;
    }
}