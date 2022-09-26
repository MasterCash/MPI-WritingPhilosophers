# MPI-WritingPhilosophers
An Assignment for CS3800 Operating Systems. Task: build a program to handle a ring of programs trying to write to same files using messaging
# Prompt
Implement a working Dining Philosopher-like program using MPI
Each Philosopher will write to a file to its left and a file to its right for n times. Philosophers share the left file and right file with another Philosopher.
The sharing is in such a way that if visualized the philosophers create a ring where a file seperates each philosopher and that file represents the file they share.

# Requirements to run
  * Linux
  * MPI (mpich & mpich-doc)
  * C++
  * Make

## Dependencies
  install c++, make, and MPI libraries(names listed above)
## Building
  run `make`
## Running
  run `mpirun -n 5 program`
