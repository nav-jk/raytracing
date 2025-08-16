#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

#define WIDTH 900
#define HEIGHT 600
#define COLOR_BLACK 0x00000000
#define RAYS_NUMBER 250

typedef struct Circle {
    double x;
    double y;
    double r;
} Circle;

typedef struct Ray {
    double x_start;
    double y_start;
    double angle;
} Ray;


void generate_rays(Circle circle, Ray rays[RAYS_NUMBER]) {
    for (int i = 0; i < RAYS_NUMBER; i++) {
        double angle = ((double)i / RAYS_NUMBER) * 2 * M_PI;
        Ray ray = { circle.x, circle.y, angle };
        rays[i] = ray;
    }
}

// smoothly interpolate colors (rainbow hue cycling)
Uint32 rainbow_color(int index, int total, double fade) {
    double t = (double)index / total; // [0,1] hue position
    double r = fabs(sin(t * M_PI * 2)) * 255;
    double g = fabs(sin(t * M_PI * 2 + 2.0 * M_PI / 3.0)) * 255;
    double b = fabs(sin(t * M_PI * 2 + 4.0 * M_PI / 3.0)) * 255;
    r *= fade; g *= fade; b *= fade;
    return (0xFF << 24) | ((int)r << 16) | ((int)g << 8) | (int)b;
}


void FillRays(SDL_Surface* surface, Ray rays[RAYS_NUMBER], Circle circle) {
    for (int i = 0; i < RAYS_NUMBER; i++) {
        Ray ray = rays[i];
        int end_of_screen = 0, object_hit = 0;
        double step = 1.0;
        double x_draw = ray.x_start, y_draw = ray.y_start;
        double distance = 0;

        while (!end_of_screen && !object_hit) {
            x_draw += step * cos(ray.angle);
            y_draw += step * sin(ray.angle);
            distance += step;

            double fade = fmax(0.2, 1.0 - distance / 400.0);
            Uint32 color = rainbow_color(i, RAYS_NUMBER, fade);

            SDL_Rect pixel = { (int)x_draw, (int)y_draw, 1, 1 };
            SDL_FillRect(surface, &pixel, color);

            if (x_draw < 0 || x_draw > WIDTH) end_of_screen = 1;
            if (y_draw < 0 || y_draw > HEIGHT) end_of_screen = 1;
            if (pow(x_draw - circle.x, 2) + pow(y_draw - circle.y, 2) < pow(circle.r, 2))
                object_hit = 1;
        }
    }
}

// gradient filled circle (object) from center→edge
void GradientCircle(Circle circle, SDL_Surface* surface, SDL_Color inner, SDL_Color outer) {
    for (int r = circle.r; r > 0; r--) {
        float t = (float)r / circle.r; // 0..1 fade

        Uint8 red   = (Uint8)(outer.r * (1 - t) + inner.r * t);
        Uint8 green = (Uint8)(outer.g * (1 - t) + inner.g * t);
        Uint8 blue  = (Uint8)(outer.b * (1 - t) + inner.b * t);

        Uint32 color = (0xFF << 24) | (red << 16) | (green << 8) | blue;

        double radius_squared = r * r;
        for (double x = circle.x - r; x <= circle.x + r; x++) {
            for (double y = circle.y - r; y <= circle.y + r; y++) {
                if ((x - circle.x) * (x - circle.x) + (y - circle.y) * (y - circle.y) <= radius_squared) {
                    SDL_Rect pixel = { (int)x, (int)y, 1, 1 };
                    SDL_FillRect(surface, &pixel, color);
                }
            }
        }
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* pwindow = SDL_CreateWindow(
        "Ray Tracing",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, 0
    );

    SDL_Surface* psurface = SDL_GetWindowSurface(pwindow);
    Circle circle = { 450, 300, 80 };

    SDL_Color inner = {50, 0, 100};  
    SDL_Color outer = {200, 150, 255}; 

    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;

            if (event.type == SDL_MOUSEMOTION) {
                SDL_FillRect(psurface, NULL, COLOR_BLACK);


                GradientCircle(circle, psurface, inner, outer);


                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                Circle source = { mouseX, mouseY, 30 };
                GradientCircle(source, psurface, (SDL_Color){0, 150, 0}, (SDL_Color){0, 255, 100});

                Ray rays[RAYS_NUMBER];
                generate_rays(source, rays);
                FillRays(psurface, rays, circle);

                SDL_UpdateWindowSurface(pwindow);
            }
        }
    }

    SDL_DestroyWindow(pwindow);
    SDL_Quit();
    return 0;
}
