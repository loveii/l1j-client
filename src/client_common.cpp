#include "sdl_user.h"
#include "client.h"
#include "config.h"
#include "connection.h"
#include "drawmode/draw_char_sel.h"
#include "drawmode/draw_game.h"
#include "drawmode/draw_loading.h"
#include "drawmode/sdl_drawmode.h"
#include "globals.h"
#include "lindes.h"
#include "packet.h"
#include "resources/briefcase.h"
#include "resources/music.h"
#include "resources/pack.h"
#include "resources/partial_table.h"
#include "resources/table.h"
#include "unsorted.h"

#include <math.h>

int client::pack_resources()
{
	return 0;
}

int client::init_packs()
{
	num_sprite_pack = 17;
	textpack = new pack("Text", 1);
	tilepack = new pack("Tile", 0);
	spritepack = new pack*[num_sprite_pack];
	spritepack[0] = new pack("Sprite", 0);
	for (int i = 0; i < (num_sprite_pack-1); i++)
	{
		char name[10];
		sprintf(name, "Sprite%02d", i);
		spritepack[i+1] = new pack(name, 0);
		//new_surf_pack and spritepack are the same thing
		//new_icon_pack and spritepack are the same thing
		//spriteFile is simply the first element of spritepack
	}
	//TODO: verify all packs were loaded successfully
	
	return 0;
}

void client::delete_packs()
{
	delete textpack;
	textpack = 0;
	
	delete tilepack;
	tilepack = 0;
	
	for (int i = 0; i < num_sprite_pack; i++)
	{
		delete spritepack[i];
		spritepack[i] = 0;
	}
	num_sprite_pack = 0;
	delete [] spritepack;
	spritepack = 0;
}

int client::init_strings()
{
	int acp = -1;
	if (acp == -1)
	{
//		printf("STUB GetACP()\n");
		//acp = GetACP();
	}
	
	partial_table test;
	test.load_local("itemdesc", textpack);
	
	//list of filtered chat words
	bad_words.load_local("obscene", textpack);
	bad_words.sort();
	
	//list of tips for new players?
	todays_tip.load_local("todaystip", textpack);
//	printf("STUB Load MercenaryIconData\n");
//	printf("STUB Load MagicDollData\n");
	
	//list of items
	solvent.load_local("solvent", textpack);
	
	//list of pet types
	pet.load_local("ntexpet", textpack);
	
	//no important items?
	important_items.load("itemimportant.tbl", textpack);
	
//	printf("STUB Load BaseStatus\n");
	
	//unknown
	teleport.load("telbook.tbl", textpack);
	
	//unknown
	un_transaction.load("untransaction.tbl", textpack);
	
//	if (battleServer != 0)
//	{
//		table notices;
//		notices.load("notice_bs.tbl");
//		notices.print();
//	}

	strings.load_local("string", textpack);
	
	return 0;
}

void client::change_drawmode(enum drawmode chg)
{
	graphics->change_drawmode(chg);
}

client::client(sdl_user *stuff)
{
	requests_mtx = SDL_CreateMutex();
	stop_thread = false;
	server_name = 0;
	request_id = 0;
	wait_for_user = true;
	
	getfiles = new files(this);
	login_opts = 0;
	num_login_opts = 0;
	login_opts_used = 0;
	login_opts_stored = 0;
	main_config = 0;
	graphics = stuff;
	server = 0;
	server_data = 0;
	num_sprite_pack = 0;
	for (int i = 0; i < 256; i++)
	{	//this loop sets up a no translation scheme for default
		convert_client_packets[i] = i;
		convert_server_packets[i] = i;
	}
}

client::~client()
{
	delete_requests();
	SDL_DestroyMutex(requests_mtx);
	delete getfiles;
	getfiles = 0;
	if (server != 0)
	{
		delete server;
		server = 0;
	}
	if (server_name != 0)
	{
		delete [] server_name;
		server_name = 0;
	}
	if (main_config != 0)
	{
		delete main_config;
		main_config = 0;
	}
	if (server_data != 0)
	{
		delete server_data;
		server_data = 0;
	}
	
	if (num_login_opts > 0)
	{
		for (int i = 0; i < num_login_opts; i++)
		{
			if (login_opts[i] != 0)
			{
				if (login_opts[i]->name != 0)
				{
					delete [] login_opts[i]->name;
					login_opts[i]->name = 0;
				}
				if (login_opts[i]->pledge != 0)
				{
					delete [] login_opts[i]->pledge;
					login_opts[i]->pledge = 0;
				}
				delete login_opts[i];
				login_opts[i] = 0;
			}
		}
		delete [] login_opts;
		login_opts = 0;
	}
	
	delete_packs();
}

/** This function checks to see if there are any requests that need to be filled. This includes things such as quitting, loading resources, sending packets to the server, etc. 
 */
void client::check_requests()
{
	while (SDL_mutexP(requests_mtx) == -1) {};
	if (stop_thread)
	{
		SDL_mutexV(requests_mtx);
		throw "Client is stopping";
	}
	if (request_list.size() > 0)
	{
		client_request *temp = request_list.front();
		request_list.pop_front();
		switch (temp->request)
		{
			case CLIENT_REQUEST_LOAD_SDL_GRAPHIC:
				switch(temp->data.rload.load_type)
				{
					case CLIENT_REQUEST_LOAD_1:
						temp->data.rload.item->do_load(
							temp->data.rload.x,
							temp->data.rload.y,
							temp->data.rload.buffer,
							temp->data.rload.type);
						break;
					case CLIENT_REQUEST_LOAD_2:
						temp->data.rload.item->do_load(
							temp->data.rload.x,
							temp->data.rload.y,
							temp->data.rload.source,
							temp->data.rload.palette,
							temp->data.rload.type);
						break;
					case CLIENT_REQUEST_LOAD_3:
						temp->data.rload.item->do_load(
							temp->data.rload.name,
							temp->data.rload.x,
							temp->data.rload.y,
							temp->data.rload.type,
							this);
						break;
					case CLIENT_REQUEST_LOAD_4:
						temp->data.rload.item->do_load(
							temp->data.rload.num,
							temp->data.rload.x,
							temp->data.rload.y,
							temp->data.rload.type,
							this);
						break;
					default:
						break;
				}
				if (temp->data.rload.name != 0)
				{
					delete [] temp->data.rload.name;
					temp->data.rload.name = 0;
				}
				break;
			case CLIENT_REQUEST_LOAD_TILE:
				temp->data.tload.item->load(
					temp->data.tload.which, this);
				break;
			case CLIENT_REQUEST_CHECK_MAP:
				temp->data.mcheck.item->check_sections(this);
				break;
			case CLIENT_REQUEST_LOAD_SPRITE:
				temp->data.sload.item->load(
					temp->data.sload.x,
					temp->data.sload.y,
					temp->data.sload.name,
					this);
				delete [] temp->data.sload.name;
				temp->data.sload.name = 0;
				break;
			default:
				break;
		}
		delete temp;
		temp = 0;
	}
	SDL_mutexV(requests_mtx);
}

void client::free_request(client_request *temp)
{
	switch (temp->request)
	{
		case CLIENT_REQUEST_LOAD_SDL_GRAPHIC:
			if (temp->data.rload.name != 0)
			{
				delete [] temp->data.rload.name;
				temp->data.rload.name = 0;
			}
			break;
		case CLIENT_REQUEST_LOAD_SPRITE:
			if (temp->data.sload.name != 0)
			{
				delete [] temp->data.sload.name;
				temp->data.sload.name = 0;
			}
		default:
			break;
	}
	delete temp;
	temp = 0;
}

/** This is used to delete all requests */
void client::delete_requests()
{
	while (SDL_mutexP(requests_mtx) == -1) {};
	while (request_list.size() > 0)
	{
		client_request *temp = request_list.front();
		request_list.pop_front();
		free_request(temp);
	}
	SDL_mutexV(requests_mtx);
}

unsigned int client::add_request(client_request rqst)
{
	unsigned int temp_val;
	if (stop_thread == false)
	{
		while (SDL_mutexP(requests_mtx) == -1) {};
		client_request *temp = new client_request;
		temp->data.rload.name = 0;
		memcpy(temp, &rqst, sizeof(rqst));
		temp->id = ++request_id;
		temp_val = request_id;
		switch (rqst.request)
		{
			case CLIENT_REQUEST_LOAD_SPRITE:
				if (rqst.data.sload.name != 0)
				{
					temp->data.sload.name = new char[strlen(rqst.data.sload.name)+1];
					strcpy(temp->data.sload.name, rqst.data.sload.name);
				}
				break;
			case CLIENT_REQUEST_LOAD_SDL_GRAPHIC:
				if (rqst.data.rload.name != 0)
				{
					temp->data.rload.name = new char[strlen(rqst.data.rload.name)+1];
					strcpy(temp->data.rload.name, rqst.data.rload.name);
				}
				break;
			case CLIENT_REQUEST_CHECK_MAP:
			case CLIENT_REQUEST_LOAD_TILE:
			default:
				break;
		}
		request_list.push_back(temp);
		SDL_mutexV(requests_mtx);
	}
	return temp_val;
}

void client::cancel_request(unsigned int id)
{
	while (SDL_mutexP(requests_mtx) == -1) {};
	for (std::list<client_request*>::iterator it = request_list.begin();
		it != request_list.end(); it++)
	{
		bool keep_going = true;
		while (keep_going)
		{
			if (it == request_list.end())
				keep_going = false;
			if (*it != 0)
			{
				if ((*it)->id == id)
				{
					free_request(*it);
					it = request_list.erase(it);
				}
				else
				{
					keep_going = false;
				}
			}
			else
			{
				keep_going = false;
			}
		}
	}
	SDL_mutexV(requests_mtx);
}

int client::check_login_chars()
{
	return login_opts_stored;
}

void client::create_chars(int used, int usable, int slots)
{	//creates the array and makes them all blank
	if (num_login_opts == 0)
	{
		num_login_opts = usable;	//the unusable slots will not be modified
		login_opts_used = used;
		login_opts = new lin_char_info*[slots];
		for (int i = 0; i < slots; i++)
		{
			login_opts[i] = 0;
		}
		for (int i = num_login_opts; i < slots; i++)
		{	
			//possibly initialize them so they will display as not usable
			printf("STUB: Slot %d is not usable\n", i+1);
		}
	}
}

config *client::get_config()
{
	return main_config;
}

lin_char_info** client::get_login_chars()
{	//retrieves the entire array
	return login_opts;
}

void client::stop()
{
	stop_thread = true;
}

void client::user_is_done()
{
	wait_for_user = false;
}

void client::wait_on_user_cleanup()
{
	while (wait_for_user)
	{
	}
}

int run_client(void *moron)
{	//the main thread for each client
	client *game = new client((sdl_user*)moron);
	sdl_user *temp;
	temp = (sdl_user*)moron;
	temp->init_client(game);
	game->run();
	game->wait_on_user_cleanup();
	delete game;
	game = 0;
	return 0;
}