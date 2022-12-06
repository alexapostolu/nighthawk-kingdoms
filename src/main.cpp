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

	Uint64 timer_second = SDL_GetTicks64();
	
	bool tutorial = true;
	
	bool left_mouse_down = true;
	Uint64 timer_mouse_drag = SDL_GetTicks64();

	int frame_count = 0;
	int fps = 0;
	Uint64 timer_fps = SDL_GetTicks64();

	short const MOUSE_DRAG_THRESHOLD = 100;

	while (true)
	{
		frame_count++;
	    if (SDL_GetTicks64() - timer_fps > 1000)
	    {
			timer_fps = SDL_GetTicks64();
			fps = frame_count;
			frame_count = 0;
	    }
		
		Screen::get().clear();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_MOUSEBUTTONDOWN:
			{
				if (event.button.button != SDL_BUTTON_LEFT)
					break;

				left_mouse_down = true;
				timer_mouse_drag = SDL_GetTicks64();

				tutorial = false;
				
				break;
			}
			case SDL_MOUSEBUTTONUP:
			{
				if (!left_mouse_down)
					break;

				left_mouse_down = false;

				int x, y;
				SDL_GetMouseState(&x, &y);

				if (SDL_GetTicks() - timer_mouse_drag < MOUSE_DRAG_THRESHOLD)
					Base::get().handle_mouse_pressed(x, y);
				else
					Base::get().handle_mouse_released(x, y);

				break;
			}
			case SDL_QUIT:
				return 0;
			default:
				break;
			}
		}

		if (left_mouse_down && SDL_GetTicks64() - timer_mouse_drag >= MOUSE_DRAG_THRESHOLD)
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
			Screen::get().text_font(sdl2::str_brygada);
			Screen::get().text_align(sdl2::TextAlign::CENTER_LEFT);

			Screen::get().text_size(24);
			Screen::get().text("Welcome to Nighthawk: Kingdoms!", 120, 100);

			Screen::get().text_size(24);
			Screen::get().text("Here you can build your own kingdom and collect resources!", 120, 140);

			Screen::get().text_size(24);
			Screen::get().text("Click the shop button to place your first building, then you are good to go!", 120, 180);
		}

		bool second_passed = SDL_GetTicks64() - timer_second >= 1000;
		if (second_passed)
			timer_second = SDL_GetTicks64();

		Base::get().display_scene(second_passed);
		Base::get().display_shop();

		Screen::get().fill(sdl2::clr_white);
		Screen::get().text_size(10);
		Screen::get().text_font(sdl2::str_brygada);
		Screen::get().text_align(sdl2::TextAlign::CENTER_LEFT);
		Screen::get().text("frames: " + std::to_string(frame_count), 10, Screen::get().SCREEN_HEIGHT - 20);


		Screen::get().update();
	}
}
