#include "building.hpp"
#include "screen.hpp"

#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>
#include <set>
#include <string>

Building::Building(std::string const& _img, sdl2::Dimension const _dim,
	int const _height_d, int const _cost_gold, int const _cost_wood, int const _cost_stone,
	int const _cost_iron)
	: img(_img), dim(_dim), height_d(_height_d), cost_gold(_cost_gold), cost_wood(_cost_wood)
	, cost_stone(_cost_stone), cost_iron(_cost_iron), id(inc++)
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

bool Building::can_buy(int gold, int wood, int stone, int iron) const
{
	return cost_gold <= gold && cost_wood <= wood && cost_stone <= stone && cost_iron <= iron;
}

void Building::add_resources() {}
void Building::display_item() const {}
void Building::collect_item(int& gold, int& wheat, int& wood, int& stone, int& iron) {}
void Building::display_item_collect() {}
bool Building::is_item_cap() const { return false; }
bool Building::is_item_pressed(int mx, int my) const { return false; }

std::shared_ptr<Building> Building::create_building(bool shrink, int x, int y) const
{
	return std::make_shared<Building>(img, dim, height_d,
		cost_gold, cost_wood, cost_stone, cost_iron);
}

bool Building::operator < (Building const& _building) const
{
	if (dim.x == _building.dim.x && dim.y == _building.dim.y)
		return id < _building.id;
	return dim.y < _building.dim.y || (dim.y == _building.dim.y && dim.x < _building.dim.x);
}

int Building::inc = 0;


ProdBuilding::ProdBuilding(std::string const& _img, sdl2::Dimension const _dim,
	int const _height_d, int const _cost_gold, int const _cost_wood, int const _cost_stone,
	int const _cost_iron, ProdType const _type, int const _rate, int const _display_cap,
	int const _storage_cap)
	: Building(_img, _dim, _height_d, _cost_gold, _cost_wood, _cost_stone, _cost_iron)
	, type(_type), rate(_rate), display_cap(_display_cap), storage_cap(_storage_cap)
	, amount(0)
{

}

void ProdBuilding::add_resources()
{
	amount = std::min(storage_cap, amount + rate);
}

void ProdBuilding::display_item() const
{
	int s = 70;
	int y = dim.y - (dim.h / 2) - (s / 2);
	Screen::get().rect(dim.x, y, s, s,
		sdl2::clr_gray, sdl2::clr_black, sdl2::Align::CENTER);

	std::string prod_img;
	sdl2::Dimension img_dim{ dim.x, y, 50, 0 };
	switch (type)
	{
	case ProdType::GOLD:
		prod_img = "gold.png";
		break;
	case ProdType::WHEAT:
		prod_img = "wheat.png";
		break;
	case ProdType::WOOD:
		prod_img = "wood.png";
		break;
	case ProdType::STONE:
		prod_img = "stone.png";
		break;
	case ProdType::IRON:
		prod_img = "iron.png";
		break;
	}

	auto p = Screen::get().get_img_dim(prod_img);
	img_dim.h = p.second / (p.first / img_dim.w);
	Screen::get().image(prod_img, img_dim, sdl2::Align::CENTER);
}

void ProdBuilding::collect_item(int& gold, int& wheat, int& wood, int& stone, int& iron)
{
	static std::random_device dev;
	static std::mt19937 rng(dev());

	static std::random_device dev1;
	static std::mt19937 rng1(dev1());

	switch (type)
	{
	case ProdType::GOLD:
		gold += amount;
		break;
	case ProdType::WHEAT:
		wheat += amount;
		break;
	case ProdType::WOOD:
		wood += amount;
		break;
	case ProdType::STONE:
		stone += amount;
		break;
	case ProdType::IRON:
		iron += amount;
		break;
	}

	amount = 0;

	std::uniform_int_distribution<int> dist6(-5, 5);
	std::uniform_int_distribution<float> dist7(0.03, 0.07);

	for (int i = 0; i < 7; ++i)
	{
		// time, x, y, a
		collect_items.push_back({ 0, (float)dist6(rng), (float)dist7(rng1), 255});
	}
}

void ProdBuilding::display_item_collect()
{
	for (auto it = collect_items.begin(); it != collect_items.end();)
	{
		auto& coefs = *it;

		coefs[0] += 1;

		std::string prod_img;
		switch (type)
		{
		case ProdType::GOLD:
			prod_img = "gold.png";
			break;
		case ProdType::WHEAT:
			prod_img = "wheat.png";
			break;
		case ProdType::WOOD:
			prod_img = "wood.png";
			break;
		case ProdType::STONE:
			prod_img = "stone.png";
			break;
		case ProdType::IRON:
			prod_img = "iron.png";
			break;
		}
		
		auto p = Screen::get().get_img_dim(prod_img);
		int h = p.second / (p.first / 45);
		int y = dim.y - (dim.h / 2) - 35;
		y += coefs[2] * std::pow(coefs[0], 2);
		Screen::get().image(prod_img, coefs[0] * coefs[1] + dim.x, y, 45, h, sdl2::Align::CENTER, coefs[3]);

		coefs[3] -= 0.8;
		if (coefs[3] <= 0)
			it = collect_items.erase(it);
		else
			++it;
	}
}

bool ProdBuilding::is_item_cap() const
{
	return amount >= display_cap;
}

bool ProdBuilding::is_item_pressed(int mx, int my) const
{
	int s = 70 / 2;
	int y = dim.y - (dim.h / 2) - s;

	return is_item_cap()
		&& mx >= dim.x - s && mx <= dim.x + s
		&& my >= y - s	   && my <= y + s;

}

std::shared_ptr<Building> ProdBuilding::create_building(bool shrink, int x, int y) const
{
	if (shrink)
	{
		auto d = dim;
		d.w *= 0.6;
		d.h *= 0.6;
		return std::make_shared<ProdBuilding>(img, d, height_d,
			cost_gold, cost_wood, cost_stone, cost_iron, type, rate, display_cap, storage_cap);
	}
	else if (x != -1)
	{
		auto d = dim;
		d.x = ((x - 5) / 20) * 20 + 5;
		d.y = (y / 20) * 20;;
		return std::make_shared<ProdBuilding>(img, d, height_d,
			cost_gold, cost_wood, cost_stone, cost_iron, type, rate, display_cap, storage_cap);
	}
	else
		return std::make_shared<ProdBuilding>(img, dim, height_d,
			cost_gold, cost_wood, cost_stone, cost_iron, type, rate, display_cap, storage_cap);
}