#include "building.hpp"
#include "screen.hpp"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <set>
#include <string>

Building::Building(std::string const& _img, sdl2::Dimension const _dim,
	int const _height_d, int const _cost_gold, int const _cost_wood, int const _cost_stone)
	: img(_img), dim(_dim), height_d(_height_d), cost_gold(_cost_gold), cost_wood(_cost_wood)
	, cost_stone(_cost_stone)
{

}

void Building::display_building(bool const transparent) const
{
	Screen::get().image(img, dim, sdl2::Align::CENTER, transparent ? 200 : 255);
}

void Building::display_backdrop(SDL_Color const& clr) const
{
	int rect_w = std::ceil(dim.w / 20.0);
	rect_w = (rect_w + (rect_w % 2)) * 20;

	int rect_h = std::ceil(dim.h / 20.0);
	rect_h = (rect_h + (rect_h % 2)) * 20;

	int h = height_d * 20;
	Screen::get().rect(dim.x, dim.y + (h / 2), rect_w, rect_h - h, clr,
		sdl2::clr_clear, sdl2::Align::CENTER);
}

void Building::display_placement_options() const
{
	int base = dim.y - (dim.h / 2) - 30;

	Screen::get().image("checkmark.png", dim.x - 40, base, 40, 40, sdl2::Align::CENTER);
	Screen::get().image("x.png",		 dim.x + 40, base, 40, 40, sdl2::Align::CENTER);
}

bool Building::is_pressed(int x, int y) const
{
	return x >= dim.x - (dim.w / 2) && x <= dim.x + (dim.w / 2)
		&& y >= dim.y - (dim.h / 2) && y <= dim.y + (dim.h / 2);
}

void Building::add_resources() {}
bool Building::is_display_cap() { return false; }
void Building::display_item() {}

std::shared_ptr<Building> Building::create_building() const
{
	return std::make_shared<Building>(img, dim, height_d,
		cost_gold, cost_wood, cost_stone);
}

bool Building::operator < (Building const& _building) const
{
	return dim.y < _building.dim.y || (dim.y == _building.dim.y && dim.x < _building.dim.x);
}

bool Building::operator == (Building const& _building) const
{
	return dim.x == _building.dim.x && dim.y == _building.dim.y;
}



ProdBuilding::ProdBuilding(std::string const& _img, sdl2::Dimension const _dim,
	int const _height_d, int const _cost_gold, int const _cost_wood, int const _cost_stone,
	ProdType const _type, int const _rate, int const _display_cap, int const _storage_cap)
	: Building(_img, _dim, _height_d, _cost_gold, _cost_wood, _cost_stone)
	, type(_type), rate(_rate), display_cap(_display_cap), storage_cap(_storage_cap)
	, amount(0)
{

}

void ProdBuilding::add_resources()
{
	amount = std::min(storage_cap, amount + rate);
}

bool ProdBuilding::is_display_cap()
{
	return amount >= display_cap;
}

void ProdBuilding::display_item()
{
	Screen::get().rect(dim.x, dim.y - 40, 100, 100, sdl2::clr_white, sdl2::clr_clear, sdl2::Align::CENTER);
}

std::shared_ptr<Building> ProdBuilding::create_building() const
{
	return std::make_shared<ProdBuilding>(img, dim, height_d,
		cost_gold, cost_wood, cost_stone, type, rate, display_cap, storage_cap);
}