#pragma once

#include "sdl2.hpp"

#include <string>

struct Building
{
	std::string img;
	sdl2::Dimension dim;
	int height_d;
	int cost_gold, cost_wood, cost_stone;

	bool operator < (Building const& _building) const;
};