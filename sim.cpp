// gravity_2d_bgi.cpp
// 2D approximation of the OpenGL simulation, using graphics.h (WinBGI).
// Left click: spawn an object (hold to "grow" mass with right button).
// Right click while initializing: increase mass.
// WASD / arrow keys: pan camera. +/- to zoom. K to toggle pause. Q to quit.#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <cmath>
#include <iostream>

using namespace std;

const double G = 0.1; // gravitational constant
bool paused = false;

// Camera controls
double camX = 0, camY = 0;
double zoom = 1.0;

// Body structure
struct Body {
    double x, y;
    double vx, vy;
    double mass;
    int radius;
};

vector<Body> bodies;

// Draw a body
void drawBody(const Body &b) {
    int screenX = (int)((b.x - camX) * zoom + getmaxx() / 2);
    int screenY = (int)((b.y - camY) * zoom + getmaxy() / 2);
    circle(screenX, screenY, max(1, (int)(b.radius * zoom)));
}

// Update physics safely
void updatePhysics(double dt) {
    int n = bodies.size();
    for (int i = 0; i < n; i++) {
        double fx = 0, fy = 0;
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            double dx = bodies[j].x - bodies[i].x;
            double dy = bodies[j].y - bodies[i].y;
            double dist2 = dx*dx + dy*dy;
            double dist = sqrt(dist2);
            if (dist < 5) dist = 5; // prevent extreme forces

            double f = G * bodies[i].mass * bodies[j].mass / dist2;
            fx += f * dx / dist;
            fy += f * dy / dist;
        }

        // Cap velocity for stability
        double vmax = 20;
        bodies[i].vx += fx / bodies[i].mass * dt;
        bodies[i].vy += fy / bodies[i].mass * dt;
        bodies[i].vx = max(min(bodies[i].vx, vmax), -vmax);
        bodies[i].vy = max(min(bodies[i].vy, vmax), -vmax);
    }

    for (auto &b : bodies) {
        b.x += b.vx * dt;
        b.y += b.vy * dt;
    }
}

// Handle keyboard & mouse input
void handleInput() {
    // Keyboard
    if (kbhit()) {
        char c = getch();
        switch (c) {
            case 'p': paused = !paused; break;
            case '+': zoom *= 1.1; break;
            case '-': zoom /= 1.1; break;
            case 'w': camY -= 20 / zoom; break;
            case 's': camY += 20 / zoom; break;
            case 'a': camX -= 20 / zoom; break;
            case 'd': camX += 20 / zoom; break;
            case 'q': closegraph(); exit(0);
        }
    }

    // Mouse: left-click = add body
    if (ismouseclick(WM_LBUTTONDOWN)) {
        int mx, my;
        getmouseclick(WM_LBUTTONDOWN, mx, my);
        Body b;
        b.x = (mx - getmaxx()/2)/zoom + camX;
        b.y = (my - getmaxy()/2)/zoom + camY;
        b.vx = 0; b.vy = 0;
        b.mass = 50;
        b.radius = 5;
        bodies.push_back(b);
    }

    // Mouse: right-click = increase mass
    if (ismouseclick(WM_RBUTTONDOWN)) {
        int mx, my;
        getmouseclick(WM_RBUTTONDOWN, mx, my);
        for (auto &b : bodies) {
            int screenX = (int)((b.x - camX)*zoom + getmaxx()/2);
            int screenY = (int)((b.y - camY)*zoom + getmaxy()/2);
            if (abs(screenX - mx) < 10 && abs(screenY - my) < 10) {
                b.mass *= 1.5;
                b.radius = (int)sqrt(b.mass);
                break;
            }
        }
    }
}

int main() {
    // Initialize graphics window
    initwindow(800, 600, "2D Gravity Simulation");

    while (true) {
        handleInput();

        if (!paused) updatePhysics(0.05);

        cleardevice();
        for (auto &b : bodies) drawBody(b);

        delay(20);
    }

    closegraph();
    return 0;
}
