# 3D-Chess-Game-with-Komodo-Chess-Engine

This repository contains a **3D Chess Game** implemented in **C++** using **OpenGL**, **ASSIMP**, and a third-party **UCI-compliant chess engine** (e.g., Komodo). The game renders a 3D chessboard and pieces, processes user moves, and dynamically updates the board based on responses from the chess engine. It also allows users to control the camera and lighting for an immersive 3D experience.

---

## Objective

To design and implement an interactive 3D chess game with the following features:
1. Render and manipulate a 3D chessboard and pieces.
2. Validate user moves entered in **UCI format**.
3. Animate chess piece movements, including sliding and knight-specific movements.
4. Integrate and communicate with a third-party chess engine for move validation and response.
5. Provide camera and lighting controls for enhanced visualization.

---

## Features

### Chess Gameplay
- **3D Rendering**:
  - The chessboard and pieces are aligned with the z-axis pointing upwards and centered at the origin.
  - Pieces and board models are loaded using ASSIMP.
- **Move Validation**:
  - Users input chess moves in **UCI format** (e.g., `e2e4`).
  - Invalid moves are detected and reported.
- **Piece Animation**:
  - Pieces slide smoothly across the board (~2–3 seconds per move).
  - Knights leap over other pieces during movement.
  - Captured pieces are removed from the board.
- **Game State Detection**:
  - Detects checkmate and announces the winner.
  - Allows users to quit the game using the `quit` command.

### Camera and Lighting Controls
- **Camera**:
  - Controlled using spherical coordinates (`Θ`, `Φ`, `R`).
  - Always points toward the origin.
  - Command: `camera Θ Φ R` (e.g., `camera 30 45 5`).
- **Lighting**:
  - Position controlled with the `light Θ Φ R` command.
  - Intensity adjusted using the `power` command (e.g., `power 100.0`).
  - Invalid commands are handled gracefully.

### Chess Engine Integration
- **ECE_ChessEngine** Class:
  - `bool InitializeEngine()`: Initializes the chess engine.
  - `bool sendMove(const std::string& strMove)`: Sends a move to the engine.
  - `bool getResponseMove(std::string& strMove)`: Retrieves the engine's response move.

---

## File Structure

### Source Files
- **chess_game.cpp**: Contains the main game logic, command parsing, and OpenGL rendering.
- **ECE_ChessEngine.cpp**: Manages interaction with the chess engine.

### Assets
- **Models**:
  - `chessboard.obj`: 3D model of the chessboard.
  - `chess-mod.obj`: 3D models of chess pieces.
- **Textures**:
  - High-resolution textures for the chessboard and pieces.

---

## How to Run

### Prerequisites
1. **C++ Compiler**: A compiler with OpenGL and ASSIMP support (e.g., GCC, Clang).
2. **Libraries**:
   - **OpenGL**
   - **ASSIMP**
   - **Komodo Chess Engine** (or any UCI-compliant engine).

---

### Compilation
1. Clone the repository:
   ```bash
   git clone <repository_url>
   cd Chess3DSimulation
   ```
2. Build the Project Using CMake:
   ```bash
   cmake --build .
   cmake ..
   ```
3. run the executable:
   ```bash
   ./3DChessGame
   ```

### Example console Commands
```bash
> Please enter a command: move e2e4
> Please enter a command: camera 10.0 45.0 5.0
> Please enter a command: light 45.0 90.0 10.0
> Please enter a command: power 150.0
> Please enter a command: quit
Thanks for playing!
```

## Gameplay Instructions

### Controls
- **Chess Moves**: Enter moves in UCI format (e.g., `e2e4`).
- **Camera**:
  - `camera Θ Φ R`: Adjust camera position using spherical coordinates.
  - Example: `camera 30 45 5`
- **Lighting**:
  - `light Θ Φ R`: Adjust light position using spherical coordinates.
  - `power <value>`: Set light power (e.g., `power 100.0`).
- **Quit**:
  - Enter `quit` to end the game.
  - Press **Escape** to exit.

### Objective
- Play chess against the engine.
- End the game when a player wins or enters `quit`.

---

## Testing and Debugging

### Key Features to Test

1. **3D Rendering**:
   - Ensure models are aligned correctly with the z-axis pointing upwards.
   - Verify scaling and positions of the chessboard and pieces.

2. **Move Validation**:
   - Test various valid and invalid moves.
   - Verify correct handling of illegal commands.

3. **Piece Animation**:
   - Validate smooth piece sliding and knight-specific movement.

4. **Camera and Lighting**:
   - Test camera and light movement commands.
   - Ensure invalid commands are handled gracefully.

5. **Chess Engine Integration**:
   - Verify moves are sent to the engine and responses are received promptly.

6. **Game State**:
   - Test for checkmate detection and correct game termination.
