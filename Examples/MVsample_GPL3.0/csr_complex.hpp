//-------------------------------------------- LICENSE NOTICE -------------------------------------------------------------
// SParse Matrix-Vector multplication example code: A Tight-binding Hamiltonian stored with a Compressed Sparse Row format
//
// Copyright (C) 2021 Hoon Ryu (Korea Institute of Science and Technology Information / E: elec1020@gmail.com)
//
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of the License, or any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <https://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------------------------------------------------------

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
