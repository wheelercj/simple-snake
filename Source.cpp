#include <Windows.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <thread>
using namespace std;

void showConsoleCursor(bool);
void printWalls(HANDLE);
void newSnakeCoords(vector<COORD>&, int);
void checkWallContact(COORD&, bool);
COORD newTarget(vector<COORD>, HANDLE);
void getInput();

// field dimensions
const int leftEdge = 29;
const int rightEdge = 72;
const int topEdge = 5;
const int bottomEdge = 19;

char snakeDirection = 'd';
bool gameOver = false;

int main()
{
	const bool deadlyWalls = false;
	const int snakeSpeed = 40;     // milliseconds of pause per snake movement (a greater number makes the snake slower). Recommended value: 40
	const int snakeGrowthRate = 4; // per target hit. Recommended value: 4

	const char scales = 178; // the ascii char for the snake's segments
	const char food = 254;   // the ascii char for the snake's food
	showConsoleCursor(false);
	const short snakeStartX = (leftEdge + rightEdge) / 2, // snake starting coordinates
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
		// continuously check for input from the user in a separate thread
		thread userInput(getInput);

		int score = 0,
			head = 0;

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

			HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if (hStdOut == INVALID_HANDLE_VALUE)
				return 0;

			// erase the oldest snake segment
			SetConsoleCursorPosition(hStdOut, snake[(head + 1) % snake.size()]);
			cout << ' ';

			newSnakeCoords(snake, head);
			checkWallContact(snake[head], deadlyWalls);
			if (gameOver)
				userInput.join();

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
					userInput.join();
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

		gameOver = false;
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

void newSnakeCoords(vector<COORD>& snake, int head)
{
	// copy previous head elements' coords into new head element
	if (head > 0)
		snake[head] = snake[head - 1];
	else
		snake[head] = snake[snake.size() - 1];

	// modify new head elements' coords based on direction
	switch (snakeDirection)
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

void checkWallContact(COORD& head, bool deadlyWalls)
{
	if (deadlyWalls)
	{
		if (head.Y < topEdge || head.Y > bottomEdge	|| head.X < leftEdge || head.X > rightEdge)
			gameOver = true;
	}
	else
	{
		// loop the field
		if (head.Y < topEdge)
			head.Y = bottomEdge;
		else if (head.Y > bottomEdge)
			head.Y = topEdge;
		else if (head.X < leftEdge)
			head.X = rightEdge;
		else if (head.X > rightEdge)
			head.X = leftEdge;
	}
}

COORD newTarget(vector<COORD> snake, HANDLE hStdOut)
{
	COORD target;
	bool invalidTarget;

	do
	{
		target = { leftEdge + rand() % (rightEdge - leftEdge + 1),
			topEdge + rand() % (bottomEdge - topEdge + 1) };
		
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

void getInput()
{
	while (!gameOver)
	{
		if (_kbhit())
		{
			char input = tolower(_getch());

			if (input == 'w' && snakeDirection != 's' || input == 'a' && snakeDirection != 'd'
				|| input == 's' && snakeDirection != 'w' || input == 'd' && snakeDirection != 'a')
				snakeDirection = input;
		}

		// reduce CPU usage down from 100% for this thread
		this_thread::sleep_for(25ms);
	}
}
