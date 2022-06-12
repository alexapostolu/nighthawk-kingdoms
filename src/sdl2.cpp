#include "sdl2.hpp"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <iostream>
#include <cmath>
#include <string>
#include <memory>

#define n_ptr else std::cout << "[error] - " << '\n';

namespace sdl2
{

void SDL_Deleter::operator()(SDL_Renderer* ptr) { if (ptr) SDL_DestroyRenderer(ptr); n_ptr }
void SDL_Deleter::operator()(SDL_Surface* ptr) { if (ptr) SDL_FreeSurface(ptr); n_ptr }
void SDL_Deleter::operator()(SDL_Texture* ptr) { if (ptr) SDL_DestroyTexture(ptr); n_ptr }

void SDL_Deleter::operator()(TTF_Font* ptr) { if (ptr) TTF_CloseFont(ptr); n_ptr }

int rand_int(int const lb, int const ub)
{
	static std::random_device dev;
	static std::mt19937 rng(dev());

	std::uniform_int_distribution<int> dist(lb, ub);
	return dist(rng);
}

double rand_dbl(double const lb, double const ub)
{
	static std::default_random_engine eng;

	std::uniform_real_distribution<double> dist(lb, ub);
	return dist(eng);
}

Text::Text(std::string const& _text, int _x, int _y, Align _align)
	: text(_text), dim({ _x, _y, 0, 0 }), align(_align)
{
	TTF_Init();

	font_ptr ptr(TTF_OpenFont(str_brygada.c_str(), 45));
	TTF_SizeText(ptr.get(), text.c_str(), &dim.w, &dim.h);
}

bool Text::clicked_on(int mx, int my)
{
	switch (align)
	{
	case Align::LEFT:
		return mx >= dim.x && mx <= dim.x + dim.w &&
			   my >= dim.y && my <= dim.y + dim.h;
	case Align::CENTER:
		return mx >= dim.x - (dim.w / 2) && mx <= dim.x + (dim.w / 2) &&
			   my >= dim.y - (dim.h / 2) && my <= dim.y + (dim.h / 2);
	case Align::RIGHT:
		return mx >= dim.x - dim.w && mx <= dim.x &&
			   my >= dim.y && my <= dim.y + dim.h;
	default:
		std::cout << "error - missing align";
		return false;
	}
}

}