#ifndef _CSR_COMPLEX_HPP_
#define _CSR_COMPLEX_HPP_
#include <stdio.h>
class CSRComplex
{
	public:
		CSRComplex(int*, int*, double*, double*, int, int, int, int);
		~CSRComplex();

		void dumpCSR(char *);

		int globalSize;
		int localSize;
		int localfirstrow;
		int Nnnz;
		int *index_i;
		int *index_j;
		double* nnz_R;
		double* nnz_I;
		int findx;

	private:

};
#endif
