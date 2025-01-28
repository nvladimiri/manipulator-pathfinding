# Manipulator Pathfinding using A*

This project implements pathfinding for a robotic manipulator on a plane using the **A* algorithm**. The manipulator consists of multiple segments, each with its own set of angles. The goal is to find the optimal path for the manipulator to reach a target position while avoiding obstacles.

## Features

- Uses A* for optimal pathfinding.
- The map is divided into a grid with obstacles and reachable positions.
- Handles 2D manipulator segments and obstacles for real-world scenarios.
- Visualization of the pathfinding process.

## Requirements

To run the project, make sure you have the following dependencies installed:

- Python 3.x
- NumPy
- Matplotlib
- `import_ipynb` (for notebook-based imports)

You can install the required libraries with:

```bash
pip install numpy matplotlib import_ipynb
