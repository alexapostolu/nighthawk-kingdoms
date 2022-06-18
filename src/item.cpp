#include "item.hpp"
#include "screen.hpp"
#include "sdl2.hpp"

#include <string>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>

Item::Item(std::string const& _img, int _x, int _y)
	: img(_img)
	, x (_x + sdl2::rand_int(-20, 20)), y (_y)
	, vx(sdl2::rand_dbl(-1, 1)),		vy(-2)
	, ax(0),							ay(sdl2::rand_dbl(0.025, 0.05))
	, jx(0),							jy(0.0003)
	, alpha(255)
{
}

void Item::display() const
{
	// make sure width and height scale together
	auto p = Screen::get().get_img_dim(img);
	int h = p.second / (p.first / 45);

	Screen::get().image(img, x, y, 45, h, sdl2::Align::CENTER, (int)alpha);
}

void Item::move()
{
	x += vx;
	y += vy;

	vx += ax;
	vy += ay;

	ax += jx;
	ay += jy;

	alpha -= 2.5;
}

bool Item::out_of_range() const
{
	return alpha <= 0;
}