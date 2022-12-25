#include "Grid.h"
#define chr (char *)&
inline bool Grid::isConduction(Vector2i from, Vector2i to)
{
	return
		!(isVoid(from) || (isSource(from) || isGate(from)) && isBasicElement(to))
		&&
		(
		(isWire(to) || isBasicElement(to))
		||
		(isSource(from) || isInput(from) || isWire(from)) && (isInput(to))
		);
}

inline bool Grid::isSelected(Vector2i pos)
{
	if (selectedPos == 0)
		return false;
	if (selectedPos == 1)
		return pos.x == pos1.x || pos.y == pos1.y;
	return pos.x >= pos1.x && pos.x < pos2.x && pos.y >= pos1.y && pos.y < pos2.y;
}

bool Grid::isScreenClick(Vector2i pos)
{
	int temp;
	if ((temp = screenClicked(MAIN, pos)) != -1)
	{
		screen = MENU;
	}
	else if ((temp = screenClicked(MENU, pos)) != -1)
	{
		switch (temp)
		{
		case 0: //examples
			screen = EXAMPLES;
			break;
		case 1: //settings
			screen = SETTINGS;
			break;
		case 2: //file
			screen = FILE;
			break;
		case 3: //exit
			close();
			break;
		}
	}
	else if ((temp = screenClicked(FILE, pos)) != -1)
	{
		switch (temp)
		{
		case 0:
			newFile();
			break;
		case 1:
			saveFile();
			break;
		case 2:
			openFile();
			break;
		}
	}
	else if ((temp = screenClicked(EXAMPLES, pos)) != -1)
	{
		if (!deser(EXAMPLESFOLDER + options.at(screen).first.at(temp).getString(), true))
		{
			createMessage("Unable to load example.");
			waitForInput();
		}
		else
			bufferOverlay = true;
	}
	else if ((temp = screenClicked(SETTINGS, pos)) != -1)
	{
		switch (temp)
		{
		case TIMELAPSE: //timelapse
			settings[TIMELAPSE] = 1 - settings[TIMELAPSE];
			createMessage(options.at(screen).first.at(temp).getString() + (settings[TIMELAPSE] ? " On": " Off"));
			break;
		case RENDERGRID:
			settings[RENDERGRID] = 1 - settings[RENDERGRID];
			createMessage(options.at(screen).first.at(temp).getString() + (settings[RENDERGRID] ? " On" : " Off"));
		}
	}
	else
		return false;
	return true;
}

int Grid::screenClicked(int _screen, Vector2i pos)
{
	int numberOfOptions = 1;
	if (screen != _screen)
		return -1;
	int totalHeight = (window.getSize().y - options.at(screen).first.at(0).getGlobalBounds().top) / interfaceScale, maxWidth = -1;
	for (int i = 0; i < options.at(screen).second; i++)
		if (maxWidth < options.at(screen).first.at(i).getGlobalBounds().width)
			maxWidth = options.at(screen).first.at(i).getGlobalBounds().width;
	Vector2f rectangle = Vector2f(maxWidth / interfaceScale, totalHeight) * interfaceScale;
	if (pos.x < 0 || pos.x >= rectangle.x || pos.y < window.getSize().y - rectangle.y || pos.y >= window.getSize().y)
		return -1;
	return (pos.y - (window.getSize().y - rectangle.y)) / (rectangle.y / options.at(screen).second);
}

void Grid::updateWires(pair<Vector2i, int> param)
{
	if(!isInBounds(param.first))
		return;
	if (!isConduction(neighbour(param.first, param.second + 2), param.first))
		return;
	bool isActive = false, temp;
	int dir;
	Vector2i pos, tempPos;
	updatedWires.clear();
	upd.push_back(param);

	//while (!Keyboard::isKeyPressed(Keyboard::Space));
	//clock_t last = clock();

	while (!upd.empty())
	{
		
		pos = upd.back().first;
		dir = upd.back().second % 4;
		upd.pop_back();

		/*pos1 = pos;
		pos2 = pos + Vector2i(1, 1);
		selectedPos = 2;
		while (clock() < last + 500 && !Keyboard::isKeyPressed(Keyboard::LControl)) {}
		last = clock();
		selectedPos = 0;*/

		if (at(pos).id == CROSS)
		{
			tempPos = neighbour(pos, dir);
			if (isConduction(pos, tempPos) && !isUpdated(tempPos) && !wasUpdated(tempPos, dir))
				upd.push_back(make_pair(tempPos, dir));
			isActive = isActive || (isSource(tempPos) || isGate(tempPos)) && at(tempPos).state;
			tempPos = neighbour(pos, dir + 2);
			isActive = isActive || (isSource(tempPos) || isGate(tempPos)) && at(tempPos).state;
		}
		else if (at(pos).id == TRANSISTOR)
		{
			tempPos = neighbour(pos, dir + 2);
			if (isInput(tempPos))
			{
				if (!isSourceInQ(pos))
				{
					//updatedBasicElems.push_back(make_pair(pos, dir));
					updatedWires.push_back(pos);
					temp = false;
					for (int i = 1; i < 4; i++)
					{
						tempPos = neighbour(pos, dir + 2 + i);
						if (isInput(tempPos))
							temp |= at(tempPos).state;
					}
					at(pos).state = temp;
					for (int i = 0; i < 4; i++)
					{
						tempPos = neighbour(pos, dir + 2 + i);
						if (isInBounds(tempPos))
						{
							if (isConduction(pos, tempPos) && !wasUpdated(tempPos, dir + 2 + i))
								addWireToUpdateQ(tempPos, dir + 2 + i);
						}
					}
				}
			}
			else if(at(pos).state)
			{
				for (int i = 1; i < 4; i++)
				{
					tempPos = neighbour(pos, dir + 2 + i);
					if (isInBounds(tempPos))
					{
						if ((isBasicElement(tempPos) || isWire(tempPos)) && !isUpdated(tempPos))
							upd.push_back(make_pair(tempPos, dir + 2 + i));
					}
				}
			}
		}
		else if (at(pos).id == DIODE)
		{
			tempPos = neighbour(pos, dir + 2);
			if (isInput(tempPos))
			{
				//if (!isSourceInQ(pos))
				{
					//updatedBasicElems.push_back(make_pair(pos, dir));
					updatedWires.push_back(pos);
					temp = false;
					for (int i = 1; i < 4; i++)
					{
						tempPos = neighbour(pos, dir + 2 + i);
						if (isInput(tempPos))
							temp |= at(tempPos).state;
					}
					//message.setString(message.getString() + (temp ? "1" : "0"));
					at(pos).state = temp;
					for (int i = 1; i < 4; i++)
					{
						tempPos = neighbour(pos, dir + 2 + i);
						if (isInBounds(tempPos))
						{
							if ((isBasicElement(tempPos) || isWire(tempPos)) && !wasUpdated(tempPos, dir + 2 + i))
								addWireToUpdateQ(tempPos, dir + 2 + i);
						}
					}
				}
			}
			else if(isBasicElement(tempPos) || isWire(tempPos))
			{
				isActive |= at(pos).state;
			}
		}
		else
		{
			if (!isUpdated(pos))
				updatedWires.push_back(pos);
			for (int i = 0; i < 4; i++)
			{
				tempPos = neighbour(pos, i);
				if (isInBounds(tempPos))
				{
					if (isConduction(pos, tempPos) && !isUpdated(tempPos) /*&& !(at(neighbour(pos, dir + 2)).id == CROSS && i == (dir + 2) % 4)*/)
						upd.push_back(make_pair(tempPos, i));
					else
						isActive = isActive || (isSource(tempPos) || isGate(tempPos)) && isConduction(tempPos, pos) && at(tempPos).state;
				}
			}
		}
	}
	for (int i = 0; i < updatedWires.size(); i++)
	{
		if (isInput(updatedWires[i]))
		{
			if (bool(m_grid[updatedWires[i].x][updatedWires[i].y].state) != isActive)
			{
				for (int j = 0; j < 4; j++)
				{
					tempPos = neighbour(updatedWires[i], j);
					if(isInBounds(tempPos))
						if (isGate(tempPos) && !isSourceInQ(tempPos))
							sourceUpdateQ.push_back(tempPos);
				}
			}
		}
		else if (isBasicElement(updatedWires[i]))
		{
			at(updatedWires[i]).state |= isActive;
			continue;
		}
		m_grid[updatedWires[i].x][updatedWires[i].y].state = isActive;
	}
	
}

inline bool Grid::isWire(Vector2i pos)
{
	return at(pos).id == WIRE ||
		at(pos).id == CROSS;
}

inline bool Grid::isBasicElement(Vector2i pos)
{
	return at(pos).id == TRANSISTOR || at(pos).id == DIODE;
}

inline bool Grid::isSource(Vector2i pos)
{
	return at(pos).id == SWITCH;
}

inline bool Grid::isGate(Vector2i pos)
{
	return at(pos).id == OR ||
		at(pos).id == NOR ||
		at(pos).id == AND ||
		at(pos).id == NAND ||
		at(pos).id == XOR ||
		at(pos).id == XNOR;
}

bool Grid::isUpdated(Vector2i pos)
{
	for (int i = 0; i < updatedWires.size(); i++)
		if (Vector2i(pos.x, pos.y) == updatedWires.at(i))
			return true;
	return false;
}

bool Grid::isWireInQ(Vector2i pos)
{
	for (int i = 0; i < wireUpdateQ.size(); i++)
		if (Vector2i(pos.x, pos.y) == wireUpdateQ.at(i).first)
			return true;
	return false;
}

bool Grid::wasUpdated(Vector2i pos, int dir)
{
	for (int i = 0; i < tickUpdatePoints.size(); i++)
		if (tickUpdatePoints[i] == make_pair(pos, dir))
			return true;
	return false;
}

void Grid::addWireToUpdateQ(Vector2i pos, int dir)
{
	wireUpdateQ.push_back(make_pair(pos, dir));
	tickUpdatePoints.push_back(make_pair(pos, dir));
}

bool Grid::isSourceInQ(Vector2i pos)
{
	for (int i = 0; i < sourceUpdateQ.size(); i++)
		if (Vector2i(pos.x, pos.y) == sourceUpdateQ.at(i))
			return true;
	return false;
}

inline bool Grid::isInput(Vector2i pos)
{
	return at(pos).id == INPUT;
}

inline Tile & Grid::at(Vector2i pos)
{
	return m_grid[pos.x][pos.y];
}

inline bool Grid::isVoid(Vector2i pos)
{
	return at(pos).id == VOID;
}

inline Tile& Grid::atInBuf(Vector2i pos)
{
	return selectBuf[pos.x][pos.y];
}

inline bool Grid::isInBounds(Vector2i pos)
{
	return pos.x >= 0 && pos.x < size.x && pos.y >= 0 && pos.y < size.y;
}

Vector2i Grid::neighbour(Vector2i pos, int i)
{
	i = i % 4;
	if(i % 2)
		pos.y += (i > 1 ? -1 : 1);
	else
		pos.x += (i > 1 ? -1 : 1);
	return pos;
}

Vector2i Grid::ptc(Vector2i pos, float ppuLocal)
{
	return Vector2i(Vector2f(pos) / ((ppuLocal == 0) ? ppu : ppuLocal) + camPos);
}

Vector2i Grid::ctp(Vector2i coords, float ppuLocal)
{
	return Vector2i(Vector2f(coords) - camPos * ((ppuLocal == 0) ? ppu: ppuLocal));
}

bool Grid::Or(vector<bool>& vals)
{
	for (int i = 0; i < vals.size(); i++)
		if (vals[i])
			return true;
	return false;
}

bool Grid:: And (vector<bool>& vals)
{
	for (int i = 0; i < vals.size(); i++)
		if (!vals[i])
			return false;
	return true;
}

bool Grid:: Xor (vector<bool>& vals)
{
	bool temp = vals[0];
	for (int i = 1; i < vals.size(); i++)
		temp = temp ^ vals[i];
	return temp;
}

bool Grid::Xnor(vector<bool>& vals)
{
	bool temp = vals[0];
	for (int i = 1; i < vals.size(); i++)
		if (temp != vals[i])
			return false;
	return true;
}

void Grid::adjustCamPos()
{
	camPos.x = min(camPos.x, size.x - window.getSize().x / 2 / float(ppu));
	camPos.y = min(camPos.y, size.y - window.getSize().y / 2 / float(ppu));
	camPos.x = max(camPos.x, -float(window.getSize().x / 2) / float(ppu));
	camPos.y = max(camPos.y, -float(window.getSize().y / 2) / float(ppu));
}

Grid::Grid(RenderWindow& w, int tps):window(w), interfaceScale(w.getSize().x / 960.0)
{
	tickTimeMicros = 1000000 / tps;
	window.setVerticalSyncEnabled(true);
	size = Vector2i(2000, 2000);
	m_grid = new Tile * [size.x];
	for (int i = 0; i < size.x; i++)
		m_grid[i] = new Tile[size.y];
	char buf[100];
	for (int i = 0; i < TOTAL_BLOCKS; i++)
	{
		if (!textures[i].loadFromFile(RESOURSES + to_string(i) + ".bmp"))
			window.close();
	}
	
	font.loadFromFile(FONTFILENAME);
	input.setFont(font);
	input.setCharacterSize(FONTSIZE * interfaceScale);
	input.setPosition(Vector2f(window.getSize()) / 2.f);
	input.setFillColor(Color::Green);
	message.setFont(font);
	message.setCharacterSize(FONTSIZE * interfaceScale);
	message.setPosition(window.getSize().x / 2.f, window.getSize().y * 0.9);
	title.setFont(font);
	title.setCharacterSize(FONTSIZE / 2 * interfaceScale);
	title.setPosition(window.getSize().x / 2.f, interfaceScale);
	tickrateText.setFont(font);
	tickrateText.setCharacterSize(FONTSIZE / 4 * interfaceScale);
	tickrateText.setPosition(window.getSize().x / 2.f, interfaceScale + FONTSIZE);
	vector<string> examplesNames;
	ifstream file(EXAMPLESLIST);
	while (!file.eof())
	{
		file.getline(buf, 100);
		examplesNames.push_back(buf);
	}
	file.close();
	int temp;
	file.open(MENUCONFIG);
	for(int i = 0; i < SCREENS; i++)
	{
		file >> temp;
		if (i != EXAMPLES)
			options.push_back(make_pair(vector<Text>(), temp));
		else
			options.push_back(make_pair(vector<Text>(), examplesNames.size()));
	}
	file.close();
	string str;
	for (int i = 0; i < SCREENS; i++)
	{
		if (i != EXAMPLES)
		{
			file.open(CONFIGFOLDER + to_string(i) + ".txt");
			for (int j = 0; j < options.at(i).second; j++)
			{
				getline(file, str, '\n');
				options.at(i).first.push_back(Text());
				options.at(i).first.back().setFont(font);
				options.at(i).first.back().setString(str);
				options.at(i).first.back().setCharacterSize(24);
				options.at(i).first.back().setFillColor(Color::White);
				options.at(i).first.back().setPosition(0, window.getSize().y);
				options.at(i).first.back().move(Vector2f(0, -FONTSIZE * interfaceScale * (options.at(i).second - j)));
			}
			file.close();
		}
		else
		{
			options.at(EXAMPLES).second = examplesNames.size();
			for (int j = 0; j < options.at(EXAMPLES).second; j++)
			{
				options.at(EXAMPLES).first.push_back(Text());
				options.at(EXAMPLES).first.back().setFont(font);
				options.at(EXAMPLES).first.back().setString(examplesNames.at(j));
				options.at(EXAMPLES).first.back().setCharacterSize(24);
				options.at(EXAMPLES).first.back().setFillColor(Color::White);
				options.at(EXAMPLES).first.back().setPosition(0, window.getSize().y);
				options.at(EXAMPLES).first.back().move(Vector2f(0, -FONTSIZE * interfaceScale * (options.at(EXAMPLES).second - j)));
			}
		}
	}
}

Grid::~Grid()
{
	for (int i = 0; i < size.x; i++)
		delete[] m_grid[i];
	delete[] m_grid;
	for (int i = 0; i < copies.size(); i++)
	{
		for (int j = 0; j < copies.at(i).second.second.x; j++)
			delete[] copies.at(i).first[j];
		delete[] copies.at(i).first;
	}
	for (int i = 0; i < selectSize.x; i++)
		delete[] selectBuf[i];
	delete[] selectBuf;
}

void multiplyColours(Sprite& s, Vector3f&& c1)
{
	auto c2 = s.getColor();
	s.setColor(Color(c1.x * c2.r, c1.y * c2.g, c1.z * c2.b));
}

void Grid::print()
{
	Vector2i mousePosUnits;
	Sprite s;
	Vector2i temp, unitsPerScreen;
	Vector2f camPosLocal;
	float ppuLocal;
	bool activeTile;

	clock_t last = clock();

	while (window.isOpen())
	{
		if (ppu != targetPpu)
		{
			Vector2f mousePosUnitsFloat = Vector2f(Mouse::getPosition(window)) / float(ppu);
			ppuStep = int(abs(ppu - targetPpu) / 20 + 1);
			if (abs(ppu - targetPpu) < ppuStep)
				ppuStep /= abs(ppuStep);
			ppu += ppuStep * (ppu < targetPpu ? 1 : -1);
			camPos += mousePosUnitsFloat - Vector2f(Mouse::getPosition(window)) / float(ppu);
			adjustCamPos();
		}

		ppuLocal = ppu;
		camPosLocal = camPos;
		mousePosUnits = ptc(Mouse::getPosition(window));
		unitsPerScreen = Vector2i(window.getSize()) / int(ppuLocal) + Vector2i(2, 2);
		s.setScale(float(ppuLocal - settings[RENDERGRID]) / TileTextureSize, float(ppuLocal - settings[RENDERGRID]) / TileTextureSize);
		window.clear(Color(48, 48, 48));
		for (int i = 0; i < unitsPerScreen.x; i++)
			for (int j = 0; j < unitsPerScreen.y; j++)
			{
				temp.x = i + camPosLocal.x;
				temp.y = j + camPosLocal.y;
				if (isInBounds(temp))
				{	
					if (bufferOverlay && temp.x - mousePosUnits.x - selectOffset.x < selectSize.x && temp.x - mousePosUnits.x - selectOffset.x >= 0 && temp.y - mousePosUnits.y - selectOffset.y < selectSize.y && temp.y - mousePosUnits.y - selectOffset.y >= 0)
					{
						s.setTexture(textures[atInBuf(temp - mousePosUnits - selectOffset).id]);
						s.setColor(Color(64, 255, 255));
						multiplyColours(s, Vector3f(0.25, 0.25, 0.25) * float(atInBuf(temp - mousePosUnits - selectOffset).state * 3 + 1));
					}
					else
					{
						s.setTexture(textures[at(temp).id]);
						s.setColor(Color::White);
						if (isSelected(temp))
							multiplyColours(s, Vector3f(0.5, 1, 0.25));
						multiplyColours(s, Vector3f(0.25, 0.25, 0.25) * float(at(temp).state * 3 + 1));
					}
					s.setPosition((temp.x - camPosLocal.x) * ppuLocal, (temp.y - camPosLocal.y) * ppuLocal);
					window.draw(s);
				}
			}
		s.setScale(interfaceScale, interfaceScale);
		for (int i = 1; i < TOTAL_BLOCKS; i++)
		{
			s.setTexture(textures[i]);
			s.setPosition(s.getTextureRect().width * 0.2, s.getTextureRect().height * (interfaceScale + 0.2) * (i - 1) + s.getTextureRect().height * 0.2);
			if (i == selectedBlock)
				s.setColor(Color(255, 255, 255));
			else
				s.setColor(Color(128, 128, 128));
			window.draw(s);
		}
		for (int i = 0; i < options.at(screen).second; i++)
			window.draw(options.at(screen).first.at(i));

		if (inputActive)
		{
			input.setString(inputString + '|');
			input.setOrigin(Vector2f(input.getLocalBounds().width, input.getLocalBounds().height) / 2.f);
			window.draw(input);
		}
		message.setOrigin(Vector2f(message.getLocalBounds().width, message.getLocalBounds().height) / 2.f);
		window.draw(message);
		if(currentFile.empty())
			if(unsaved)
				title.setString(string(WINDOWTITLE) + "Unsaved");
			else
				title.setString(string(WINDOWTITLE) + "New");
		else
			title.setString(WINDOWTITLE + currentFile);
		title.setOrigin(Vector2f(title.getLocalBounds().width / 2, 0));
		tickrateText.setString("TPS: " + to_string(tickrate));
		tickrateText.setOrigin(Vector2f(tickrateText.getLocalBounds().width / 2, 0));
		if(unsaved)
			title.setFillColor(Color::Yellow);
		else
			title.setFillColor(Color::White);
		if (settings[TIMELAPSE])
			tickrateText.setFillColor(Color::Green);
		else if(paused)
			tickrateText.setFillColor(Color::Yellow);
		else
			tickrateText.setFillColor(Color::White);
		window.draw(title);
		window.draw(tickrateText);
		window.display();
	}
}

inline IntRect Grid::getRectById(int id)
{
	return IntRect (id * TileTextureSize, 0, TileTextureSize, TileTextureSize);
}
/*void Grid::getQuad(VertexArray& quad, int id, Vector2f pos, float ppuLocal)
{
	auto rect = getRectById(id);
	quad[0].texCoords = Vector2f(rect.left, rect.top);//top left
	quad[1].texCoords = Vector2f(rect.left + rect.width, rect.top);//top right
	quad[2].texCoords = Vector2f(rect.left + rect.width, rect.top + rect.height);//bottom right
	quad[3].texCoords = Vector2f(rect.left, rect.top + rect.height);//bottom left
	quad[0].position = Vector2f(pos.x * ppuLocal, pos.y * ppuLocal);
	quad[1].position = Vector2f((pos.x + 1) * ppuLocal, pos.y * ppuLocal);
	quad[2].position = Vector2f((pos.x + 1) * ppuLocal, (pos.y + 1) * ppuLocal);
	quad[3].position = Vector2f(pos.x * ppuLocal, (pos.y + 1) * ppuLocal);
}

void Grid::print()
{
	Vector2i mousePosUnits;
	Vector2f camPosLocal, pos, unitsPerScreen;
	float ppuLocal;
	VertexArray quad(Quads, 4);
	RenderStates states;
	states.texture = &texture;

	while (window.isOpen())
	{
		
		unitsPerScreen = Vector2f(window.getSize()) / ppuLocal + Vector2f(1, 1);
		states.transform.scale(float(ppuLocal - settings[RENDERGRID]) / ppuLocal, float(ppuLocal - settings[RENDERGRID]) / ppuLocal);
		window.clear();
		for(pos.x = -1; pos.x < unitsPerScreen.x; pos.x += 0.999)
			for (pos.y = -1; pos.y < unitsPerScreen.y; pos.y += 0.999)
			{
				if (isInBounds(Vector2i(pos + camPosLocal)))
				{
					getQuad(quad, at(Vector2i(pos + camPosLocal)).id, pos - camPosLocal + Vector2f(Vector2i(camPosLocal)), ppuLocal);
					window.draw(quad, states);
				}
			}
		//
		Sprite s(texture);
		s.setScale(interfaceScale, interfaceScale);
		for (int i = 1; i < TOTAL_BLOCKS; i++)
		{
			s.setTextureRect(getRectById(i));
			s.setPosition(TileTextureSize * 0.2, TileTextureSize * (interfaceScale + 0.2) * (i - 1) + TileTextureSize * 0.2);
			if (i == selectedBlock)
				s.setColor(Color(255, 255, 255));
			else
				s.setColor(Color(128, 128, 128));
			window.draw(s);
		}
		for (int i = 0; i < options.at(screen).second; i++)
			window.draw(options.at(screen).first.at(i));

		if (inputActive)
		{
			input.setString(tempStr + '|');
			input.setOrigin(Vector2f(input.getLocalBounds().width, input.getLocalBounds().height) / 2.f);
			window.draw(input);
		}
		message.setOrigin(Vector2f(message.getLocalBounds().width, message.getLocalBounds().height) / 2.f);
		window.draw(message);
		title.setString(WINDOWTITLE + currentFile);
		title.setOrigin(Vector2f(title.getLocalBounds().width, 0));
		tickrateText.setString("TPS: " + to_string(tickrate));
		tickrateText.setOrigin(Vector2f(tickrateText.getLocalBounds().width, 0));
		if (unsaved)
			title.setFillColor(Color::Yellow);
		else
			title.setFillColor(Color::White);
		if (settings[TIMELAPSE])
			tickrateText.setFillColor(Color::Green);
		else if (paused)
			tickrateText.setFillColor(Color::Yellow);
		else
			tickrateText.setFillColor(Color::White);
		window.draw(title);
		window.draw(tickrateText);
		window.display();
	}
}*/

void Grid::getInputs(vector<bool> &inputs, Vector2i pos)
{
	Vector2i tempPos;
	inputs.clear();
	for (int i = 0; i < 4; i++)
	{
		tempPos = neighbour(pos, i);
		if(isInBounds(tempPos))
			if (isInput(tempPos))
				inputs.push_back(at(tempPos).state);
	}
	if (inputs.size() == 0)
		inputs.push_back(false);
}

void Grid::tick()
{
	Vector2i pos, tempPos;
	vector<bool> inputs;
	int state = 0, prevState, tickTimes = 0, ticks = 0;
	Clock tickrateUpdateClock;
	while (window.isOpen())
	{
		if (settings[TIMELAPSE] || !paused && tpsClock.getElapsedTime().asMicroseconds() > tickTimeMicros)
		{
			ticks++;
			if (tickrateUpdateClock.getElapsedTime().asMicroseconds() > 1000000 / tickrateUpdatesPerSec)
			{
				tickrateUpdateClock.restart();
				tickrate = float(ticks) / (1000000 / tickrateUpdatesPerSec) * 1000000;
				ticks = 0;
			}
			tpsClock.restart();
			if (sourceUpdateQ.size() > 0)
			{
				ticksHappen = true;
				unsaved = true;
			}
			else
			{
				ticksHappen = false;
			}
			for (int i = 0; i < sourceUpdateQ.size(); i++)
			{
				pos = sourceUpdateQ[i];
				getInputs(inputs, pos);
				prevState = at(pos).state;
				switch (at(pos).id)
				{
				case SWITCH:
					state = !prevState;
					break;
				case OR:
					state = Or(inputs);
					break;
				case NOR:
					state = !Or(inputs);
					break;
				case AND:
					state = And(inputs);
					break;
				case NAND:
					state = !And(inputs);
					break;
				case XOR:
					state = Xor(inputs);
					break;
				case XNOR:
					state = Xnor(inputs);
					break;
				default:
					continue;
				}
				if (state != prevState)
				{
					m_grid[pos.x][pos.y].state = state;
					for (int i = 0; i < 4; i++)
					{
						tempPos = neighbour(pos, i);
						if (isInBounds(tempPos))
							if (isConduction(pos, tempPos))
								addWireToUpdateQ(tempPos, i);
					}
				}
			}
			sourceUpdateQ.clear();
			while (!wireUpdateQ.empty())
			{
				updateWires(wireUpdateQ.front());
				wireUpdateQ.pop_front();
			}
			tickUpdatePoints.clear();
		}
	}
}

void Grid::scroll(int delta)
{
	selectedBlock -= delta; 
	if (selectedBlock <= VOID) 
		selectedBlock = TOTAL_BLOCKS - 1;
	if (selectedBlock >= TOTAL_BLOCKS)
		selectedBlock = WIRE; 
}

void Grid::rightCLick(Vector2i pos)
{
	Vector2i tempPos = pos;
	pos = ptc(pos);

	if (!isInBounds(pos))
		return;
	if (isSelected(pos))
		if (selectedPos > 1)
		{
			paste(ptc(tempPos), selectedBlock);
			return;
		}
	switch (at(pos).id)
	{
	case VOID:
		unsaved = true;
		at(pos) = Tile(selectedBlock, false);

		if (isGate(pos))
			sourceUpdateQ.push_back(pos);

		for (int i = 0; i < 4; i++)
		{
			tempPos = neighbour(pos, i);
			if (isInBounds(tempPos))
				if (isConduction(pos, tempPos))
					addWireToUpdateQ(tempPos, i);
		}
		break;
	case SWITCH:
		sourceUpdateQ.push_back(pos);
		break;
	}
}

void Grid::leftClick(Vector2i pos, bool alternative)
{
	if (isScreenClick(pos))
		return;
	screen = MAIN;
	message.setString("");
	if (!alternative)
		return;
	pos = ptc(pos);
	Vector2i tempPos;

	if (!isInBounds(pos))
		return;
	if (at(pos).id != VOID)
	{
		unsaved = true;
		for (int i = 0; i < 4; i++)
		{
			tempPos = neighbour(pos, i);
			if (isInBounds(tempPos))
				if (isSource(tempPos) || isGate(tempPos))
					sourceUpdateQ.push_back(tempPos);
				else if (isWire(tempPos) || isInput(tempPos))
					addWireToUpdateQ(tempPos, i);
		}
		at(pos).state = false;
		at(pos).id = VOID;
	}
}

void Grid::select(Vector2i pos)
{
	pos = ptc(pos);
	if (!isInBounds(pos))
		return;
	if (selectedPos == 0)
	{
		pos1 = pos;
		pos2 = pos;
		pos2.y++;
		pos2.x++;
	}
	else if(selectedPos == 1)
	{
		pos2 = pos;
		if (pos1.x > pos2.x)
		{
			auto temp = pos1.x;
			pos1.x = pos2.x;
			pos2.x = temp;
		}
		if (pos1.y > pos2.y)
		{
			auto temp = pos1.y;
			pos1.y = pos2.y;
			pos2.y = temp;
		}
		pos2.y++;
		pos2.x++;
		selectOffset = pos1 - pos;
	}
	selectedPos = (selectedPos + 1) % 3;
}

void Grid::copy(Vector2i pos, bool mirrorH, bool mirrorV)
{
	pos = ptc(pos);
	if (!isInBounds(pos1) || !isInBounds(pos2))
		return;
	if (selectedPos != 2)
		return;
	selectOffset = pos1 - pos;
	selectedPos = 0;

	for (int i = 0; i < selectSize.x; i++)
		delete[] selectBuf[i];
	delete[] selectBuf;
	selectSize = pos2 - pos1;
	selectBuf = new Tile * [selectSize.x];
	for (int i = 0; i < selectSize.x; i++)
		selectBuf[i] = new Tile[selectSize.y];

	for (int i = 0; i < selectSize.x; i++)
		for (int j = 0; j < selectSize.y; j++)
			selectBuf[(mirrorH ? selectSize.x - i - 1 : i)][(mirrorV ? selectSize.y - j - 1 : j)] = m_grid[i + pos1.x][j + pos1.y];
	bufferOverlay = true;
}

void Grid::cut(Vector2i pos, bool mirrorH, bool mirrorV)
{
	copy(pos, mirrorH, mirrorV);
	pos = ptc(pos);
	if (!isInBounds(pos1) || !isInBounds(pos2))
		return;
	for (int i = 0; i < selectSize.x; i++)
		for (int j = 0; j < selectSize.y; j++)
			m_grid[i + pos1.x][j + pos1.y] = Tile();
	unsaved = true;
}

void Grid::paste(Vector2i pos, int id)
{
	if (id == -1)
	{
		pos = ptc(pos);
		if (!isInBounds(pos + selectOffset) || !isInBounds(pos + selectOffset + selectSize - Vector2i(1, 1)))
			return;
	}
	Tile** newCopy;
	bool fill = id != -1 && selectedPos == 2;
	Vector2i tempSize;
	if (fill)
	{
		tempSize = pos2 - pos1;
		newCopy = new Tile * [tempSize.x];
		for (int i = 0; i < tempSize.x; i++)
		{
			newCopy[i] = new Tile[tempSize.y];
			for (int j = 0; j < tempSize.y; j++)
			{
				newCopy[i][j] = m_grid[i + pos1.x][j + pos1.y];
				m_grid[i + pos1.x][j + pos1.y] = Tile(id, false);
			}
		}
		selectedPos = 0;
	}
	else
	{
		newCopy = new Tile * [selectSize.x];
		for (int i = 0; i < selectSize.x; i++)
		{
			newCopy[i] = new Tile[selectSize.y];
			for (int j = 0; j < selectSize.y; j++)
			{
				newCopy[i][j] = m_grid[i + selectOffset.x + pos.x][j + selectOffset.y + pos.y];
				m_grid[i + selectOffset.x + pos.x][j + selectOffset.y + pos.y] = selectBuf[i][j];
			}
		}
	}
	if (copyIndex != copies.size())
	{
		for (int i = copies.size() - 1; i >= copyIndex; i--)
		{
			for (int j = copies.at(i).second.first.x; j < copies.at(i).second.second.x; j++)
				delete[] copies.at(i).first[j];
			delete[] copies.at(i).first;
			copies.erase(copies.begin() + i);
		}
		copyIndex = copies.size();
	}
	
	copies.push_back(make_pair(newCopy, make_pair((fill?pos1:(selectOffset + pos)), (fill?tempSize:selectSize))));
	if (copies.size() > maxCopies)
	{
		for (int i = copies.front().second.first.x; i < copies.front().second.second.x; i++)
			delete[] copies.front().first[i];
		delete[] copies.front().first;
		copies.pop_front();
	}
	copyIndex = copies.size();
	unsaved = true;
}

void Grid::undo()
{
	if (copyIndex <= 0)
		return;
	copyIndex--;
	auto pos = copies.at(copyIndex).second.first, size = copies.at(copyIndex).second.second;
	Tile ** ar = copies.at(copyIndex).first;
	for (int i = 0; i < size.x; i++)
		for (int j = 0; j < size.y; j++)
			std::swap(ar[i][j], m_grid[i + pos.x][j + pos.y]);
	unsaved = true;
}

void Grid::redo()
{
	if (copyIndex >= copies.size())
		return;
	auto pos = copies.at(copyIndex).second.first, size = copies.at(copyIndex).second.second;
	Tile** ar = copies.at(copyIndex).first;
	for (int i = 0; i < size.x; i++)
		for (int j = 0; j < size.y; j++)
			std::swap(ar[i][j], m_grid[i + pos.x][j + pos.y]);
	copyIndex++;
	unsaved = true;
}

int Grid::askForSave()//-2 Cannot save due to ticks happening, -1 Escape, 0 No, 1 Yes, 2 No save needed
{
	int code = 0;
	if (unsaved)
	{
		while (true)
		{
			createMessage("Save current changes? Y/N/ESC");
			code = waitForInput();
			if (code == Keyboard::Escape)
			{
				message.setString("");
				return -1;
			}
			else if (code == Keyboard::Y)
			{
				message.setString("");
				if (!saveFile())
				{
					return -2;
				}
				return 1;
			}
			else if (code == Keyboard::N)
			{
				message.setString("");
				return 0;
			}
		}
	}
	return 2;
}

void Grid::close()
{
	int code = askForSave();
	if (code == -1)//Escape pressed
		return;
	else if (code == -2)//Ticks happen
		return;
	window.close();
}

bool Grid::ser(string filename)
{
	if (filename.empty())
		return false;
	ofstream file;
	if (selectedPos == 2)
	{
		file.open(filename, ios::binary);
		if (!file.is_open())
			return false;
		Vector2i tempSize = (pos2 - pos1);
		file.write(chr tempSize, sizeof(selectSize));
		for (int i = pos1.x; i < pos2.x; i++)
			for (int j = pos1.y; j < pos2.y; j++)
				file.write(chr m_grid[i][j], sizeof(m_grid[i][j]));
		file.close();
	}
	else
	{
		file.open(filename, ios::binary);
		if (!file.is_open())
			return false;
		file.write(chr size, sizeof(size));
		for (int i = 0; i < size.x; i++)
			for (int j = 0; j < size.y; j++)
				file.write(chr m_grid[i][j], sizeof(m_grid[i][j]));
	}
	file.close();
	return true;
}

bool Grid::deser(string filename, bool example)
{
	if (filename.empty())
		return false;
	Vector2i newSize;
	ifstream file(filename, ios::binary);
	if (!file.is_open())
		return false;
	Tile** newPtr;
	bool resize = false;
	if (example)
	{
		file.read(chr newSize, sizeof(newSize));
		resize = selectSize.x != newSize.x || selectSize.y != newSize.y;
		if (resize)
		{
			newPtr = new Tile * [newSize.x];
			for (int i = 0; i < newSize.x; i++)
				newPtr[i] = new Tile[newSize.y];
		}
		else
			newPtr = selectBuf;
		for (int i = 0; i < newSize.x; i++)
			for (int j = 0; j < newSize.y; j++)
				file.read(chr newPtr[i][j], sizeof(newPtr[i][j]));
		if (resize)
		{
			std::swap(newSize, selectSize);
			std::swap(newPtr, selectBuf);
			for (int i = 0; i < newSize.x; i++)
				delete[] newPtr[i];
			delete[] newPtr;
		}
		selectOffset = Vector2i(0, 0);
	}
	else
	{
		file.read(chr newSize, sizeof(newSize));
		resize = size.x != newSize.x || size.y != newSize.y;
		if (resize)
		{
			newPtr = new Tile * [newSize.x];
			for (int i = 0; i < newSize.x; i++)
				newPtr[i] = new Tile[newSize.y];
		}
		else
			newPtr = m_grid;
		for (int i = 0; i < newSize.x; i++)
			for (int j = 0; j < newSize.y; j++)
				file.read(chr newPtr[i][j], sizeof(newPtr[i][j]));
		if (resize)
		{
			std::swap(size, newSize);
			std::swap(newPtr, m_grid);
			for (int i = 0; i < newSize.x; i++)
				delete[] newPtr[i];
			delete[] newPtr;
		}
	}
	file.close();
	return true;
}

bool Grid::inputStr()
{
	Event ev;
	inputActive = true;
	inputString.clear();
	while(true)
	{
		if (window.pollEvent(ev))
		{
			if (ev.type == Event::KeyPressed)
			{
				message.setString("");
				if (ev.key.code == Keyboard::Escape)
				{
					screen = MAIN;
					inputActive = false;
					return false;
				}
				if (ev.key.code == Keyboard::Space)
				{
					inputString += ' ';
				}
				else if (ev.key.code == Keyboard::BackSpace)
				{
					if(inputString.size() > 0)
						inputString.pop_back();
				}
				else if (ev.key.code == Keyboard::Enter)
				{
					inputActive = false;
					return !inputString.empty();
				}
				else
				{
					if(ev.key.code >= Keyboard::A && ev.key.code <= Keyboard::Z)
						if(Keyboard::isKeyPressed(Keyboard::LShift) || Keyboard::isKeyPressed(Keyboard::RShift))
							inputString += char(ev.key.code - Keyboard::A + 'A');
						else
							inputString += char(ev.key.code + 'a');
					else if(ev.key.code >= Keyboard::Num0 && ev.key.code <= Keyboard::Num9)
						inputString += char(ev.key.code - Keyboard::Num0 + '0');
				}
			}
		}
	}
}

bool Grid::saveFile()
{
	bool saveAsSketch = selectedPos == 2;
	if (ticksHappen)
	{
		createMessage("Turn off all circuits before saving");
		waitForInput();
		return false;
	}
	while (true)
	{
		if (currentFile.empty() || saveAsSketch)
		{
			createMessage(saveAsSketch ? "Enter sketch name" : "Enter file name to save to");
			if (!inputStr())
				return false;
			if(!saveAsSketch)
				currentFile = inputString;
		}
		if (!ser(saveAsSketch ? EXAMPLESFOLDER + inputString:currentFile))
		{
			createMessage("Unable to save file.");
			waitForInput();
			currentFile.clear();
		}
		else if(saveAsSketch)
		{
			selectedPos = 0;
			return true;
		}
		else
		{
			unsaved = false;
			return true;
		}
	}
}

void Grid::newFile()
{
	askForSave();
	currentFile.clear();

	int newWidth = 0, newHeight = 0;

	while (newWidth <= 0)
	{
		createMessage("Enter width");
		if (!inputStr())
			return;
		newWidth = atoi(inputString.c_str());
	}
	while (newHeight <= 0)
	{
		createMessage("Enter height");
		if (!inputStr())
			return;
		newHeight = atoi(inputString.c_str());
	}
	if (newWidth != size.x || newHeight != size.y)
	{
		Vector2i newSize(newWidth, newHeight);
		Tile** newPtr = new Tile * [newSize.x];
		for (int i = 0; i < newSize.x; i++)
			newPtr[i] = new Tile[newSize.y];
		std::swap(size, newSize);
		std::swap(newPtr, m_grid);
		for (int i = 0; i < newSize.x; i++)
			delete[] newPtr[i];
		delete[] newPtr;
	}
	else
	{
		for (int i = 0; i < size.x; i++)
			for (int j = 0; j < size.y; j++)
				m_grid[i][j] = Tile();
	}
	unsaved = false;
}

bool Grid::openFile()
{
	if (askForSave() == -1)
		return false;
	while (true)
	{
		createMessage("Enter file name to open.");
		if (!inputStr())
			return false;
		currentFile = inputString;
		if (!deser(currentFile))
		{
			createMessage("File not found.");
			waitForInput();
			currentFile.clear();
		}
		else
		{
			unsaved = false;
			return true;
		}
	}
}
