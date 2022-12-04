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
		std::cout << "error - failed to initialize SDL\n    " << SDL_GetError();
		return 1;
	}
	if (TTF_Init() == -1)
	{
		std::cout << "error - failed to initialize TTF\n    " << TTF_GetError();
		return 1;
	}
	

	SDL_PumpEvents();

	Screen::get().set_window();
	Base::get().set_building_dim();

	int b = SDL_GetTicks(), frames = 0;
	int c = SDL_GetTicks();
	bool mouse_down = false;
	bool tutorial = true;
	int timer = SDL_GetTicks();
	int framec = 0;
	while (true)
	{
		frames++;
	    if (SDL_GetTicks() - b > 1000)
	    {
			//std::cout << "fps: " << frames << '\n';
	        b = SDL_GetTicks();
			framec = frames;
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

				c = SDL_GetTicks();
				mouse_down = true;

				int x, y;
				SDL_GetMouseState(&x, &y);

				//std::cout << "mouse pos: " << x << ' ' << y << '\n';

				Base::get().handle_mouse_pressed(x, y);
				tutorial = false;
				
				break;
			}
			case SDL_MOUSEBUTTONUP: {
				c = SDL_GetTicks();
				mouse_down = false;

				int x, y;
				SDL_GetMouseState(&x, &y);
				Base::get().handle_mouse_released(x, y);

				break;
			}
			case SDL_QUIT:
				goto END_SDL;
			default:
				break;
			}
		}

		if (mouse_down && SDL_GetTicks() - c >= 100)
		{
			int x, y;
			SDL_GetMouseState(&x, &y);
			Base::get().handle_mouse_dragged(x, y);
		}

		if (tutorial)
		{
			Screen::get().fill(sdl2::clr_black);
			Screen::get().stroke(sdl2::clr_white);
			Screen::get().rect(100, 80, 850, 200);
		}

		Base::get().display_resources();

		if (tutorial)
		{
			Screen::get().fill(sdl2::clr_yellow);
			Screen::get().text("Welcome to Nighthawk: Kingdoms!", sdl2::str_brygada, 24, 120, 100, sdl2::Align::LEFT);
			Screen::get().text("Here you can build your own kingdom and collect resources!", sdl2::str_brygada, 24, 120, 140, sdl2::Align::LEFT);
			Screen::get().text("Click the shop button to place your first building, then you are good to go!", sdl2::str_brygada, 24, 120, 180, sdl2::Align::LEFT);
		}

		bool const second = SDL_GetTicks() - timer >= 1000;
		if (second)
			timer = SDL_GetTicks();

		Base::get().display_scene(second);
		Base::get().display_shop();

		Screen::get().fill(sdl2::clr_white);
		Screen::get().text("frames: " + std::to_string(framec),
			sdl2::str_brygada, 10, 10, Screen::get().SCREEN_HEIGHT - 20, sdl2::Align::LEFT);


		Screen::get().update();
	}

END_SDL:
	return 0;
}
