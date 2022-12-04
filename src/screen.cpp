#include "screen.hpp"
#include "sdl2.hpp"

#include <SDL.h>
#include <SDL_ttf.h>

#include <iostream>
#include <string>
#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <map>

Screen& Screen::get()
{
	static Screen instance;
	return instance;
}

Screen::Screen()
	: SCREEN_WIDTH(1170), SCREEN_HEIGHT(525)
	, fill_clr(sdl2::clr_clear), stroke_clr(sdl2::clr_clear)
	, stroke_weight(1)
	, m_line_mode(sdl2::LineMode::ANTIALIASING) {}

void Screen::set_window()
{
	window.reset(
		SDL_CreateWindow("Nighthawk - Kingdoms",
						 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
						 SCREEN_WIDTH, SCREEN_HEIGHT, 0),
		[](auto p) { SDL_DestroyWindow(p); }
	);

	if (!window)
	{
		std::cout << "error - failed to open window\n    " << SDL_GetError();
		return;
	}

	renderer.reset(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));

	if (!renderer)
	{
		std::cout << "error - failed to create renderer\n    " << SDL_GetError();
		return;
	}

	SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);
}

void Screen::update()
{
	SDL_RenderPresent(renderer.get());
}

void Screen::clear()
{
	auto p_mode = m_rect_align;

	rect_align(sdl2::RectAlign::CORNERS);
	image("grass.png", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	rect_align(p_mode);
}

void Screen::fill(SDL_Color const& clr)
{
	fill_clr = clr;
}

void Screen::fill(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	fill_clr = SDL_Color{ r, g, b, a };
}

void Screen::stroke(SDL_Color const& clr)
{
	stroke_clr = clr;
}

void Screen::line(int x0, int y0, int x1, int y1)
{
	switch (m_line_mode)
	{
	case sdl2::LineMode::ALIASING:
		line_aliase(x0, y0, x1, y1);
		break;
	case sdl2::LineMode::ANTIALIASING:
		line_antialiase(x0, y0, x1, y1);
		break;
	default:
		throw std::runtime_error("unhandled case");
	}
}

void Screen::trig(int x0, int y0, int x1, int y1, int x2, int y2,
	sdl2::TrigQuad const stroke_quad)
{
	// stroke
	
	line(x0, y0, x1, y1);
	line(x0, y0, x2, y2);
	line(x1, y1, x2, y2);

	// fill

	auto plot = [&](int x, int y) {
		SDL_SetRenderDrawColor(renderer.get(), fill_clr.r, fill_clr.g, fill_clr.b, fill_clr.a);
		SDL_RenderDrawPoint(renderer.get(), x, y);
	};

	auto interpolate = [](int x0, int y0, int x1, int y1) {
		if (x0 == x1)
			return std::vector<int>{ y0 };
		
		std::vector<int> vals;
		double a = 1.0 * (y1 - y0) / (x1 - x0);
		double d = y0;
		for (int i = x0; i <= x1; ++i)
		{
			vals.push_back(d);
			d += a;
		}

		return vals;
	};

	if (y1 < y0)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	if (y2 < y0)
	{
		std::swap(x0, x2);
		std::swap(y0, y2);
	}
	if (y2 < y1)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	auto x01 = interpolate(y0, x0, y1, x1);
	auto x12 = interpolate(y1, x1, y2, x2);
	auto x02 = interpolate(y0, x0, y2, x2);

	x01.pop_back();

	auto x012 = x01;
	x012.insert(x012.end(), x12.begin(), x12.end());

	std::vector<int> x_left, x_right;
	int m = x012.size() / 2;
	if (x02[m] < x012[m])
	{
		x_left = x02;
		x_right = x012;
	}
	else
	{
		x_left = x012;
		x_right = x02;
	}

	auto tmp_stroke = stroke_clr;

	stroke(fill_clr);
	line_mode(sdl2::LineMode::ALIASING);

	for (int y = y0 + 2; y <= y2 - 2; ++y)
		line(x_left[y - y0] + 1, y, x_right[y - y0] - 1, y);

	line_mode(sdl2::LineMode::ANTIALIASING);

	stroke(tmp_stroke);
}

void Screen::rect(int x, int y, int w, int h)
{
	SDL_Rect rect = rect_align_coords(m_rect_align, x, y, w, h);

	SDL_SetRenderDrawColor(renderer.get(), fill_clr.r, fill_clr.g, fill_clr.b, fill_clr.a);
	SDL_RenderFillRect(renderer.get(), &rect);

	SDL_SetRenderDrawColor(renderer.get(), stroke_clr.r, stroke_clr.g, stroke_clr.b, stroke_clr.a);
	SDL_RenderDrawRect(renderer.get(), &rect);
}

void Screen::rect(int x, int y, int w, int h, int r)
{
	if (r > w / 2 || r > h / 2)
		throw std::runtime_error("radius too large");

	auto tmp_stroke = stroke_clr;

	switch (m_rect_align)
	{
	case sdl2::RectAlign::CENTER: {
		stroke(sdl2::clr_clear);
		rect(x, y, w - (r * 2), h);
		rect(x - (w / 2) + (r / 2), y, r, h - (r * 2));
		rect(x + (w / 2) - (r / 2) - 1, y, r + 0, h - (r * 2));
		
		stroke(tmp_stroke);
		circle(x - ((w / 2) - r) - 1, y - ((h / 2) - r) - 1, r + 1, sdl2::CircleQuad::TOP_LEFT);
		circle(x + ((w / 2) - r) - 1, y - ((h / 2) - r) - 1, r + 1, sdl2::CircleQuad::TOP_RIGHT);
		circle(x - ((w / 2) - r) - 1, y + ((h / 2) - r) - 1, r + 1, sdl2::CircleQuad::BOTTOM_LEFT);
		circle(x + ((w / 2) - r) - 1, y + ((h / 2) - r) - 1, r + 1, sdl2::CircleQuad::BOTTOM_RIGHT);

		stroke(tmp_stroke);
		line(x - (w / 2) - 2, y - (h / 2) + r, x - (w / 2) - 2, y + (h / 2) - r);
		line(x + (w / 2),     y - (h / 2) + r, x + (w / 2),     y + (h / 2) - r);

		stroke({ stroke_clr.r, stroke_clr.g, stroke_clr.b, (unsigned char)(stroke_clr.a / 3) });
		line(x + (w / 2) - 1, y - (h / 2) + r, x + (w / 2) - 1, y + (h / 2) - r);
		line(x - (w / 2) + r, y - (h / 2) - 2, x + (w / 2) - r, y - (h / 2) - 2);
		line(x - (w / 2) + r, y + (h / 2), x + (w / 2) - r, y + (h / 2));

		break;
	}
	default:
		throw std::runtime_error("unhandled case");
	}

	stroke(tmp_stroke);
}

void Screen::rhom(int x, int y, int w, int h)
{
	trig(x - (w / 2), y, x, y - (h / 2), x, y + (h / 2), sdl2::TrigQuad::MIDDLE);
	trig(x + (w / 2), y, x, y - (h / 2), x, y + (h / 2), sdl2::TrigQuad::MIDDLE);
}

void Screen::circle(int const x, int const y, int const r,
	sdl2::CircleQuad quad)
{
	int w1, w2, h1, h2;
	switch (quad)
	{
	case sdl2::CircleQuad::ALL:
		w1 = 0;
		w2 = r * 2;
		h1 = 0;
		h2 = r * 2;
		break;
	case sdl2::CircleQuad::TOP_LEFT:
		w1 = r;
		w2 = r * 2 + stroke_weight;
		h1 = r;
		h2 = r * 2 + stroke_weight;
		break;
	case sdl2::CircleQuad::TOP_RIGHT:
		w1 = -stroke_weight;
		w2 = r;
		h1 = r;
		h2 = r * 2 + stroke_weight;
		break;
	case sdl2::CircleQuad::BOTTOM_LEFT:
		w1 = r;
		w2 = r * 2 + stroke_weight;
		h1 = -stroke_weight;
		h2 = r;
		break;
	case sdl2::CircleQuad::BOTTOM_RIGHT:
		w1 = -stroke_weight;
		w2 = r;
		h1 = -stroke_weight;
		h2 = r;
		break;
	default:
		throw std::runtime_error("unhandled case");
	}

	for (int w = w1; w < w2; ++w)
	{
		for (int h = h1; h < h2; ++h)
		{
			double dx = r - w;
			double dy = r - h;
			double d = std::sqrt((dx * dx) + (dy * dy)) - r;
			if (d < -1.5)
				SDL_SetRenderDrawColor(renderer.get(), fill_clr.r, fill_clr.g, fill_clr.b, fill_clr.a);
			else if (d <= -1)
				SDL_SetRenderDrawColor(renderer.get(), fill_clr.r, fill_clr.g, fill_clr.b, fill_clr.a / 3);
			else if (d <= -0.5)
				SDL_SetRenderDrawColor(renderer.get(), stroke_clr.r, stroke_clr.g, stroke_clr.b, stroke_clr.a / 2);
			else if (d <= 0)
				SDL_SetRenderDrawColor(renderer.get(), stroke_clr.r, stroke_clr.g, stroke_clr.b, stroke_clr.a);
			else if (d <= stroke_weight - 1)
				SDL_SetRenderDrawColor(renderer.get(), stroke_clr.r, stroke_clr.g, stroke_clr.b, stroke_clr.a / 2);
			else if (d <= stroke_weight)
				SDL_SetRenderDrawColor(renderer.get(), stroke_clr.r, stroke_clr.g, stroke_clr.b, stroke_clr.a / 3);
			else
				continue;

			SDL_RenderDrawPoint(renderer.get(), x + dx, y + dy);
		}
	}
}

void Screen::text(std::string const& text, int x, int y)
{
	sdl2::font_ptr ttf_font(TTF_OpenFont(text_font.c_str(), text_size));
	sdl2::surface_ptr text_surface(TTF_RenderText_Solid(ttf_font.get(), text.c_str(), fill_clr));
	sdl2::texture_ptr text_texture(SDL_CreateTextureFromSurface(renderer.get(), text_surface.get()));

	SDL_Rect text_rect = rect_align_coords(m_text_align, x, y);
	TTF_SizeText(ttf_font.get(), text.c_str(), &text_rect.w, &text_rect.h);
	
	SDL_RenderCopy(renderer.get(), text_texture.get(), NULL, &text_rect);
}

std::pair<int, int> Screen::get_img_dim(std::string const& img)
{
	auto const it = images.find(img);
	if (it == images.end())
	{
		sdl2::surface_ptr image(IMG_Load(std::string("../assets/" + img).c_str()));
		if (image == nullptr)
			std::cout << "[error] - image '" + img + "' could not load\n";

		images[img] = sdl2::texture_ptr(SDL_CreateTextureFromSurface(renderer.get(), image.get()));
	}
	
	SDL_Point size;
	SDL_QueryTexture(images[img].get(), NULL, NULL, &size.x, &size.y);

	return { size.x, size.y };
}

void Screen::image(std::string const& img, int x, int y, int w, int h, int alpha)
{
	if (images.find(img) == images.end())
	{
		sdl2::surface_ptr image(IMG_Load(std::string("../assets/" + img).c_str()));
		if (image == nullptr)
			std::cout << "[fatal error] - image '" + img + "' could not load\n";

		images[img] = sdl2::texture_ptr(SDL_CreateTextureFromSurface(renderer.get(), image.get()));
	}

	SDL_Rect rect = rect_align_coords(m_image_align, x, y, w, h);

	SDL_SetTextureAlphaMod(images[img].get(), alpha);
	SDL_RenderCopy(renderer.get(), images[img].get(), NULL, &rect);
}

void Screen::image(std::string const& img, sdl2::Dimension const& dim, int alpha)
{
	image(img, dim.x, dim.y, dim.w, dim.h, alpha);
}

void Screen::line_mode(sdl2::LineMode const& mode)
{
	m_line_mode = mode;
}

void Screen::rect_align(sdl2::RectAlign const& align)
{
	m_rect_align = align;
}

void Screen::image_align(sdl2::RectAlign const& align)
{
	m_image_align = align;
}

std::vector<SDL_Point> Screen::line_arr(int x0, int y0, int x1, int y1)
{
	std::vector<SDL_Point> pts;

	auto plot = [&](double x, double y, double a) 
	{
		pts.push_back(SDL_Point{ (int)x, (int)y });
		
		SDL_SetRenderDrawColor(renderer.get(), stroke_clr.r, stroke_clr.g, stroke_clr.b, stroke_clr.a * a);
		SDL_RenderDrawPoint(renderer.get(), (int)x, (int)y);
	};

	auto round = [&](double x) { return std::floor(x + 0.5); };
	auto fpart = [](double x) { return x - floor(x); };
	auto rfpart = [&](double x) { return 1 - fpart(x); };

	bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

	if (steep)
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	double dx = x1 - x0;
	double dy = y1 - y0;

	double gradient = 0;
	if (dx == 0)
		gradient = 1.0;
	else
		gradient = dy / dx;

	double xend = round(x0);
	double yend = y0 + gradient * (xend - x0);
	double xgap = rfpart(x0 + 0.5);
	double xpxl1 = xend;
	double ypxl1 = std::floor(yend);
	if (steep)
	{
		plot(ypxl1, xpxl1, rfpart(yend) * xgap);
		plot(ypxl1 + 1, xpxl1, fpart(yend) * xgap);
	}
	else
	{
		plot(xpxl1, ypxl1, rfpart(yend) * xgap);
		plot(xpxl1, ypxl1 + 1, fpart(yend) * xgap);
	}

	double intery = yend + gradient;

	xend = round(x1);
	yend = y1 + gradient * (xend - x1);
	xgap = fpart(x1 + 0.5);
	double xpxl2 = xend;
	double ypxl2 = std::floor(yend);
	if (steep)
	{
		plot(ypxl2, xpxl2, rfpart(yend) * xgap);
		plot(ypxl2 + 1, xpxl2, fpart(yend) * xgap);
	}
	else
	{
		plot(xpxl2, ypxl2, rfpart(yend) * xgap);
		plot(xpxl2, ypxl2 + 1, fpart(yend) * xgap);
	}

	for (double i = xpxl1 + 1; i <= xpxl2 - 1; ++i)
	{
		if (steep)
		{
			plot(std::floor(intery), i, rfpart(intery));
			plot(std::floor(intery) + 1, i, fpart(intery));
		}
		else
		{
			plot(i, std::floor(intery), rfpart(intery));
			plot(i, std::floor(intery) + 1, fpart(intery));
		}

		intery += gradient;
	}

	return pts;
}

SDL_Rect Screen::rect_align_coords(sdl2::RectAlign align, int x, int y, int w, int h) const
{
	SDL_Rect rect{ -1, -1, w, h };

	switch (align)
	{
	case sdl2::RectAlign::CORNERS:
		rect.x = x;
		rect.y = y;
		break;
	case sdl2::RectAlign::CENTER_LEFT:
		rect.x = x;
		rect.y = y - (rect.w / 2);
		break;
	case sdl2::RectAlign::CENTER:
		rect.x = x - (rect.w / 2);
		rect.y = y - (rect.h / 2);
		break;
	case sdl2::RectAlign::CENTER_RIGHT:
		rect.x = x - rect.w;
		rect.y = y - (rect.h / 2);
		break;
	default:
		throw std::runtime_error("unhandled statement");
	};

	return rect;
}

void Screen::line_aliase(int x0, int y0, int x1, int y1)
{
	auto plot = [&](int x, int y) {
		SDL_SetRenderDrawColor(renderer.get(), stroke_clr.r, stroke_clr.g, stroke_clr.b, stroke_clr.a);
		SDL_RenderDrawPoint(renderer.get(), x, y);
	};

	int dx = std::abs(x1 - x0);
	int sx = x0 < x1 ? 1 : -1;
	int dy = -std::abs(y1 - y0);
	int sy = y0 < y1 ? 1 : -1;
	int error = dx + dy;
	
	while (true)
	{
		plot(x0, y0);

		if (x0 == x1 && y0 == y1)
			break;
		
		int e2 = error * 2;
		if (e2 >= dy)
		{
			if (x0 == x1)
				break;

			error += dy;
			x0 = x0 + sx;
		}

		if (e2 <= dx)
		{
			if (y0 == y1)
				break;

			error += dx;
			y0 = y0 + sy;
		}
	}
}

void Screen::line_antialiase(int x0, int y0, int x1, int y1)
{
	auto plot = [&](double x, double y, double a) {
		SDL_SetRenderDrawColor(renderer.get(), stroke_clr.r, stroke_clr.g, stroke_clr.b, stroke_clr.a * a);
		SDL_RenderDrawPoint(renderer.get(), (int)x, (int)y);
	};

	auto round = [&](double x) { return std::floor(x + 0.5); };
	auto fpart = [](double x) { return x - floor(x); };
	auto rfpart = [&](double x) { return 1 - fpart(x); };

	bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

	if (steep)
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	double dx = x1 - x0;
	double dy = y1 - y0;

	double gradient = 0;
	if (dx == 0)
		gradient = 1.0;
	else
		gradient = dy / dx;

	double xend = round(x0);
	double yend = y0 + gradient * (xend - x0);
	double xgap = rfpart(x0 + 0.5);
	double xpxl1 = xend;
	double ypxl1 = std::floor(yend);
	if (steep)
	{
		plot(ypxl1, xpxl1, rfpart(yend) * xgap);
		plot(ypxl1 + 1, xpxl1, fpart(yend) * xgap);
	}
	else
	{
		plot(xpxl1, ypxl1, rfpart(yend) * xgap);
		plot(xpxl1, ypxl1 + 1, fpart(yend) * xgap);
	}

	double intery = yend + gradient;

	xend = round(x1);
	yend = y1 + gradient * (xend - x1);
	xgap = fpart(x1 + 0.5);
	double xpxl2 = xend;
	double ypxl2 = std::floor(yend);
	if (steep)
	{
		plot(ypxl2, xpxl2, rfpart(yend) * xgap);
		plot(ypxl2 + 1, xpxl2, fpart(yend) * xgap);
	}
	else
	{
		plot(xpxl2, ypxl2, rfpart(yend) * xgap);
		plot(xpxl2, ypxl2 + 1, fpart(yend) * xgap);
	}

	for (double i = xpxl1 + 1; i <= xpxl2 - 1; ++i)
	{
		if (steep)
		{
			plot(std::floor(intery), i, rfpart(intery));
			plot(std::floor(intery) + 1, i, fpart(intery));
		}
		else
		{
			plot(i, std::floor(intery), rfpart(intery));
			plot(i, std::floor(intery) + 1, fpart(intery));
		}

		intery += gradient;
	}
}