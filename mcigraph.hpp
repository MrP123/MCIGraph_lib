// This is a port of the original mcigraph library (originally developed by Sebastian Stabinger)
// to raylib 5.0 in order to get away from SDL2 as a backend for all of the rendering and to make extending this library easier for motivated students
// For example it now allows directly rendering text using ttf fonts, drawing circles, etc.
// 2023-12-22 - Matthias Panny

#ifndef MCIGRAPH_H
#define MCIGRAPH_H

// Warning: Putting everything in the header file is not good style.
// This is done here for ease of use for educational purposes only!!

#include "raylib.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace mcigraph{

struct MciGraphException {
  std::string message;
  MciGraphException(std::string m) {
    std::cout << "MciGraphException: " << m << std::endl;
    message = m;
  }
};

class TextureCache{

private:
    std::unordered_map<std::string, Texture2D> _cache;

public:
    Texture2D load(std::string filename){
        if(_cache.count(filename) == 0){
            Texture2D tex = LoadTexture(filename.c_str());
            if(tex.id <= 0) throw MciGraphException("The specified image could not be loaded --> verify that the path is correct and the image actually exists!");
            _cache[filename] = tex;
        }
        return _cache[filename];
    }

    ~TextureCache(){
        for(auto i : _cache){
            UnloadTexture(i.second);
        }
    }

    //modified from: https://github.com/raylib-extras/extras-c
    bool SearchAndSetResourceDir(const char* folderName){

        //check the working dir
        if (DirectoryExists(folderName)){
            ChangeDirectory(TextFormat("%s/%s", GetWorkingDirectory(), folderName));
            return true;
        }

        const char* appDir = GetApplicationDirectory();

        //check the application dir
        const char* dir = TextFormat("%s%s", appDir, folderName);
        if (DirectoryExists(dir)){
            ChangeDirectory(dir);
            return true;
        }

        //check 3 levels up from the application dir
        const char* search_paths[] = {"%s../%s", "%s../../%s", "%s../../../%s"};
        for(int i = 0; i < 3; i++){
            dir = TextFormat(search_paths[i], appDir, folderName);
            if (DirectoryExists(dir)){
                ChangeDirectory(dir);
                return true;
            }
        }
        return false;
    }

};

class MciGraph{

private:
    TextureCache _textureCache;

private:
    MciGraph(){
        InitWindow(1280, 720, "mcigraph");
        SetTargetFPS(60);
        //SetExitKey(KEY_NULL); //uncomment if you want to disable closing the application via the escape key

        if (!_textureCache.SearchAndSetResourceDir("tiles"))
            throw MciGraphException("Could not find the \"tiles\" folder");
        TraceLog(LOG_INFO, "Using working/resource dir %s", GetWorkingDirectory());
    }

public:
    bool running(){
        return !WindowShouldClose();
    }

    void clear(){
        ClearBackground(CLITERAL(Color){ 239, 239, 239, 255 });
    }

    void begin_drawing(){
        BeginDrawing();
        clear();
    }

    void end_drawing(){
        EndDrawing();
    }

    void draw_rect(int x, int y, int width, int height, bool outline = false, int red = 0x00, int green = 0x00, int blue = 0x00) {
        if(outline) DrawRectangleLines(x, y, width, height, CLITERAL(Color){ (unsigned char)red, (unsigned char)green, (unsigned char)blue, 255 });
        else DrawRectangle(x, y, width, height, CLITERAL(Color){ (unsigned char)red, (unsigned char)green, (unsigned char)blue, 255 });
    }

    void draw_circle(int cx, int cy, int radius, bool outline = false, int red = 0x00, int green = 0x00, int blue = 0x00) {
        if(outline) DrawCircleLines(cx, cy, radius, CLITERAL(Color){ (unsigned char)red, (unsigned char)green, (unsigned char)blue, 255 });
        else DrawCircle(cx, cy, radius, CLITERAL(Color){ (unsigned char)red, (unsigned char)green, (unsigned char)blue, 255 });
    }

    void draw_line(int x1, int y1, int x2, int y2, int red = 0x00, int green = 0x00, int blue = 0x00) {
        DrawLine(x1, y1, x2, y2, CLITERAL(Color){ (unsigned char)red, (unsigned char)green, (unsigned char)blue, 255 });
    }

    void draw_point(int x, int y, int red = 0x00, int green = 0x00, int blue = 0x00) {
        DrawPixel(x, y, CLITERAL(Color){ (unsigned char)red, (unsigned char)green, (unsigned char)blue, 255 });
    }

    void draw_text(const char* text, int x, int y, int fontSize = 18, int red = 0x00, int green = 0x00, int blue = 0x00){
        DrawText(text, x, y, fontSize, CLITERAL(Color){ (unsigned char)red, (unsigned char)green, (unsigned char)blue, 255 });
    }

    void draw_image(std::string filename, int x, int y, float scale = 1.0f){
        Texture2D texture = _textureCache.load(filename);
        DrawTextureEx(texture, CLITERAL(Vector2){(float)x, (float)y}, 1.0f, scale, WHITE);
    }

    bool is_pressed(int key){
        return IsKeyDown(key);
    }

    bool was_pressed(int key){
        return IsKeyPressed(key) || IsKeyPressedRepeat(key);
    }

    double get_delta_time(){
        return (double) GetFrameTime(); //in seconds
    }

    void set_fps(int fps){
        if(fps < 1) throw MciGraphException("Target FPS cannot be smaller than 1 fps");
        SetTargetFPS(fps);
    }

    int get_screen_width(){
        return GetScreenWidth();
    }

    int get_screen_height(){
        return GetScreenHeight();
    }

    ~MciGraph(){
        CloseWindow();
    }

    static MciGraph &get_instance() {
        static MciGraph mcigraph;
        return mcigraph;
    }

private:
  // Prevent copying and assigning of MciGraph
  MciGraph(const MciGraph &);
  MciGraph &operator=(const MciGraph &);

};

} //namespace mcigraph

// Some fishy stuff is going on after here. This is only done to
// make teaching of an introductory course in C++ easier and should not be taken
// as a good example of how to do things

// All the following "easy-access-functions" are defined to be inline
// to circumvent the one-definition rule for functions. Otherwise this
// would lead to compile errors when using this library in more than
// one file

inline bool running(){
    return mcigraph::MciGraph::get_instance().running();
}

inline void begin_drawing(){
    return mcigraph::MciGraph::get_instance().begin_drawing();
}

inline void end_drawing(){
    return mcigraph::MciGraph::get_instance().end_drawing();
}

inline void draw_rect(int x, int y, int width, int height, bool outline = false, int red = 0x00, int green = 0x00, int blue = 0x00) {
    return mcigraph::MciGraph::get_instance().draw_rect(x, y, width, height, outline, red, green, blue);
}

inline void draw_circle(int cx, int cy, int radius, bool outline = false, int red = 0x00, int green = 0x00, int blue = 0x00) {
    return mcigraph::MciGraph::get_instance().draw_circle(cx, cy, radius, outline, red, green, blue);
}

inline void draw_line(int x1, int y1, int x2, int y2, int red = 0x00, int green = 0x00, int blue = 0x00) {
    return mcigraph::MciGraph::get_instance().draw_line(x1, y1, x2, y2, red, green, blue);
}

inline void draw_point(int x, int y, int red = 0x00, int green = 0x00, int blue = 0x00) {
    return mcigraph::MciGraph::get_instance().draw_point(x, y, red, green, blue);
}

inline void draw_text(const char* text, int x, int y, int fontSize = 18, int red = 0x00, int green = 0x00, int blue = 0x00){
    return mcigraph::MciGraph::get_instance().draw_text(text, x, y, fontSize, red, green, blue);
}

inline void draw_text(std::string text, int x, int y, int fontSize = 18, int red = 0x00, int green = 0x00, int blue = 0x00){
    return mcigraph::MciGraph::get_instance().draw_text(text.c_str(), x, y, fontSize, red, green, blue);
}

inline void draw_image(std::string filename, int x, int y, float scale=1.0f){
    return mcigraph::MciGraph::get_instance().draw_image(filename, x, y, scale);
}

inline bool is_pressed(int key){
    return mcigraph::MciGraph::get_instance().is_pressed(key);
}

inline bool was_pressed(int key){
    return mcigraph::MciGraph::get_instance().was_pressed(key);
}

inline double get_delta_time(){
    return mcigraph::MciGraph::get_instance().get_delta_time();
}

inline void set_fps(int fps){
    return mcigraph::MciGraph::get_instance().set_fps(fps);
}

inline int get_screen_width(){
    return mcigraph::MciGraph::get_instance().get_screen_width();
}

inline int get_screen_height(){
    return mcigraph::MciGraph::get_instance().get_screen_height();
}

//https://github.com/raysan5/raylib/blob/77eeb0010e957a2468deea3ac9f7c74fd3674202/src/raylib.h#L568
const auto KEY_0 = KEY_ZERO;
const auto KEY_1 = KEY_ONE;
const auto KEY_2 = KEY_TWO;
const auto KEY_3 = KEY_THREE;
const auto KEY_4 = KEY_FOUR;
const auto KEY_5 = KEY_FIVE;
const auto KEY_6 = KEY_SIX;
const auto KEY_7 = KEY_SEVEN;
const auto KEY_8 = KEY_EIGHT;
const auto KEY_9 = KEY_NINE;

#endif // MCIGRAPH_H
