#pragma once

#include <string>

#include "raylib.h"

struct Label {
    Font* font;
    std::string text;
    int size_font;
    int spacing;
    Vector2 position;
    Vector2 size;
    Color color{255,255,255,255};

    Label() {}

    Label(Font* font, const std::string& text, int size_font, int spacing, Vector2 position) : font{font}, text{text}, size_font{size_font}, spacing{spacing}, position{position} {
        size = MeasureTextEx(*font, text.c_str(),size_font,spacing);
    }

    void set_position(Vector2 pos) {
        size = MeasureTextEx(*font, text.c_str(),size_font,spacing);
        position = pos;
    }

    bool is_colliding(Vector2 pos) {
        Vector2 center_pos = Vector2Subtract(position,Vector2Scale(size,0.5f));
        return CheckCollisionPointRec(pos, Rectangle{center_pos.x,center_pos.y,size.x,size.y});
    }

    void change_text(const std::string& text) {
        this->text = text;
        size = MeasureTextEx(*font, text.c_str(),size_font,spacing);
    }

    void change_color(Color color) {
        this->color = color;
    }

    void draw(bool always_on_screen = true) {
        Vector2 center_pos = Vector2Subtract(position,Vector2Scale(size,0.5f));
        if(always_on_screen) {
            if(center_pos.x+size.x/2 > GetScreenWidth()) center_pos.x = GetScreenWidth() - size.x/2;
            else if(center_pos.x < 0) center_pos.x = position.x;
            if(center_pos.y+size.y/2 > GetScreenHeight()) center_pos.y = GetScreenHeight() - size.y/2;
            else if (center_pos.y < 0) center_pos.y = position.y;
        }
        DrawTextEx(*font, text.c_str(), center_pos, size_font, spacing, color);
    }
};