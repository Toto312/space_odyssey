#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "raylib.h"
#include "entity.hpp"
#include "gui.hpp"

enum class Mode {
    MENU,
    GAME,
    OPTIONS
};

class Game {
private:
    float dt;
    float fps{60};
    Vector2 screen_size{800,600};
    
    int score = 0;
    Mode mode = Mode::MENU;

    bool pause = false;
    bool debug = false;
    bool sound_on = true;

    Image icon;
    Texture2D player_texture;
    Texture2D asteroid_texture;
    Texture2D logo_texture;
    Texture2D bg_texture;

    Sound fx_shoot;
    Sound fx_die;
    Sound fx_explotion;

    Font font;
    Label score_label;
    Label bullets_label;
    Label play_label;
    Label options_label;
    Label exit_label;
    Label sound_label;

    Player player;
    std::vector<Asteroid> asteroids;
    std::vector<Bullet> bullets;

    int bullet_max = 5;
    int bullet_avaible = bullet_max;
    float time_since_last_reload = 0;
    float reload_threshold = 0.5f;

    float time_since_last_asteroid = 0;
    float asteroid_appear_threshold = 0.6f;

public:
    Game() {
        SetTraceLogLevel(LOG_ERROR);
        InitWindow(screen_size.x, screen_size.y, "Space Odyssey");
        InitAudioDevice();
    
        SetExitKey(KEY_NULL);
        load();
    
        SetWindowIcon(icon);

        SetTargetFPS(fps);
    }

    void make_bullet() {
        if(pause) return;
        if(GetTime() - time_since_last_reload < reload_threshold) {
            return;
        }

        bullet_avaible--;

        if(bullet_avaible<=0) {
            bullet_avaible = bullet_max;
            time_since_last_reload = GetTime();
        }

        make_sound(fx_shoot);

        Bullet bullet = Bullet(&asteroid_texture,player.get_position(), Vector2{10,5},player.get_rotation()-90);
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

        Asteroid asteroid = Asteroid(&asteroid_texture,position,size,angle);
        asteroids.push_back(asteroid);
    }

    void make_sound(Sound sound) {
        if(sound_on && !pause) PlaySound(sound);
    }

    void restart() {
        asteroids.clear();
        bullets.clear();
        score = 0;
        bullet_avaible = bullet_max;
        asteroid_appear_threshold = 0.6f;
        player.set_position(Vector2Scale(screen_size,0.5f));
    }

    bool is_out_of_window(Entity* ent) {
        return (ent->get_position().x < 0 || ent->get_position().x > screen_size.x || ent->get_position().y < 0 || ent->get_position().y > screen_size.y);
    }

    void update_collisions() {
        if(!asteroids.empty()) {
            std::cout << "yea\n";
            for (auto& asteroid : asteroids) {
                // check player collision
                if(asteroid.is_colliding(&player)) {
                    make_sound(fx_die);
                    restart();
                    return;
                }
            }
        }

        if(!bullets.empty()) {
            for (std::vector<Bullet>::iterator it = bullets.begin(); it != bullets.end();) {
                if(is_out_of_window(&(*it))) {
                    it = bullets.erase(it);
                } else {
                    ++it;
                }
            }
        } else {
            return;
        }

        for (std::vector<Asteroid>::iterator it1 = asteroids.begin(); it1 != asteroids.end();) {
            for (std::vector<Bullet>::iterator it = bullets.begin(); it != bullets.end();) {
                if(it1->is_colliding(&(*it))) {
                    make_sound(fx_explotion);

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

    void update() {
        if(IsKeyPressed(KEY_P)) {
            pause = !pause;
        }

        if(IsKeyPressed(KEY_ESCAPE)) {
            if(mode==Mode::GAME) {
                mode = Mode::MENU;
                pause = true;
            } else if(mode==Mode::MENU) {
                mode = Mode::GAME;
                pause = false;
            }
        }

        if(IsKeyPressed(KEY_SPACE)) {
            make_bullet();
        }

        if(GetTime() - time_since_last_asteroid >= asteroid_appear_threshold) {
            make_asteroid();
            time_since_last_asteroid = GetTime();
        }

        if(score % 10 == 0 && score != 0) {
            asteroid_appear_threshold = -1.f/200*score+0.5f;
        }

        if(!pause) {
            if(!asteroids.empty()) {
                for (auto& asteroid : asteroids) {
                    asteroid.update(dt);
                }
            }
            if(!bullets.empty()){
                for (auto& bullet : bullets) {
                    bullet.update(dt);
                }
            }
            if(!asteroids.empty() || !bullets.empty()) {
                update_collisions();
            }
            player.update(dt);
        }
        update_ui();
    }

    void update_ui() {
        score_label.change_text(std::string("Score: ")+std::to_string(score));
        bullets_label.change_text(std::string("Bullets: ")+(GetTime() - time_since_last_reload < reload_threshold ? "0" : std::to_string(bullet_avaible)));
        sound_label.change_text(std::string("Sound: ")+(sound_on ? "ON" : "OFF"));
        if(sound_on) sound_label.change_color(WHITE);
        else if(sound_on) sound_label.change_color(RED);


        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 position = GetMousePosition();

            switch(mode) {
                case Mode::MENU:
                    if(play_label.is_colliding(position)) {
                        mode = Mode::GAME;
                        pause = false;
                    } else if(options_label.is_colliding(position)) {
                        mode = Mode::OPTIONS;
                        pause = true;
                    } else if(exit_label.is_colliding(position)) {
                        game_exit();
                    }
                    break;

                case Mode::OPTIONS:

                    if(sound_label.is_colliding(position)) {
                        sound_on = !sound_on;
                    }
                    break;
                case Mode::GAME:
        
                    
                    break;
                default:
                    break;
            }
        }
    }

    void draw() {     
        DrawTextureEx(bg_texture, Vector2{0,0}, 0, 13, WHITE);
        switch(mode) {
            case Mode::GAME:
                for (auto& bullet : bullets) {
                    bullet.draw();
                    if(debug) bullet.debug_draw();
                }

                for (auto& asteroid : asteroids) {
                    asteroid.draw();
                    if(debug) asteroid.debug_draw();
                } 

                player.draw();
                if(debug) player.debug_draw();

                score_label.draw();
                bullets_label.draw();

                break;
            case Mode::MENU:
                DrawTextureEx(logo_texture, Vector2{screen_size.x/2 - (logo_texture.width*2.5f)/2, screen_size.y*0.1f - logo_texture.height/2}, 0, 2.5f, WHITE);
                    
                play_label.draw();
                options_label.draw();
                exit_label.draw();

                break;
            case Mode::OPTIONS:
                sound_label.draw();
                break;
            default:
                break;
        }
    }

    void game_exit() {
        unload();
        CloseWindow();
        exit(0);
    }

    void mainloop() {
        while (!WindowShouldClose())
        {
            dt = GetFrameTime();

            update();

            BeginDrawing();
                ClearBackground(BLACK);
                draw();
            EndDrawing();
        }
        game_exit();
    }

    void load() {
        player_texture = LoadTexture("assets/player.png");
        asteroid_texture = LoadTexture("assets/asteroid.png");
        logo_texture = LoadTexture("assets/logo.png");
        bg_texture = LoadTexture("assets/bg.png");
        icon = LoadImage("assets/Asteroid Brown.png");

        if(!player_texture.id || !asteroid_texture.id || !logo_texture.id || !bg_texture.id || icon.data==NULL) {
            // show message error
        }

        player = Player(&player_texture,Vector2Scale(screen_size,0.5f),Vector2{32,32});

        font = LoadFont("assets/font.ttf");

        score_label = Label{&font,"Score: 0",30,1,Vector2{screen_size.x*0.8f,screen_size.y*0.1f}};
        bullets_label = Label{&font,"Bullets: 5",30,1,Vector2{screen_size.x*0.1f,screen_size.y*0.1f}};
        play_label = Label{&font,"PLAY",30,1,Vector2{screen_size.x*0.5f,screen_size.y*0.5f}};
        options_label = Label{&font,"OPTIONS",30,1,Vector2{screen_size.x*0.5f,screen_size.y*0.6f}};
        exit_label = Label{&font,"EXIT",30,1,Vector2{screen_size.x*0.5f,screen_size.y*0.7f}};
        sound_label = Label{&font,"Sound: ON",30,1,Vector2{screen_size.x*0.3f,screen_size.y*0.3f}};
        fx_shoot = LoadSound("assets/shooting.mp3");
        fx_die = LoadSound("assets/die.mp3");
        fx_explotion = LoadSound("assets/explotion.mp3");
    }

    void unload() {
        UnloadTexture(player_texture);
        UnloadTexture(asteroid_texture);
        UnloadTexture(logo_texture);
        UnloadTexture(bg_texture);

        UnloadImage(icon);

        UnloadFont(font);
        UnloadSound(fx_shoot);
        UnloadSound(fx_die);
        UnloadSound(fx_explotion);

        CloseAudioDevice();
    }
};