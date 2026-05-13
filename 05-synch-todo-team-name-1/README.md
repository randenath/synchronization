# CS441/541 Synchronization Project

## Author(s):

Rohan Hari

## Date:

04/23/2026

## Description:
This is the synchronization project where we solve the intersection problem using semaphores.

## How to build the software

To build the software, you must go to the project directory, and type in "make" on the command line.

## How to use the software

To use the software, you must type in "./stoplight <time to live> <numer of cars>. The program will run your inputs and print out the statistics and path taken by all cars.

## How the software was tested

To test the software, I ran it with different input values for the "time to live" and "nuber of cars". I did it with high time to live along with high and low number of cars and vice versa. 

I also check what would happen if we did not give the correct commands. Say we forgot one or both commands, an error message would be printed to tell you that you are a bad user.

## Known bugs and problem areas

One oversight that I had was that there could be a race condition when we are trying to update the stats. If there were two cars trying to update their stats at the same time, the race condition could happen and nobody wants that. So I put in a new semaphore to lock the usage of updating stats to only allow one car at a time. 

Other than that, no other known bugs. 
