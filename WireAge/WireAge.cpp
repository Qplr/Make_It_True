#include <iostream>
#include "Grid.h"
#include <thread>
volatile bool paused = false, timelapse = false;
RenderWindow window;
Grid grid(window, 10, 144);
inline float vectorLength(Vector2i v) { return sqrt(v.x * v.x + v.y * v.y); }
int WinMain()
{
	window.setKeyRepeatEnabled(false);
	window.setActive(false);

	Vector2i startPos;
	Event ev;
	bool LMB_Held = false;
	while (window.isOpen())
	{
		while (window.pollEvent(ev))
		{
			if (ev.type == Event::Closed)
				grid.close();
			if (ev.type == Event::MouseWheelMoved)
				if (Keyboard::isKeyPressed(Keyboard::LControl))
					grid.scale(ev.mouseWheel.delta);
				else
					grid.scroll(ev.mouseWheel.delta);
			else if(ev.type == Event::MouseButtonPressed)
				switch (ev.key.code)
				{
				case Mouse::Left:
					startPos = Mouse::getPosition(window);
					if (!Keyboard::isKeyPressed(Keyboard::LControl))
					{
						LMB_Held = true;
						startPos = Mouse::getPosition(window);
					}
					break;
				case Mouse::Middle:
					grid.middleClick(Mouse::getPosition(window));
					break;
				case Mouse::Right:
					if (Keyboard::isKeyPressed(Keyboard::LControl))
						grid.select(Mouse::getPosition(window));
					else
						grid.rightCLick(Mouse::getPosition(window));
					break;
				}
			else if (ev.type == Event::MouseButtonReleased)
			{
				switch (ev.key.code)
				{
				case Mouse::Left:
					if (vectorLength(Mouse::getPosition(window) - startPos) < 1)
						grid.leftClick(Mouse::getPosition(window), !LMB_Held);
					LMB_Held = false;
					break;
				}
			}
			else if (ev.type == Event::MouseMoved)
			{
				if (LMB_Held)
				{
					grid.moveCamPixels(startPos - Mouse::getPosition(window));
					startPos = Mouse::getPosition(window);
				}
			}
			else if(ev.type == Event::KeyPressed)
				switch (ev.key.code)
				{
				case Keyboard::LAlt:
					grid.toggleBufferOverlay();
					break;
				case Keyboard::P:
					grid.togglePause();
					break;
				case Keyboard::Escape:
					grid.close();
					break;
				case Keyboard::C:
					if(Keyboard::isKeyPressed(Keyboard::LControl))
						grid.copy(Mouse::getPosition(window), Keyboard::isKeyPressed(Keyboard::H), Keyboard::isKeyPressed(Keyboard::V));
					break;
				case Keyboard::V:
					if (Keyboard::isKeyPressed(Keyboard::LControl))
						grid.paste(Mouse::getPosition(window));
					break;
				case Keyboard::X:
					if (Keyboard::isKeyPressed(Keyboard::LControl))
						grid.cut(Mouse::getPosition(window), Keyboard::isKeyPressed(Keyboard::H), Keyboard::isKeyPressed(Keyboard::V));
					break;
				case Keyboard::Z:
					if (Keyboard::isKeyPressed(Keyboard::LControl))
						grid.undo();
					break;
				case Keyboard::Y:
					if (Keyboard::isKeyPressed(Keyboard::LControl))
						grid.redo();
					break;
				case Keyboard::E:
					grid.erase();
					break;
				}
		}
	}
}
