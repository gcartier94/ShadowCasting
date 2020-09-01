/*

	What is this?
	~~~~~~~~~~~~~

	This is my implementation of the code originally developed by javidx9 
	for a 2D shadow casting algorithm using the olcPixelGameEngine.

	License (OLC-3)
	~~~~~~~~~~~~~~~
	Copyright 2018 OneLoneCoder.com
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:
	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.
	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.
	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
using namespace std;

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

/* 
Data structure for the edges.
Instead of analyzing each block individually 
we will analyse the edges formed by the groups of blocks
(efficency!)
*/

struct sEdge {
	float startPointX, startPointY;
	float endPointX, endPointY;
};

/* 
The basic data structure for a cell.
A block might exist in it or not. 
It also might contain up to 4 edges (a single block).
*/
struct sCell {
	int edge_id[4];
	bool edge_exist[4];
	bool exist = false;
};

#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3

class ShadowCasting : public olc::PixelGameEngine {
public:
    ShadowCasting() {
        sAppName = "ShadowCasting";
    }

private: 
	// Defining the array for the world and the size of it
	sCell *world;
	int nWorldWidth = 40;
	int nWorldHeight = 30;

	// Define the vector for the pool of edges
	vector<sEdge> vecEdges;


	/* 
	Function that will convert the tile based map
	on a polygon based map
	*/
	void ConvertTileMapToPolyMap(int startPointX, int startPointY, int width, int height,
		float fBlockWidth, int pitch) {

		// Clear "PolyMap"
		vecEdges.clear();

		// Resets the edges for all cell
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				for (int j = 0; j < 4; j++) {
					int cellIndex = (y + startPointY) * pitch + (x + startPointX);
					world[cellIndex].edge_exist[j] = false;
					world[cellIndex].edge_id[j] = 0;

				}
			}
		}

		// Iterate through region from top left to bottom right
		// P.S: The reason the index starts at 1 and goes until
		// var - 1 is to avoid out of boundaries memory problems
		for (int x = 1; x < width - 1; x++) {
			for (int y = 1; y < height - 1; y++) {
				// Convenient indices
				int currentCellIndex = (y + startPointY) * pitch + (x + startPointX);
				int northenNeighbourIndex = (y + startPointY - 1) * pitch + (x + startPointX);
				int southernNeighbourIndex = (y + startPointY + 1) * pitch + (x + startPointX);
				int westernNeighbourIndex = (y + startPointY) *pitch + (x + startPointX - 1);
				int easternNeighbourIndex = (y + startPointY) * pitch + (x + startPointX + 1);

				// If this cell exists, check if it needs edges
				if (world[currentCellIndex].exist) {

					// If this cell has not western neighbour, it needs a western edge
					if (!world[westernNeighbourIndex].exist) {

						// It can either extend it from its northen neighbour if they have
						// one, or it can start a new one
						if (world[northenNeighbourIndex].edge_exist[WEST]) {
							// Northen neighbour has a western edge, so grow it downwards
							vecEdges[world[northenNeighbourIndex].edge_id[WEST]].endPointY += fBlockWidth;
							world[currentCellIndex].edge_id[WEST] = world[northenNeighbourIndex].edge_id[WEST];
							world[currentCellIndex].edge_exist[WEST] = true;
						}
						else {
							// Northen neighbour doesn't have an edge, we will have to create one

							// Initialize new edge
							sEdge edge;
							edge.startPointX = (startPointX + x) * fBlockWidth;
							edge.startPointY = (startPointY + y) * fBlockWidth;
							edge.endPointX = edge.startPointX;
							edge.endPointY = edge.endPointY + fBlockWidth;

							// Add edge to the Polygon pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							// Update tile information with the edge information
							world[currentCellIndex].edge_id[WEST] = edge_id;
							world[currentCellIndex].edge_exist[WEST] = true;

						}

					}

					// If this cell has not eastern neighbour, it needs a western edge
					if (!world[easternNeighbourIndex].exist) {

						// It can either extend it from its northen neighbour if they have
						// one, or it can start a new one
						if (world[northenNeighbourIndex].edge_exist[EAST]) {
							// Northen neighbour has a western edge, so grow it downwards
							vecEdges[world[northenNeighbourIndex].edge_id[EAST]].endPointY += fBlockWidth;
							world[currentCellIndex].edge_id[EAST] = world[northenNeighbourIndex].edge_id[EAST];
							world[currentCellIndex].edge_exist[EAST] = true;
						}
						else {
							// Northen neighbour doesn't have an edge, we will have to create one
							// Initialize new edge
							sEdge edge;
							edge.startPointX = (startPointX + x) * fBlockWidth;
							edge.startPointY = (startPointY + y) * fBlockWidth;
							edge.endPointX = edge.startPointX;
							edge.endPointY = edge.endPointY + fBlockWidth;

							// Add edge to the Polygon pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							// Update tile information with the edge information
							world[currentCellIndex].edge_id[EAST] = edge_id;
							world[currentCellIndex].edge_exist[EAST] = true;

						}

					}

					// If this cell has not northen neighbour, it needs a western edge
					if (!world[northenNeighbourIndex].exist) {

						// It can either extend it from its western neighbour if they have
						// one, or it can start a new one
						if (world[westernNeighbourIndex].edge_exist[NORTH]) {
							// Northen neighbour has a western edge, so grow it eastwards
							vecEdges[world[westernNeighbourIndex].edge_id[NORTH]].endPointX += fBlockWidth;
							world[currentCellIndex].edge_id[NORTH] = world[westernNeighbourIndex].edge_id[NORTH];
							world[currentCellIndex].edge_exist[NORTH] = true;
						}
						else {
							// Western neighbour doesn't have an edge, we will have to create one
							// Initialize new edge
							sEdge edge;
							edge.startPointX = (startPointX + x) * fBlockWidth;
							edge.startPointY = (startPointY + y) * fBlockWidth;
							edge.endPointX = edge.startPointX + fBlockWidth;
							edge.endPointY = edge.endPointY;

							// Add edge to the Polygon pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							// Update tile information with the edge information
							world[currentCellIndex].edge_id[NORTH] = edge_id;
							world[currentCellIndex].edge_exist[NORTH] = true;

						}

					}

					// If this cell has not southern neighbour, it needs a western edge
					if (!world[southernNeighbourIndex].exist) {

						// It can either extend it from its western neighbour if they have
						// one, or it can start a new one
						if (world[westernNeighbourIndex].edge_exist[SOUTH]) {
							// Northen neighbour has a western edge, so grow it eastwards
							vecEdges[world[westernNeighbourIndex].edge_id[SOUTH]].endPointX += fBlockWidth;
							world[currentCellIndex].edge_id[SOUTH] = world[westernNeighbourIndex].edge_id[SOUTH];
							world[currentCellIndex].edge_exist[SOUTH] = true;
						}
						else {
							// Western neighbour doesn't have an edge, we will have to create one
							// Initialize new edge
							sEdge edge;
							edge.startPointX = (startPointX + x) * fBlockWidth;
							edge.startPointY = (startPointY + y) * fBlockWidth;
							edge.endPointX = edge.startPointX + fBlockWidth;
							edge.endPointY = edge.endPointY;

							// Add edge to the Polygon pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							// Update tile information with the edge information
							world[currentCellIndex].edge_id[SOUTH] = edge_id;
							world[currentCellIndex].edge_exist[SOUTH] = true;

						}

					}
				}
			}
		}
	}


public:
    bool OnUserCreate() override {
		//Allocating the memory for the world
		world = new sCell[nWorldWidth * nWorldHeight];
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
		// Defining the size of the block within cell
		float fBlockWidth = 16.0f;

		// Get a snapshot of the mouse coordinate
		float fSourceX = GetMouseX();
		float fSourceY = GetMouseY();

		// On mouse click (released)
		if (GetMouse(0).bReleased) {
			// Get the index representing which block was selected/clicked
			// i = y * width + x
			int i = ((int)fSourceY / (int)fBlockWidth) * nWorldWidth + ((int)fSourceX / (int)fBlockWidth);
			
			// Toggle the exist flag from cell
			world[i].exist = !world[i].exist;
		}


		// Drawing
		Clear(olc::BLACK);

		for (int x = 0; x < nWorldWidth; x++) {
			for (int y = 0; y < nWorldHeight; y++) {
				int cellIndex = y * nWorldWidth + x;
				if (world[cellIndex].exist)
					FillRect(x * fBlockWidth, y * fBlockWidth, fBlockWidth, fBlockWidth, olc::RED);
			}
		}


        return true;
    }
};

int main()
{
    ShadowCasting demo;
    if (demo.Construct(640, 480, 2, 2))
        demo.Start();
}
