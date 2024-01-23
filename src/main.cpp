#include <iostream>
#include <array>
#include <cstdlib>
#include <algorithm>
#include <vector>

#include "raylib.h"
#include "raymath.h"

#define PRINT_VECTOR(v1) std::cout << "x: " << (v1).x << ", y: " << (v1).y << "\n";

Font font;

int score = 0;
float dt = 0;
Vector2 screen_size = Vector2{ 800, 600 };
bool pause = false;

Texture2D player_texture;
Vector2 player_position = Vector2{ 400, 300 };
float player_speed = 400.f;
float player_rotation = 0;
float rotation_speed = 200.f;

Texture2D asteroid_texture;
float time_since_last_asteroid = 0;
float asteroid_appear_threshold = 0.5f;

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


float randf() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

int main()
{
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(screen_size.x, screen_size.y, "Space Odyssey");

    load();

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
    font = LoadFont("assets/font.ttf");
}

void unload() {
    UnloadTexture(player_texture);
    UnloadTexture(asteroid_texture);
    UnloadFont(font);
}

void make_bullet() {
    // make the bullet appear in the top of the player
    Bullet bullet = Bullet{player_position,player_rotation-90.f};
    bullets.push_back(bullet);
}

void make_asteroid() {
    int random_val = GetRandomValue(32,static_cast<int>(screen_size.x/4));
    Vector2 size = Vector2{static_cast<float>(random_val),static_cast<float>(random_val)};
    // Generate x-coordinate outside of the window
    int x = (rand() % 2 == 0) ? GetRandomValue(-size.x, 0) : GetRandomValue(screen_size.x, screen_size.y + size.x);

    // Generate y-coordinate outside of the window
    int y = (rand() % 2 == 0) ? GetRandomValue(-size.y, 0) : GetRandomValue(screen_size.y, screen_size.y + size.y);

    Vector2 position = Vector2{static_cast<float>(x), static_cast<float>(y)};

    Vector2 random_point = Vector2{static_cast<float>(GetRandomValue(0,screen_size.x)),static_cast<float>(GetRandomValue(0,screen_size.y))};
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
        if(CheckCollisionCircles(asteroid.position, asteroid.size.x/4, player_position, 16)) {
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
    player_position = Vector2{ 400, 300 };
}

void update() {
    if(IsKeyPressed(KEY_P)) {
        pause = !pause;
    }
    update_player();
    update_bullets();
    update_asteroids();
    update_collisions();
}

void draw() {
    std::string score_text = std::string("Score: ")+std::to_string(score);
    int size_font = 30;
    int spacing = 1;
    Vector2 size = MeasureTextEx(font, score_text.c_str(), size_font, spacing);
    Vector2 score_position = Vector2{screen_size.x - size.x - size.x*0.1f,screen_size.x*0.05f};

    for (auto& bullet : bullets) {
        DrawRectanglePro(Rectangle{bullet.position.x+2.5f,bullet.position.y,10,5}, Vector2{5,5}, bullet.rotation, WHITE);
    }

    for (auto& asteroid : asteroids) {
        DrawRectangleV(asteroid.position,asteroid.size,RED);
        DrawTexturePro(asteroid_texture, Rectangle{0,0,64,64}, Rectangle{asteroid.position.x,asteroid.position.y,asteroid.size.x,asteroid.size.y}, Vector2{size.x/2,size.y/2}, asteroid.rotation, WHITE);
    }

    DrawTexturePro(player_texture, Rectangle{0,0,32,32}, Rectangle{player_position.x,player_position.y,32,32}, Vector2{16,16}, player_rotation, WHITE);
    DrawCircleV(player_position, 16, RED);

    DrawTextEx(font, score_text.c_str(), score_position, size_font, spacing, WHITE);
}