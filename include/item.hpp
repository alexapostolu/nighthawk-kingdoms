#pragma once

#include <string>

class Item
{
public:
	Item(std::string const& _img, int _x, int _y);

public:
	void display() const;
	void move();
	bool out_of_range() const;

private:
	std::string img;

	double x, y;
	double vx, vy;
	double ax, ay;
	double jx, jy; // jerk!

	double alpha;
};