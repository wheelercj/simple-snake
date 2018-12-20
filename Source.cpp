#include <Windows.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
using namespace std;

void showConsoleCursor(bool);
void printWalls(HANDLE);
void newSnakeCoords(vector<COORD>&, int, char);
void checkWallContact(COORD&, bool&, bool);
COORD newTarget(vector<COORD>, HANDLE);
void getInput(char&);

// field dimensions
const int leftEdge = 29;
const int rightEdge = 72;
const int topEdge = 5;
const int bottomEdge = 19;

int main()
{
	// game settings
	bool loopedField = true; // if false, the walls are deadly
	int snakeSpeed = 90;     // milliseconds of pause per snake movement (a greater number makes the snake slower)
	int snakeGrowthRate = 4; // segments gained per target hit
	const char scales = 178; // the ascii char for the snake's segments
	const char food = 254;   // the ascii char for the snake's food
	showConsoleCursor(false);
	short snakeStartX = (leftEdge + rightEdge) / 2, // snake starting coordinates
		snakeStartY = (topEdge + bottomEdge) / 2;
	
	vector<COORD> snake;
	srand((unsigned)time(0));
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut == INVALID_HANDLE_VALUE)
		return 0;

	// print the snake in its starting position
	SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN | BACKGROUND_GREEN | BACKGROUND_INTENSITY);
	SetConsoleCursorPosition(hStdOut, { snakeStartX, snakeStartY });
	cout << scales;
	SetConsoleCursorPosition(hStdOut, { snakeStartX - 1, snakeStartY });
	cout << scales;
	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	printWalls(hStdOut);

	// print game instructions
	SetConsoleCursorPosition(hStdOut, { 1, 1 });
	cout << "Use WASD to control the snake.";
	SetConsoleCursorPosition(hStdOut, { 3, 3 });
	cout << "Press any key to start.";
	_getch();

	while (true)
	{
		int score = 0,
			head = 0;
		char direction = 'd';
		bool gameOver = false;

		system("cls"); // clear screen
		snake.clear();
		snake.push_back({ snakeStartX + 1, snakeStartY });
		snake.push_back({ snakeStartX, snakeStartY });
		snake.push_back({ snakeStartX - 1, snakeStartY });

		SetConsoleCursorPosition(hStdOut, { 6, 3 });
		cout << " Score:  0";
		printWalls(hStdOut);
		COORD target = newTarget(snake, hStdOut);
		cout << food;

		// start the game
		while (!gameOver) // this loop iterates many times every second
		{
			head = (head + 1) % snake.size(); // The snake coordinate vector is a circular queue. The snake grows as the vector does.
			Sleep(snakeSpeed);

			// check for input from the user
			getInput(direction);

			HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if (hStdOut == INVALID_HANDLE_VALUE)
				return 0;

			// erase the oldest snake segment
			if (head < snake.size() - 1)
				SetConsoleCursorPosition(hStdOut, snake[head + 1]);
			else
				SetConsoleCursorPosition(hStdOut, snake[0]);
			cout << ' ';

			newSnakeCoords(snake, head, direction);
			checkWallContact(snake[head], gameOver, loopedField);

			// print the new snake segment
			SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN | BACKGROUND_GREEN | BACKGROUND_INTENSITY);
			SetConsoleCursorPosition(hStdOut, snake[head]);
			cout << scales;
			SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

			// detect target hit
			if (snake[head].X == target.X && snake[head].Y == target.Y)
			{
				snake.resize(snake.size() + snakeGrowthRate);
				score += 5;
				SetConsoleCursorPosition(hStdOut, { 15, 3 });
				cout << score;

				target = newTarget(snake, hStdOut);
				cout << food;
			}

			// detect whether the snake bit itself
			for (int i = 0; i < snake.size(); i++)
			{
				if (i != head && snake[i].X == snake[head].X && snake[i].Y == snake[head].Y)
				{
					gameOver = true;
					SetConsoleTextAttribute(hStdOut, FOREGROUND_RED);
					SetConsoleCursorPosition(hStdOut, snake[i]);
					cout << scales;
					SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
					break;
				}
			}
		}

		// game over
		SetConsoleCursorPosition(hStdOut, { 7, 9 });
		cout << "Game Over!";
		Sleep(1000);
		SetConsoleCursorPosition(hStdOut, { 5, 11 });
		cout << "Play again? (y/n)";
		char playAgain;
		do
		{
			playAgain = tolower(_getch());
			if (playAgain == 'n')
				return 0;
		} while (playAgain != 'y');
	}
}

void showConsoleCursor(bool flag)
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(out, &cursorInfo);

	cursorInfo.bVisible = flag;
	SetConsoleCursorInfo(out, &cursorInfo);
}

void printWalls(HANDLE hStdOut)
{
	const char box = 219; // the ascii char used for the walls
	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_BLUE);

	// top wall
	SetConsoleCursorPosition(hStdOut, { leftEdge - 1, topEdge - 1 });
	for (short i = 0; i < rightEdge - leftEdge + 3; i++)
		cout << box;

	// bottom wall
	SetConsoleCursorPosition(hStdOut, { leftEdge - 1, bottomEdge + 1 });
	for (short i = 0; i < rightEdge - leftEdge + 3; i++)
		cout << box;

	// left and right walls
	for (short i = topEdge - 1; i < bottomEdge + 1; i++)
	{
		SetConsoleCursorPosition(hStdOut, { leftEdge - 1, i });
		cout << box;
		SetConsoleCursorPosition(hStdOut, { rightEdge + 1, i });
		cout << box;
	}

	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void newSnakeCoords(vector<COORD>& snake, int head, char direction)
{
	// copy previous head elements' coords into new head element
	if (head > 0)
		snake[head] = snake[head - 1];
	else
		snake[head] = snake[snake.size() - 1];

	// modify new head elements' coords based on direction
	switch (direction)
	{
	case 'w':
		snake[head].Y -= 1;
		break;
	case 'a':
		snake[head].X -= 1;
		break;
	case 's':
		snake[head].Y += 1;
		break;
	case 'd':
		snake[head].X += 1;
	}
}

void checkWallContact(COORD& head, bool& gameOver, bool loopedField)
{
	if (loopedField)
	{
		if (head.Y < topEdge)
			head.Y = bottomEdge;
		else if (head.Y > bottomEdge)
			head.Y = topEdge;
		else if (head.X < leftEdge)
			head.X = rightEdge;
		else if (head.X > rightEdge)
			head.X = leftEdge;
	}
	else // touching the walls causes death
	{
		if (head.Y < topEdge)
			gameOver = true;
		else if (head.Y > bottomEdge)
			gameOver = true;
		else if (head.X < leftEdge)
			gameOver = true;
		else if (head.X > rightEdge)
			gameOver = true;
	}
}

COORD newTarget(vector<COORD> snake, HANDLE hStdOut)
{
	COORD target;
	bool invalidTarget;

	do
	{
		target = { leftEdge + rand() % (rightEdge - leftEdge + 1), topEdge + rand() % (bottomEdge - topEdge + 1) };
		
		// prevent the target from appearing inside the snake
		for (int i = 0; i < snake.size(); i++)
		{
			if (target.X == snake[i].X && target.Y == snake[i].Y)
			{
				invalidTarget = true;
				break;
			}
			else
				invalidTarget = false;
		}
	} while (invalidTarget);

	SetConsoleCursorPosition(hStdOut, target);
	return target;
}

void getInput(char& direction)
{
	if (_kbhit())
	{
		char input = tolower(_getch());

		if (input == 'w' && direction != 's' || input == 'a' && direction != 'd'
			|| input == 's' && direction != 'w' || input == 'd' && direction != 'a')
			direction = input;
	}
}