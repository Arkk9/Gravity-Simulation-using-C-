#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <string>

using namespace std;

struct Vec {
    double x, y;
    Vec(double X=0,double Y=0):x(X),y(Y){}
    Vec operator+(const Vec&o)const{return Vec(x+o.x,y+o.y);}
    Vec operator-(const Vec&o)const{return Vec(x-o.x,y-o.y);}
    Vec operator*(double s)const{return Vec(x*s,y*s);}
};

struct Ball {
    Vec pos, vel;
    double r, m;
    int color;
    Ball(Vec p, Vec v, double R, int c)
        :pos(p),vel(v),r(R),color(c){ m = R*R*0.1; }
};

int screenW, screenH;

void resolveCollision(Ball &a, Ball &b) {
    Vec n = b.pos - a.pos;
    double dist = sqrt(n.x*n.x + n.y*n.y);
    if (dist <= 0 || dist > a.r + b.r) return;
    Vec nunit = n * (1.0/dist);

    double overlap = a.r + b.r - dist;
    a.pos = a.pos - nunit * (overlap/2);
    b.pos = b.pos + nunit * (overlap/2);

    Vec rv = b.vel - a.vel;
    double velAlongNormal = rv.x*nunit.x + rv.y*nunit.y;
    if (velAlongNormal > 0) return;

    double e = 0.85;
    double j = -(1+e)*velAlongNormal / (1/a.m + 1/b.m);
    Vec impulse = nunit * j;

    a.vel = a.vel - impulse * (1/a.m);
    b.vel = b.vel + impulse * (1/b.m);
}

void introScreen() {
    cleardevice();
    int cx = screenW/2, cy = screenH/2;
    Ball L(Vec(cx-250,cy), Vec(3,0), 50, LIGHTRED);
    Ball R(Vec(cx+250,cy), Vec(-3,0), 50, LIGHTBLUE);

    double pulse = 0;
    while (true) {
        cleardevice();
        settextstyle(BOLD_FONT, HORIZ_DIR, 6);
        setcolor(YELLOW);
        outtextxy(cx-280, cy-200, (char*)"COLLISION  SIMULATION");

        L.pos.x += L.vel.x;
        R.pos.x += R.vel.x;
        resolveCollision(L,R);

        setfillstyle(SOLID_FILL,L.color);
        fillellipse(L.pos.x,L.pos.y,L.r,L.r);
        setfillstyle(SOLID_FILL,R.color);
        fillellipse(R.pos.x,R.pos.y,R.r,R.r);

        pulse += 0.08;
        int bright = (sin(pulse)*127)+128;
        setcolor(COLOR(bright,bright,bright));
        settextstyle(BOLD_FONT,HORIZ_DIR,3);
        outtextxy(cx-180, cy+200, (char*)"Press ENTER to Start");

        delay(20);
        if (GetAsyncKeyState(VK_RETURN)) break;
        if (GetAsyncKeyState(VK_ESCAPE)) exit(0);
    }
}

int main(){
    srand((unsigned)time(NULL));
    screenW = GetSystemMetrics(SM_CXSCREEN);
    screenH = GetSystemMetrics(SM_CYSCREEN);
    initwindow(screenW,screenH,"Collision Simulation",-3,-3,false,true);
    ShowCursor(TRUE);

    introScreen();

    vector<Ball> balls;
    bool gravityOn = true, paused = false;
    const double G = 0.6;
    int floorY = screenH-100;
    bool dragging=false; int sx=0,sy=0;

    settextstyle(DEFAULT_FONT,HORIZ_DIR,2);

    while(true){
        cleardevice();
        setcolor(GREEN);
        line(0,floorY,screenW,floorY);

        setcolor(WHITE);
        outtextxy(20,20,(char*)"Left-drag = slingshot | Right-click = delete | G=gravity | P=pause | C=clear | ESC=exit");

        // ---- Mouse ----
        if (ismouseclick(WM_LBUTTONDOWN)) {
            getmouseclick(WM_LBUTTONDOWN,sx,sy);
            dragging=true;
        }
        if (ismouseclick(WM_LBUTTONUP) && dragging) {
            int mx,my; getmouseclick(WM_LBUTTONUP,mx,my);
            int dx=mx-sx, dy=my-sy;
            // reverse direction like a slingshot
            double vscale=0.25;
            Vec v(-dx*vscale,-dy*vscale);
            double R=min(70.0,max(10.0,sqrt(dx*dx+dy*dy)/5.0));
            int col=1+rand()%14;
            balls.push_back(Ball(Vec(sx,sy),v,R,col));
            dragging=false;
        }
        if (ismouseclick(WM_RBUTTONDOWN)) {
            int mx,my; getmouseclick(WM_RBUTTONDOWN,mx,my);
            if(!balls.empty()){
                int idx=0; double best=1e12;
                for(int i=0;i<(int)balls.size();++i){
                    double d=(balls[i].pos.x-mx)*(balls[i].pos.x-mx)+(balls[i].pos.y-my)*(balls[i].pos.y-my);
                    if(d<best){best=d;idx=i;}
                }
                balls.erase(balls.begin()+idx);
            }
        }

        // ---- Keys ----
        if (kbhit()) {
            int ch=getch();
            if(ch=='g'||ch=='G') gravityOn=!gravityOn;
            else if(ch=='p'||ch=='P') paused=!paused;
            else if(ch=='c'||ch=='C') balls.clear();
            else if(ch==27) break;
        }

        // ---- Dragging indicator ----
        if(dragging){
            int mx=mousex(), my=mousey();
            setcolor(LIGHTCYAN);
            line(sx,sy,mx,my);
            outtextxy(sx+10,sy+10,(char*)"Release to launch (slingshot)");
        }

        // ---- Physics ----
        if(!paused){
            for(auto &b:balls){
                if(gravityOn) b.vel.y+=G;
                b.pos.x+=b.vel.x;
                b.pos.y+=b.vel.y;

                if(b.pos.x-b.r<0){b.pos.x=b.r;b.vel.x*=-0.8;}
                if(b.pos.x+b.r>screenW){b.pos.x=screenW-b.r;b.vel.x*=-0.8;}
                if(b.pos.y-b.r<0){b.pos.y=b.r;b.vel.y*=-0.8;}
                if(b.pos.y+b.r>floorY){b.pos.y=floorY-b.r;b.vel.y*=-0.75;b.vel.x*=0.98;}
            }
            for(int i=0;i<(int)balls.size();++i)
                for(int j=i+1;j<(int)balls.size();++j)
                    resolveCollision(balls[i],balls[j]);
        }

        // ---- Draw balls + info ----
        for(auto &b:balls){
            setcolor(b.color); setfillstyle(SOLID_FILL,b.color);
            fillellipse((int)b.pos.x,(int)b.pos.y,(int)b.r,(int)b.r);
            setcolor(WHITE);
            line((int)b.pos.x,(int)b.pos.y,(int)(b.pos.x+b.vel.x*3),(int)(b.pos.y+b.vel.y*3));
            char buf[128];
            sprintf(buf,"m=%.1f  v=(%.1f,%.1f)  pos=(%.0f,%.0f)",b.m,b.vel.x,b.vel.y,b.pos.x,b.pos.y);
            outtextxy(b.pos.x+b.r+6,b.pos.y-10,(char*)buf);
        }

        delay(15);
    }

    closegraph();
    return 0;
}
