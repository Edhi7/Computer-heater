#include "SDL.h"
#include "SDL_ttf.h"
#include "stdio.h"

/* personal preference */
#define range(s, e) for (int i = s; i < e; i++
#define if if (
#define then )
#define done }
#define while while (
#define do ){
#define done }
#define bool char
#define true 1
#define false 0
#define succesful 0
#define new(type) malloc(sizeof(type));

typedef struct __thread_list {
	SDL_Thread *trd;
	/* set to false when thread should game end it self */
	bool run;
	struct __thread_list *nxt;
} Thread_list;

typedef struct {
	SDL_Window *win;
	SDL_Surface *srf;
	bool run;
	int wdt;
	int hgt;
	int trd;
	int frm;
	Thread_list *trl;
} Screen;

void destroy_screen(Screen *scr)
{
	SDL_FreeSurface(scr->srf);
	SDL_DestroyWindow(scr->win);
}

void cleanup(Screen *scr)
{
	destroy_screen(scr);
	if TTF_WasInit() then TTF_Quit();
	SDL_Quit();
}

void die(char *err, Screen *scr)
{
	puts(err);
	cleanup(scr);
}

int fib(int n)
{
	if n > 1 then return fib(n - 1) + fib(n - 2);
	else return 1;
}

static int heater(void *data)
{
	Thread_list *this = (Thread_list *) data;
	int i = 0;
	while true do 
		i += fib(10);
		/* i increases with 1 each iteration */
		i -= 78;
		/* this thread has been told to game end itself */
		if this->run != true then break;
	done
	printf("going to game end myself\n");
	free(this);
	return i;
}

void spawn_heater(Screen *scr)
{
	char name[20];
	snprintf(name, 19, "thread number %d", scr->trd);
	Thread_list *trl = new(Thread_list);
	trl->run = true;
	SDL_Thread *thread = SDL_CreateThread(heater, name, trl);
	if thread do
		printf("spawned %s\n", SDL_GetThreadName(thread));
		trl->trd = thread;
		if scr->trl == NULL do
			scr->trl = trl;
			trl->nxt = NULL;
		done else {
			trl->nxt = scr->trl;
			scr->trl = trl;
		}
		scr->trd++;
	done
}

static int start_heater(void *data)
{
	Screen *scr = (Screen*) data;
	range(0, 16) do
		spawn_heater(scr);
	done
	return 0;
}

void kill_heater(Screen *scr)
{
	Thread_list *trl = scr->trl;
	if trl == NULL then return;
	if trl->run == false then puts("readan gjort");
	scr->trl = scr->trl->nxt;
	printf("killing %s\n", SDL_GetThreadName(trl->trd));
	trl->run = false;
	/* the memory is now only accesable from the thread and can therefore
	safely be freed from there */
	scr->trd--;
}

Screen *init_sdl(void)
{
	Screen *scr = new(Screen);
	scr->wdt = 300;
	scr->hgt = 200;
	scr->run = true;
	scr->trd = 0;
	scr->frm = 0;
	srand(1337);
	if SDL_Init(SDL_INIT_VIDEO) != succesful then
		die("Could not initialize SDL", scr);
	scr->win = SDL_CreateWindow("Computer heating service",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		scr->wdt, scr->hgt, SDL_WINDOW_SHOWN);
	if scr->win == NULL then
		die("Window could not be created", scr);
	scr->srf = SDL_GetWindowSurface(scr->win);
	/*if TTF_Init() != succesful then
		die("could not initialize SDL_tff", scr);*/
}

void handle_keypress(Screen *scr, SDL_Event *ev)
{
	switch (ev->key.keysym.sym) {
		case SDLK_q: scr->run = false; break;
	}
}

void handle_button_press(Screen *scr,  SDL_MouseButtonEvent *ev)
{
	if ev->y < scr->hgt - 32
		&& ev->y > scr->hgt - 64 then
		if ev->x < (scr->wdt / 2) - 6
			&& ev->x > (scr->wdt / 2) - 40
			then spawn_heater(scr);
		else if ev->x > (scr->wdt / 2) + 32
			&& ev->x < (scr->wdt / 2) + 64
			then kill_heater(scr);
}

void handle_event(Screen *scr, SDL_Event *ev)
{
	switch (ev->type) {
		case SDL_QUIT: scr->run = false; 						break;
		case SDL_KEYUP: handle_keypress(scr, ev); 				break;
		case SDL_MOUSEBUTTONDOWN:
			handle_button_press(scr, (SDL_MouseButtonEvent *)ev); break;
	}
}

void draw_plus_minus(Screen *scr)
{
	SDL_Rect sign;
	const int size = 32;

	/* draw plus first */
	sign.x = (scr->wdt / 2) - 12 - (size / 2);
	sign.y = scr->hgt - 32 - size;
	sign.w = 8;
	sign.h = size;
	SDL_FillRect(scr->srf, &sign,
		SDL_MapRGB(scr->srf->format, 0x5A, 0x92, 0x71));

	/* the horizontal part of the plus */
	sign.x = (scr->wdt / 2) - 8 - size;
	sign.y = scr->hgt - 32 - (size / 2) - 4;
	sign.w = size;
	sign.h = 8;
	SDL_FillRect(scr->srf, &sign,
		SDL_MapRGB(scr->srf->format, 0x5A, 0x92, 0x71));

	/* to draw the minus sign we only have to move the horizontal part of 
		the plus sign */
	sign.x = (scr->wdt / 2) + size;
	SDL_FillRect(scr->srf, &sign,
		SDL_MapRGB(scr->srf->format, 0x5A, 0x92, 0x71));
}

void draw_ui(Screen *scr)
{
	draw_plus_minus(scr);
}

void draw_loading_bar(Screen *scr)
{
	SDL_Rect bar;
	int x = ((scr->frm * 3) % scr->wdt);
	const int width = 160;
	const int height = 4;
	bar.x = x;
	bar.y = scr->hgt - height;
	bar.w = width;
	bar.h = height;
	SDL_FillRect(scr->srf, &bar,
		SDL_MapRGB(scr->srf->format, 0x5A, 0x92, 0x71));
	if x > scr->wdt - width do
		/* bar is partly outside window and will therefore loop arond */
		bar.x = 0;
		bar.y = scr->hgt - height;
		bar.w = x - scr->wdt + width;
		bar.h = height;
		SDL_FillRect(scr->srf, &bar,
			SDL_MapRGB(scr->srf->format, 0x5A, 0x92, 0x71));
	done
}

void draw_threads(Screen *scr)
{
	SDL_Rect square;
	int rnx, rny;
	square.x = 8;
	square.y = 8;
	square.w = 8;
	square.h = 8;
	/*printf("thread count is %d\n", scr->trd);*/
	range(0, scr->trd) do
		if square.x > scr->wdt - 24 do
			square.x = 8;
			square.y += 16;
		done
		/* the random numbers can be -1, 0, or 1 */ 
		rnx = rand() % 3 - 1;
		rny = rand() % 3 - 1;
		square.x += rnx;
		square.y += rny;
		SDL_FillRect(scr->srf, &square,
			SDL_MapRGB(scr->srf->format, 0x5A, 0x92, 0x71));
		square.x -= rnx;
		square.y -= rny;
		square.x += 16;
	done
}

void draw(Screen *scr)
{
	SDL_FillRect(scr->srf, NULL,
		SDL_MapRGB(scr->srf->format, 0xFA, 0xFA, 0xFA));
	draw_ui(scr);
	draw_threads(scr);
	draw_loading_bar(scr);
	SDL_UpdateWindowSurface(scr->win);
}

/* Never returns */
void loop(Screen *scr)
{
	SDL_Event e;
	while scr->run do
		while SDL_PollEvent(&e) != 0 do
			handle_event(scr, &e);
		done
		draw(scr);
		SDL_Delay(10);
		scr->frm++;
	done
}

/* TODO: gör så man kan ändra processantalet genom GUI och se
processoranvändning och temp etc...*/

int main(int argc, char **argv)
{
	Screen *scr = init_sdl();
	SDL_Thread *thread = SDL_CreateThread(start_heater, "heater", scr);
	if thread == NULL then puts("thread could not be created");
	loop(scr);
	cleanup(scr);
	return 0;
}