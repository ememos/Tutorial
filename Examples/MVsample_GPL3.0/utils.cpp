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

#include "utils.hpp"

void convert(double *dmatrix_R, double *dmatrix_I, int *index_i, int *index_j, double *nz_R, double *nz_I, int DIM)
{
	for(unsigned int i = 0; i < DIM; i++)
	{
		const unsigned int nSubStart = index_i[i];
		const unsigned int nSubEnd = index_i[i+1];
                
		for(unsigned int j = nSubStart; j < nSubEnd; j++)
		{
			const unsigned int nColIndex = index_j[j];
			const double m_real = nz_R[j];
			const double m_imaginary = nz_I[j];

			dmatrix_R[i*DIM + nColIndex] = m_real;
			dmatrix_I[i*DIM + nColIndex] = m_imaginary;                        
                }
	}
}

void dmv(double *dmatrix_R, double *dmatrix_I, double *xR, double *xI, double *yR, double *yI, int DIM)
{
	int mysize = DIM;

        #pragma omp parallel for 
        for(unsigned int i = 0; i < mysize; i++)
        {
                double real_sum = 0.0;
                double imaginary_sum = 0.0;

                for(unsigned int j = 0; j < mysize; j++)
                {
                        real_sum += dmatrix_R[i*DIM+j] * xR[j] - dmatrix_I[i*DIM+j] * xI[j];
                        imaginary_sum += dmatrix_R[i*DIM+j] * xI[j] + dmatrix_I[i*DIM+j] * xR[j];
                }

                yR[i] = real_sum;
                yI[i] = imaginary_sum;
        }
	
}

void dump_dmatrix(double *dmatrix_R, double *dmatrix_I, int DIM, char* filename)
{
	FILE *fp;
	fp = fopen(filename,"wt");
	for (int ii=0; ii<DIM; ii++)
		for (int jj=0; jj<DIM; jj++)
			fprintf(fp, "%d		%d		%18.6e		%18.6e\n", ii+1, jj+1, dmatrix_R[ii*DIM+jj], dmatrix_I[ii*DIM+jj]);
	fclose(fp);
}

void load(int *index_i, int* index_j, double *nz_R, double *nz_I, char* file_i, char* file_j, char *file_n)
{
	int count;
	int buffer1;
	double buffer2[2];
	FILE *fp;

	fp = fopen(file_i,"rb");
	count = 0;
	while(!feof(fp))
	{
		fread(&buffer1,sizeof(int),1,fp);
		index_i[count++] = buffer1;
	}
	printf("Number of index_i count = %d\n", count);
	fclose(fp);

	fp = fopen(file_j,"rb");
	count = 0;
	while(!feof(fp))
	{
		fread(&buffer1,sizeof(int),1,fp);
		index_j[count++] = buffer1;
	}
	printf("Number of index_j count = %d\n", count);
	fclose(fp);

	fp = fopen(file_n,"rb");
	count = 0;
	while(!feof(fp))
	{
		fread(buffer2,sizeof(double),2,fp);
		nz_R[count] = buffer2[0];
		nz_I[count++] = buffer2[1];
	}
	printf("Number of nz count = %d\n", count);
	fclose(fp);
} 

void spmv(CSRComplex *A, double *xR, double *xI, double *yR, double *yI)
{
	int mysize = A->localSize;

	#pragma omp parallel for 
	for(unsigned int i = 0; i < mysize; i++)
	{
		double real_sum = 0.0;
		double imaginary_sum = 0.0;
		const unsigned int nSubStart = A->index_i[i];
		const unsigned int nSubEnd = A->index_i[i+1];
                
		for(unsigned int j = nSubStart; j < nSubEnd; j++)
		{
			const unsigned int nColIndex = A->index_j[j];
			const double m_real = A->nnz_R[j];
			const double m_imaginary = A->nnz_I[j];
			const double v_real = xR[nColIndex];
			const double v_imaginary = xI[nColIndex];
                        
			real_sum += m_real * v_real - m_imaginary * v_imaginary;
			imaginary_sum += m_real * v_imaginary + m_imaginary * v_real;
		}

		yR[i] = real_sum;
		yI[i] = imaginary_sum;
        }
}
