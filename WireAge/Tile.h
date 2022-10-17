#pragma once
struct Tile
{
	char id = 0;
	bool state = 0;
	Tile() {}
	Tile(const Tile & from):id(from.id), state(from.state) {}
	Tile(int id, bool state) :id(id), state(state) {}
};

