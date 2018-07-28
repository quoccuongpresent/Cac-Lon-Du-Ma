// BotDemo.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <time.h>
#include <math.h>

using namespace std;

const int N_PLAYER = 5;
const int TEAM_ID_A = 0;
const int TEAM_ID_B = 1;
const int RAG_TO_SHOOT = 200;
const int GOALWIDTH = 3000;
const int RAG_TO_GOAL = 6000;
const int HALF_1 = 1;
const int HALF_2 = 2;
const int MATCH_EXTRA = 3;
const int MATCH_PENALTY = 4;

struct Position
{
	int x;
	int y;
	Position() : x(0), y(0) {}
	Position(int _x, int _y) : x(_x), y(_y) {}
	Position operator =(Position pos) { x = pos.x; y = pos.y; return *this; }
};

enum Action
{
	WAIT = 0,
	RUN,
	SHOOT
};

bool CanShoot(Position mPos, Position ballPos);
float DistancePos(Position PosA, Position PosB);

Position Rand(int mW, int mH)
{
	return Position(rand() % mW, rand() % mH);
}

class Object
{
public:
	int ID;
	Position m_pos;
	Position m_targetPos; //direction of moving/target pos
	Position m_moveSpeed;

	int m_action;
	int m_force;
};

int main()
{

	int gameTurn = 0;
	int scoreTeamA = 0;
	int scoreTeamB = 0;
	int stateMath = 0;
	int myTeamID = 1;
	int maxTurn = 0;
	int mapWidth = 14400;
	int mapHeight = 9600;

	Object *myTeam;
	Object Player_A[N_PLAYER];
	Object Player_B[N_PLAYER];
	Object oBall;

	Position oppGoal;

	//init timer rand
	srand(time(NULL));

	
	//***************************************************************
	// Write an action using std::cout or printf()					|
	// To debug log: std:cerr << "Debug messages here...";			|
	//***************************************************************

	//Formation your team, set your players position on field
	//If you don't send this, server will auto set postion for all your players.
	//format: [Player1_posX][Player1_posY][Player2_posX][Player2_posY]...[Player5_posX][Player5_posY]
	//cout <<"2660 2400 5320 2400 2660 240 2660 7200 5320 7200";
	cout << " 5810 4195 4700 4905 2200 2490 2200 4795 2200 7105";

	//Init game: Server send team ID, Mapsize WH and maxTurn
	std::cin >> myTeamID >> mapWidth >> mapHeight >> maxTurn;
	myTeam = Player_A;
	if (myTeamID == TEAM_ID_B){
		myTeam = Player_B;

	}	

	while (gameTurn++ < maxTurn)
	{

		//Server send to Bot each turn :
		//[Turn] [m_scoreTeamA] [m_scoreTeamB] [stateMath] [ballPosX] [ballPosY] [ballSpeedX] [ballSpeedY] [Player1_Team1] [Player1_Team1_posX] [Player1_Team1_posY] ...[Player1_Team2][Player1_Team2_posX][Player1_Team2_posY]...[Player5_Team2][Player5_Team2_posX][Player5_Team2_posY]

		//input common infos
		std::cin >> gameTurn >> scoreTeamA >> scoreTeamB >> stateMath;
		std::cin >> oBall.m_pos.x >> oBall.m_pos.y >> oBall.m_moveSpeed.x >> oBall.m_moveSpeed.y;

		//input team players A
		for (int i = 0; i < N_PLAYER; i++){
			std::cin >> Player_A[i].ID >> Player_A[i].m_pos.x >> Player_A[i].m_pos.y;
		}

		//Input team players B
		for (int i = 0; i < N_PLAYER; i++){
			std::cin >> Player_B[i].ID >> Player_B[i].m_pos.x >> Player_B[i].m_pos.y;
		}


		//====================	
		//Update players	  |
		//====================
		//Send action to server each turn with format:
		//[Player1_action] [X1] [Y1] [F1] [Player_2_action] [X2] [Y2] [F2]... [Player_5_action] [X5] [Y5] [F5]
		//Action: WAIT - SHOOT - RUN ;
		//X,Y: Target of the action;
		//F: force of shooting (If action == SHOOT)
		//Example output format: std::cout << "1 2 4 5 1 58 38 10 2 11 5 20 1 229 188 0 2 259 395 99";

		//Demo send players' action to server		
		for (int i = 0; i < N_PLAYER; i++)
		{
			if( stateMath != MATCH_PENALTY )
			{
				myTeam[i].m_action = RUN;
				bool nextMove = myTeam[i].m_pos.x == myTeam[i].m_targetPos.x && myTeam[i].m_pos.y == myTeam[i].m_targetPos.y;
				if (gameTurn % 20 == 0 || nextMove)
				{
					//Move player 0 to Ball
					if (i == 0)
					{
						myTeam[i].m_targetPos = oBall.m_pos;
					}
					else //random moving
					{
						Position m_targetPos_temp = Rand(mapWidth, mapHeight);
						if (myTeamID == TEAM_ID_A)
						{
							myTeam[i].m_targetPos = m_targetPos_temp;
						}
						else
						{
							myTeam[i].m_targetPos = Position(mapWidth - m_targetPos_temp.x, mapHeight - m_targetPos_temp.y);
						}
					}				

				}
				myTeam[i].m_force = 0; //this doesn't effect to RUN action
				if (CanShoot(myTeam[i].m_pos, oBall.m_pos))
				{
					myTeam[i].m_action = SHOOT;
				
					//Detect the rival goal 
					Position goalRivalPos = Position(mapWidth, mapHeight/2);
					if ((myTeamID == TEAM_ID_B && stateMath != HALF_2) || (myTeamID == TEAM_ID_A && stateMath == HALF_2)) goalRivalPos = Position(0, mapHeight / 2);
					if (DistancePos(goalRivalPos, oBall.m_pos) < RAG_TO_GOAL)
					{
						//Shoot to goal if ball nearly goal
						myTeam[i].m_targetPos = goalRivalPos;
						myTeam[i].m_force = 100;//[0-100]
					}
					else
					{
						//pass ball to next player
						myTeam[i].m_targetPos = (i + 1) < N_PLAYER ? myTeam[i + 1].m_pos : myTeam[0].m_pos;
						myTeam[i].m_force = 80; //[0-100]
					}
								
				}
			}
			else
			{
				if( i == 0 )
				{
					//shoot
					if( ( myTeamID == TEAM_ID_A && gameTurn == 0 ) || ( myTeamID == TEAM_ID_B && gameTurn == 1 ) )
					{
						myTeam[i].m_action = SHOOT;
						myTeam[i].m_force = 100;
						myTeam[i].m_targetPos = Position( 0, 6000 );
					}
					//gk
					else
					{
						if( CanShoot( myTeam[i].m_pos, oBall.m_pos ) )
						{
							myTeam[i].m_action = SHOOT;
							myTeam[i].m_force = 100;
							myTeam[i].m_targetPos = Position( mapWidth, mapHeight );
						}
						else
						{
							myTeam[i].m_action =RUN;
							myTeam[i].m_targetPos = Position( mapWidth, mapHeight / 2 );
						}
					}
				}
			}

			printf("%d %d %d %d ", myTeam[i].m_action, myTeam[i].m_targetPos.x, myTeam[i].m_targetPos.y, myTeam[i].m_force);
		}

	}
	return 0;
}



bool CanShoot(Position mPos, Position ballPos)
{
	if (RAG_TO_SHOOT >= DistancePos(mPos, ballPos))
		return true;
	return false;
}

float DistancePos(Position PosA, Position PosB){
	float dis = sqrt((PosA.x - PosB.x)*(PosA.x - PosB.x) + (PosA.y - PosB.y)*(PosA.y - PosB.y));
	return dis;
}
