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
	// SDL draws shapes based on top corner (x,y) and (w,h), so shape functions
	//   should convert coordinates to that format according to their mode

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

	void line(int x0, int y0, int x1, int y1);

	void trig(int x0, int y0, int x1, int y1, int x2, int y2,
		sdl2::TrigQuad const stroke_quad = sdl2::TrigQuad::ALL);
	
	void rect(int x, int y, int w, int h);
	void rect(int x, int y, int w, int h, int r);
	
	void rhom(int x, int y, int w, int h);

	void circle(int const x, int const y, int const r,
		sdl2::CircleQuad quad = sdl2::CircleQuad::ALL);

	void text(std::string const& text, int x, int y);

	std::pair<int, int> get_img_dim(std::string const& img);
	void image(std::string const& img, int x, int y, int w, int h, int alpha = 255);
	void image(std::string const& img, sdl2::Dimension const& dim, int alpha = 255);


	void line_mode(sdl2::LineMode const& mode);
	void rect_align(sdl2::RectAlign const& align);
	void image_align(sdl2::ImageAlign const& align);
	void text_font(std::string const& font);
	void text_size(int size);
	void text_align(sdl2::TextAlign const& align);

public:
	int const SCREEN_WIDTH,
			  SCREEN_HEIGHT;

private:
	Screen();

	// returns array of points on a line
	// fill_dir: 0 - left, 1 - right
	std::vector<SDL_Point> line_arr(int x0, int y0, int x1, int y1);

	SDL_Rect rect_align_coords(sdl2::RectAlign align, int x, int y, int w = -1, int h = -1) const;

	void line_aliase(int x0, int y0, int x1, int y1);
	void line_antialiase(int x0, int y0, int x1, int y1);

private:
	sdl2::window_ptr window;
	sdl2::renderer_ptr renderer;

	std::unordered_map<std::string, sdl2::texture_ptr> images;

	SDL_Color fill_clr;
	SDL_Color stroke_clr;
	int stroke_weight;

	sdl2::LineMode m_line_mode;
	sdl2::RectAlign m_rect_align;
	sdl2::ImageAlign m_image_align;
	sdl2::TextAlign m_text_align;
	std::string m_text_font;

	std::string m_text_font;
	int m_text_size;

	// StrokeAlign stroke_align;
};
