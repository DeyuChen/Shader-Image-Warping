#include "SDLWindow.h"
#include <iostream>
#include <fstream>

const int DEFAULT_SCREEN_WIDTH = 640;
const int DEFAULT_SCREEN_HEIGHT = 480;

SDLWindow::SDLWindow(){
    window = NULL;
    renderer = NULL;

    width = DEFAULT_SCREEN_WIDTH;
    height = DEFAULT_SCREEN_HEIGHT;

    viewX = viewY = viewZ = 0;
    speed = 0.1;
    azimuth = elevation = 0;

    shown = false;
    focused = false;
    depthEnabled = false;
}

SDLWindow::~SDLWindow(){
    if (window != NULL){
        SDL_DestroyWindow(window);
    }
}

bool SDLWindow::init(){
    window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (window == NULL){
        cerr << "Creating window failed" << endl;
        return false;
    }

    context = SDL_GL_CreateContext(window);
    if (context == NULL){
        printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK){
        printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
    }

    if (SDL_GL_SetSwapInterval(1) < 0){
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    windowID = SDL_GetWindowID(window);
    string title("Window ");
    title += to_string(windowID);
    SDL_SetWindowTitle(window, title.c_str());

    glGenVertexArrays(1, &vao);
    glGenBuffers(2, vbo);
    glGenBuffers(1, &ibo);
    
    return true;
}

void SDLWindow::setWindowSize(int w, int h){
    SDL_GL_MakeCurrent(window, context);
    
    width = w;
    height = h;
    SDL_SetWindowSize(window, width, height);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glViewport(0, 0, width, height);
}

void SDLWindow::enable3D(bool enable){
    SDL_GL_MakeCurrent(window, context);

    depthEnabled = enable;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, MIN_DISTANCE, MAX_DISTANCE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void SDLWindow::handleEvent(SDL_Event& e, int x, int y){
    if (e.type == SDL_WINDOWEVENT && e.window.windowID == windowID){
        switch(e.window.event){
            case SDL_WINDOWEVENT_SHOWN:
                shown = true;
                break;

            case SDL_WINDOWEVENT_HIDDEN:
                shown = false;
                break;

            case SDL_WINDOWEVENT_FOCUS_GAINED:
                focused = true;
                break;

            case SDL_WINDOWEVENT_FOCUS_LOST:
                focused = false;
                break;

            case SDL_WINDOWEVENT_CLOSE:
                SDL_HideWindow(window);
                break;
        }
    }
    else if (e.type == SDL_KEYDOWN){
        switch (e.key.keysym.sym){
            case SDLK_w:
                viewZ += speed * cos(azimuth * RAD_UNIT);
                viewX -= speed * sin(azimuth * RAD_UNIT);
                break;

            case SDLK_d:
                viewZ -= speed * sin(azimuth * RAD_UNIT);
                viewX -= speed * cos(azimuth * RAD_UNIT);
                break;

            case SDLK_s:
                viewZ -= speed * cos(azimuth * RAD_UNIT);
                viewX += speed * sin(azimuth * RAD_UNIT);
                break;

            case SDLK_a:
                viewZ += speed * sin(azimuth * RAD_UNIT);
                viewX += speed * cos(azimuth * RAD_UNIT);
                break;

            case SDLK_e:
                viewY += speed;
                break;

            case SDLK_q:
                viewY -= speed;
                break;
        }
    }
}

void SDLWindow::mouseMotion(int x, int y){
    azimuth += 180 * x / (float)width;
    elevation += 180 * y / (float)height;
}

void SDLWindow::render(){
    SDL_GL_MakeCurrent(window, context);

    glUseProgram(shaderProgram);

    glLoadIdentity();
    gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);
    glRotatef(elevation, 1, 0, 0);
    glRotatef(azimuth, 0, 1, 0);
    glTranslatef(viewX, viewY, viewZ);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawElements(depthEnabled? GL_QUADS : GL_POINTS, indSize, GL_UNSIGNED_INT, NULL);
    
    glUseProgram(0);

    SDL_GL_SwapWindow(window);
}

void SDLWindow::setVertices(GLfloat* vertices, int size){
    SDL_GL_MakeCurrent(window, context);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, depthEnabled? 3 : 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
}

void SDLWindow::setColors(GLfloat* colors, int size){
    SDL_GL_MakeCurrent(window, context);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(GLfloat), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
}

void SDLWindow::setIndices(GLuint* indices, int size){
    indSize = size;
    SDL_GL_MakeCurrent(window, context);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(GLuint), indices, GL_STATIC_DRAW);
}

bool SDLWindow::loadProgram(string vsFile, string fsFile){
    SDL_GL_MakeCurrent(window, context);
    if (!loadVSProgram(vsFile)){
        cerr << "Loading vertex shader program failed" << endl;
        return false;
    }
    if (!loadFSProgram(fsFile)){
        cerr << "Loading fragment shader program failed" << endl;
        return false;
    }

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glBindAttribLocation(shaderProgram, 0, "in_Position");
    glBindAttribLocation(shaderProgram, 1, "in_Color");

    glLinkProgram(shaderProgram);

    GLint isLinked = GL_TRUE;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE){
        cerr << "Failed to link shader program" << endl;
        return false;
    }
}


bool SDLWindow::isShown(){
    return shown;
}

bool SDLWindow::isFocused(){
    return focused;
}

int SDLWindow::getWindowID(){
    return windowID;
}

bool SDLWindow::readColors(GLfloat* buf, size_t bufSize){
    if (bufSize != 3 * sizeof(GLfloat) * width * height)
        return false;

    SDL_GL_MakeCurrent(window, context);

    unsigned char *pixelRGB = new unsigned char[3 * width * height];
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixelRGB);
    for (int i = 0; i < 3 * width * height; i++){
        buf[i] = (GLfloat)pixelRGB[i] / 255.0;
    }
    delete[] pixelRGB;

    return true;
}

GLuint SDLWindow::loadShaderFromFile(string filename, GLenum shaderType){
	GLuint shaderID = 0;
	string content;
	ifstream ifs(filename.c_str());

    if (ifs){
        content.assign((istreambuf_iterator<char>(ifs) ), istreambuf_iterator<char>());

        shaderID = glCreateShader(shaderType);
        const char *pcontent = content.c_str();
        glShaderSource(shaderID, 1, (const GLchar**)&pcontent, NULL);
        glCompileShader(shaderID);

        GLint shaderCompiled = GL_FALSE;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
        if (shaderCompiled != GL_TRUE){
            cerr << "Shader failed to compile" << endl;

            int maxLength;
            char *log;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);
            log = (char *)malloc(maxLength);
            glGetShaderInfoLog(shaderID, maxLength, &maxLength, log);
            cout << log << endl;
            free(log);

            glDeleteShader(shaderID);
            shaderID = 0;
        }
	}
    else {
        cout << "Open shader file failed" << endl;
    }

	return shaderID;
}

bool SDLWindow::loadVSProgram(string filename){
    vertexShader = loadShaderFromFile(filename, GL_VERTEX_SHADER);

    if (vertexShader == 0){
        return false;
    }

    return true;
}

bool SDLWindow::loadFSProgram(string filename){
    fragmentShader = loadShaderFromFile(filename, GL_FRAGMENT_SHADER);

    if (fragmentShader == 0){
        return false;
    }

    return true;
}

