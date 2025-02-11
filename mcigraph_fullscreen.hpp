// This is a port of the original mcigraph library (originally developed by Sebastian Stabinger)
// to raylib 5.0 in order to get away from SDL2 as a backend for all of the rendering and to make extending this library easier for motivated students
// For example it now allows directly rendering text using ttf fonts, drawing circles, etc.

// Changelog --> for details see the git history
// 2023-12-22 - Matthias Panny - created initial version
// 2024-03-12 - Matthias Panny - added fullscreen options
// 2024-06-12 - Matthias Panny - fixed bug with image rotation
// 2025-02-11 - Matthias Panny - reworked MciGraphException to inherit from std::runtime_error as an example for exceptions
// 2025-02-11 - Matthias Panny - Changed library to render into a render texture that is drawn at the window scale. This allows better borderless windowed fullscreen support. There is some caveats regarding mouse support.

#ifndef MCIGRAPH_H
#define MCIGRAPH_H

// Warning: Putting everything in the header file is not good style.
// This is done here for ease of use for educational purposes only!!

#include "raylib.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <exception>

namespace mcigraph{

struct MciGraphException : std::runtime_error {
    MciGraphException(const std::string& m) : std::runtime_error(m){};
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

    RenderTexture2D _target;
    int _gameScreenWidth = 1280;
    int _gameScreenHeight = 720;
    float _scale = 1.0f;
    bool _isBorderlessFullscreen = false;

private:
    MciGraph(){
        InitWindow(_gameScreenWidth, _gameScreenHeight, "mcigraph");
        SetTargetFPS(60);
        //SetExitKey(KEY_NULL); //uncomment if you want to disable closing the application via the escape key

        if (!_textureCache.SearchAndSetResourceDir("tiles"))
            throw MciGraphException("Could not find the \"tiles\" folder");
        TraceLog(LOG_INFO, "Using working/resource dir %s", GetWorkingDirectory());

        _target = LoadRenderTexture(_gameScreenWidth, _gameScreenHeight);
        SetTextureFilter(_target.texture, TEXTURE_FILTER_ANISOTROPIC_4X);  // Texture scale filter to use
    }

public:
    bool running(){
        return !WindowShouldClose();
    }

    void clear(){
        ClearBackground(CLITERAL(Color){ 239, 239, 239, 255 });
    }

    void begin_drawing(){
        //Check if window has rescaled this frame --> could also be chandled in the toggle_fullscreen() method
        float wScale = (float)GetScreenWidth()  / _gameScreenWidth;
        float hScale = (float)GetScreenHeight() / _gameScreenHeight;

        float new_scale = (wScale < hScale) ? wScale : hScale;
        if(_scale != new_scale){
            _scale = new_scale;
            SetMouseOffset(-(GetScreenWidth() - (_gameScreenWidth*_scale))*0.5f, -(GetScreenHeight() - (_gameScreenHeight*_scale))*0.5f);
            SetMouseScale(1/_scale, 1/_scale);
        }

        //If it is undesirable that GetMousePosition delivers negative values with wSacle & hScale are not equal, then a virtual mouse pointer can be implemented
        //This requires a (private) member variable _virtualMouse of type Vector2 to be defined in the MciGraph class.
        //If both the normal and virtual mouse pointer are used it is required that the offset and scale are set to 0 and 1 respectively before the vitual mouse is initialized with the normal one.
        //This must be done each frame, not only when the scale changes!
        //SetMouseOffset(0, 0);
        //SetMouseScale(1.0f, 1.0f);
        //Vector2 mouse = GetMousePosition();
        //_virtualMouse.x = (mouse.x - (GetScreenWidth()  - (_gameScreenWidth  * _scale)) * 0.5f) / _scale;
        //_virtualMouse.y = (mouse.y - (GetScreenHeight() - (_gameScreenHeight * _scale)) * 0.5f) / _scale;
        //_virtualMouse = Vector2Clamp(_virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float)_gameScreenWidth, (float)_gameScreenHeight });

        //Begin drawing on the render texture --> will only be drawn to screen after end_drawing()
        BeginTextureMode(_target);
        clear(); //clear render texture
    }

    void end_drawing(){
        //Ends the drawing to the render texture
        EndTextureMode();

        //and actually draws it
        BeginDrawing();
            clear(); //clear screen
            DrawTexturePro(
                _target.texture,
                (Rectangle){ 0.0f, 0.0f, (float)_target.texture.width, (float)-_target.texture.height },
                (Rectangle){
                    (GetScreenWidth()  - ((float)_gameScreenWidth  * _scale)) * 0.5f,
                    (GetScreenHeight() - ((float)_gameScreenHeight * _scale)) * 0.5f,
                    (float)_gameScreenWidth  * _scale,
                    (float)_gameScreenHeight * _scale
                },
                (Vector2){ 0, 0 }, 0.0f, WHITE
            );
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

    void draw_image(std::string filename, int x, int y, float _scale = 1.0f, float rot_deg = 0.0f){
        Texture2D texture = _textureCache.load(filename);
        DrawTextureEx(texture, CLITERAL(Vector2){(float)x, (float)y}, rot_deg, _scale, WHITE);
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

    void set_fullscreen(){
        if(!_isBorderlessFullscreen) toggle_fullscreen();
    }

    void unset_fullscreen(){
        if(_isBorderlessFullscreen) toggle_fullscreen();
    }

    void toggle_fullscreen(){
        _isBorderlessFullscreen = !_isBorderlessFullscreen;
        ToggleBorderlessWindowed();
    }

    bool is_fullscreen(){
        //borderless windowed fullscreen is not the same as "real" fullscreen --> custom member of MciGraph class is used to keep track of this
        return _isBorderlessFullscreen;
    }

    int get_screen_width(){
        return _gameScreenWidth;
        //return GetScreenWidth();
    }

    int get_screen_height(){
        return _gameScreenHeight;
        //return GetScreenHeight();
    }

    ~MciGraph(){
        UnloadRenderTexture(_target);
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

inline void draw_image(std::string filename, int x, int y, float scale = 1.0f, float rot_deg = 0.0f){
    return mcigraph::MciGraph::get_instance().draw_image(filename, x, y, scale, rot_deg);
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

inline void set_fullscreen(){
    return mcigraph::MciGraph::get_instance().set_fullscreen();
}

inline void unset_fullscreen(){
    return mcigraph::MciGraph::get_instance().unset_fullscreen();
}

inline void toggle_fullscreen(){
    return mcigraph::MciGraph::get_instance().toggle_fullscreen();
}

inline bool is_fullscreen(){
    return mcigraph::MciGraph::get_instance().is_fullscreen();
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
