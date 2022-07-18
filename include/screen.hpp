#pragma once

#include "sdl2.hpp"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <memory>
#include <unordered_map>
#include <vector>

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

	void fill(SDL_Color const& clr);
	void fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	void stroke(SDL_Color const& clr);

	// Xiaolin Wu's algorithm
	void line(int x0, int y0, int x1, int y1);

	void trig(int x0, int y0, int x1, int y1, int x2, int y2,
		sdl2::Align const align = sdl2::Align::CENTER,
		sdl2::TrigQuad const stroke_quad = sdl2::TrigQuad::ALL);
	
	void rect(int x, int y, int w, int h,
		sdl2::Align align = sdl2::Align::LEFT);

	void rect(int x, int y, int w, int h, int r,
		sdl2::Align align = sdl2::Align::LEFT);
	
	void rhom(int x, int y, int w, int h,
		sdl2::Align align = sdl2::Align::CENTER);

	void circle(int const x, int const y, int const r,
		sdl2::Align align = sdl2::Align::CENTER,
		sdl2::CircleQuad quad = sdl2::CircleQuad::ALL);

	// x, y is where the text starts, align is where the text starts
	void text(std::string const& text, std::string const& font, int size,
		int x, int y, sdl2::Align alignment);

	std::pair<int, int> get_img_dim(std::string const& img);
	void image(std::string const& img, int x, int y, int w, int h, sdl2::Align alignment, int alpha = 255);
	void image(std::string const& img, sdl2::Dimension const& dim, sdl2::Align alignment, int alpha = 255);

public:
	int const SCREEN_WIDTH,
			  SCREEN_HEIGHT;

private:
	Screen();

	// returns array of points on a line
	// fill_dir: 0 - left, 1 - right
	std::vector<SDL_Point> line_arr(int x0, int y0, int x1, int y1);

	void set_render_fill_colour(int r, int g, int b, int a);
	void set_render_stroke_colour(int r, int g, int b, int a);

private:
	sdl2::window_ptr window;
	sdl2::renderer_ptr renderer;

	std::unordered_map<std::string, sdl2::texture_ptr> images;

	SDL_Color fill_clr;
	SDL_Color stroke_clr;

	int stroke_weight;
};
