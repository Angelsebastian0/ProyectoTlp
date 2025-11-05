
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

// ---------------- Configuración ----------------
const int WIN_W = 640;
const int WIN_H = 480;
const int TARGET_FPS = 60;
const int FRAME_DELAY_MS = 1000 / TARGET_FPS;

// Brick (ladrillo) 
struct Brick {
    int x, y;
    int w, h;
    SDL_Color color;
};


struct InputState {
    bool left = false;
    bool right = false;
    bool quit = false;
};

// -------------- Utilidades gráficas --------------
void drawRect(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color col) {
    SDL_Rect r{ x, y, w, h };
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    SDL_RenderFillRect(renderer, &r);
}

void drawBrick(SDL_Renderer* renderer, const Brick& b) {

    SDL_Color border = { (Uint8)std::max(0, b.color.r - 40), (Uint8)std::max(0, b.color.g - 40), (Uint8)std::max(0, b.color.b - 40), 255 };
    drawRect(renderer, b.x, b.y, b.w, b.h, b.color);

    SDL_Rect br{ b.x, b.y, b.w, 3 };
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
    SDL_RenderFillRect(renderer, &br);
}

SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color) {
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surf) return nullptr;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

void drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    SDL_Texture* tex = renderText(renderer, font, text, color);
    if (!tex) return;
    int tw, th;
    SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
    SDL_Rect dst{ x, y, tw, th };
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

void drawScore(SDL_Renderer* renderer, TTF_Font* font, int score, int x, int y) {
    SDL_Color white{ 255,255,255,255 };
    drawText(renderer, font, "Score: " + std::to_string(score), x, y, white);
}

// -------------- Main --------------
int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << "\n";
        return 1;
    }
    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init error: " << TTF_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("MiniEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, 0);
    if (!window) {
        std::cerr << "Window create error: " << SDL_GetError() << "\n";
        TTF_Quit(); SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer create error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window); TTF_Quit(); SDL_Quit();
        return 1;
    }

    // Cargar fuente (intenta cargar una fuente del sistema)
    // Si falla, el texto no se mostrará pero todo lo demás funciona.
    TTF_Font* font = TTF_OpenFont("DejaVuSans.ttf", 18);
    if (!font) {
        // intentar fuente alternativa en Windows (no fatal)
        font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 18);
    }
    if (!font) {
        std::cerr << "Advertencia: no se pudo cargar fuente (DejaVuSans.ttf o Arial). El texto no se renderizará.\n";
    }

    // Inicializar estado del juego
    Brick player{ WIN_W/2 - 30, WIN_H - 40, 60, 16, {200, 60, 60, 255} };
    int playerSpeed = 300; // px / seg
    InputState input;
    int score = 0;

    // Ejemplo 
    Brick sampleBrick{ 100, 60, 50, 20, {70,160,220,255} };

    auto lastTime = std::chrono::high_resolution_clock::now();

    // Bucle principal
    while (!input.quit) {
        // Tiempo
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> dtf = now - lastTime;
        lastTime = now;
        double dt = dtf.count(); // segundos

        // Eventos
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) input.quit = true;
            else if (ev.type == SDL_KEYDOWN) {
                SDL_Keycode k = ev.key.keysym.sym;
                if (k == SDLK_LEFT || k == SDLK_a) input.left = true;
                if (k == SDLK_RIGHT || k == SDLK_d) input.right = true;
                if (k == SDLK_ESCAPE) input.quit = true;
            } else if (ev.type == SDL_KEYUP) {
                SDL_Keycode k = ev.key.keysym.sym;
                if (k == SDLK_LEFT || k == SDLK_a) input.left = false;
                if (k == SDLK_RIGHT || k == SDLK_d) input.right = false;
            }
        }

        // Actualizar lógica
        int move = 0;
        if (input.left) move -= 1;
        if (input.right) move += 1;

        player.x += (int)std::round(move * playerSpeed * dt);
        // limitar a ventana
        if (player.x < 0) player.x = 0;
        if (player.x + player.w > WIN_W) player.x = WIN_W - player.w;

        // para demostración, si el jugador toca el sampleBrick sube score
        SDL_Rect rPlayer{ player.x, player.y, player.w, player.h };
        SDL_Rect rSample{ sampleBrick.x, sampleBrick.y, sampleBrick.w, sampleBrick.h };
        if (SDL_HasIntersection(&rPlayer, &rSample)) {
            score += 1;
            // alejar sample brick para evitar sumar infinito
            sampleBrick.x = (sampleBrick.x + 120) % (WIN_W - sampleBrick.w);
        }

        // Render
        // Limpiar (fondo)
        SDL_SetRenderDrawColor(renderer, 18, 18, 28, 255);
        SDL_RenderClear(renderer);

        // Dibujar ladrillos y elementos
        drawBrick(renderer, sampleBrick);
        drawBrick(renderer, player);

        // Dibujar texto y score
        if (font) {
            SDL_Color white{ 255,255,255,255 };
            drawText(renderer, font, "MiniEngine - Segunda entrega", 10, 8, white);
            drawScore(renderer, font, score, WIN_W - 140, 8);
            drawText(renderer, font, "Usa <- -> o A D para mover el ladrillo", 10, WIN_H - 28, white);
        }

        SDL_RenderPresent(renderer);

        // Control simple de FPS
        auto frameTime = std::chrono::high_resolution_clock::now() - now;
        auto frameMs = std::chrono::duration_cast<std::chrono::milliseconds>(frameTime).count();
        if (frameMs < FRAME_DELAY_MS) {
            SDL_Delay((Uint32)(FRAME_DELAY_MS - frameMs));
        }
    }

    // Cleanup
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
