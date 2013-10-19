#ifndef _SDL_GRAPHIC_H_
#define _SDL_GRAPHIC_H_

#include <SDL/SDL.h>

class client;

enum SDL_GRAPH_TYPES
{
	GRAPH_IMG,
	GRAPH_PNG,
	GRAPH_STIL,
	GRAPH_LTIL,
	GRAPH_RTIL
};

class graphics_data;

class sdl_graphic
{
	public:
		sdl_graphic(int num, int x, int y, client *packs, int type);
		sdl_graphic(char *name, int x, int y, client *packs, int type);
		sdl_graphic(int x, int y, SDL_RWops *source, short *palette, int type);
		sdl_graphic(int x, int y, short *source, int type);
		sdl_graphic(int x, int y, int w, int h);
		void disable_clear();
		Uint32 color(Uint8 r, Uint8 g, Uint8 b);
		void drawat(int x, int y, SDL_Surface *display);
		void draw(SDL_Surface *display);	//draws this tile onto display
		void mod_with(SDL_Surface *display);
		SDL_Surface *get_surf();
		~sdl_graphic();
		int getx();
		int gety();
		int getw();
		int geth();
		void erase();
		
		void setmw(int nmw);
		void setmh(int nmh);
		void setmx(int nmx);
		void setmy(int nmy);
		
		void make_bmp(char *name);
	private:
		SDL_Surface *img;
		SDL_Rect *pos;
		SDL_Rect *mask;
		Uint32 transp_color;
		bool cleanup;
};

#endif