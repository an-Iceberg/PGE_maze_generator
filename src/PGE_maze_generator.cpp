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
  std::vector<cell> maze; // Vector containing all cells and their data/information
  // TODO: maybe we can do without visitedCellsCounter
  int visitedCellsCounter; // Number of cells that has been visited
  std::stack<olc::vi2d> unvisitedCells; // Contains all maze cells (as coordinates) who's direction has not yet been set
  float delay; // Delay in seconds
  float timePassed;
  int UISectionHeight;

public:
  bool OnUserCreate() override
  {
    // Initializing the random number generator
    srand(time(nullptr));

    mazeHeight = 50;
    mazeWidth = 50;
    cellCount = mazeWidth * mazeHeight;
    pathWidth = 3;
    visitedCellsCounter = cellCount + 1;
    delay = 0.01f;
    UISectionHeight = 20;

    // Set all the maze cells to have no direction
    for (int i = 0; i < cellCount; i++)
    {
      maze.push_back(cell());
    }

    Clear(olc::BLACK);

    // Drawing the UI section
    DrawRect(0, 0, ScreenWidth() - 1, UISectionHeight - 1, olc::CYAN);
    DrawString(1, 2, "[ENTER]", olc::MAGENTA);
    DrawString(57, 2, ": new maze", olc::GREY);
    DrawString(1, 10, "adjust delay:", olc::GREY);
    DrawString(113, 10, "<- ", olc::MAGENTA);
    DrawString(137, 10, std::to_string(delay).substr(3, 2) + "ms", olc::GREY);
    DrawString(169, 10, " ->", olc::MAGENTA);

    PaintingRoutine();

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    // Decreasing delay
    if (GetKey(olc::Key::LEFT).bPressed)
    {
      delay -= 0.001f;

      // Accounting for negative overflow
      if (delay < 0.000f)
      {
        delay = 0.000f;
      }

      // Re-painting the number in the UI section
      FillRect(137, 10, 31, 7, olc::BLACK);
      DrawString(137, 10, std::to_string(delay).substr(3, 2) + "ms", olc::GREY);
    }

    // Increasing delay
    if (GetKey(olc::Key::RIGHT).bPressed)
    {
      delay += 0.001f;

      // The biggest delay shall be 10 millisecond
      if (delay > 0.01f)
      {
        delay = 0.01f;
      }

      // Re-painting the number in the UI section
      FillRect(137, 10, 31, 7, olc::BLACK);
      DrawString(137, 10, std::to_string(delay).substr(3, 2) + "ms", olc::GREY);
    }

    // Generate new maze when ENTER key is pressed
    if (GetKey(olc::Key::ENTER).bPressed || GetMouse(0).bPressed)
    {
      FillRect(0, UISectionHeight, ScreenWidth(), ScreenHeight(), olc::BLACK);

      visitedCellsCounter = 1;

      // Resetting all maze data
      for (std::vector<cell>::iterator cell = maze.begin(); cell < maze.end(); cell++)
      {
        cell->direction = NOT_SET;
        cell->hasBeenPainted = false;
      }

      // The top leftmost cell is going to be the starting point for the maze
      unvisitedCells.push(olc::vi2d{0, 0});
    }

    timePassed += fElapsedTime;

    // Only draw after a certain delay time has been reached
    if (timePassed > delay)
    {
      // Reset delay timer
      timePassed = 0;

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

          cell& currentCell = maze[IndexOfCurrentCell()];

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
          cell& previousCell = maze[IndexOfCurrentCell()];

          unvisitedCells.pop();

          switch (maze[IndexOfCurrentCell()].direction)
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
    }

    return true;
  }


  // -----


private:
  // Draws the maze to the screen
  void PaintingRoutine()
  {
    // Draws each cell
    for (std::vector<cell>::iterator cell = maze.begin(); cell < maze.end(); cell++)
    {
      int currentCellIndex = std::distance(maze.begin(), cell);

      // Calculating the x and y coordinates of the current cell
      // x = index % height
      // y = index / height
      olc::vi2d currentCell = {currentCellIndex % mazeHeight, currentCellIndex / mazeHeight};

      // Painting the cell only if it hasn't been painted before
      if (!cell->hasBeenPainted)
      {
        olc::Pixel interiorColor;

        // Paints the cell interior
        if (cell->direction == NOT_SET)
        {
          interiorColor = olc::BLUE;
        }
        else
        {
          interiorColor = olc::WHITE;
        }

        paintCellInterior(currentCell, interiorColor);
        paintCellWall(currentCell, cell->direction);

        cell->hasBeenPainted = true;
      }

      // If this cell is the top of the stack
      // HACK: the -1 correction on the x is necessary for some reason
      currentCell.x--;

      if (!unvisitedCells.empty() && currentCell.x == unvisitedCells.top().x && currentCell.y == unvisitedCells.top().y)
      {
        // On the last painting cycle the top of the stack is painted as a regular cell
        if (visitedCellsCounter == cellCount)
        {
          paintCellInterior(currentCell, olc::WHITE);

          continue;
        }

        paintCellInterior(currentCell, olc::GREEN);

        maze[currentCellIndex].hasBeenPainted = false;
      }
    }
  }

  // Paints the interior of a cell
  void paintCellInterior(const olc::vi2d& currentCell, const olc::Pixel& interiorColor)
  {
    // Bottom triangle
    Draw(currentCell.x + (currentCell.x * pathWidth + 0) + 1, currentCell.y + (currentCell.y * pathWidth + 1) + 1 + UISectionHeight, interiorColor);
    Draw(currentCell.x + (currentCell.x * pathWidth + 0) + 1, currentCell.y + (currentCell.y * pathWidth + 2) + 1 + UISectionHeight, interiorColor);
    Draw(currentCell.x + (currentCell.x * pathWidth + 1) + 1, currentCell.y + (currentCell.y * pathWidth + 2) + 1 + UISectionHeight, interiorColor);

    // Top triangle
    Draw(currentCell.x + (currentCell.x * pathWidth + 1) + 1, currentCell.y + (currentCell.y * pathWidth + 0) + 1 + UISectionHeight, interiorColor);
    Draw(currentCell.x + (currentCell.x * pathWidth + 2) + 1, currentCell.y + (currentCell.y * pathWidth + 0) + 1 + UISectionHeight, interiorColor);
    Draw(currentCell.x + (currentCell.x * pathWidth + 2) + 1, currentCell.y + (currentCell.y * pathWidth + 1) + 1 + UISectionHeight, interiorColor);

    // Paints the cell diagonal
    for (int i = 0; i < pathWidth; i++)
    {
      Draw(currentCell.x + (currentCell.x * pathWidth + i) + 1, currentCell.y + (currentCell.y * pathWidth + i) + 1 + UISectionHeight, interiorColor);
    }
  }

  // Paints the walls of a cell
  void paintCellWall(const olc::vi2d& currentCell, const Direction& direction)
  {
    for (int i = 0; i < (direction == NOT_SET ? pathWidth + 1 : pathWidth); i++)
    {
      switch (direction)
      {
        case NOT_SET:
          Draw(currentCell.x + (currentCell.x * pathWidth + pathWidth) + 1, currentCell.y + (currentCell.y * pathWidth + i) + 1 + UISectionHeight, olc::BLACK);
          Draw(currentCell.x + (currentCell.x * pathWidth + i) + 1, currentCell.y + (currentCell.y * pathWidth + pathWidth) + 1 + UISectionHeight, olc::BLACK);
        break;

        case UP:
          Draw(currentCell.x + (currentCell.x * pathWidth + i) + 1, currentCell.y + (currentCell.y * pathWidth - 1) + 1 + UISectionHeight);
        break;

        case LEFT:
          Draw(currentCell.x + (currentCell.x * pathWidth - 1) + 1, currentCell.y + (currentCell.y * pathWidth + i) + 1 + UISectionHeight);
        break;

        case DOWN:
          Draw(currentCell.x + (currentCell.x * pathWidth + i) + 1, currentCell.y + (currentCell.y * pathWidth + pathWidth) + 1 + UISectionHeight);
        break;

        case RIGHT:
          Draw(currentCell.x + (currentCell.x * pathWidth + pathWidth) + 1, currentCell.y + (currentCell.y * pathWidth + i) + 1 + UISectionHeight);
        break;
      }
    }
  }

  // Returns an vector of valid directions to choose from.
  // Maze cells outside the edge of the maze are not added to the vector
  void addAllValidNeighbours(std::vector<Direction>& neighbours)
  {
    // If the upper neighbour exists and is not set, add it as a valid neighbour
    if (unvisitedCells.top().y > 0 && maze[IndexOfNeighbour(Direction{UP})].direction == NOT_SET)
    {
      neighbours.push_back(Direction{UP});
    }

    // If the left neighbour exists and is not set, add it as a valid neighbour
    if (unvisitedCells.top().x > 0 && maze[IndexOfNeighbour(Direction{LEFT})].direction == NOT_SET)
    {
      neighbours.push_back(Direction{LEFT});
    }

    // If the lower neighbour exists and is not set, add it as a valid neighbour
    if (unvisitedCells.top().y < mazeWidth - 1 && maze[IndexOfNeighbour(Direction{DOWN})].direction == NOT_SET)
    {
      neighbours.push_back(Direction{DOWN});
    }

    // If the right neighbour exists and is not set, add it as a valid neighbour
    if (unvisitedCells.top().x < mazeHeight - 1 && maze[IndexOfNeighbour(Direction{RIGHT})].direction == NOT_SET)
    {
      neighbours.push_back(Direction{RIGHT});
    }
  }

  // Returns the index of a cell's neighbour in maze
  int IndexOfNeighbour(olc::vi2d direction)
  {
    return (unvisitedCells.top().y + direction.y) * mazeWidth + (unvisitedCells.top().x + direction.x);
  }

  // Returns the index of a cell's neighbour in maze or the current cell's
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

  if (instance.Construct(201, 221, 4, 4))
  {
    instance.Start();
  }

  return 0;
}