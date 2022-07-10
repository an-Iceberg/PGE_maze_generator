#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <stack>

enum Direction
{
  NOT_SET,
  UP,
  LEFT,
  DOWN,
  RIGHT
};

struct cell {
  bool hasBeenPainted;
  Direction direction;

public:
  cell()
  {
    hasBeenPainted = false;
    direction = NOT_SET;
  }

  cell(bool hasBeenPainted, Direction direction)
  {
    this->hasBeenPainted = hasBeenPainted;
    this->direction = direction;
  }
};

class MazeGenerator : public olc::PixelGameEngine
{
public:
  MazeGenerator()
  {
    sAppName = "Maze generator";
  }

private:

  int mazeWidth; // Maze width in maze cells
  int mazeHeight; // Maze height in maze cells
  int cellCount; // Number of cells in the maze
  int pathWidth; // Path width in pixels
  std::vector<cell> mazeDirections; // Vector containing all cells and their data/information
  // ? why is this necessary?
  int visitedCellsCounter; // Number of cells that has been visited
  std::stack<olc::vi2d> unvisitedCells; // Contains all maze cells (as coordinates) who's direction has not yet been set

public:
  bool OnUserCreate() override
  {
    // Initializing the random number generator
    srand(time(nullptr));

    mazeHeight = 50;
    mazeWidth = 50;
    cellCount = mazeWidth * mazeHeight;

    // Set all the maze cells to have no direction
    for (int i = 0; i < cellCount; i++)
    {
      mazeDirections.push_back(cell());
    }

    unvisitedCells.push(olc::vi2d{0, 0});

    visitedCellsCounter = 1;

    pathWidth = 3;

    return true;
  }

  // TODO: user adjustable maze drawing speed delay
  bool OnUserUpdate(float fElapsedTime) override
  {
    // Generate new maze when ENTER key is pressed
    if (GetKey(olc::Key::ENTER).bPressed)
    {
      Clear(olc::BLACK);

      visitedCellsCounter = 1;

      // Resetting all maze data
      for (std::vector<cell>::iterator cell = mazeDirections.begin(); cell < mazeDirections.end(); cell++)
      {
        cell->direction = NOT_SET;
        cell->hasBeenPainted = false;
      }

      // The top leftmost cell is going to be the starting point for the maze
      unvisitedCells.push(olc::vi2d{0, 0});
    }

    // As long as there are unvisited cells, update the maze
    if (visitedCellsCounter < cellCount)
    {
      std::vector<Direction> validNeighbours;

      // Checks if neighbours exist and if their direction has been set
      addAllValidNeighbours(validNeighbours);

      // If there are any valid neighbours choose a random one
      if (!validNeighbours.empty())
      {
        // Chooses a random neighbour from all valid neighbours
        Direction nextCellDirection = validNeighbours[rand() % validNeighbours.size()];

        cell& currentCell = mazeDirections[IndexOfCurrentCell()];

        // Set the current cell's direction to point towards the selected neighbour
        currentCell.direction = nextCellDirection;
        currentCell.hasBeenPainted = false;

        // Push the selected cell onto the stack
        unvisitedCells.push(CoordinatesOfNeighbour(nextCellDirection));

        visitedCellsCounter++;
      }
      // There are no valid neighbours so we need to back-track until we find some valid ones
      else
      {
        // Setting the enpoint cell's direction to point to its previous cell on the stack
        // While backtracking we reverse all the directions that have been set (dunno why but it seems to work)
        cell& previousCell = mazeDirections[IndexOfCurrentCell()];

        unvisitedCells.pop();

        switch (mazeDirections[IndexOfCurrentCell()].direction)
        {
          case UP:
            previousCell.direction = DOWN;
          break;

          case LEFT:
            previousCell.direction = RIGHT;
          break;

          case DOWN:
            previousCell.direction = UP;
          break;

          case RIGHT:
            previousCell.direction = LEFT;
          break;
        }

        previousCell.hasBeenPainted = false;
      }

      PaintingRoutine();
    }

    // TODO: paint the top of the stack
    // Draws the top of the stack
    // olc::vi2d topOfTheStack = {unvisitedCells.top().x, unvisitedCells.top().y};
    // for (int py = 0; py < pathWidth; py++)
    // {
    //   for (int px = 0; px < pathWidth; px++)
    //   {
    //     Draw(unvisitedCells.top().x * (pathWidth + 1) + px + 1, unvisitedCells.top().y * (pathWidth + 1) + py + 1, olc::GREEN);
    //   }
    // }

    // TODO: fill in the last cell and don't paint the top of the stack anymore

    // TODO: add user adjustable delay

    return true;
  }


  // -----


  /**
   * @brief Draws the maze to the screen
   */
  void PaintingRoutine()
  {
    // Clear(olc::BLACK);

    // Draws each cell
    for (std::vector<cell>::iterator cell = mazeDirections.begin(); cell < mazeDirections.end(); cell++)
    {
      int currentCellIndex = std::distance(mazeDirections.begin(), cell);

      // Calculating the x and y coordinates of the current cell
      // x = index % height
      // y = index / height
      olc::vi2d currentCell = {currentCellIndex % mazeHeight, currentCellIndex / mazeHeight};

      // Painting the cell only if it hasn't been painted before
      if (!cell->hasBeenPainted)
      {
        // Paints the cell
        for (int i = 0; i < (cell->direction == NOT_SET ? pathWidth + 1 : pathWidth); i++)
        {
          // Paints the cell interior

          // Diagonal
          Draw(currentCell.x + (currentCell.x * pathWidth + i) + 1, currentCell.y + (currentCell.y * pathWidth + i) + 1, (cell->direction == NOT_SET ? olc::BLUE : olc::WHITE));

          // Bottom triangle
          Draw(currentCell.x + (currentCell.x * pathWidth + 0) + 1, currentCell.y + (currentCell.y * pathWidth + 1) + 1, (cell->direction == NOT_SET ? olc::BLUE : olc::WHITE));
          Draw(currentCell.x + (currentCell.x * pathWidth + 0) + 1, currentCell.y + (currentCell.y * pathWidth + 2) + 1, (cell->direction == NOT_SET ? olc::BLUE : olc::WHITE));
          Draw(currentCell.x + (currentCell.x * pathWidth + 1) + 1, currentCell.y + (currentCell.y * pathWidth + 2) + 1, (cell->direction == NOT_SET ? olc::BLUE : olc::WHITE));

          // Top triangle
          Draw(currentCell.x + (currentCell.x * pathWidth + 1) + 1, currentCell.y + (currentCell.y * pathWidth + 0) + 1, (cell->direction == NOT_SET ? olc::BLUE : olc::WHITE));
          Draw(currentCell.x + (currentCell.x * pathWidth + 2) + 1, currentCell.y + (currentCell.y * pathWidth + 0) + 1, (cell->direction == NOT_SET ? olc::BLUE : olc::WHITE));
          Draw(currentCell.x + (currentCell.x * pathWidth + 2) + 1, currentCell.y + (currentCell.y * pathWidth + 1) + 1, (cell->direction == NOT_SET ? olc::BLUE : olc::WHITE));

          // Paints the respective wall
          switch (cell->direction)
          {
            case NOT_SET:
              Draw(currentCell.x + (currentCell.x * pathWidth + pathWidth) + 1, currentCell.y + (currentCell.y * pathWidth + i) + 1, olc::BLACK);
              Draw(currentCell.x + (currentCell.x * pathWidth + i) + 1, currentCell.y + (currentCell.y * pathWidth + pathWidth) + 1, olc::BLACK);
            break;

            case UP:
              Draw(currentCell.x + (currentCell.x * pathWidth + i) + 1, currentCell.y + (currentCell.y * pathWidth - 1) + 1);
            break;

            case LEFT:
              Draw(currentCell.x + (currentCell.x * pathWidth - 1) + 1, currentCell.y + (currentCell.y * pathWidth + i) + 1);
            break;

            case DOWN:
              Draw(currentCell.x + (currentCell.x * pathWidth + i) + 1, currentCell.y + (currentCell.y * pathWidth + pathWidth) + 1);
            break;

            case RIGHT:
              Draw(currentCell.x + (currentCell.x * pathWidth + pathWidth) + 1, currentCell.y + (currentCell.y * pathWidth + i) + 1);
            break;
          }
        }

        cell->hasBeenPainted = true;
      }
    }
  }

  /**
   * @brief Returns an vector of valid directions to choose from.
   * Maze cells outside the edge of the maze are not added to the vector
   *
   * @param neighbours Vector with all valid directions to choose from
   */
  void addAllValidNeighbours(std::vector<Direction>& neighbours)
  {
    // Check if upper neighbour exists
    if (unvisitedCells.top().y > 0)
    {
      // If the upper neightbour's direction is not set add it as a valid neighbour
      if (mazeDirections[IndexOfNeighbour(Direction{UP})].direction == NOT_SET)
      {
        neighbours.push_back(Direction{UP});
      }
    }

    // Check if left neighbour exists
    if (unvisitedCells.top().x > 0)
    {
      // If the left neightbour's direction is not set add it as a valid neighbour
      if (mazeDirections[IndexOfNeighbour(Direction{LEFT})].direction == NOT_SET)
      {
        neighbours.push_back(Direction{LEFT});
      }
    }

    // Check if lower neighbour exists
    if (unvisitedCells.top().y < mazeWidth - 1)
    {
      // If the lower neightbour's direction is not set add it as a valid neighbour
      if (mazeDirections[IndexOfNeighbour(Direction{DOWN})].direction == NOT_SET)
      {
        neighbours.push_back(Direction{DOWN});
      }
    }

    // Check if right neighbour exists
    if (unvisitedCells.top().x < mazeHeight - 1)
    {
      // If the right neightbour's direction is not set add it as a valid neighbour
      if (mazeDirections[IndexOfNeighbour(Direction{RIGHT})].direction == NOT_SET)
      {
        neighbours.push_back(Direction{RIGHT});
      }
    }
  }

  // Returns the index of a cell's neighbour in mazeDirections
  int IndexOfNeighbour(olc::vi2d direction)
  {
    return (unvisitedCells.top().y + direction.y) * mazeWidth + (unvisitedCells.top().x + direction.x);
  }

  // Returns the index of a cell's neighbour in mazeDirections or the current cell's
  int IndexOfNeighbour(Direction direction)
  {
    int index;

    switch (direction)
    {
      case NOT_SET:
        index = (unvisitedCells.top().y) * mazeWidth + (unvisitedCells.top().x);
      break;

      case UP:
        index = (unvisitedCells.top().y - 1) * mazeWidth + (unvisitedCells.top().x);
      break;

      case LEFT:
        index = (unvisitedCells.top().y) * mazeWidth + (unvisitedCells.top().x - 1);
      break;

      case DOWN:
        index = (unvisitedCells.top().y + 1) * mazeWidth + (unvisitedCells.top().x);
      break;

      case RIGHT:
        index = (unvisitedCells.top().y) * mazeWidth + (unvisitedCells.top().x + 1);
      break;
    }

    return index;
  }

  // Returns the index of the element on the top of the stack
  int IndexOfCurrentCell()
  {
    return unvisitedCells.top().y * mazeWidth + unvisitedCells.top().x;
  }

  // Returns the coordinates of a cell neighbouring the current cell (top of the stack)
  olc::vi2d CoordinatesOfNeighbour(Direction direction)
  {
    olc::vi2d neighbour;

    switch (direction)
    {
      case NOT_SET:
        neighbour.x = unvisitedCells.top().x;
        neighbour.y = unvisitedCells.top().y;
      break;

      case UP:
        neighbour.x = unvisitedCells.top().x;
        neighbour.y = unvisitedCells.top().y - 1;
      break;

      case LEFT:
        neighbour.x = unvisitedCells.top().x - 1;
        neighbour.y = unvisitedCells.top().y;
      break;

      case DOWN:
        neighbour.x = unvisitedCells.top().x;
        neighbour.y = unvisitedCells.top().y + 1;
      break;

      case RIGHT:
        neighbour.x = unvisitedCells.top().x + 1;
        neighbour.y = unvisitedCells.top().y;
      break;
    }

    return neighbour;
  }

  // Returns the coordinates of the current cell (top of stack)
  olc::vi2d CoordinatesOfCurrentCell()
  {
    return olc::vi2d{unvisitedCells.top().x, unvisitedCells.top().y};
  }
};

int main()
{
  MazeGenerator instance;

  if (instance.Construct(201, 201, 4, 4))
  {
    instance.Start();
  }

  return 0;
}
