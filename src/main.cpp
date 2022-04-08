#include "base.hpp"
#include "tile.hpp"
#include "screen.hpp"
#include "sdl2.hpp"

#include <SDL.h>
#include <SDL_ttf.h>

#include <iostream>
#include <memory>
#include <string>

int main(int argc, char* argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "error - failed to initialize SDL\n" << SDL_GetError();
		return 1;
	}
	if (TTF_Init() == -1)
	{
		std::cout << "error - failed to initialize TTF\n" << TTF_GetError();
		return 1;
	}
	
	Screen::get().set_window();

	SDL_PumpEvents();

	int b = SDL_GetTicks(), frames = 0;
	bool tutorial = true;
	while (true)
	{
		frames++;
	    if (SDL_GetTicks() - b > 1000)
	    {
			std::cout << "fps: " << frames << '\n';
	        b = SDL_GetTicks();
			frames = 0;
	    }
		
		Screen::get().clear();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_MOUSEBUTTONDOWN: {
				if (event.button.button != SDL_BUTTON_LEFT)
					break;

				int x, y;
				SDL_GetMouseState(&x, &y);

				std::cout << "mouse pos: " << x << ' ' << y << '\n';

				Base::get().handle_mouse_pressed(x, y);
				tutorial = false;
				
				break;
			}
			case SDL_MOUSEBUTTONUP: {
				Base::get().handle_mouse_released(x, y);
			}
			case SDL_QUIT:
				goto END_SDL;
			default:
				break;
			}
		}

		if (tutorial)
		{
			Screen::get().rect(100, 80, 850, 200, sdl2::clr_black, sdl2::clr_white);
		}

		Base::get().display_resources();

		if (tutorial)
		{
			Screen::get().text("Welcome to Nighthawk: Kingdoms!", sdl2::clr_yellow, sdl2::str_brygada, 24, 120, 100, sdl2::Align::LEFT);
			Screen::get().text("Here you can build your own kingdom and collect resources!", sdl2::clr_yellow, sdl2::str_brygada, 24, 120, 140, sdl2::Align::LEFT);
			Screen::get().text("Click the shop button to place your first building, then you are good to go!", sdl2::clr_yellow, sdl2::str_brygada, 24, 120, 180, sdl2::Align::LEFT);
		}

		Base::get().display_scene();
		Base::get().display_shop();



		Screen::get().update();
	}

END_SDL:
	return 0;
}
