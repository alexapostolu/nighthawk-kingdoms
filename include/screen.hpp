#pragma once

#include "sdl2.hpp"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <memory>
#include <unordered_map>

class Screen
{
public:
	static Screen& get();

public:
	Screen(Screen const&) = delete;
	void operator=(Screen const&) = delete;

	void set_window();

public:
	void update();
	void clear();

	void rect(int x, int y, int w, int h, SDL_Color const& fill, SDL_Color const& stroke, sdl2::Align alignment = sdl2::Align::LEFT);
	void rhombus(int x, int y, int w, int h, SDL_Color const& fill, SDL_Color const& stroke, sdl2::Align alignment = sdl2::Align::LEFT);
	void circle(int x, int y, int r);

	// x, y is where the text starts, align is where the text starts
	void text(std::string const& text, SDL_Color const& colour, std::string const& font, int size,
		int x, int y, sdl2::Align alignment);

	std::pair<int, int> get_img_dim(std::string const& img);
	void image(std::string const& img, int x, int y, int w, int h, sdl2::Align alignment);
	void image(std::string const& img, sdl2::Dimension const& dim, sdl2::Align alignment);
	void image(std::string const& img, int x, int y, float scale, sdl2::Align alignment);

public:
	int const SCREEN_WIDTH,
			  SCREEN_HEIGHT;

private:
	Screen();

private:
	sdl2::window_ptr window;
	sdl2::renderer_ptr renderer;

	std::unordered_map<std::string, sdl2::texture_ptr> images;
};
