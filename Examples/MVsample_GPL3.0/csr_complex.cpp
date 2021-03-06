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


#include "csr_complex.hpp"
/************************************************************************************************/

CSRComplex::CSRComplex(int *CSR_rowPtr, int *CSR_colIndex, double* CSR_nnz_R, double* CSR_nnz_I, int CSR_globalNgrid, int CSR_localNgrid, int CSR_Nnonzero, int CSR_localFirstrow)
{
	globalSize    = CSR_globalNgrid;
	localSize     = CSR_localNgrid;
	Nnnz          = CSR_Nnonzero;	
	localfirstrow = CSR_localFirstrow; 
	index_i       = CSR_rowPtr;
	index_j       = CSR_colIndex;
	nnz_R         = CSR_nnz_R;
	nnz_I	      = CSR_nnz_I;
}

/************************************************************************************************/

/************************************************************************************************/

CSRComplex::~CSRComplex()
{
	index_i = NULL;
	index_j = NULL;
	nnz_R   = NULL;
	nnz_I   = NULL;
}

void CSRComplex::dumpCSR(char *filename) 
{
	FILE *CSR_file;
	//char dump_csr_filename[255];
	int i, j, rowIndex, colIndex;

	//sprintf(dump_csr_filename,"%s_CSR_%d.dat",filename,mpi_my_id);
	//printf("[CSR:%d] Dumping CSR data to %s ...\n",mpi_my_id,dump_csr_filename);
	//CSR_file=fopen(dump_csr_filename,"w");
	CSR_file=fopen(filename,"w");
	int count = 0;
	for(i=0;i<localSize;i++) 
	{
		rowIndex = localfirstrow + i-1;
		int startIndex = index_i[i];
		int endIndex = index_i[i+1];

		if(endIndex - startIndex <= 0)
			printf("localSize=%d, startindex=%d, endindex=%d, currentrow = %d\n", localSize, startIndex, endIndex, i+1);

		count = count + (endIndex-startIndex);
		//printf("[DEBUG] startIndex = %d, endIndex = %d, rowIndex = %d\n", startIndex, endIndex, rowIndex);
		for (j=startIndex; j<endIndex; j++) 
		{
			colIndex = index_j[j];
			fprintf(CSR_file,"%d     %d     %lf     %lf\n", rowIndex+1, colIndex+1, nnz_R[j], nnz_I[j]);
		}
	}

	//    printf("[dump] total NNZ = %d\n", count);
	fclose(CSR_file);
}
/************************************************************************************************/

