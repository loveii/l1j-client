#include "draw_admin_main.h"

#include "globals.h"
#include "resources/prepared_graphics.h"
#include "sdl_user.h"
#include "widgets/sdl_animate_button.h"
#include "widgets/sdl_input_box.h"
#include "widgets/sdl_text_button.h"

void change_mode(void *a, void* b)
{
	sdl_user *owner;
	owner = (sdl_user*)a;
	enum drawmode temp;
	int temp2;
	temp2 = reinterpret_cast<int>(b);
	temp = (drawmode)temp2;
	owner->change_drawmode(temp);
}

draw_admin_main::draw_admin_main(graphics_data *stuff, sdl_user *self)
	: sdl_drawmode(stuff, self)
{
	owner->game_music.change_music("sound/music1.mp3");
	pg = new prepared_graphics;
	pg->num_pg = 1;
	
	int index;
	SDL_Rect *rect;

	pg->pg = new prepared_graphic[1];
	
	pg->pg[0].surf = get_png_image(811, graphx->spritepack);
	pg->pg[0].mask = NULL;
	pg->pg[0].position = NULL;
	pg->pg[0].cleanup = false;
	
	pg->ready = true;
	num_widgets = 5;
	widgets = new sdl_widget*[num_widgets];
	
	int x = 200;
	int y = 200;
	widgets[0] = new sdl_text_button("IMG Files", x, y, graphx, 
		(funcptr*)new dam_ptr(this, DRAWMODE_MAINT_IMG));
	widgets[0]->set_key_focus(true);
	widgets[0]->cursor_on();
	widgets[1] = new sdl_text_button("PNG Files", x, y+(15*1), graphx, 
		(funcptr*)new dam_ptr(this, DRAWMODE_MAINT_PNG));
	widgets[1]->set_key_focus(true);
	widgets[2] = new sdl_text_button("TIL Files", x, y+(15*2), graphx, 
		(funcptr*)new dam_ptr(this, DRAWMODE_MAINT_TIL));
	widgets[2]->set_key_focus(true);
	widgets[3] = new sdl_text_button("MAPS", x, y+(15*3), graphx, 
		(funcptr*)new dam_ptr(this, DRAWMODE_MAINT_MAP));
	widgets[3]->set_key_focus(true);
	widgets[4] = new sdl_text_button("Exit", x, y+(15*4), graphx, 0);
	widgets[4]->set_key_focus(true);
}

draw_admin_main::~draw_admin_main()
{
}

bool draw_admin_main::mouse_leave()
{
	return false;
}

void draw_admin_main::mouse_click(SDL_MouseButtonEvent *here)
{
	if (num_widgets > 0)
	{
		int index = get_widget(here->x, here->y);
		if (index != -1)
		{
			widgets[index]->mouse_click(here);
		}
	}
}

void draw_admin_main::mouse_to(SDL_MouseMotionEvent *to)
{
	int which = get_widget(to->x, to->y);
	if (which >= 0)
	{
		widgets[which]->mouse_to(to);
	}
}

void draw_admin_main::mouse_from(SDL_MouseMotionEvent *from)
{
	int which = get_widget(from->x, from->y);
	if (which >= 0)
	{
		widgets[which]->mouse_from(from);
	}
}

void draw_admin_main::mouse_move(SDL_MouseMotionEvent *from, SDL_MouseMotionEvent *to)
{	//TODO: handle click and drag movable widgets
	if (num_widgets > 0)
	{
		int from_w = get_widget(from->x, from->y);
		int to_w = get_widget(to->x, to->y);
		if ((from_w != -1) && (to_w != -1))
		{
			if (from_w != to_w)
			{	//only send events if the widgets are different
				mouse_from(from);
				mouse_to(to);
			}
		}
		if ((from_w == -1) && (to_w != -1))
		{
			mouse_to(to);
		}
		if ((from_w != -1) && (to_w == -1))
		{
			mouse_from(from);
		}
	}
}
