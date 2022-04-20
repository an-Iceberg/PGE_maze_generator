#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <stack>

class MazeGenerator : public olc::PixelGameEngine
{
	public:
		MazeGenerator()
		{
			sAppName = "Maze generator";
		}

	private:
		enum Direction
		{
			NOT_SET,
			NORTH,
			WEST,
			SOUTH,
			EAST
		};

		// Maze width in maze cells
		int i_MazeWidth;

		// Maze height in maze cells
		int i_MazeHeight;

		// Path width in pixels
		int i_PathWidth;

		// Vector of directions
		std::vector<Direction> v_Maze;

		int i_VisitedCells;

		// Contains all maze cells who's direction has not yet been set
		std::stack<std::pair<int, int>> i_Stack;

	public:
		bool OnUserCreate() override
		{
			// Initializing the random number generator
			srand(time(nullptr));

			// TODO OPTIONAL: make maze width and height user adjustable
			i_MazeHeight = 50;
			i_MazeWidth = 50;

			// Set all the maze cells to have no direction
			for (int i = 0; i < i_MazeWidth * i_MazeHeight; i++)
			{
				v_Maze.push_back(NOT_SET);
			}

			i_Stack.push(std::make_pair(0, 0));

			i_VisitedCells = 1;

			i_PathWidth = 3;

			return true;
		}

		// TODO: user adjustable maze drawing speed delay
		bool OnUserUpdate(float fElapsedTime) override
		{
			// Generate new maze when ENTER key is pressed
			if (GetKey(olc::Key::ENTER).bPressed)
			{
				i_VisitedCells = 1;

				// Setting all maze cell's directions to NOT_SET
				for (int element = 0; element < v_Maze.size(); element++)
				{
					v_Maze[element] = NOT_SET;
				}

				// The top leftmost cell is going to be the starting point for the maze
				i_Stack.push(std::make_pair(0, 0));
			}

			// As long as there are unvisited cells
			if (i_VisitedCells < i_MazeWidth * i_MazeHeight)
			{
				std::vector<int> v_ValidNeighbours;

				// Checks if neighbours exist and if their direction has been set
				CheckForValidNeighbours(v_ValidNeighbours);

				// If there are neighbours who's direction is not yet set
				if (!v_ValidNeighbours.empty())
				{
				  // Choosing a random neighbour from all valid neighbours
					int i_NextCellDirection = v_ValidNeighbours[rand() % v_ValidNeighbours.size()];

					switch (i_NextCellDirection)
					{
						// North
						case 1:
							ChangeDirectionAndPush(0, -1, NORTH);
						break;

						// West
						case 2:
							ChangeDirectionAndPush(-1, 0, WEST);
						break;

						// South
						case 3:
							ChangeDirectionAndPush(0, 1, SOUTH);
						break;

						// East
						case 4:
							ChangeDirectionAndPush(1, 0, EAST);
						break;
					}

					i_VisitedCells++;
				}
				// The cell who has been visited/who's direction has been set can be removed from the stack
				else
				{
					i_Stack.pop();
				}
			}

			DrawingRoutine();

			return true;
		}

		/**
		 * @brief Draws the maze to the screen
		 */
		void DrawingRoutine()
		{
			// Drawing the maze
			Clear(olc::VERY_DARK_BLUE);

			// Draws cell positions
			for (int x = 0; x < i_MazeWidth; x++)
			{
				for (int y = 0; y < i_MazeHeight; y++)
				{

					// Fills in cell positions with cell interior and walls
					for (int py = 0; py < i_PathWidth; py++)
					{
						for (int px = 0; px < i_PathWidth; px++)
						{
							int i_XPosition = x * (i_PathWidth + 1) + px;
							int i_YPosition = y * (i_PathWidth + 1) + py;

							// Cell has a direction
							if (v_Maze[y * i_MazeWidth + x] != NOT_SET)
							{
								Draw(i_XPosition, i_YPosition);
							}
							// Cell does not have a direction
							else
							{
								Draw(i_XPosition, i_YPosition, olc::BLUE);
							}
						}
					}

					// Connects cells by overriding the walls
					for (int p = 0; p < i_PathWidth; p++)
					{
						// The direction of the current maze cell
						Direction direction = v_Maze[y * i_MazeWidth + x];

						if (direction == SOUTH)
						{
							Draw(x * (i_PathWidth + 1) + p, y * (i_PathWidth + 1) + i_PathWidth);
						}

						if (direction == EAST)
						{
							Draw(x * (i_PathWidth + 1) + i_PathWidth, y * (i_PathWidth + 1) + p);
						}

						if (direction == WEST)
						{
							Draw(x * (i_PathWidth + 1) - 1, y * (i_PathWidth + 1) + p);
						}

						if (direction == NORTH)
						{
							Draw(x * (i_PathWidth + 1) + p, y * (i_PathWidth + 1) - 1);
						}
					}
				}
			}

			// Drawing the top of the stack
			for (int py = 0; py < i_PathWidth; py++)
			{
				for (int px = 0; px < i_PathWidth; px++)
				{
					Draw(i_Stack.top().first * (i_PathWidth + 1) + px, i_Stack.top().second * (i_PathWidth + 1) + py, olc::GREEN);
				}
			}

		}

		/**
		 * @brief Returns an array of valid directions to choose from.
		 * Accounts for the edge-case of maze cells on the literal edge of the maze
		 * returning directions that would lead outside the maze.
		 *
		 * @param neighbours Vector with all valid directions to choose from
		 */
		void CheckForValidNeighbours(std::vector<int>& neighbours)
		{
			// Check for valid northern neighbour
			if (i_Stack.top().second > 0)
			{
				CheckNeighbour(0, -1, NORTH, neighbours);
			}

			// Check for valid western neighbour
			if (i_Stack.top().first > 0)
			{
				CheckNeighbour(-1, 0, WEST, neighbours);
			}

			// Check for valid southern neighbour
			if (i_Stack.top().second < i_MazeWidth - 1)
			{
				CheckNeighbour(0, 1, SOUTH, neighbours);
			}

			// Check for valid eastern neighbour
			if (i_Stack.top().first < i_MazeHeight - 1)
			{
				CheckNeighbour(1, 0, EAST, neighbours);
			}
		}

		/**
		 * @brief If the neighbour's direction is not set, push the 'direction' parameter onto the 'neighbours' vector.
		 *
		 * @param x North/south neighbour indicator
		 * @param y East/west neighbour indicator
		 * @param direction The direction the neighbour is currently facing
		 * @param neighbours Vector with all valid directions to choose from
		 */
		void CheckNeighbour(int x, int y, int direction, std::vector<int>& neighbours)
		{
			if (v_Maze[Position(x, y)] == NOT_SET)
			{
				neighbours.push_back(direction);
			}
		}

		// ? function is still not entirely clear
		// ... and pushes a new pair onto the stack
		// Only changes direction values if they were NOT_SET before
		void ChangeDirectionAndPush(int x, int y, Direction direction)
		{
			// ? why do we check for the top element?
			if(v_Maze[Position(0, 0)] == NOT_SET)
			{
				v_Maze[Position(0, 0)] = direction;
			}

			if (v_Maze[Position(x, y)] == NOT_SET)
			{
				// TODO: account for values other than 1, 2, 3 or 4
				switch (direction)
				{
					case 1:
						v_Maze[Position(x, y)] = SOUTH;
					break;

					case 2:
						v_Maze[Position(x, y)] = EAST;
					break;

					case 3:
						v_Maze[Position(x, y)] = NORTH;
					break;

					case 4:
						v_Maze[Position(x, y)] = WEST;
					break;
				}
			}

			// ? why do we only push the coordinates onto the stack?
			i_Stack.push(std::make_pair(i_Stack.top().first + x, i_Stack.top().second + y));
		}

		// Returns the position of a cell in v_Maze (basically converts a pair to a single integer)
		int Position(int x, int y)
		{
			return (i_Stack.top().second + y) * i_MazeWidth + (i_Stack.top().first + x);
		}
};

int main()
{
	MazeGenerator instance;
	if (instance.Construct(200, 200, 4, 4))
		instance.Start();
	return 0;
}
