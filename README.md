# Lazarus

Lazarus is a C++, bitboard-based chess engine with neural network and other cool stuff.

The origin of the name comes from the history of the project's creation and the multiple, deep refactorings it has experienced.

## In development
Unfortunately, this is not a final version of the project. 
Despite completing most of the key stages and bringing the project to a usable state, there are still some significant improvements and functionalities missing, without which the project cannot be considered ready.
For this reason, the engine is not yet ready for final testing and measurement of its playing strength.

Among the milestones already completed, the most important ones can be highlighted:
- Board and pieces representation&nbsp; <img src="md/check.png" alt="Check icon" width="20" height="20">
- Sliding piece attacks, magic bitboards&nbsp; <img src="md/check.png" alt="Check icon" width="20" height="20">
- Full game rules implementation - enpassant, pins, checks and promotions&nbsp; <img src="md/check.png" alt="Check icon" width="20" height="20">
- Move generation algorithms&nbsp; <img src="md/check.png" alt="Check icon" width="20" height="20">
- Transposition table&nbsp; <img src="md/check.png" alt="Check icon" width="20" height="20">
- Basic search&nbsp; <img src="md/check.png" alt="Check icon" width="20" height="20">
- Quiescence search&nbsp; <img src="md/check.png" alt="Check icon" width="20" height="20">
- Selectivity heuristics&nbsp; <img src="md/check.png" alt="Check icon" width="20" height="20">
- Neural network model (NNUE) implementation and training&nbsp; <img src="md/check.png" alt="Check icon" width="20" height="20">

The most important missing functionalities are:
- Multi-threaded search&nbsp; <img src="md/delete.png" alt="Delete icon" width="20" height="20">
- Opening book & endgame tablebase connection&nbsp; <img src="md/delete.png" alt="Delete icon" width="20" height="20">
- Time management&nbsp; <img src="md/delete.png" alt="Delete icon" width="20" height="20">
- UCI and connection with custom GUI&nbsp; <img src="md/delete.png" alt="Delete icon" width="20" height="20">

## Build and run
To build and run the project, you need to have CMake and a C++ compiler installed. The project supports two build modes:

1. **CLI (Command-Line Interface)**: The engine runs in the terminal without a GUI.
2. **GUI (Graphical User Interface)**: The engine is linked with SFML and includes a graphical interface (requires the `USE_GUI` flag).

Additionally, you can enable **development mode** with the `DEV` flag, which includes tests for internal validation.

### Prerequisites

- **CMake** version 3.10 or higher
- **C++20** compiler (e.g., GCC, Clang, MSVC)
- **SFML 2.5.1** (only for GUI mode)

### Build Steps

#### 1. Clone the repository
If you haven't cloned the repository yet, you can do so using:

```bash
git clone https://github.com/IgorSwat/Lazarus.git
cd Lazarus
```

#### 2. Create the build directory
Create a build directory where the project will be compiled:

```bash
mkdir build
cd build
```

#### 3. Configure the project with CMake
To configure the project and generate the appropriate build files, run the following command:
- For **CLI mode** (default):
  ```
  cmake .. -DUSE_GUI=OFF
  ```
- For **GUI mode** (with SFML 2.5.1 enabled):
  ```
  cmake .. -DUSE_GUI=ON
  ```
  If you already have SFML installed on your system, you can specify its location manually using the *SFML_DIR* option:
  ```
  cmake .. -DUSE_GUI=ON -DSFML_DIR=/path/to/SFML-2.5.1/lib/cmake/SFML
  ```
  If SFML is not found, it will be automatically downloaded and built from source along with the project.
  
- If you want to enable development tests (DEV):
  ```
  cmake .. -DDEV=ON
  ```

#### 4. Build the project
Once the configuration is complete, you can build the project using:
```bash
cmake --build . --config Release
```
It is recommended to use Release mode for optimal engine speed.

#### 5. Run the engine
Run the engine **from the build directory**:
```bash
cd ..  # Step back to the project root directory
./Lazarus
```

## CLI mode
CLI is a default mode for the project. It does not require any external dependencies.

CLI allows you to specify position (with appropriate [FEN notation](https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation)), as well as search depth. 
Then, engine performs search at given depth and returns the best move and corresponding evaluation of the position (see example below).

![CLI example usage](md/cli.png)


## GUI mode
GUI mode is an alternative to CLI. Since it was an optional part of the project and it's main purpose was to allow quicker testing, it's not necessarily of the highest quality.
It requires SFMl 2.5.1 library and can be activated with USE_GUI flag set during compilation (see **Build and run** section).

GUI allows you to manipulate the position quickly - make and unmake moves, load position from FEN notation, reset to starting position and run the engine with given depth (see example below).

<img src="md/gui.png" width="600">

## UCI
Not ready yet
