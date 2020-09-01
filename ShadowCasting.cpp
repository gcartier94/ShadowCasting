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
	float startX, startY;
	float endX, endY;
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
	bool boundary = false;
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


	olc::Sprite *sprLightCast;
	olc::Sprite *buffLightRay;
	olc::Sprite *buffLightTex;

	// Define the vector for the pool of edges
	vector<sEdge> vecEdges;


	vector<tuple<float, float, float>> vecVisibilityPolygonPoints;

	void ConvertTileMapToPolyMap(int startX, int startY, int inputWidth, int inputHeigth, float fBlockWidth, int pitch) {
		// Clear "PolyMap"
		vecEdges.clear();

		for (int x = 0; x < inputWidth; x++)
			for (int y = 0; y < inputHeigth; y++)
				for (int j = 0; j < 4; j++)
				{
					world[(y + startY) * pitch + (x + startX)].edge_exist[j] = false;
					world[(y + startY) * pitch + (x + startX)].edge_id[j] = 0;
				}

		// Iterate through region from top left to bottom right
		for (int x = 1; x < inputWidth - 1; x++)
			for (int y = 1; y < inputHeigth - 1; y++)
			{
				// Create some convenient indices
				int i = (y + startY) * pitch + (x + startX);		// This
				int n = (y + startY - 1) * pitch + (x + startX);	// Northern Neighbour
				int s = (y + startY + 1) * pitch + (x + startX);	// Southern Neighbour
				int w = (y + startY) * pitch + (x + startX - 1);	// Western Neighbour
				int e = (y + startY) * pitch + (x + startX + 1);	// Eastern Neighbour

				// If this cell exists, check if it needs edges
				if (world[i].exist)
				{
					// If this cell has no western neighbour, it needs a western edge
					if (!world[w].exist)
					{
						// It can either extend it from its northern neighbour if they have
						// one, or It can start a new one.
						if (world[n].edge_exist[WEST])
						{
							// Northern neighbour has a western edge, so grow it downwards
							vecEdges[world[n].edge_id[WEST]].endY += fBlockWidth;
							world[i].edge_id[WEST] = world[n].edge_id[WEST];
							world[i].edge_exist[WEST] = true;
						}
						else
						{
							// Northern neighbour does not have one, so create one
							sEdge edge;
							edge.startX = (startX + x) * fBlockWidth; edge.startY = (startY + y) * fBlockWidth;
							edge.endX = edge.startX; edge.endY = edge.startY + fBlockWidth;

							// Add edge to Polygon Pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							// Update tile information with edge information
							world[i].edge_id[WEST] = edge_id;
							world[i].edge_exist[WEST] = true;
						}
					}

					// If this cell dont have an eastern neignbour, It needs a eastern edge
					if (!world[e].exist)
					{
						// It can either extend it from its northern neighbour if they have
						// one, or It can start a new one.
						if (world[n].edge_exist[EAST])
						{
							// Northern neighbour has one, so grow it downwards
							vecEdges[world[n].edge_id[EAST]].endY += fBlockWidth;
							world[i].edge_id[EAST] = world[n].edge_id[EAST];
							world[i].edge_exist[EAST] = true;
						}
						else
						{
							// Northern neighbour does not have one, so create one
							sEdge edge;
							edge.startX = (startX + x + 1) * fBlockWidth; edge.startY = (startY + y) * fBlockWidth;
							edge.endX = edge.startX; edge.endY = edge.startY + fBlockWidth;

							// Add edge to Polygon Pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							// Update tile information with edge information
							world[i].edge_id[EAST] = edge_id;
							world[i].edge_exist[EAST] = true;
						}
					}

					// If this cell doesnt have a northern neignbour, It needs a northern edge
					if (!world[n].exist)
					{
						// It can either extend it from its western neighbour if they have
						// one, or It can start a new one.
						if (world[w].edge_exist[NORTH])
						{
							// Western neighbour has one, so grow it eastwards
							vecEdges[world[w].edge_id[NORTH]].endX += fBlockWidth;
							world[i].edge_id[NORTH] = world[w].edge_id[NORTH];
							world[i].edge_exist[NORTH] = true;
						}
						else
						{
							// Western neighbour does not have one, so create one
							sEdge edge;
							edge.startX = (startX + x) * fBlockWidth; edge.startY = (startY + y) * fBlockWidth;
							edge.endX = edge.startX + fBlockWidth; edge.endY = edge.startY;

							// Add edge to Polygon Pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							// Update tile information with edge information
							world[i].edge_id[NORTH] = edge_id;
							world[i].edge_exist[NORTH] = true;
						}
					}

					// If this cell doesnt have a southern neignbour, It needs a southern edge
					if (!world[s].exist)
					{
						// It can either extend it from its western neighbour if they have
						// one, or It can start a new one.
						if (world[w].edge_exist[SOUTH])
						{
							// Western neighbour has one, so grow it eastwards
							vecEdges[world[w].edge_id[SOUTH]].endX += fBlockWidth;
							world[i].edge_id[SOUTH] = world[w].edge_id[SOUTH];
							world[i].edge_exist[SOUTH] = true;
						}
						else
						{
							// Western neighbour does not have one, so I need to create one
							sEdge edge;
							edge.startX = (startX + x) * fBlockWidth; edge.startY = (startY + y + 1) * fBlockWidth;
							edge.endX = edge.startX + fBlockWidth; edge.endY = edge.startY;

							// Add edge to Polygon Pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							// Update tile information with edge information
							world[i].edge_id[SOUTH] = edge_id;
							world[i].edge_exist[SOUTH] = true;
						}
					}

				}

			}
	}

	void CalculateVisibilityPolygon(float originX, float originY, float radius) {
		// Get rid of existing polygon
		vecVisibilityPolygonPoints.clear();

		// For each edge in PolyMap
		for (auto &edge1 : vecEdges)
		{
			// Take the start point, then the end point (we could use a pool of
			// non-duplicated points here, it would be more optimal)
			for (int i = 0; i < 2; i++)
			{
				float rdx, rdy;
				rdx = (i == 0 ? edge1.startX : edge1.endX) - originX;
				rdy = (i == 0 ? edge1.startY : edge1.endY) - originY;

				float base_ang = atan2f(rdy, rdx);

				float ang = 0;
				// For each point, cast 3 rays, 1 directly at point
				// and 1 a little bit either side
				for (int j = 0; j < 3; j++)
				{
					if (j == 0)	ang = base_ang - 0.0001f;
					if (j == 1)	ang = base_ang;
					if (j == 2)	ang = base_ang + 0.0001f;

					// Create ray along angle for required distance
					rdx = radius * cosf(ang);
					rdy = radius * sinf(ang);

					float min_t1 = INFINITY;
					float min_px = 0, min_py = 0, min_ang = 0;
					bool bValid = false;

					// Check for ray intersection with all edges
					for (auto &edge2 : vecEdges)
					{
						// Create line segment vector
						float sdx = edge2.endX - edge2.startX;
						float sdy = edge2.endY - edge2.startY;

						if (fabs(sdx - rdx) > 0.0f && fabs(sdy - rdy) > 0.0f)
						{
							// t2 is normalised distance from line segment start to line segment end of intersect point
							float t2 = (rdx * (edge2.startY - originY) + (rdy * (originX - edge2.startX))) / (sdx * rdy - sdy * rdx);
							// t1 is normalised distance from source along ray to ray length of intersect point
							float t1 = (edge2.startX + sdx * t2 - originX) / rdx;

							// If intersect point exists along ray, and along line 
							// segment then intersect point is valid
							if (t1 > 0 && t2 >= 0 && t2 <= 1.0f)
							{
								// Check if this intersect point is closest to source. If
								// it is, then store this point and reject others
								if (t1 < min_t1)
								{
									min_t1 = t1;
									min_px = originX + rdx * t1;
									min_py = originY + rdy * t1;
									min_ang = atan2f(min_py - originY, min_px - originX);
									bValid = true;
								}
							}
						}
					}

					if (bValid)// Add intersection point to visibility polygon perimeter
						vecVisibilityPolygonPoints.push_back({ min_ang, min_px, min_py });
				}
			}
		}

		// Sort perimeter points by angle from source. This will allow
		// us to draw a triangle fan.
		sort(
			vecVisibilityPolygonPoints.begin(),
			vecVisibilityPolygonPoints.end(),
			[&](const tuple<float, float, float> &t1, const tuple<float, float, float> &t2)
			{
				return get<0>(t1) < get<0>(t2);
			});

	}



public:
    bool OnUserCreate() override {

		//Allocating the memory for the world
		world = new sCell[nWorldWidth * nWorldHeight];

		// Add a boundary to the world
		for (int x = 1; x < (nWorldWidth - 1); x++)
		{
			world[1 * nWorldWidth + x].exist = true;
			world[(nWorldHeight - 2) * nWorldWidth + x].exist = true;

			world[(nWorldHeight - 2) * nWorldWidth + x].boundary = true;
			world[1 * nWorldWidth + x].boundary = true;
		}

		for (int x = 1; x < (nWorldHeight - 1); x++)
		{
			world[x * nWorldWidth + 1].exist = true;
			world[x * nWorldWidth + (nWorldWidth - 2)].exist = true;

			world[x * nWorldWidth + (nWorldWidth - 2)].boundary = true;
			world[x * nWorldWidth + 1].boundary = true;
		}

		sprLightCast = new olc::Sprite("light_cast.png");

		// Create some screen-sized off-screen buffers for lighting effect
		buffLightTex = new olc::Sprite(ScreenWidth(), ScreenHeight());
		buffLightRay = new olc::Sprite(ScreenWidth(), ScreenHeight());


		

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {

		// Defining a "debug" mode for the edges visualisation
		bool debugMode = false;

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

			// Take a region of the Tile map and convert it to a "PolyMap" 
			ConvertTileMapToPolyMap(0, 0, 40, 30, fBlockWidth, nWorldWidth);

		}

		if (GetMouse(1).bHeld)
		{
			CalculateVisibilityPolygon(fSourceX, fSourceY, 1000.0f);
		}



		// Drawing
		SetDrawTarget(nullptr);
		Clear(olc::BLACK);


		int nRaysCast = vecVisibilityPolygonPoints.size();

		// Remove duplicate (or simply similar) points from polygon
		auto it = unique(
			vecVisibilityPolygonPoints.begin(),
			vecVisibilityPolygonPoints.end(),
			[&](const tuple<float, float, float> &t1, const tuple<float, float, float> &t2)
			{
				return fabs(get<1>(t1) - get<1>(t2)) < 0.1f && fabs(get<2>(t1) - get<2>(t2)) < 0.1f;
			});

		vecVisibilityPolygonPoints.resize(distance(vecVisibilityPolygonPoints.begin(), it));

		int nRaysCast2 = vecVisibilityPolygonPoints.size();
		DrawString(4, 4, "Rays Cast: " + to_string(nRaysCast) + " Rays Drawn: " + to_string(nRaysCast2));


		// If drawing rays, set an offscreen texture as our target buffer
		if (GetMouse(1).bHeld && vecVisibilityPolygonPoints.size() > 1)
		{
			// Clear offscreen buffer for sprite
			SetDrawTarget(buffLightTex);
			Clear(olc::BLACK);

			// Draw "Radial Light" sprite to offscreen buffer, centered around 
			// source location (the mouse coordinates, buffer is 512x512)
			DrawSprite(fSourceX - 255, fSourceY - 255, sprLightCast);

			// Clear offsecreen buffer for rays
			SetDrawTarget(buffLightRay);
			Clear(olc::BLANK);

			// Draw each triangle in fan
			for (int i = 0; i < vecVisibilityPolygonPoints.size() - 1; i++)
			{
				FillTriangle(
					fSourceX,
					fSourceY,

					get<1>(vecVisibilityPolygonPoints[i]),
					get<2>(vecVisibilityPolygonPoints[i]),

					get<1>(vecVisibilityPolygonPoints[i + 1]),
					get<2>(vecVisibilityPolygonPoints[i + 1]));

			}

			// Fan will have one open edge, so draw last point of fan to first
			FillTriangle(
				fSourceX,
				fSourceY,

				get<1>(vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1]),
				get<2>(vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1]),

				get<1>(vecVisibilityPolygonPoints[0]),
				get<2>(vecVisibilityPolygonPoints[0]));

			// Wherever rays exist in ray sprite, copy over radial light sprite pixels
			SetDrawTarget(nullptr);
			for (int x = 0; x < ScreenWidth(); x++)
				for (int y = 0; y < ScreenHeight(); y++)
					if (buffLightRay->GetPixel(x, y).r > 0)
						Draw(x, y, buffLightTex->GetPixel(x, y));
		}



		// Draw Blocks from TileMap
		for (int x = 0; x < nWorldWidth; x++)
			for (int y = 0; y < nWorldHeight; y++)
			{
				if (world[y * nWorldWidth + x].exist)
					if (world[y * nWorldWidth + x].boundary) {
						FillRect(x * fBlockWidth, y * fBlockWidth, fBlockWidth, fBlockWidth, olc::DARK_RED);
					}
					else {
						FillRect(x *fBlockWidth, y *fBlockWidth, fBlockWidth, fBlockWidth, olc::BLUE);
					}

			}

		// Draw Edges from PolyMap
		if (GetKey(olc::Key::D).bHeld) {
			debugMode = true;

		}


		if (debugMode) {
			for (auto &edge : vecEdges)
			{
				DrawLine(edge.startX, edge.startY, edge.endX, edge.endY);
				FillCircle(edge.startX, edge.startY, 3, olc::BLUE);
				FillCircle(edge.endX, edge.endY, 3, olc::BLUE);
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
