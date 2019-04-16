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

//run compiled code (for 5 philosophers) with mpirun -n 5 program

using namespace std;

const int ASK = 0;
const int DONE = 1;
const int EXIT = 2;

//this is how many poems you want each Phil to construct & save
const int MAXMESSAGES = 10;

//if you change this base, update the Makefile "clean" accordingly
const string fileBase = "outFile";

int main ( int argc, char *argv[] )
{
  int id; //my MPI ID
  int p;  //total MPI processes
  MPI::Status status;
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
  int msgIn, msgOut;
  int leftNeighbor = (id + p - 1) % p;
  int rightNeighbor = (id + 1) % p;

  pomerize P;

  string lFile = fileBase + to_string(id);
  string rFile = fileBase + to_string(rightNeighbor);
  ofstream foutLeft(lFile.c_str(), ios::out | ios::app );
  ofstream foutRight(rFile.c_str(), ios::out | ios::app );

  while (numWritten < MAXMESSAGES)
  {
    if(!leftOpen || !rightOpen)
    {
      msgOut = ASK;
      if(!leftOpen && !leftExited)
      {
        cerr << id << ": send to Left: " << msgOut << endl;
        MPI::COMM_WORLD.Send (&msgOut, 1, MPI::INT, leftNeighbor, tag);
        cerr << id << ": sent to Left: " << msgOut << endl;
        cerr << id << ": Recving from Left: " << endl;
        MPI::COMM_WORLD.Recv (&msgIn, 1, MPI::INT, leftNeighbor, tag, status);
        cerr << id << ": Recvd form Left: " << msgIn << endl;
        if(msgIn == DONE)
          leftOpen = true;
        else if(msgIn == EXIT)
          leftExited = true;
      }
      if(!rightOpen && !rightExited)
      {
        cerr << id << ": send to Right: " << msgOut << endl;
        MPI::COMM_WORLD.Send (&msgOut, 1, MPI::INT, rightNeighbor, tag);
        cerr << id << ": sent to Right: " << msgOut << endl;
        cerr << id << ": Recving from Right: " << msgOut << endl;
        MPI::COMM_WORLD.Recv (&msgIn, 1, MPI::INT, rightNeighbor, tag, status);
        cerr << id << ": Recvd from Right: " << msgOut << endl;
        if(msgIn == DONE)
          rightOpen = true;
        else if(msgIn == EXIT)
          rightExited = true;
      }
    }

    //construct poem & output stanzas into the files 'simultaneously'
    //we do this with an intermediate variable so both files contain the same poem!
    cerr << id << ": Printing to files right and left" << endl;
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

    numWritten++;
    
    cerr << id << ": Printing finished" << endl;
    if(leftOpen)
    {
      cerr << id << ": listening for calls for Left side from left Neighbor" << endl;
      MPI::COMM_WORLD.Recv (&msgIn, 1, MPI::INT, leftNeighbor, tag, status);
      cerr << id << ": Finished listening for calls for Left side" << endl;
      cerr << id << ": Recvd from Left: " << msgIn << endl;
      
      if(msgIn == ASK )
      {
        cerr << id << ": Giving up Left file" << endl;
        leftOpen = false;
        msgOut = (numWritten == MAXMESSAGES ? EXIT : DONE);
        cerr << id << ": send to Left: " << msgOut << endl;
        MPI::COMM_WORLD.Send (&msgOut, 1, MPI::INT, leftNeighbor, tag);
      }
      else if(msgIn == EXIT)
      {
        leftExited = true;
      }
    }
    if(rightOpen)
    {
      cerr << id << ": listening for calls for Right side from Right Neighbor" << endl;
      MPI::COMM_WORLD.Recv (&msgIn, 1, MPI::INT, rightNeighbor, tag, status);
      cerr << id << ": Finished listening for calls for Right side" << endl;
      cerr << id << ": Recvd from Right: " << msgIn << endl;
      if(msgIn == ASK)
      {
        cerr << id << ": Giving up Right file" << endl;
        rightOpen = false;
        msgOut = (numWritten == MAXMESSAGES ? EXIT : DONE);
        cerr << id << ": send to Right: " << msgOut << endl;
        MPI::COMM_WORLD.Send (&msgOut, 1, MPI::INT, rightNeighbor, tag);
      }
      else if(msgIn == EXIT)
      {
        rightExited = true;
      }
    }

  }

  foutLeft.close();
  foutRight.close();

  //  Terminate MPI.
  MPI::Finalize ( );
  return 0;
}
