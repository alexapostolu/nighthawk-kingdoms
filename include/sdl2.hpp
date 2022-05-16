#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

#include <memory>
#include <string>

namespace sdl2
{

// smart pointer wrappers

struct SDL_Deleter
{
	void operator()(SDL_Renderer* ptr);
	void operator()(SDL_Surface* ptr);
	void operator()(SDL_Texture* ptr);
	void operator()(TTF_Font* ptr);
};

using window_ptr = std::shared_ptr<SDL_Window>;
using renderer_ptr = std::unique_ptr<SDL_Renderer, SDL_Deleter>;
using surface_ptr  = std::unique_ptr<SDL_Surface,  SDL_Deleter>;
using texture_ptr  = std::unique_ptr<SDL_Texture,  SDL_Deleter>;

using font_ptr = std::unique_ptr<TTF_Font, SDL_Deleter>;



// text fonts, colours, and alignment

enum class Align
{
	LEFT, CENTER, RIGHT
};

struct Dimension
{
	int x, y, w, h;
};

struct Text
{
	Text(int _x, int _y, Align _align);
	bool clicked_on(int mx, int my);

	Dimension dim;
	Align align;
};

std::string const str_brygada = "../assets/brygada.ttf";
SDL_Color const clr_black{ 0, 0, 0, 255 };
SDL_Color const clr_yellow{ 255, 239, 0, 255 };
SDL_Color const clr_green{ 60, 220, 0, 150 };
SDL_Color const clr_white{ 255, 255, 255, 255 };
SDL_Color const clr_clear{ 0, 0, 0, 0 };
SDL_Color const clr_red{ 255, 50, 50, 150 };
SDL_Color const clr_gray{ 240, 240, 240, 170 };

}