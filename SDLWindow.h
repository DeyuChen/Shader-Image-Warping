#ifndef SDLWINDOW_H
#define SDLWINDOW_H

#include "common.h"
#include <string>

using namespace std;

const float MIN_DISTANCE = 0.1f;
const float MAX_DISTANCE = 100.0f;
const float RAD_UNIT = 3.14159265 / 180.0;

class SDLWindow {
public:
    SDLWindow();
    ~SDLWindow();

    bool init();
    void close();
    void setTitle(string title);
    void setPosition(int x, int y);
    void setWindowSize(int w, int h);
    void enable3D(bool enable);
    void handleEvent(SDL_Event& e, int x, int y);
    void mouseMotion(int x, int y);
    void render();
    void render2D(glm::mat4 refMVP);

    void setVertices(GLfloat* vertices, int size);
    void setColors(GLfloat* colors, int size);
    void setDepths(GLfloat* depths, int size);
    void setIndices(GLuint* indices, int size);

    bool loadProgram(string vsFile, string fsFile);

    bool isShown();
    bool isFocused();
    bool isMouseFocused();
    int getWindowID();

    bool readColors(GLfloat* buf, size_t bufSize);
    bool readDepths(GLfloat* buf, size_t bufSize);
    glm::mat4 getMVP();

private:
    GLuint loadShaderFromFile(string filename, GLenum shaderType);
    bool loadVSProgram(string filename);
    bool loadFSProgram(string filename);

    SDL_Window* window;
    SDL_GLContext context;
    SDL_Renderer* renderer;
    int windowID;

    int width;
    int height;

    float viewX, viewY, viewZ;
    float speed;
    float azimuth;
    float elevation;
    
    bool shown;
    bool focused;
    bool mfocused;
    bool depthEnabled;

    GLuint vao;
    GLuint vbo[3]; // position, color, depth
    GLuint ibo;
    int indSize;

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint shaderProgram;
};

#endif
