#include "screen.hpp"
#include "sdl2.hpp"

#include <SDL.h>
#include <SDL_ttf.h>

#include <iostream>
#include <string>
#include <cassert>
#include <stdexcept>

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

		SDL_SetRenderDrawColor(renderer.get(), stroke.r, stroke.g, stroke.b, stroke.a);
		SDL_RenderDrawLine(renderer.get(), x - (w / 2) - 2, y - (h / 2) + r, x - (w / 2) - 2, y + (h / 2) - r);
		SDL_RenderDrawLine(renderer.get(), x + (w / 2), y - (h / 2) + r, x + (w / 2), y + (h / 2) - r);
		SDL_RenderDrawLine(renderer.get(), x - (w / 2) + r, y - (h / 2) - 2, x + (w / 2) - r, y - (h / 2) - 2);
		SDL_RenderDrawLine(renderer.get(), x - (w / 2) + r, y + (h / 2), x + (w / 2) - r, y + (h / 2));

		break;
	}
	default:
		throw std::runtime_error("unhandled case");
	}
}

void Screen::circle(int const x, int const y, int const r, SDL_Color const& fill,
	SDL_Color const& stroke, int weight, sdl2::Align alignment, sdl2::CircleQuad quad)
{
	SDL_SetRenderDrawColor(renderer.get(), fill.r, fill.g, fill.b, fill.a);
	switch (quad)
	{
	case sdl2::CircleQuad::ALL: {
		for (int w = 0; w < r * 2; ++w)
		{
			for (int h = 0; h < r * 2; ++h)
			{
				int dx = r - w;
				int dy = r - h;
				if ((dx * dx) + (dy * dy) <= r * r)
					SDL_RenderDrawPoint(renderer.get(), x + dx, y + dy);
			}
		}
		break;
	}
	case sdl2::CircleQuad::TOP_LEFT: {
		for (int w = r; w < r * 2 + weight; ++w)
		{
			for (int h = r; h < r * 2 + weight; ++h)
			{
				int dx = r - w;
				int dy = r - h;
				int d = (dx * dx) + (dy * dy);
				if (d <= r * r)
					SDL_RenderDrawPoint(renderer.get(), x + dx, y + dy);
				
				double t = (double)std::sqrt(d) - r;
				if (t >= 0 && t <= weight)
				{
					SDL_SetRenderDrawColor(renderer.get(), stroke.r, stroke.g, stroke.b, stroke.a);
					SDL_RenderDrawPoint(renderer.get(), x + dx, y + dy);
					SDL_SetRenderDrawColor(renderer.get(), fill.r, fill.g, fill.b, fill.a);
				}
			}
		}
		break;
	}
	case sdl2::CircleQuad::TOP_RIGHT: {
		for (int w = -weight; w < r; ++w)
		{
			for (int h = r; h < r * 2 + weight; ++h)
			{
				int dx = r - w;
				int dy = r - h;
				int d = (dx * dx) + (dy * dy);
				if (d < r * r)
					SDL_RenderDrawPoint(renderer.get(), x + dx, y + dy);

				double t = (double)std::sqrt(d) - r;
				if (t >= 0 && t <= weight)
				{
					SDL_SetRenderDrawColor(renderer.get(), stroke.r, stroke.g, stroke.b, stroke.a);
					SDL_RenderDrawPoint(renderer.get(), x + dx, y + dy);
					SDL_SetRenderDrawColor(renderer.get(), fill.r, fill.g, fill.b, fill.a);
				}
			}
		}
		break;
	}
	case sdl2::CircleQuad::BOTTOM_LEFT: {
		for (int w = r; w < r * 2 + weight; ++w)
		{
			for (int h = -weight; h < r; ++h)
			{
				int dx = r - w;
				int dy = r - h;
				int d = (dx * dx) + (dy * dy);
				if (d <= r * r)
					SDL_RenderDrawPoint(renderer.get(), x + dx, y + dy);

				double t = (double)std::sqrt(d) - r;
				if (t >= 0 && t <= weight)
				{
					SDL_SetRenderDrawColor(renderer.get(), stroke.r, stroke.g, stroke.b, stroke.a);
					SDL_RenderDrawPoint(renderer.get(), x + dx, y + dy);
					SDL_SetRenderDrawColor(renderer.get(), fill.r, fill.g, fill.b, fill.a);
				}
			}
		}
		break;
	}
	case sdl2::CircleQuad::BOTTOM_RIGHT: {
		for (int w = -weight; w < r; ++w)
		{
			for (int h = -weight; h < r; ++h)
			{
				int dx = r - w;
				int dy = r - h;
				int d = (dx * dx) + (dy * dy);
				if ((dx * dx) + (dy * dy) <= r * r)
					SDL_RenderDrawPoint(renderer.get(), x + dx, y + dy);

				double t = (double)std::sqrt(d) - r;
				if (t >= 0 && t <= weight)
				{
					SDL_SetRenderDrawColor(renderer.get(), stroke.r, stroke.g, stroke.b, stroke.a);
					SDL_RenderDrawPoint(renderer.get(), x + dx, y + dy);
					SDL_SetRenderDrawColor(renderer.get(), fill.r, fill.g, fill.b, fill.a);
				}
			}
		}
		break;
	}
	default:
		throw std::runtime_error("unhandled case");
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