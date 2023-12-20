#include <iostream>
#include <chrono>
#include <ctime>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <stack>
#include <set>
#include <sstream>
using namespace std;
using namespace chrono;
using Duration = duration<double>;

const int N = 9;

enum Difficulty
{
    EASY = 15,
    MEDIUM = 25,
    HARD = 35
};

// Declaration of functions
void printBoardWithMistakes(const vector<vector<int>> &board, const vector<vector<int>> &solution, int row, int col);
void printBoard(const vector<vector<int>> &board);
void saveGame(const vector<vector<int>> &puzzle, const vector<vector<int>> &solution,
              const stack<vector<vector<int>>> &moveStack, const Duration &elapsedTime, Difficulty difficulty);
void loadGame(vector<vector<int>> &puzzle, vector<vector<int>> &solution,
              stack<vector<vector<int>>> &moveStack, Duration &elapsedTime, Difficulty &difficulty);
bool isSafe(const vector<vector<int>> &board, int row, int col, int num);
bool solveSudoku(vector<vector<int>> &board);
void generatePuzzle(vector<vector<int>> &puzzle, Difficulty difficulty);
bool isValidMove(const vector<vector<int>> &board, int row, int col, int num);
void provideHint(const vector<vector<int>> &puzzle, const vector<vector<int>> &solution);
set<pair<int, int>> checkMistakes(const vector<vector<int>> &board, const vector<vector<int>> &solution);
void loadLeaderboard(map<string, double> &leaderboard, Difficulty difficulty);
void saveLeaderboard(const map<string, double> &leaderboard, Difficulty difficulty);
void autoSolveSudoku(vector<vector<int>> &board);
string getDifficultyName(Difficulty difficulty);
string getDifficultyFileName(Difficulty difficulty);

// Function to print the Sudoku board
void printBoard(const vector<vector<int>> &board)
{
    for (int i = 0; i < N; i++)
    {
        if (i % 3 == 0 && i != 0)
            cout << "---------------------" << endl;

        for (int j = 0; j < N; j++)
        {
            if (j % 3 == 0 && j != 0)
                cout << "| ";

            if (board[i][j] == 0)
                cout << ". ";
            else
                cout << board[i][j] << " ";
        }
        cout << endl;
    }
}

// Function to print the Sudoku board with mistakes highlighted
void printBoardWithMistakes(const vector<vector<int>> &board, const vector<vector<int>> &solution, int row, int col)
{
    for (int i = 0; i < N; i++)
    {
        if (i % 3 == 0 && i != 0)
            cout << "---------------------" << endl;

        for (int j = 0; j < N; j++)
        {
            if (j % 3 == 0 && j != 0)
                cout << "| ";

            if (board[i][j] == 0)
                cout << ". ";
            else
            {
                if (i == row && j == col && board[i][j] != solution[i][j])
                    cout << "\033[1;31m" << board[i][j] << "\033[0m "; // Highlight the entered incorrect number in red
                else
                    cout << board[i][j] << " ";
            }
        }
        cout << endl;
    }
}
// Function to save the game state to a file
void saveGame(const vector<vector<int>> &puzzle, const vector<vector<int>> &solution,
              const stack<vector<vector<int>>> &moveStack, const Duration &elapsedTime, Difficulty difficulty)
{
    ofstream saveFile("saved_game.txt");

    if (!saveFile.is_open())
    {
        cout << "Error: Unable to save the game.\n";
        return;
    }

    if (saveFile.is_open())
    {
        // Serialize and save the puzzle
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                saveFile << puzzle[i][j] << " ";
            }
        }
        saveFile << endl;

        // Serialize and save the solution
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                saveFile << solution[i][j] << " ";
            }
        }
        saveFile << endl;

        // Serialize and save the move stack
        stack<vector<vector<int>>> tempStack = moveStack;
        while (!tempStack.empty())
        {
            vector<vector<int>> tempBoard = tempStack.top();
            tempStack.pop();
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < N; j++)
                {
                    saveFile << tempBoard[i][j] << " ";
                }
            }
            saveFile << endl;
        }

        // Save the elapsed time
        saveFile << elapsedTime.count() << endl;

        // Save the difficulty
        saveFile << difficulty << endl;

        saveFile.close();
        cout << "Game saved successfully.\n";
    }
    else
    {
        cout << "Unable to save the game.\n";
    }
}
// Function to load the game state from a file
void loadGame(vector<vector<int>> &puzzle, vector<vector<int>> &solution,
              stack<vector<vector<int>>> &moveStack, Duration &elapsedTime, Difficulty &difficulty)
{
    ifstream loadFile("saved_game.txt");
    if (loadFile.is_open())
    {
        // Deserialize and load the puzzle
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                loadFile >> puzzle[i][j];
            }
        }

        // Deserialize and load the solution
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                loadFile >> solution[i][j];
            }
        }

        // Deserialize and load the move stack
        int value;
        while (loadFile >> value)
        {
            vector<vector<int>> tempBoard(N, vector<int>(N, 0));
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < N; j++)
                {
                    tempBoard[i][j] = value;
                    loadFile >> value;
                }
            }
            moveStack.push(tempBoard);
        }

        // Load the elapsed time
        double elapsedTimeValue;
        loadFile >> elapsedTimeValue;
        elapsedTime = Duration(elapsedTimeValue);

        // Load the difficulty
        int difficultyValue;
        loadFile >> difficultyValue;
        difficulty = static_cast<Difficulty>(difficultyValue);

        loadFile.close();
        cout << "Game loaded successfully.\n";
        printBoard(puzzle);
    }
    else
    {
        cout << "No saved game found.\n";
        printBoard(puzzle);
    }
}
// Function to check if it's safe to place a number in a given position
bool isSafe(const vector<vector<int>> &board, int row, int col, int num)
{
    // Check if the number is already in the row or column
    for (int i = 0; i < N; i++)
    {
        if (board[row][i] == num || board[i][col] == num)
            return false;
    }

    // Check if the number is already in the 3x3 subgrid
    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (board[i + startRow][j + startCol] == num)
                return false;
        }
    }

    return true;
}

// Function to solve the Sudoku puzzle using backtracking
bool solveSudoku(vector<vector<int>> &board)
{
    int row, col;

    // Find an empty position
    bool foundEmpty = false;
    for (row = 0; row < N; row++)
    {
        for (col = 0; col < N; col++)
        {
            if (board[row][col] == 0)
            {
                foundEmpty = true;
                break;
            }
        }
        if (foundEmpty)
            break;
    }

    // If there are no empty positions, the puzzle is solved
    if (!foundEmpty)
        return true;

    // Try placing a number from 1 to 9 in the empty position
    for (int num = 1; num <= 9; num++)
    {
        if (isSafe(board, row, col, num))
        {
            // Place the number if it's safe
            board[row][col] = num;

            // Recursively try to solve the rest of the puzzle
            if (solveSudoku(board))
                return true;

            // If placing the number doesn't lead to a solution, backtrack
            board[row][col] = 0;
        }
    }

    // No number can be placed in this position
    return false;
}

// Function to generate a Sudoku puzzle
void generatePuzzle(vector<vector<int>> &puzzle, Difficulty difficulty)
{
    // Create a copy of the puzzle to use as the solution
    vector<vector<int>> solution = puzzle;

    // Solve the copied puzzle to get the solution
    solveSudoku(solution);

    // Copy the solution to the original puzzle
    puzzle = solution;

    // Remove numbers to create the puzzle
    vector<pair<int, int>> cells;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            cells.push_back(make_pair(i, j));
        }
    }

    random_shuffle(cells.begin(), cells.end());

    int filledCells;
    switch (difficulty)
    {
    case EASY:
        filledCells = 15;
        break;
    case MEDIUM:
        filledCells = 30;
        break;
    case HARD:
        filledCells = 40;
        break;
    default:
        filledCells = 30; // Default to medium difficulty
        break;
    }

    for (int i = 0; i < filledCells; i++)
    {
        int row = cells[i].first;
        int col = cells[i].second;
        puzzle[row][col] = 0;
    }
}

// Function to check if the player's move is valid
bool isValidMove(const vector<vector<int>> &board, int row, int col, int num)
{
    // Check if the move is within the board boundaries
    if (row < 0 || row >= N || col < 0 || col >= N)
        return false;

    // Check if the position is empty in the original puzzle
    if (board[row][col] != 0)
        return false;

    // Check if the number is valid in the current position
    return isSafe(board, row, col, num);
}

// Function to provide a hint for the next move
void provideHint(const vector<vector<int>> &puzzle, const vector<vector<int>> &solution)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (puzzle[i][j] == 0)
            {
                cout << "Hint: Place " << (solution[i][j]) << " at position (" << i + 1 << ", " << j + 1 << ").\n";
                return;
            }
        }
    }
    cout << "No hints available. The puzzle is already complete.\n";
}

// Function to check mistakes in the player's input
set<pair<int, int>> checkMistakes(const vector<vector<int>> &board, const vector<vector<int>> &solution)
{
    set<pair<int, int>> mistakes;

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (board[i][j] != 0 && board[i][j] != solution[i][j])
            {
                mistakes.insert({i, j});
            }
        }
    }

    return mistakes;
}

// Function to load leaderboard data from a file
void loadLeaderboard(map<string, double> &leaderboard, Difficulty difficulty)
{
    ifstream file(getDifficultyFileName(difficulty));
    if (file.is_open())
    {
        string playerName;
        double bestTime;

        while (file >> playerName >> bestTime)
        {
            leaderboard[playerName] = bestTime;
        }

        file.close();
    }
}

// Function to save leaderboard data to a file
void saveLeaderboard(const map<string, double> &leaderboard, Difficulty difficulty)
{
    ofstream file(getDifficultyFileName(difficulty));
    if (file.is_open())
    {
        for (const auto &entry : leaderboard)
        {
            file << entry.first << " " << entry.second << endl;
        }

        file.close();
    }
}

// Function to automatically solve the Sudoku puzzle
void autoSolveSudoku(vector<vector<int>> &board)
{
    // Check if the puzzle is solvable
    if (solveSudoku(board))
    {
        cout << "Auto-Solving...\n";
        printBoard(board);
    }
    else
    {
        cout << "The puzzle is not solvable.\n";
    }
}

// Function to get the filename for the leaderboard based on difficulty
string getDifficultyFileName(Difficulty difficulty)
{
    switch (difficulty)
    {
    case EASY:
        return "leaderboard_easy.txt";
    case MEDIUM:
        return "leaderboard_medium.txt";
    case HARD:
        return "leaderboard_hard.txt";
    default:
        return "leaderboard_default.txt";
    }
}

int main()
{
    map<string, double> leaderboard;      // Player name -> Best time
    Difficulty selectedDifficulty = EASY; // Default difficulty

    stack<vector<vector<int>>> moveStack; // Stack to store the game state after each move
    Duration elapsedTime;

    // Check if there is a saved game and ask the user if they want to resume

    auto start = system_clock::now(); // Start the timer
    vector<vector<int>> puzzle(N, vector<int>(N, 0));

    vector<vector<int>> solution = puzzle;
    solveSudoku(solution);
    char resumeChoice;
    cout << "Do you want to resume a saved game? (y/n): ";
    cin >> resumeChoice;
    bool loaded = false;
    if (tolower(resumeChoice) == 'y')
    {
        loadGame(puzzle, solution, moveStack, elapsedTime, selectedDifficulty);
        loaded = true;
    }
    while (true)
    {
        string playerName;
        if (!loaded)
        {
            cout << "Enter your name: ";
            cin >> playerName;

            cout << "Welcome to Sudoku, " << playerName << "!\n";
            cout << "Choose a difficulty level:\n";
            cout << "1. Easy\n2. Medium\n3. Hard\n";
            int difficultyChoice;
            cin >> difficultyChoice;

            switch (difficultyChoice)
            {
            case 1:
                selectedDifficulty = EASY;
                break;
            case 2:
                selectedDifficulty = MEDIUM;
                break;
            case 3:
                selectedDifficulty = HARD;
                break;
            default:
                selectedDifficulty = EASY;
                break;
            }
        }
        // Load leaderboard data from file
        loadLeaderboard(leaderboard, selectedDifficulty);
        if (!loaded)
        {
            generatePuzzle(puzzle, selectedDifficulty); // Adjust the difficulty level
            cout << "Fill in the empty cells. Enter row, column, and value (separated by spaces) to make a move.\n";
            printBoard(puzzle);
        }

        // stack<vector<vector<int>>> moveStack; // Stack to store the game state after each move

        while (true)
        {
            int row, col, num;
            cout << "Enter your move (row col value, -1 to undo, 0 for hint, -2 to auto-solve, -3 to save): ";
            cin >> row >> col >> num;
            if (num == -3)
            {
                // Save the game
                saveGame(puzzle, solution, moveStack, elapsedTime, selectedDifficulty);
            }
            else if (num == -1)
            {
                // Undo move
                if (!moveStack.empty())
                {
                    puzzle = moveStack.top();
                    moveStack.pop();
                    printBoard(puzzle);
                }
                else
                {
                    cout << "Cannot undo further.\n";
                }
            }
            else if (num == 0)
            {
                // Provide a hint
                provideHint(puzzle, solution);
            }
            else if (num == -2)
            {
                // Auto-Solve
                autoSolveSudoku(puzzle);
                break;
            }
            else if (isValidMove(puzzle, row - 1, col - 1, num))
            {
                moveStack.push(puzzle); // Store the current state in the stack
                puzzle[row - 1][col - 1] = num;
                printBoardWithMistakes(puzzle, solution, row - 1, col - 1);

                // Check if the player has completed the puzzle
                bool isPuzzleComplete = true;
                for (int i = 0; i < N; i++)
                {
                    for (int j = 0; j < N; j++)
                    {
                        if (puzzle[i][j] != solution[i][j])
                        {
                            isPuzzleComplete = false;
                            break;
                        }
                    }
                    if (!isPuzzleComplete)
                        break;
                }

                if (isPuzzleComplete)
                {
                    auto end = system_clock::now(); // Stop the timer
                    duration<double> elapsed_seconds = end - start;
                    cout << "Congratulations, " << playerName << "! You've solved the puzzle!\n";
                    cout << "Time taken: " << elapsed_seconds.count() << " seconds\n";

                    // Update the leaderboard
                    if (leaderboard.find(playerName) == leaderboard.end() || elapsed_seconds.count() < leaderboard[playerName])
                    {
                        leaderboard[playerName] = elapsed_seconds.count();
                    }

                    // Display the leaderboard
                    cout << "\nLeaderboard for " << getDifficultyName(selectedDifficulty) << ":\n";
                    for (const auto &entry : leaderboard)
                    {
                        cout << entry.first << ": " << entry.second << " seconds\n";
                    }

                    // Save leaderboard data to file
                    saveLeaderboard(leaderboard, selectedDifficulty);

                    break;
                }
            }
            else
            {
                cout << "Invalid move. Try again.\n";
            }
        }

        char playAgain;
        cout << "Do you want to play again? (y/n): ";
        cin >> playAgain;

        if (tolower(playAgain) != 'y')
            break;
    }

    return 0;
}

// Function to get the name of the difficulty level
string getDifficultyName(Difficulty difficulty)
{
    switch (difficulty)
    {
    case EASY:
        return "Easy";
    case MEDIUM:
        return "Medium";
    case HARD:
        return "Hard";
    default:
        return "Default";
    }
}