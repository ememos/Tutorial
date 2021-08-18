#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "utils.hpp"
#include "csr_complex.hpp"

#define MONITORING

#ifdef MONITORING

#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#define MAX_ROI 100

int roi_num = 0;

static void timeval_subtract (struct timeval *x, struct timeval *s)
{
        struct timeval y = {
                .tv_sec = s->tv_sec,
                .tv_usec = s->tv_usec
        };

        /* Perform the carry for the later subtraction by updating y. */
        if (x->tv_usec < y.tv_usec) {
                int nsec = (y.tv_usec - x->tv_usec) / 1000000 + 1;
                y.tv_usec -= 1000000 * nsec;
                y.tv_sec += nsec;
        }
        if (x->tv_usec - y.tv_usec > 1000000) {
                int nsec = (x->tv_usec - y.tv_usec) / 1000000;
                y.tv_usec += 1000000 * nsec;
                y.tv_sec -= nsec;
        }

        /* subtract updated y from x */
        x->tv_sec -= y.tv_sec;
        x->tv_usec -= y.tv_usec;
}

struct timeval monitoring_beg[MAX_ROI];
struct timeval monitoring_end[MAX_ROI];

#endif

int main(int argc, char **argv) {

	int nnz = 0;
	int *row_ptr, *col_ind;
	int localfirst, mysize, count, niter;
	int localend, globalsize, globalnnz, row_ptr_ind;
	int Nx, Ny, xcindfirst, xcindend, DIM;
	double *nzeroR, *nzeroI, *dmatrixR, *dmatrixI;
	double *VR, *VI, *WR, *WI, *YR, *YI;
	CSRComplex *smatrix;
	char *myfilename;
	FILE *fp;


#ifdef MONITORING
	memset(monitoring_beg, 0, sizeof(monitoring_beg));
	memset(monitoring_end, 0, sizeof(monitoring_end));

	gettimeofday(&monitoring_beg[roi_num], NULL);
#endif

	DIM = 5120;
	niter = 10;
	nnz = 175164;
	localfirst = 1;
	localend = DIM;
	mysize = localend - localfirst + 1;
	globalsize = mysize;
	printf("Dimension is initialized.\n");

	WR = new double[mysize]();
	WI = new double[mysize]();
	VR = new double[mysize]();
	VI = new double[mysize]();
	YR = new double[mysize]();
	YI = new double[mysize]();
	count = 0;

	for(int ii = localfirst; ii <= localend; ii++)
	{
		VR[ii] = double(rand())/RAND_MAX;
		VI[ii] = double(rand())/RAND_MAX;
		YR[ii] = VR[ii];
		YI[ii] = VI[ii];
	}
	printf("Vectors are constructed.\n");

	row_ptr = new int[mysize+1]();
	col_ind = new int[nnz]();
	nzeroR  = new double[nnz](); 		 
	nzeroI  = new double[nnz]();
#ifdef MONITORING
	gettimeofday(&monitoring_end[roi_num], NULL);
#endif

#ifdef MONITORING
	// roi_num = 1
	++roi_num;
	gettimeofday(&monitoring_beg[roi_num], NULL);
#endif
	load(row_ptr, col_ind, nzeroR, nzeroI, "./data/index_i.dat", "./data/index_j.dat", "./data/nonzeros.dat");
#ifdef MONITORING
	gettimeofday(&monitoring_end[roi_num], NULL);
#endif

#ifdef MONITORING
	// roi_num = 2
	++roi_num;
	gettimeofday(&monitoring_beg[roi_num], NULL);
#endif
	
	smatrix = new CSRComplex(row_ptr, col_ind, nzeroR, nzeroI, globalsize, mysize, nnz, localfirst);
	printf("Matrix is constructed.\n");
#ifdef MONITORING
	gettimeofday(&monitoring_end[roi_num], NULL);
#endif

#ifdef MONITORING
	// roi_num = 3
	++roi_num;
	gettimeofday(&monitoring_beg[roi_num], NULL);
#endif
	for(int ii = 0; ii < niter; ii++)
	{
		printf("SPMV at iteration %d\n", ii+1);

#ifdef MONITORING
		spmv(smatrix, VR, VI, WR, WI, roi_num);
		spmv(smatrix, WR, WI, VR, VI, roi_num);
#else
		spmv(smatrix, VR, VI, WR, WI);
		spmv(smatrix, WR, WI, VR, VI);
#endif
	}
#ifdef MONITORING
	gettimeofday(&monitoring_end[roi_num], NULL);
#endif

#ifdef MONITORING
	printf("roi_num ==> %d, after spmv\n", roi_num); // TODO
	// roi_num = 4
	++roi_num;
	gettimeofday(&monitoring_beg[roi_num], NULL);
#endif
	system("rm -rf ./result");
	system("mkdir ./result");  	

	smatrix->dumpCSR("./result/smatrix.dat");

	fp = fopen("./result/spmv_output.dat","wt");
	for(int ii = 0; ii < DIM; ii++)
		fprintf(fp, "%18.6e    %18.6e\n", VR[ii], VI[ii]);
	fclose(fp); 
#ifdef MONITORING
	gettimeofday(&monitoring_end[roi_num], NULL);
#endif

#ifdef MONITORING
	// roi_num = 5
	++roi_num;
	gettimeofday(&monitoring_beg[roi_num], NULL);
#endif

	dmatrixR = new double[mysize*mysize]();
	dmatrixI = new double[mysize*mysize]();
#ifdef MONITORING
	gettimeofday(&monitoring_end[roi_num], NULL);
#endif

#ifdef MONITORING
	// roi_num = 6
	++roi_num;
	gettimeofday(&monitoring_beg[roi_num], NULL);
#endif

	convert(dmatrixR, dmatrixI, row_ptr, col_ind, nzeroR, nzeroI, DIM);
	//dump_dmatrix(dmatrixR, dmatrixI, DIM, "./result/dmatrix.dat");
#ifdef MONITORING
	gettimeofday(&monitoring_end[roi_num], NULL);
#endif

#ifdef MONITORING
	// roi_num = 7
	++roi_num;
	gettimeofday(&monitoring_beg[roi_num], NULL);
#endif

	for(int ii = 0; ii < niter; ii++)
	{
		printf("DMV at iteration %d\n", ii+1);
#ifdef MONITORING
		//dmv(dmatrixR, dmatrixI, YR, YI, WR, WI, DIM, ++roi_num);
		//dmv(dmatrixR, dmatrixI, WR, WI, YR, YI, DIM, ++roi_num);
		dmv(dmatrixR, dmatrixI, YR, YI, WR, WI, DIM, roi_num);
		dmv(dmatrixR, dmatrixI, WR, WI, YR, YI, DIM, roi_num);
#else
		dmv(dmatrixR, dmatrixI, YR, YI, WR, WI, DIM);
		dmv(dmatrixR, dmatrixI, WR, WI, YR, YI, DIM);
#endif
	}
#ifdef MONITORING
	gettimeofday(&monitoring_end[roi_num], NULL);
#endif

#ifdef MONITORING
	// roi_num = 8
	printf("roi_num ==> %d, after dmv\n", roi_num); // TODO
	++roi_num;
	gettimeofday(&monitoring_beg[roi_num], NULL);
#endif
	fp = fopen("./result/dmv_output.dat","wt");
	for(int ii = 0; ii < DIM; ii++)
		fprintf(fp, "%18.6e    %18.6e\n", YR[ii], YI[ii]);
	fclose(fp);
#ifdef MONITORING
	gettimeofday(&monitoring_end[roi_num], NULL);
#endif

#ifdef MONITORING
	// roi_num = 9
	++roi_num;
	gettimeofday(&monitoring_beg[roi_num], NULL);
#endif

	delete [] dmatrixI;
	delete [] dmatrixR;

	delete smatrix;	
	delete [] nzeroI;
	delete [] nzeroR;
	delete [] col_ind;
	delete [] row_ptr;
	delete [] YI;
	delete [] YR;
	delete [] VI;
	delete [] VR;
	delete [] WI;
	delete [] WR;
#ifdef MONITORING
	gettimeofday(&monitoring_end[roi_num], NULL);
	printf("roi_num ==> %d, final\n", roi_num); // TODO
#endif

	printf("Memory: Deallocated.\n");
#ifdef MONITORING
	int k;
	for (k = 0; k < roi_num+1; k++) {
		timeval_subtract(&monitoring_end[k], &monitoring_beg[k]);

		printf("[%d] %lu.%06lu\n", k, monitoring_end[k].tv_sec, monitoring_end[k].tv_usec);
	}
#endif
	return 0;
}
