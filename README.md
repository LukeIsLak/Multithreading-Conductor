## Description
This is a project that allows us to simulate scheduling by using multithreading and the pthread library.

This simulates a automated control system for a railway track.
## Cloning the Repo

To clone the repo, you need to execute the following commands

```
cd existing_repo
git remote add origin https://github.com/LukeIsLak/Multithreading-Conductor.git
git branch -M main
git push -uf origin main
```

## Usage
To initialize the project, use a MAKE commander
```
make
```

You will generate a mts.o file. To run this file, you should have a filepath to a .txt file.
For example, if you had a file called input.txt, you should run the following command.

```
./mts input.txt
```

MTS is the main executable, it takes a filepath to a .txt file and while it does not output anything to the standard output, it
does output to a "output.txt"

To clean the project, run the following command in your terminal
```
make clean
```

## Project status
MTS is complete

## Sample Input Output
The following input.txt uses this input case

```
e 5 1
w 1 6
W 2 1
W 3 1
```

The following output of the code will look like if the command
```
./mts input.txt
```
was run

```
00:00:00.1 Train  1 is ready to go West
00:00:00.1 Train  1 is ON the main track going West
00:00:00.2 Train  2 is ready to go West
00:00:00.3 Train  3 is ready to go West
00:00:00.5 Train  0 is ready to go East
00:00:00.7 Train  1 is OFF the main track after going West
00:00:00.7 Train  2 is ON the main track going West
00:00:00.8 Train  2 is OFF the main track after going West
00:00:00.8 Train  0 is ON the main track going East
00:00:00.9 Train  0 is OFF the main track after going East
00:00:00.9 Train  3 is ON the main track going West
00:00:01.0 Train  3 is OFF the main track after going West
```
