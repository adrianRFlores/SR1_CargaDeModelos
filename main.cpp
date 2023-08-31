#include <iostream>
#include <math.h>
#include <algorithm>
#include <SDL2/SDL.h>
#include "loadOBJ.h"
#include <omp.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//Framebuffer
const int FRAMEBUFFER_WIDTH = 1000, FRAMEBUFFER_HEIGHT = 800;
const int FRAMEBUFFER_SIZE = FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT;

//Matrices de escala, traslación y rotación
glm::mat4 m = glm::mat4(1.0f); //Solo se inicializa, se define en main
glm::mat4 scale = glm::mat4(80.0f);
glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(400, 200, 0));
std::vector<glm::vec4> vec4Array;

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    Color(int red = 0, int green = 0, int blue = 0, int alpha = 255)
        : r(colorClamp(red)), g(colorClamp(green)), b(colorClamp(blue)), a(colorClamp(alpha)) {}

    private:
        //Clamp para evitar valores invalidos
        unsigned char colorClamp(const int value) {
            if (value > 255) return 255;
            else if (value < 0) return 0;
            else return value;
        }      

};

Color framebuffer[FRAMEBUFFER_SIZE];
Color currentColor(255,255,255,255);
Color clearColor(0,0,0,255);

void setCurrentColor(const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a) {
	currentColor = Color(r, g, b, a);
}

void clear() {
    for (int i = 0; i < FRAMEBUFFER_SIZE; i++) {
        framebuffer[i] = clearColor;
    }
}

//Pone un punto con el color actual en x, y de un vec4
//c es el alfa del color a dibujar (0 = negro a 1 = 100%)
inline void point(const glm::vec4& vert, const float c) {
    if (vert.x+500 >= 0 && vert.x+500 < FRAMEBUFFER_WIDTH && vert.y+400 >= 0 && vert.y+400 < FRAMEBUFFER_HEIGHT) {
        framebuffer[(int)(vert.y+400) * FRAMEBUFFER_WIDTH + (int)vert.x+500] = Color(currentColor.r * c, currentColor.g * c, currentColor.b * c);
    }
}

//Se utiliza solo para el algoritmo de anti-aliasing
inline void point(const int x, const int y, const float c) {
    if (x+500 >= 0 && x+500 < FRAMEBUFFER_WIDTH && y+400 >= 0 && y+400 < FRAMEBUFFER_HEIGHT) {
        framebuffer[(y+400) * FRAMEBUFFER_WIDTH + x + 500] = Color(currentColor.r * c, currentColor.g * c, currentColor.b * c);
    }
}

void renderBuffer(SDL_Renderer* renderer) {
    // Create a texture
    SDL_Texture* texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        FRAMEBUFFER_WIDTH, 
        FRAMEBUFFER_HEIGHT
    );

    // Update the texture with the pixel data from the framebuffer
    SDL_UpdateTexture(
        texture, 
        NULL, 
        framebuffer, 
        FRAMEBUFFER_WIDTH * sizeof(Color)
    );

    // Copy the texture to the renderer
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Destroy the texture
    SDL_DestroyTexture(texture);
}

//Método de linea de bresenham
//rápido pero feo
void line(const glm::vec3& p1, const glm::vec3& p2){
    glm::vec4 x0(p1.x, p1.y, p1.z, 0.0f);
	int dx = abs((int)(p2.x - p1.x));
	int sx = p1.x < p2.x ? 1 : -1;
	int dy = -abs((int)(p2.y - p1.y));
	int sy = p1.y < p2.y ? 1 : -1;
	int error = dx + dy;

	while(true) {
		point(x0, 1);
		if((int)x0.x*100 == (int)p2.x*100 && (int)x0.y*100 == (int)p2.y*100) break;
		int e2 = 2 * error;
		if(e2 >= dy) {
			if((int)x0.x*100 == (int)p2.x*100) break;
			error += dy;
			x0 = glm::vec4(x0.x + sx, x0.y, x0.z, 0.0f);
		}
		if(e2 <= dx) {
			if((int)x0.y*100 == (int)p2.y*100) break;
			error += dx;
			x0 = glm::vec4(x0.x, x0.y + sy, x0.z, 0.0f);
		}
	}
}

//Retorna la parte decimal de x
float fpart(const float& x) {
    return x - floor(x);
}

//Retorna el complemento (cantidad para llegar al siguiente entero) de la parte decimal de x
float rfpart(const float& x) {
    return 1 - fpart(x);
}

//Método para dibujar una linea de p1 a p2 utilizando el metodo de linea de Xiaolin Wu (anti-aliasing)
//obviamente es un poco más lento que la linea de bresenham
void AAline(const glm::vec4& p1, const glm::vec4& p2) {

    glm::vec4 v1 = p1;
    glm::vec4 v2 = p2;

    bool steep = fabs(p2.y - p1.y) > fabs(p2.x - p1.x);

    float temp;

    if(steep) {
        temp = v1.x;
        v1.x = v1.y;
        v1.y = temp;
        temp = v2.x;
        v2.x = v2.y;
        v2.y = temp;
    }
    if(v1.x > v2.x) {
        temp = v1.x;
        v1.x = v2.x;
        v2.x = temp;
        temp = v1.y;
        v1.y = v2.y;
        v2.y = temp;
    }

    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;

    float gradient;

    if((int)(v2.x*100) - (int)(v1.x*100) == 0){
        gradient = 1.0f;
    } else {
        gradient = dy / dx;
    }

    int xend = round(v1.x);
    float yend = v1.y + gradient * (xend - v1.x);
    float xgap = rfpart(v1.x + 0.5f);
    int xpxl1 = xend;
    int ypxl1 = floor(yend);

    if(steep) {
        point(ypxl1, xpxl1, rfpart(yend) * xgap);
        point(ypxl1+1, xpxl1, fpart(yend) * xgap);
    } else {
        point(xpxl1, ypxl1, rfpart(yend) * xgap);
        point(xpxl1, ypxl1+1, fpart(yend) * xgap);
    }

    float intery = yend + gradient;

    xend = round(v2.x);
    yend = v2.y + gradient * (xend - v2.x);
    xgap = fpart(v2.x + 0.5);
    int xpxl2 = xend;
    int ypxl2 = floor(yend);

    if(steep) {
        point(ypxl2, xpxl2, rfpart(yend) * xgap);
        point(ypxl2+1, xpxl2, fpart(yend) * xgap);
    } else {
        point(xpxl2, ypxl2, rfpart(yend) * xgap);
        point(xpxl2, ypxl2+1, fpart(yend) * xgap);
    }

    if(steep) {
        for(int x = xpxl1 + 1; x < xpxl2 -1; x++) {
            point(floor(intery), x, rfpart(intery));
            point(floor(intery)+1, x, fpart(intery));
            intery += gradient;
        }
    } else{
        for(int x = xpxl1 + 1; x < xpxl2 -1; x++) {
            point(x, floor(intery), rfpart(intery));
            point(x, floor(intery)+1, fpart(intery));
            intery += gradient;
        }
    }
    
}

//Triangulo que usa los metodos de linea de Bresenham
void triangle(const glm::vec4& p1, const glm::vec4& p2, const glm::vec4& p3){
    line(p1, p2);
    line(p2, p3);
    line(p3, p1);
}

//Triangulo que usa los métodos con anti-aliasing
void AAtriangle(const glm::vec4& p1, const glm::vec4& p2, const glm::vec4& p3){
    AAline(p1, p2);
    AAline(p2, p3);
    AAline(p3, p1);
}

//Crea un array ordenado a partir de Face donde cada 3 vertices es una cara
//Solo se deben llamar los vectores de 3 en 3 luego de esto
std::vector<glm::vec3> setupVertexArray(const std::vector<glm::vec3>& vertices, const std::vector<Face>& faces) {
    std::vector<glm::vec3> vertexArray;
    
    // For each face
    for (const auto& face : faces)
    {
        // For each vertex in the face
        for (const auto& vertexIndices : face.vertexIndices)
        {
            // Get the vertex position and normal from the input arrays using the indices from the face
            glm::vec3 vertexPosition = vertices[vertexIndices[0]];

            // Add the vertex position and normal to the vertex array
            vertexArray.push_back(vertexPosition);
        }
    }

    return vertexArray;
}

//Agarra los vertices en grupos de 3 (porque se supone que están ordenados) y los dibuja con la funcion de linea de bresenham
void drawTriVec(const std::vector<glm::vec4>& vertexArray){
    for(int i = 0; i < vertexArray.size(); i +=3){
        triangle(vertexArray[i], vertexArray[i+1], vertexArray[i+2]);
    }
}

//Agarra los vertices en grupos de 3 (porque se supone que están ordenados) y los dibuja con la funcion con anti-aliasing
void AAdrawTriVec(const std::vector<glm::vec4>& vertexArray){
    #pragma omp parallel for //paralelismo
    for(int i = 0; i < vertexArray.size(); i +=3){
        AAtriangle(vertexArray[i], vertexArray[i+1], vertexArray[i+2]);
    }
}

void render(SDL_Renderer* renderer) {

    clear();

    // Cada ciclo, se le aplica la matriz de rotación a cada punto
    #pragma omp parallel for //paralelismo
    for (int i = 0; i < vec4Array.size(); i++) {
        vec4Array[i] = m * vec4Array[i];
    }

    // Se dibujan los triangulos con los vectores rotados
    AAdrawTriVec(vec4Array);

    // Render the framebuffer to the screen
    renderBuffer(renderer);
    
}

int main(int argc, char* argv[]) {

    //Vanidad
    setCurrentColor(255,255,255,255);

    // Vectores que se le pasan a loadOBJ para poder almacenar la lectura del archivo
    std::vector<glm::vec3> vertices;
    std::vector<Face> faces;

    //Se cargan los vertices y las caras del archivo
    bool loaded = loadOBJ("nave.obj", vertices, faces);
    //Se utiliza el resultado de vertices y caras para crear un solo array con los vertices ordenados
    std::vector<glm::vec3> vertexArray = setupVertexArray(vertices, faces);

    //Para la mayoria de archivos .obj de blender, los modelos resultantes quedan muuuuuy peques
    //Se aplica la matriz de escala para cada vector para aumentarle el tamaño
    for (const auto& vec : vertexArray) {
        vec4Array.push_back(scale * glm::vec4(vec.x, vec.y, vec.z, 0.0f));
    }

    //Se define la matriz de rotación
    //se rota por 0.01deg cada ciclo porque si no se pone un límite de fps llega a girar demasiado rapido
    m = glm::rotate(m, 0.01f, glm::vec3(0.0f, 1.0f, 1.0f)); //Se gira sobre el eje y (vertical) y z (atras/adelante)

    for(int i = 0; i<vec4Array.size(); i++){
        vec4Array[i] = m * vec4Array[i];
    }

    //se supone que esto mueve el objeto al centro de la pantalla pero no hace nada xd
    //(es show)
    for(int i = 0; i<vec4Array.size(); i++){
        vec4Array[i] = translate * vec4Array[i];
    }

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("SR1", 50, 50, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    bool running = true;
    SDL_Event event;
    Uint32 frameStart, frameTime;
 
    while (running) {

        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Call our render function
        render(renderer);

        // Present the frame buffer to the screen
        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;

        if (frameTime > 0) {
            std::ostringstream titleStream;
            titleStream << "FPS: " << 1000.0 / frameTime;  // Milliseconds to seconds
            SDL_SetWindowTitle(window, titleStream.str().c_str());
        }

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}