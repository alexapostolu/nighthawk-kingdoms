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
	: SCREEN_WIDTH(1170), SCREEN_HEIGHT(525) {}

void Screen::set_window()
{
	window.reset(SDL_CreateWindow("Nighthawk - Kingdoms",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1170, 525, 0), [](auto p) { SDL_DestroyWindow(p); });

	if (!window)
	{
		std::cout << "error - failed to open window\n" << SDL_GetError();
		return;
	}

	renderer.reset(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));
	if (!renderer)
	{
		std::cout << "error - failed to create renderer\n" << SDL_GetError();
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
	image("grass.png", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, sdl2::Align::LEFT);
}

void Screen::line(int x0, int y0, int x1, int y1, SDL_Color const& color, int weight)
{
	auto plot = [&](double x, double y, double a) {
		SDL_SetRenderDrawColor(renderer.get(), color.r, color.g, color.b, color.a * a);
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

void Screen::trig(int x0, int y0, int x1, int y1, int x2, int y2,
	SDL_Color const& fill, SDL_Color const& stroke, int weight,
	sdl2::Align const align, sdl2::TrigQuad const stroke_quad)
{
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

	if (x0 == x1)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	auto pts_01 = line_arr(x0, y0, x1, y1, stroke_quad == sdl2::TrigQuad::TOP ? sdl2::clr_clear : stroke, weight);
	auto pts_02 = line_arr(x0, y0, x2, y2, stroke_quad == sdl2::TrigQuad::MIDDLE ? sdl2::clr_clear : stroke, weight);
	auto pts_12 = line_arr(x1, y1, x2, y2, stroke_quad == sdl2::TrigQuad::BOTTOM ? sdl2::clr_clear : stroke, weight);

	// y, x
	std::map<int, int> fill_pts, o;

	for (auto const& [x, y] : pts_01)
	{
		if (fill_pts.find(y) == fill_pts.end() || (x1 <= x0 && x > fill_pts[y]) || (x1 > x0 && x < fill_pts[y]))
			fill_pts[y] = x;
	}

	for (auto const& [x, y] : pts_12)
	{
		if (fill_pts.find(y) == fill_pts.end() || (x1 <= x0 && x > fill_pts[y]) || (x1 > x0 && x < fill_pts[y]))
			fill_pts[y] = x;
	}

	for (auto const& [x, y] : pts_02)
	{
		if (o.find(y) == o.end() || (x1 <= x0 && x < fill_pts[y]) || (x1 > x0 && x > fill_pts[y]))
			o[y] = x;
	}

	for (auto const& [y, x] : o)
	{
		assert(fill_pts.find(y) != fill_pts.end());
		line(fill_pts[y], y, x, y, fill, 1);
	}
}

void Screen::rect(int x, int y, int w, int h, SDL_Color const& fill,
	SDL_Color const& stroke, sdl2::Align alignment)
{
	SDL_Rect rect{ 0, 0, w, h };
	switch (alignment)
	{
	case sdl2::Align::LEFT:
		rect.x = x;
		rect.y = y;
		break;
	case sdl2::Align::CENTER:
		rect.x = x - (rect.w / 2);
		rect.y = y - (rect.h / 2);
		break;
	case sdl2::Align::RIGHT:
		rect.x = x - (rect.w);
		rect.y = y;
		break;
	default:
		throw std::runtime_error("unhandled statement");
	};

	SDL_SetRenderDrawColor(renderer.get(), fill.r, fill.g, fill.b, fill.a);
	SDL_RenderFillRect(renderer.get(), &rect);

	SDL_SetRenderDrawColor(renderer.get(), stroke.r, stroke.g, stroke.b, stroke.a);
	SDL_RenderDrawRect(renderer.get(), &rect);
}

void Screen::rect(int x, int y, int w, int h, int r, SDL_Color const& fill,
	SDL_Color const& stroke, sdl2::Align alignment)
{
	if (r > w / 2 || r > h / 2)
		throw std::runtime_error("radius too large");

	switch (alignment)
	{
	case sdl2::Align::CENTER: {
		rect(x, y, w - (r * 2), h, fill, sdl2::clr_clear, alignment);
		rect(x - (w / 2) + (r / 2), y, r, h - (r * 2), fill, sdl2::clr_clear, alignment);
		rect(x + (w / 2) - (r / 2) - 1, y, r + 0, h - (r * 2), fill, sdl2::clr_clear, alignment);
		
		circle(x - ((w / 2) - r) - 1, y - ((h / 2) - r) - 1, r + 1, fill, stroke, 1, alignment, sdl2::CircleQuad::TOP_LEFT);
		circle(x + ((w / 2) - r) - 1, y - ((h / 2) - r) - 1, r + 1, fill, stroke, 1, alignment, sdl2::CircleQuad::TOP_RIGHT);
		circle(x - ((w / 2) - r) - 1, y + ((h / 2) - r) - 1, r + 1, fill, stroke, 1, alignment, sdl2::CircleQuad::BOTTOM_LEFT);
		circle(x + ((w / 2) - r) - 1, y + ((h / 2) - r) - 1, r + 1, fill, stroke, 1, alignment, sdl2::CircleQuad::BOTTOM_RIGHT);

		line(x - (w / 2) - 2, y - (h / 2) + r, x - (w / 2) - 2, y + (h / 2) - r, stroke, 1);
		line(x + (w / 2), y - (h / 2) + r, x + (w / 2), y + (h / 2) - r, stroke, 1);
		line(x + (w / 2) - 1, y - (h / 2) + r, x + (w / 2) - 1, y + (h / 2) - r, { stroke.r, stroke.g, stroke.b,
			(unsigned char)(stroke.a / 3) }, 1);
		line(x - (w / 2) + r, y - (h / 2) - 2, x + (w / 2) - r, y - (h / 2) - 2, stroke, 1);
		line(x - (w / 2) + r, y + (h / 2), x + (w / 2) - r, y + (h / 2), stroke, 1);

		break;
	}
	default:
		throw std::runtime_error("unhandled case");
	}
}

void Screen::rhom(int x, int y, int w, int h, SDL_Color const& fill,
	SDL_Color const& stroke, int weight, sdl2::Align align)
{
	trig(x - (w / 2), y, x, y - (h / 2), x, y + (h / 2),
		fill, stroke, weight, sdl2::Align::CENTER, sdl2::TrigQuad::MIDDLE);

	trig(x + (w / 2), y, x, y - (h / 2), x, y + (h / 2),
		fill, stroke, weight, sdl2::Align::CENTER, sdl2::TrigQuad::MIDDLE);
}

void Screen::circle(int const x, int const y, int const r, SDL_Color const& fill,
	SDL_Color const& stroke, int weight, sdl2::Align alignment, sdl2::CircleQuad quad)
{
	int w1, w2, h1, h2;
	SDL_SetRenderDrawColor(renderer.get(), fill.r, fill.g, fill.b, fill.a);
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
		w2 = r * 2 + weight;
		h1 = r;
		h2 = r * 2 + weight;
		break;
	case sdl2::CircleQuad::TOP_RIGHT:
		w1 = -weight;
		w2 = r;
		h1 = r;
		h2 = r * 2 + weight;
		break;
	case sdl2::CircleQuad::BOTTOM_LEFT:
		w1 = r;
		w2 = r * 2 + weight;
		h1 = -weight;
		h2 = r;
		break;
	case sdl2::CircleQuad::BOTTOM_RIGHT:
		w1 = -weight;
		w2 = r;
		h1 = -weight;
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
				SDL_SetRenderDrawColor(renderer.get(), fill.r, fill.g, fill.b, fill.a);
			else if (d <= -1)
				SDL_SetRenderDrawColor(renderer.get(), fill.r, fill.g, fill.b, fill.a / 3);
			else if (d <= -0.5)
				SDL_SetRenderDrawColor(renderer.get(), stroke.r, stroke.g, stroke.b, stroke.a / 2);
			else if (d <= 0)
				SDL_SetRenderDrawColor(renderer.get(), stroke.r, stroke.g, stroke.b, stroke.a);
			else if (d <= weight - 1)
				SDL_SetRenderDrawColor(renderer.get(), stroke.r, stroke.g, stroke.b, stroke.a / 2);
			else if (d <= weight)
				SDL_SetRenderDrawColor(renderer.get(), stroke.r, stroke.g, stroke.b, stroke.a / 3);
			else
				continue;

			SDL_RenderDrawPoint(renderer.get(), x + dx, y + dy);
		}
	}
}

void Screen::text(std::string const& text, SDL_Color const& colour, std::string const& font, int size,
	int x, int y, sdl2::Align alignment)
{
	sdl2::font_ptr ttf_font(TTF_OpenFont(font.c_str(), size));
	sdl2::surface_ptr text_surface(TTF_RenderText_Solid(ttf_font.get(), text.c_str(), colour));
	sdl2::texture_ptr text_texture(SDL_CreateTextureFromSurface(renderer.get(), text_surface.get()));

	SDL_Rect text_rect;
	TTF_SizeText(ttf_font.get(), text.c_str(), &text_rect.w, &text_rect.h);

	switch (alignment)
	{
	case sdl2::Align::LEFT:
		text_rect.x = x;
		text_rect.y = y;
		break;
	case sdl2::Align::CENTER:
		text_rect.x = x - (text_rect.w / 2);
		text_rect.y = y;
		break;
	case sdl2::Align::RIGHT:
		text_rect.x = x - (text_rect.w);
		text_rect.y = y;
		break;
	default:
		std::cout << "error - not align\n";
	};
	
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

void Screen::image(std::string const& img, int x, int y, int w, int h, sdl2::Align alignment, int alpha)
{
	auto const it = images.find(img);
	if (it == images.end())
	{
		sdl2::surface_ptr image(IMG_Load(std::string("../assets/" + img).c_str()));
		if (image == nullptr)
			std::cout << "[fatal error] - image '" + img + "' could not load\n";

		images[img] = sdl2::texture_ptr(SDL_CreateTextureFromSurface(renderer.get(), image.get()));
	}

	SDL_Rect rect{ x, y, w, h };
	switch (alignment)
	{
	case sdl2::Align::LEFT:
		rect.x = x;
		rect.y = y;
		break;
	case sdl2::Align::CENTER:
		rect.x = x - (w / 2);
		rect.y = y - (h / 2);
		break;
	case sdl2::Align::RIGHT:
		rect.x = x - w;
		rect.y = y;
		break;
	};

	SDL_SetTextureAlphaMod(images[img].get(), alpha);
	SDL_RenderCopy(renderer.get(), images[img].get(), NULL, &rect);
}

void Screen::image(std::string const& img, sdl2::Dimension const& dim, sdl2::Align alignment, int alpha)
{
	auto const it = images.find(img);
	if (it == images.end())
	{
		sdl2::surface_ptr image(IMG_Load(std::string("../assets/" + img).c_str()));
		if (image == nullptr)
			std::cout << "[error] - image '" + img + "' could not load\n";

		images[img] = sdl2::texture_ptr(SDL_CreateTextureFromSurface(renderer.get(), image.get()));
	}

	SDL_Rect rect{ dim.x, dim.y, dim.w, dim.h };
	switch (alignment)
	{
	case sdl2::Align::LEFT:
		rect.x = dim.x;
		rect.y = dim.y;
		break;
	case sdl2::Align::CENTER:
		rect.x = dim.x - (dim.w / 2);
		rect.y = dim.y - (dim.h / 2);
		break;
	case sdl2::Align::RIGHT:
		rect.x = dim.x - dim.w;
		rect.y = dim.y;
		break;
	};

	SDL_SetTextureAlphaMod(images[img].get(), alpha);
	SDL_RenderCopy(renderer.get(), images[img].get(), NULL, &rect);
}

std::vector<SDL_Point> Screen::line_arr(int x0, int y0, int x1, int y1, SDL_Color const& color, int weight)
{
	std::vector<SDL_Point> pts;

	auto plot = [&](double x, double y, double a) 
	{
		pts.push_back(SDL_Point{ (int)x, (int)y });
		
		SDL_SetRenderDrawColor(renderer.get(), color.r, color.g, color.b, color.a * a);
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
