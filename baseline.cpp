/* Names: Josh Cash and Hannah Reinbolt
** Date: 4-16-2019
** Assignment: Writing Phils Homework
*/
#include <cstdlib>
#include <iostream>
#include <fstream> 
#include <cerrno>
#include <unistd.h>
#include "mpi.h"
#include "pomerize.h"
#define DEBUG false

//run compiled code (for 5 philosophers) with mpirun -n 5 program

using namespace std;

const int NONE = -1;
const int ASK = 0;
const int DONE = 1;
const int WORKING = 2;
const int EXIT = 3;

//this is how many poems you want each Phil to construct & save
const int MAXMESSAGES = 10;

//if you change this base, update the Makefile "clean" accordingly
const string fileBase = "outFile";

void output(ostream & foutRight, ostream & foutLeft, int id);

bool trySend(const int, const int, const int, const bool = true);
int tryRecv(const int, const int, const bool = true);

void print(const string);

int main ( int argc, char *argv[] )
{
  int id; //my MPI ID
  int p;  //total MPI processes
  MPI::Status status;
  MPI::Request request;
  int tag = 1;

  // control vars for left and right side
  bool rightOpen = false;
  bool leftOpen = false;
  bool rightExited = false;
  bool leftExited = false;

  //  Initialize MPI.
  MPI::Init ( argc, argv );

  //  Get the number of processes.
  p = MPI::COMM_WORLD.Get_size ( );

  //  Determine the rank of this process.
  id = MPI::COMM_WORLD.Get_rank ( );

  //Safety check - need at least 2 philosophers to make sense
  if (p < 2) {
    MPI::Finalize ( );
    std::cerr << "Need at least 2 philosophers! Try again" << std::endl;
    return 1; //non-normal exit
  }

  // first person gets both access
  if(id == p -1)
  {
    rightOpen = true;
    leftOpen = true;
  }
  // everyone else but last gets left
  else if(id != 0)
  {
    leftOpen = true;
  }
  // last gets none poor Philosopher


  srand(id + time(NULL)); //ensure different seeds...

  int numWritten = 0;

  //setup message storage locations
  int msgIn;
  int leftNeighbor = (id + p - 1) % p;
  int rightNeighbor = (id + 1) % p;


  string lFile = fileBase + to_string(id);
  string rFile = fileBase + to_string(rightNeighbor);
  ofstream foutLeft(lFile.c_str(), ios::out | ios::app );
  ofstream foutRight(rFile.c_str(), ios::out | ios::app );

  while (numWritten < MAXMESSAGES)
  {
    // if no one is home anymore, dont ask
    if(leftExited)
      leftOpen = true;
    if(rightExited)
      rightOpen = true;
    // if we have the lock tell people
    if(leftOpen || rightOpen)
    {
      // let left know we are busy 
      // not giving up till we process
      if(leftOpen)
      {
        trySend(id, WORKING, leftNeighbor);
      }
      // let right know we are busy and not giving it up
      if(rightOpen)
        trySend(id, WORKING, rightNeighbor);
    }
    // ask for what you are missing 
    if(!leftOpen)
      trySend(id, ASK, leftNeighbor);
    if(!rightOpen)
      trySend(id, ASK, rightNeighbor);
    // Recieve a message
    bool asked = false;
    msgIn = tryRecv(id, leftNeighbor);

    // if get a message keep processing them
    if(msgIn != NONE)
    {
      do
      {
        asked = false;
        msgIn = tryRecv(id, leftNeighbor);
        // if they are asking tell them we are not giving it up
        if(msgIn == ASK)
        {
          asked = true;
        }
        // if they are working then we can't
        else if(msgIn == WORKING)
        {
          leftOpen = false;
        }
        // if they are done, then we can work
        else if(msgIn == DONE)
        {
          leftOpen = true;
        }
        // if they are exited, then we can always work
        else if(msgIn == EXIT)
        {
          leftOpen = true;
          leftExited = true;
        }
      } while (msgIn != NONE);
      if(asked)
      {
        trySend(id, WORKING, leftNeighbor);
        leftOpen = true;
      }
    }
    // if no messages at all assume lock
    else
    {
      // set open 
      leftOpen = true;
      // let other know we are working now
      trySend(id, WORKING, leftNeighbor);
    }
    
    msgIn = tryRecv(id, rightNeighbor);
    // if get a message keep processing them
    if(msgIn != NONE)
    {
      do
      {
        asked = false;
        msgIn = tryRecv(id, rightNeighbor);
        // if they are asking tell them we are not giving it up
        if(msgIn == ASK)
        {
          asked = true;
        }
        // if they are working then we can't
        else if(msgIn == WORKING)
        {
          rightOpen = false;
        }
        // if they are done, then we can work
        else if(msgIn == DONE)
        {
          rightOpen = true;
        }
        // if they are exited, then we can always work
        else if(msgIn == EXIT)
        {
          rightOpen = true;
          rightExited = true;
        }
      } while (msgIn != NONE);
      if(asked)
      {
        trySend(id, WORKING, rightNeighbor);
        rightOpen = true;
      }
      
    }
    // if no messages at all assume lock
    else
    {
      // set open 
      rightOpen = true;
      // let other know we are working now
      trySend(id, WORKING, rightNeighbor);
    }
    
    if(leftOpen && rightOpen)
    {
      trySend(id, WORKING, leftNeighbor);
      trySend(id, WORKING, rightNeighbor);
      output(foutRight, foutLeft, id);
      rightOpen = false;
      leftOpen = false;
      trySend(id, DONE, leftNeighbor);
      trySend(id, DONE, rightNeighbor);
      numWritten++;
    }
}
  MPI::COMM_WORLD.Send(&EXIT, 1, MPI::INT, leftNeighbor, tag);
  MPI::COMM_WORLD.Send(&EXIT, 1, MPI::INT, rightNeighbor, tag);

  foutLeft.close();
  foutRight.close();

  //  Terminate MPI.
  MPI::Finalize ( );
  return 0;
}

void output(ostream & foutRight, ostream & foutLeft, int id)
{
  print(to_string(id) + ": Outputing");
  pomerize P;
  foutLeft << id << "'s poem:" << endl;
  foutRight << id << "'s poem:" << endl;

  string stanza1, stanza2, stanza3;
  stanza1 = P.getLine();
  foutLeft << stanza1 << endl;
  foutRight << stanza1 << endl;

  stanza2 = P.getLine();
  foutLeft << stanza2 << endl;
  foutRight << stanza2 << endl;

  stanza3 = P.getLine();
  foutLeft << stanza3 << endl << endl;
  foutRight << stanza3 << endl << endl;

  return;
}

bool trySend(const int id, const int msg, const int sender, const bool test)
{
  print("" + to_string(id) + ": Trying to send Message to: " + to_string(sender) + " msg: " + to_string(msg));
  int processed = 0;
  MPI::Status status;
  MPI::Request request = MPI::COMM_WORLD.Isend(&msg, 1, MPI::INT, sender, 1);
  print("" + to_string(id) + ": Tried to send Message to: " + to_string(sender) + " msg: " + to_string(msg));
  if(false && test)
  {
    MPI_Test(request, &processed, status);
    processed ?   
      print("" + to_string(id) + ": Succeed to send Message to: " + to_string(sender) + " msg: " + to_string(msg)) : 
      print("" + to_string(id) + ": Failed to send Message to: " + to_string(sender) + " msg: " + to_string(msg));
  }
  return processed;
}

int tryRecv(const int id, const int sender, const bool test)
{
  print("" + to_string(id) + ": Trying to recv Message from: " + to_string(sender));
  int processed = 0;
  int msg;
  MPI::Status status;
  MPI::Request request;
  request = MPI::COMM_WORLD.Irecv(&msg, 1, MPI::INT, sender, 1);
  print("" + to_string(id) + ": Tried to recv Message from: " + to_string(sender));
  if(test)
  {
    MPI_Test(request, &processed, status);
    processed ?
      print("" + to_string(id) + ": Successed to recv Message from: " + to_string(sender) + " recieved: " + to_string(msg)) :
      print("" + to_string(id) + ": Failed to recv Message from: " + to_string(sender) + " recieved: " + to_string(msg));
  }
  return processed ? msg : NONE;
}

void print(const string s)
{
  if(DEBUG)
  {
    cerr << s << endl;
  }
}