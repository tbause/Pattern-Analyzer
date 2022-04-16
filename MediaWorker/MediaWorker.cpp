// MediaWorker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

void PrintTask(int TaskNo)
{
	int Counter = 0;

	for (Counter = 0; Counter < 100; Counter++)
	{
		cout << "Task[" << TaskNo << "]: " << Counter << endl;
		this_thread::sleep_for(chrono::milliseconds(500));
	}

}


int main(int argc, char* argv[])
{
	thread Task1(PrintTask, 1);
	thread Task2(PrintTask, 2);
	thread Task3(PrintTask, 3);
	cout << "All tasks started" << endl;
	Task1.join();
	Task2.join();
	Task3.join();
	cout << "All tasks ended" << endl;
	cin;
	return 0;
}

