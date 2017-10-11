#include <stdio.h>
#include "mpi.h"


#include <iostream>
#include <cstdlib>
#include <cassert>

#include "slurp_file.h"


void computeMatVec (std::vector<int> *v1, std::vector<int> v0, int x0)
{
	for (std::vector<int>::iterator i=v0.begin(); i!=v0.end(); i++)
	{
		v1->push_back (*i * x0);
	}
}

void addVectors (std::vector<int> *v3, std::vector<int> v1, std::vector<int> v0)
{
	for (int i=0; i<v0.size(); i++)
	{
		v3->push_back(v1[i] +v0[i]);
	}
}

int main(int argc, char *argv[])
{

	//Argument to be sent:
	//1. Filename
	//2. number of elements in each column


	char *filename = argv[1];
	int size_of_col = atoi(argv[2]);
		
	
	int numtasks, taskid, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];
	std::vector<int> v1;
	std::vector<int> v2;
	std::vector<int> v3;
	
	MPI_Status st;
    MPI_Init (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank (MPI_COMM_WORLD, &taskid);
    
    MPI_Get_processor_name (hostname, &len);

	std::vector<int> v[numtasks];
    slurp_file_line(filename, taskid, &v[taskid]);

	std::vector<int> x;
	slurp_file_line (filename, numtasks, &x);

	if (taskid == 0)
	{
		computeMatVec (&v1, v[taskid], x[taskid]);
#if 0
		for (std::vector<int>::iterator i=v1.begin(); i!=v1.end(); i++)
		{
			std::cout << "ELEMENT " << *i << "\n";
		}
#endif	
		//Send it to processor 1
		MPI_Send (&v1[0], size_of_col, MPI_INT, taskid+1, 0, MPI_COMM_WORLD);
	}

	if ((taskid > 0) && (taskid < numtasks-1))
	{
		v2.resize (size_of_col);
		
		MPI_Recv (&v2[0], size_of_col, MPI_INT, taskid-1, 0, MPI_COMM_WORLD, &st);
		computeMatVec (&v1, v[taskid], x[taskid]);
		addVectors (&v3, v1, v2);
		MPI_Send (&v3[0], size_of_col, MPI_INT, taskid+1, 0, MPI_COMM_WORLD);
	}

	if (taskid == numtasks-1)
	{
		v2.resize (size_of_col);
		MPI_Recv (&v2[0], size_of_col, MPI_INT, taskid-1, 0, MPI_COMM_WORLD, &st);
		computeMatVec (&v1, v[taskid], x[taskid]);
		addVectors (&v3, v1, v2);

#if 1
		std::cout << "TOTAL SUM IS " << "\n";
		for (std::vector<int>::iterator i=v3.begin(); i!=v3.end(); i++)
		{
			std::cout << *i << "\n";
		}
		std::cout << "\n";
#endif
	}
	MPI_Finalize();
}
