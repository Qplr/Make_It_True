#pragma once
#define RESOURSES "resources/"
#define EXAMPLESFOLDER "examples/"
#define EXAMPLESLIST "examples/examples.txt"
#define FONTFILENAME "resources/font.ttf"
#define CONFIGFOLDER "config/"
#define MENUCONFIG   "config/menuConfig.txt"
#define WINDOWTITLE "Make It True - "
#define FONTSIZE 24
#include "SFML/Graphics.hpp"
#include <stack>
#include "Tile.h"
#include <iostream>
#include <fstream>
using namespace std;
using namespace sf;
class Grid
{
	static enum Blocks
	{
		VOID = 0, WIRE, INPUT, CROSS, SWITCH, OR, NOR, AND, NAND, XOR, XNOR, DIODE, TRANSISTOR, TOTAL_BLOCKS
	};
	static enum Screens
	{
		MAIN = 0, MENU, EXAMPLES, SETTINGS, FILE, SCREENS
	};
	enum Settings
	{
		TIMELAPSE, RENDERGRID, TOTAL_SETTINGS
	};
	volatile int settings[TOTAL_SETTINGS] = {0, 1};
	Tile** m_grid, **selectBuf = nullptr;
	RenderWindow& window;
	Font font;
	Text input, message, title, tickrateText;
	Vector2i size = Vector2i(0, 0), selectSize = Vector2i(0, 0), selectOffset, pos1, pos2;
	Vector2f camPos;
	Clock tpsClock, frameClock;
	Time tickTimeMicros, frameTimeMicros;
	int screen = 0;
	float tickrate;
	string currentFile = "", inputString = "";
	volatile bool inputActive = false, paused = false, unsaved = false, ticksHappen = true, bufferOverlay = false;
	float ppu = 20;
	int targetPpu = ppu;
	float interfaceScale;
	const int minGridSize = 10, minPpu = 3, maxCopies = 1000;
	Texture textures[TOTAL_BLOCKS];
	const int TileTextureSize = 32;
	int selectedBlock = WIRE;
	int selectedPos = 0, copyIndex = 0;
	vector<pair<vector<Text>, int>> options;
	vector<Vector2i> updatedWires, sourceUpdateQ;
	deque<pair<Vector2i, int>> wireUpdateQ, tickUpdatePoints;
	deque<pair<Vector2i, int>> upd;
	deque<pair<Tile**, pair<Vector2i, Vector2i>>> copies;
	inline bool isSelected(Vector2i pos);
	bool isScreenClick(Vector2i pos);
	int screenClicked(int _screen, Vector2i pos);
	void updateWires(pair<Vector2i, int> param);
	inline bool isConduction(Vector2i from, Vector2i to);
	inline bool isWire(Vector2i pos);
	inline bool isBasicElement(Vector2i pos);
	inline bool isSource(Vector2i pos);
	inline bool isGate(Vector2i pos);
	inline bool isInput(Vector2i pos);
	inline bool isVoid(Vector2i pos);
	inline Tile & at(Vector2i pos);
	inline Tile & atInBuf(Vector2i pos);
	inline bool isInBounds(Vector2i pos);
	bool isUpdated(Vector2i pos);
	bool isWireInQ(Vector2i pos);
	bool wasUpdated(Vector2i pos, int dir);
	void addWireToUpdateQ(Vector2i pos, int dir);
	bool isSourceInQ(Vector2i pos);
	void getInputs(vector<bool> &inputs, Vector2i pos);
	Vector2i neighbour(Vector2i pos, int i);
	Vector2i ptc(Vector2i pix);
	Vector2i ctp(Vector2i coords);
	bool Or(vector<bool>& vals);
	bool And(vector<bool>& vals);
	bool Xor (vector<bool> & vals);
	bool Xnor(vector<bool>& vals);
	bool ser(string filename);
	bool deser(string filename, bool example = false);
	bool inputStr();
	bool saveFile();
	void newFile();
	bool openFile();
	int askForSave();
	void createMessage(string str)
	{
		message.setFillColor(Color::Red);
		message.setString(str);
	}
	int waitForInput()
	{
		Event ev;
		while (window.pollEvent(ev))
		{
			if (ev.type == Event::KeyPressed || ev.type == Event::MouseButtonPressed)
				return ev.key.code;
		}
	}
	void adjustCamPos();
	//void getQuad(VertexArray &quadint, int id, Vector2f pos, float ppuLocal);
	IntRect getRectById(int id);
public:
	Grid(RenderWindow& w, int tps, int fps);
	~Grid();
	int sizeX()const { return size.x; }
	int sizey()const { return size.y; }
	void print();
	void tick();
	void setTps(int tps) { tickTimeMicros = microseconds(1000000 / tps); }
	void setFps(int fps) { frameTimeMicros = microseconds(1000000 / fps); }
	void scroll(int delta);
	void rightCLick(Vector2i pos);
	void middleClick(Vector2i pos);
	void leftClick(Vector2i pos, bool alternative);
	void toggleBufferOverlay() { bufferOverlay = 1 - bufferOverlay; }
	void togglePause() { paused = !paused; }
	void select(Vector2i pos);
	void copy(Vector2i pos, bool mirrorH, bool mirrorV);
	void cut(Vector2i pos, bool mirrorH, bool mirrorV);
	void paste(Vector2i pos, int id = -1);
	void erase() { paste(pos1, VOID); }
	void undo();
	void redo();
	void close();
	inline void moveCamPixels(Vector2i vec)
	{
		camPos += Vector2f(vec) / float(ppu);
		adjustCamPos();
	}
	inline void scale(int delta)
	{
		targetPpu = max(targetPpu + int(delta + delta * sqrt(targetPpu) * int(log10(targetPpu))), minPpu);
	}
};
