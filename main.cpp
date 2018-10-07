#include "SDLWindow.h"
#include <iostream>
#include <unordered_map>

using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

bool initGL(){
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 0;
    }
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")){
        printf("Warning: Linear texture filtering not enabled!");
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR){
        printf("Error initializing OpenGL! %s\n", gluErrorString(error));
        return false;
    }

    return true;
}

int main(){
    if (!initGL){
        cout << "Initialize OpenGL failed" << endl;
        return 0;
    }
    
    SDLWindow windows[2];
    unordered_map<int, int> IDMap;
    
    for (int i = 1; i >= 0; i--){
        if (!windows[i].init()){
            cout << "Window initialization failed" << endl;
            return 0;
        }
        IDMap[windows[i].getWindowID()] = i;
    }
    
    GLfloat vertices[24] = {
    1.0, 1.0, 1.0,
    -1.0, 1.0, 1.0,
    1.0, -1.0, 1.0,
    -1.0, -1.0, 1.0,
    1.0, 1.0, -1.0,
    -1.0, 1.0, -1.0,
    1.0, -1.0, -1.0,
    -1.0, -1.0, -1.0};
    
    GLfloat colors[24] = {
    0.0, 1.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 0.0, 1.0,
    0.0, 1.0, 1.0,
    0.0, 0.0, 1.0,
    0.0, 0.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 1.0, 1.0};
    
    GLuint indices[14] = {3, 2, 7, 6, 4, 2, 0, 3, 1, 7, 5, 4, 1, 0};
    
    GLfloat *pixelVertices = new GLfloat[2 * SCREEN_WIDTH * SCREEN_HEIGHT];
    int count = 0;
    for (int i = 0; i < SCREEN_HEIGHT; i++){
        for (int j = 0; j < SCREEN_WIDTH; j++){
            pixelVertices[count++] = (GLfloat)j * 2.0 / SCREEN_WIDTH - 1.0;
            pixelVertices[count++] = (GLfloat)i * 2.0 / SCREEN_HEIGHT - 1.0;
        }
    }

    GLfloat *pixelColors = new GLfloat[3 * SCREEN_WIDTH * SCREEN_HEIGHT];

    GLuint *pixelIndices = new GLuint[SCREEN_WIDTH * SCREEN_HEIGHT];
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++){
        pixelIndices[i] = i;
    }
    
    GLfloat *pixelDepths = new GLfloat[SCREEN_WIDTH * SCREEN_HEIGHT];

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);

    windows[1].setTitle("Warped Image");
    windows[1].setPosition(DM.w / 2, DM.h / 2 - SCREEN_HEIGHT / 2);
    windows[1].setWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    windows[1].setVertices(pixelVertices, 2 * SCREEN_WIDTH * SCREEN_HEIGHT);
    windows[1].setIndices(pixelIndices, SCREEN_WIDTH * SCREEN_HEIGHT);
    windows[1].loadProgram("2DShader.vert", "2DShader.frag");
    
    windows[0].setTitle("Original Image");
    windows[0].setPosition(DM.w / 2 - SCREEN_WIDTH, DM.h / 2 - SCREEN_HEIGHT / 2);
    windows[0].setWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    windows[0].enable3D(true);
    windows[0].setVertices(vertices, 24);
    windows[0].setColors(colors, 24);
    windows[0].setIndices(indices, 14);
    windows[0].loadProgram("3DShader.vert", "3DShader.frag");

    bool quit = false;
    SDL_Event e;
    SDL_StartTextInput();
    SDL_SetRelativeMouseMode(SDL_TRUE);
    while (!quit){
        int x = 0, y = 0;
        SDL_GetRelativeMouseState(&x, &y);
        
        while (SDL_PollEvent(&e) != 0){
            if (e.type == SDL_QUIT){
                quit = true;
                break;
            }

            windows[IDMap[e.window.windowID]].handleEvent(e, x, y);
        }
        
        for (int i = 0; i < 2; i++){
            if (windows[i].isFocused() && windows[i].isMouseFocused()){
                windows[i].mouseMotion(x, y);
            }
        }
        
        if (!(windows[0].isShown() && windows[1].isShown())){
            quit = true;
        }
        else {
            windows[0].render();
            windows[0].readColors(pixelColors, sizeof(GLfloat) * 3 * SCREEN_WIDTH * SCREEN_HEIGHT);
            windows[0].readDepths(pixelDepths, sizeof(GLfloat) * SCREEN_WIDTH * SCREEN_HEIGHT);
            if (windows[1].isShown()){
                windows[1].setColors(pixelColors, 3 * SCREEN_WIDTH * SCREEN_HEIGHT);
                windows[1].setDepths(pixelDepths, SCREEN_WIDTH * SCREEN_HEIGHT);
                windows[1].render2D(windows[0].getMVP());
            }
        }
    }
    
    SDL_StopTextInput();
    for (int i = 0; i < 2; i++){
        windows[i].close();
    }
    
    delete[] pixelVertices;
    delete[] pixelColors;
    delete[] pixelIndices;

    return 0;
}
