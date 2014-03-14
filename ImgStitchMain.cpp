// Author: Scott Walker
// Project: 4 Image Stich
// File: ImgStitchMain.cpp
// Description: This program will take a bunch of pieces of a whole picture and assemble them together.


#include "EasyBMP.h"
#include <math.h>
#include <iostream>
#include <climits>

using namespace std;


// Filenames for input and output
const string INFILENAMES [16] = {"img2.bmp", "img16.bmp", "img3.bmp",
	"img4.bmp", "img5.bmp", "img6.bmp", "img7.bmp", "img8.bmp",
	"img9.bmp", "img10.bmp", "img11.bmp", "img12.bmp",
	"img13.bmp", "img14.bmp", "img15.bmp", "img1.bmp"};


int rowMatch (BMP& UseTop, BMP& UseBottom )
{ // Compares the top edge of UseTop to the bottom edge of UseBottom.
  // Assumes UseTop and UseBottom are squares of same size
  // score obtained by adding the difference between color components
	RGBApixel TopPixel, BottomPixel;
	int distance = 0, score = 0;
	for (int i = 0; i < UseTop.TellWidth() - 1; i++)
	{
		TopPixel = UseTop.GetPixel(i, 0);
		BottomPixel = UseBottom.GetPixel(i, UseTop.TellHeight() - 1);
		distance = abs(TopPixel.Red - BottomPixel.Red) + abs(TopPixel.Green - BottomPixel.Green) +
				abs(TopPixel.Blue - BottomPixel.Blue);
		score += distance;
	}
	return score;

}

int columnMatch ( BMP& UseRight, BMP& UseLeft )
{
  // Compares the top edge of UseTop to the bottom edge of UseBottom.
  // Assumes UseRight and UseLeft are squares of same size
  // score obtained by adding the difference between color components
  // similar to the rowMatch
	RGBApixel RightPixel, LeftPixel;
	int distance = 0, score = 0;
	for (int i = 0; i < UseRight.TellHeight() - 1; i++)
	{
		RightPixel = UseRight.GetPixel(UseLeft.TellWidth() - 1, i);
		LeftPixel = UseLeft.GetPixel(0, i);
		distance = abs(RightPixel.Red - LeftPixel.Red) + abs(RightPixel.Green - LeftPixel.Green) +
				abs(RightPixel.Blue - LeftPixel.Blue);
		score += distance;
	}
	return score;

}

void printScore (int score[2][16][16])
{
	cout << "South Scores     West Scores" << endl;
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			cout << " " <<score[1][i][j] << "             " << score[0][16][16] << endl;
		}
	}
}

void finalScore (BMP (& images)[16], int (& score)[2][16][16]) // these were PBR
{
// uses the rowMatch and columnMatch to create scores
// score[0][i][j] is the EAST SCORE and score[1][i][j] is the NORTH SCORE
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{  
				score[1][i][j] = rowMatch(images[i], images[j]); 
				score[0][j][i] = columnMatch(images[i], images[j]);
		}
	}
	printScore(score);
}

int findNorthWest(int score[2][16][16]) 
{
// finds the North West tile by adding the best North and West scores for each tile and
// choosing the one that maximizes the score
	int tempscore = 0, max = 0, max_tile = 0;
	int northMin = 0, westMin = 0;
	for (int i = 0; i < 16; i++)
	{
		northMin = score[1][i][0];
		westMin = score[0][i][0];
		
		for(int j = 1; j < 16; ++j)
		{
			if(northMin > score[1][i][j])
			{
				northMin = score[1][i][j];
			}

			if(westMin > score[0][i][j])
			{
				westMin = score[0][i][j];
			}
		}
		tempscore = northMin + westMin;
		
		if (tempscore > max)
		{
			max = tempscore;
			max_tile = i;
		}


	}
	return max_tile;
}

int findEastNeighbor(int score[2][16][16], int tile, bool remaining[16])
{
   // for a given tile, find its eastern neighbor among the remaining ones
   // remaining[j] is true for tiles that have not yet been placed in the final image

	// If tile already used, or if all tiles used, exit/continue (use remaining[16])
	int min = INT_MAX, temp = tile, min_tile = 17;
	for (int i = 0; i < 16; i++)
	{
		// east is 0
		temp = score[0][i][tile]; // i, tile
		if (temp < min && remaining[i] && tile != i)
		{
			min = temp;
			min_tile = i;
		}
	}
	return min_tile;
}
 
int findSouthNeighbor(int score[2][16][16], int tile, bool remaining[16]) 
{
   // for a given tile, find its southern neighbor, among the remaining ones
   // remaining[j] is true for tiles that have not yet been selected for placement
   // similar to findEastNeighbor
	int min = INT_MAX, temp = tile, min_tile = 17;
	for (int i = 0; i < 16; i++)
	{
		// north is 1
		temp = score[1][i][tile]; // tile, i
		if (temp < min && remaining[i] && tile != i)
		{
			min = temp;
			min_tile = i;
		}

	}
	return min_tile;
}

void copy(BMP & InImg, BMP & OutImg, int c, int r) 
{
  // copy image to larger final picture so that the InImg is placed in row i, column j of OutImg
	for (int i = 0; i < InImg.TellWidth() && i < OutImg.TellWidth(); i++)
	{
		for (int j = 0; j < InImg.TellHeight() && i < OutImg.TellHeight(); j++)
		{
			RGBApixel pixel = InImg.GetPixel(i, j);
			OutImg.SetPixel(c * InImg.TellWidth() + i, r * InImg.TellHeight() + j, pixel);
		}
	}
}

void greedy(int score[2][16][16], BMP InImg[16], BMP & OutImg) 
{
  //greedy algorithm to put the image together
	bool remaining[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	int ordered[16];
	int index = 0;
	
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			index = 4 * i + j;
			if (i == 0 && j == 0)
			{
				// find northwest
				ordered[index] = findNorthWest(score);
				remaining[ordered[index]] = 0;
				copy(InImg[ordered[index]], OutImg, 0, 0);
			}
			else
			{
				if (j > 0)
				{
					// find east neighbor
					ordered[index] = findEastNeighbor(score, ordered[index - 1], remaining);
					remaining[ordered[index]] = 0;
					copy(InImg[ordered[index]], OutImg, j, i); // j, i
				}
				else
				{
					// find south neighbor
					ordered[index] = findSouthNeighbor(score, ordered[index - 4], remaining);
					remaining[ordered[index]] = 0;
					copy(InImg[ordered[index]], OutImg, j, i); // j, i
				}
			}
		}

	}
}

int main()
{
  BMP InImg[16], OutImg; // vector of input images and output image
  int score [2][16][16] ;       // holds EAST and NORTH scores
  for( int i=0; i<16; ++i ) // Read in the sub-images
    InImg[i].ReadFromFile( INFILENAMES[i].c_str());
  int subsize = InImg[0].TellWidth();
  OutImg.SetSize( 4*subsize, 4*subsize ); // Set size of output image
  finalScore(InImg, score);
  greedy( score, InImg, OutImg);
  cout << "Writing to file" << endl;
  system("Pause");
  OutImg.WriteToFile("FinalImage.bmp");

  return 0;
 }