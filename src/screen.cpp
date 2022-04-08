#include "screen.hpp"
#include "sdl2.hpp"

#include <SDL.h>
#include <SDL_ttf.h>

#include <iostream>
#include <string>

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

Screen::Screen()
	: SCREEN_WIDTH(1170), SCREEN_HEIGHT(525) {}

void Screen::update()
{
	SDL_RenderPresent(renderer.get());
}

void Screen::clear()
{
	int fw = SCREEN_WIDTH / 5, fh = SCREEN_HEIGHT / 2;
	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 2; ++j)
			image("grass.png", i * fw, j * fh, fw + 1, fh + 1, sdl2::Align::LEFT);
	}
}

void Screen::rect(int x, int y, int w, int h, SDL_Color const& fill, SDL_Color const& stroke, sdl2::Align alignment)
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
		std::cout << "error - not align\n";
	};

	SDL_SetRenderDrawColor(renderer.get(), fill.r, fill.g, fill.b, fill.a);
	SDL_RenderFillRect(renderer.get(), &rect);

	SDL_SetRenderDrawColor(renderer.get(), stroke.r, stroke.g, stroke.b, stroke.a);
	SDL_RenderDrawRect(renderer.get(), &rect);
}

void Screen::circle(int center_x, int center_y, int r)
{
	const int diameter = r * 2;

	int x = r - 1;
	int y = 0;
	int tx = 1;
	int ty = 1;
	int error = tx - diameter;

	while (x >= y)
	{
		SDL_RenderDrawPoint(renderer.get(), center_x + x, center_y - y);
		SDL_RenderDrawPoint(renderer.get(), center_x + x, center_y + y);
		SDL_RenderDrawPoint(renderer.get(), center_x - x, center_y - y);
		SDL_RenderDrawPoint(renderer.get(), center_x - x, center_y + y);
		SDL_RenderDrawPoint(renderer.get(), center_x + y, center_y - x);
		SDL_RenderDrawPoint(renderer.get(), center_x + y, center_y + x);
		SDL_RenderDrawPoint(renderer.get(), center_x - y, center_y - x);
		SDL_RenderDrawPoint(renderer.get(), center_x - y, center_y + x);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
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

void Screen::image(std::string const& img, int x, int y, int w, int h, sdl2::Align alignment)
{
	auto const it = images.find(img);
	if (it == images.end())
	{
		sdl2::surface_ptr image(IMG_Load(std::string("../assets/" + img).c_str()));
		if (image == nullptr)
			std::cout << "[error] - image '" + img + "' could not load\n";

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
		rect.x = x - (w);
		rect.y = y;
		break;
	};

	SDL_RenderCopy(renderer.get(), images[img].get(), NULL, &rect);
}

void Screen::image(std::string const& img, sdl2::Dimension const& dim, sdl2::Align alignment)
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
		rect.x = dim.x - (dim.w);
		rect.y = dim.y;
		break;
	};

	SDL_RenderCopy(renderer.get(), images[img].get(), NULL, &rect);
}