#pragma once

#include "raylib.h"
#include "raymath.h"

class Entity {
protected:
    Texture2D* texture;
    Vector2 position;
    Vector2 size;

    float rotation;
    float rotation_speed{300.f};
    float speed;
public:
    Entity() {}
    Entity(Texture* texture, Vector2 pos, Vector2 size, float rotation = 0.f, float speed=100.f) : texture{texture}, position{pos}, size{size}, rotation{rotation}, speed{speed} {}
    virtual ~Entity() = default;

    virtual void update(float dt) = 0;
    virtual void draw() = 0;

    void rotate(float angle) { rotation += angle; }
    void set_position(Vector2 pos) { position = pos; }

    virtual bool is_colliding(Entity* other) const = 0;
    
    Vector2 get_position() const { return position; }
    Vector2 get_size() const { return size; }
    float get_rotation() const { return rotation; }

    bool operator==(const Entity& other) const {
        return position.x == other.position.x && position.y == other.position.y && rotation == other.rotation;
    }

};

class MovableEntity : public Entity {
public:
    MovableEntity(Texture* texture, Vector2 pos, Vector2 size, float rotation = 0.f, float speed = 0.f) : Entity(texture,pos,size,rotation,speed) {}

    void update(float dt) {
        Vector2 rotation_position = Vector2{cos(rotation * DEG2RAD),
                                            sin(rotation * DEG2RAD)};
        Vector2 direction = Vector2Scale(rotation_position, speed * dt);
        position = Vector2Add(position, direction);
    };
};

class Asteroid : public MovableEntity {
public:
    Asteroid();
    Asteroid(Texture2D* texture, Vector2 pos, Vector2 size, float rotation = 0.f) : MovableEntity(texture,pos,size,rotation,100.f) {}

    bool is_colliding(Entity* other) const {
        return CheckCollisionCircles(Vector2{position.x+size.x/2,position.y+size.y/2}, size.x/2.5, other->get_position(), other->get_size().x/2.5f);
    } 

    void debug_draw() {
        DrawCircle(position.x+size.x/2,position.y+size.y/2,size.x/2.5,RED);
    }

    void draw() {
        DrawTexturePro(*texture, Rectangle{0,0,(float)texture->width,(float)texture->height}, 
                       Rectangle{position.x+size.x/2,position.y+size.y/2,size.x,size.y},
                       Vector2{size.x/2,size.y/2}, rotation, WHITE);
    }
};

class Bullet : public MovableEntity {
public:
    Bullet();
    Bullet(Texture2D* texture, Vector2 pos, Vector2 size = Vector2{10,5}, float rotation = 0.f) : MovableEntity(texture,pos,size,rotation,600.f) {}

    bool is_colliding(Entity* other) const {
        return CheckCollisionCircles(Vector2{position.x,position.y}, size.x/2.5, other->get_position(), other->get_size().x/2.5f);
    } 

    void debug_draw() {
        DrawCircle(position.x,position.y,size.x/2.5,RED);
    }

    void draw() {
        DrawRectanglePro(Rectangle{position.x+2.5f,position.y+2.5f,size.x,size.y}, Vector2{5,5}, rotation, WHITE);
    }
};

class Player : public Entity {
public:
    Player() : Entity() {}
    Player(Texture2D* texture,Vector2 pos, Vector2 size, float rotation = 0.f) : Entity(texture,pos,Vector2{32,32},rotation,400.f) {}

    Texture* get_texture() {
        return texture;
    }

    bool is_colliding(Entity* other) const {
        return CheckCollisionCircles(Vector2{position.x,position.y}, size.x/2.5, other->get_position(), other->get_size().x/2.5f);
    }

    void debug_draw() {
        DrawCircle(position.x,position.y,size.x/2.5,RED);
    }

    void update(float dt) {
        Vector2 direction = Vector2Zero();

        if (IsKeyDown(KEY_RIGHT)) {
            rotation += rotation_speed * dt;
        }
        if (IsKeyDown(KEY_LEFT)) {
            rotation -= rotation_speed * dt;
        }

        if (IsKeyDown(KEY_UP)) {
            // the direction have to take account of the rotation
            direction.y -= cos(rotation * DEG2RAD);
            direction.x += sin(rotation * DEG2RAD);
        }
        if (IsKeyDown(KEY_DOWN)) {
            // the direction have to take account of the rotation
            direction.y += cos(rotation * DEG2RAD);
            direction.x -= sin(rotation * DEG2RAD);
        }

        direction = Vector2Normalize(direction);
        direction = Vector2Scale(direction, speed * dt);

        position = Vector2Add(position,direction);
    }

    void draw() {
        DrawTexturePro(*texture, Rectangle{0,0,(float)texture->width,(float)texture->height}, Rectangle{position.x,position.y,size.x,size.y}, Vector2{size.x/2,size.y/2}, rotation, WHITE);
    }
};