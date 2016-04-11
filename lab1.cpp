//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
extern "C" {
	#include "fonts.h"
}

//Macros
#define rnd() (((double)rand())/(double)RAND_MAX)

#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 360

#define MAX_PARTICLES 4000
#define GRAVITY 0.1

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures
struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Game {
	Shape box1, box2, box3, box4, box5, circle;
	Particle particle[MAX_PARTICLES];
	int n;
    int bPressed;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	
	Game game;
	game.n=0;
    game.bPressed = 0;

	//declare a box1 shape
	game.box1.width = 75;
	game.box1.height = 12;
	game.box1.center.x = 100;
	game.box1.center.y = WINDOW_HEIGHT - 57;
	
	//declare a box2 shape
	game.box2.width = 75;
	game.box2.height = 12;
	game.box2.center.x = 160;
	game.box2.center.y = WINDOW_HEIGHT - 107;
	
	//declare box3 shape
	game.box3.width = 75;
	game.box3.height = 12;
	game.box3.center.x = 220;
	game.box3.center.y = WINDOW_HEIGHT - 157;
	
	//declare box4 shape
	game.box4.width = 75;
	game.box4.height = 12;
	game.box4.center.x = 280;
	game.box4.center.y = WINDOW_HEIGHT - 207;
	
	//declare box5 shape
	game.box5.width = 75;
	game.box5.height = 12;
	game.box5.center.x = 340;
	game.box5.center.y = WINDOW_HEIGHT - 257;
	
	//declare circle shape
	game.circle.radius = 110;
	game.circle.center.x = WINDOW_WIDTH - 40;
	game.circle.center.y = -40;

	//start animation
	while(!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Hw1   Waterfall Model");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
							ButtonPress | ButtonReleaseMask |
							PointerMotionMask |
							StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
					InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

void makeParticle(Game *game, int x, int y) {
	if (game->n >= MAX_PARTICLES)
		return;
	//std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = rnd();
	p->velocity.x = rnd();
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			makeParticle(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		if (++n < 10)
			return;
		int y = WINDOW_HEIGHT - e->xbutton.y;
		makeParticle(game, e->xbutton.x, y);
	}
}

int check_keys(XEvent *e, Game *game) {
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
			return 1;
		}
		//You may check other keys here.
		else if(key == XK_b) {
			if (game->bPressed)
                game->bPressed = 0;
            else 
                game->bPressed = 1;
		}	
	}
	return 0;
}

void collision(Particle *p, Shape *s) {
	if(p->s.center.y >= s->center.y - (s->height) && 
		p->s.center.y <= s->center.y + (s->height) &&
		p->s.center.x >= s->center.x - (s->width) &&
		p->s.center.x <= s->center.x + (s->width)){
            p->velocity.y *= -.7;
            //p->velocity.x +=  .0005;
	}
}

void movement(Game *game) {
	Particle *p;

	if (game->n <= 0)
            return;
	for(int i = 0; i < game->n; i++){
        p = &game->particle[i];
        p->s.center.x += p->velocity.x;
        p->s.center.y += p->velocity.y;
	
        //gravity
        p->velocity.y -= GRAVITY;

        //check for collision with shapes...
        collision(p, &game->box1);
        collision(p, &game->box2);
        collision(p, &game->box3);
        collision(p, &game->box4);
        collision(p, &game->box5);
        
        float x = p->s.center.x - game->circle.center.x;
        float y = p->s.center.y - game->circle.center.y;
        float distance = sqrt(x*x + y*y);
        
        if(distance <= game->circle.radius) {
            if(p->s.center.x >game->circle.center.x-15)
                p->velocity.x = rnd(); //Bounce some particles to right
            else
                p->velocity.x = -rnd();
            
            p->velocity.y = rnd();
        }	

        //check for off-screen
		if (p->s.center.y < 0.0) {
			std::cout << "off screen" << std::endl;
			game->particle[i] = game->particle[game->n-1];
			game->n--;
		}
	}
}

void drawBox(Shape *t){
	float w, h;
	
	glPushMatrix();
	glTranslatef(t->center.x, t->center.y, t->center.z);
	w = t->width;
	h = t->height;
	glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
	glEnd();
	glPopMatrix();
}

void render(Game *game)
{    
    Rect r;
    float w, h;
    glClear(GL_COLOR_BUFFER_BIT);  
    
    glColor3ub(90,140,90);
    
    //draw particles if B pressed
    if (game->bPressed) 
        makeParticle(game, 120, WINDOW_HEIGHT - 10);

 	//draw boxes
	drawBox(&game->box1);
	drawBox(&game->box2);
	drawBox(&game->box3);
	drawBox(&game->box4);
	drawBox(&game->box5);
    
	//Draw circle
	glBegin(GL_POLYGON);
	int radius = game->circle.radius;
	int x = game->circle.center.x;
	int y = game->circle.center.y;
	for(int i = 0; i < 360; i++){
		float deg = (float)i * 180/M_PI;
		glVertex2i(radius*cos(deg) + x, radius*sin(deg) + y);
	}
	glEnd();
    
	//draw text
    r.bot = WINDOW_HEIGHT - 20;
    r.left = 10;
    r.center = 0;
    ggprint8b(&r, 16, 0xffffff, "Waterfall Model");
    
    r.bot = WINDOW_HEIGHT - 68;
    r.left = 40;
    ggprint16(&r, 16, 0xffff00, "Requirements");
    
    r.bot = WINDOW_HEIGHT - 118;
    r.left = 130;
    ggprint16(&r, 16, 0xffff00, "Design");
    
    r.bot = WINDOW_HEIGHT - 168;
    r.left = 190;
    ggprint16(&r, 16, 0xffff00, "Coding");
    
    r.bot = WINDOW_HEIGHT - 218;
    r.left = 245;
    ggprint16(&r, 16, 0xffff00, "Testing");
    
    r.bot = WINDOW_HEIGHT - 268;
    r.left = 288;
    ggprint16(&r, 16, 0xffff00, "Maintenance");
        
    
	//draw all particles here
	glPushMatrix();
	glColor3ub(150,160,220);
	for(int i = 0; i <game->n; i++){
		Vec *c = &game->particle[i].s.center;
		w = 2;
		h = 2;
		glBegin(GL_QUADS);
			glVertex2i(c->x-w, c->y-h);
			glVertex2i(c->x-w, c->y+h);
			glVertex2i(c->x+w, c->y+h);
			glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
}



