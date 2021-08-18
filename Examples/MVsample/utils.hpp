#ifndef _UTILS
#define _UTILS

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "csr_complex.hpp"

void convert(double*, double*, int*, int*, double*, double*, int);
void load(int*, int*, double*, double*, char*, char*, char*);
#define MONITORING
#ifdef MONITORING
void dmv(double*, double*, double*, double*, double*, double*, int, int);
void spmv(CSRComplex*, double*, double*, double*, double*, int);
#else
void dmv(double*, double*, double*, double*, double*, double*, int);
void spmv(CSRComplex*, double*, double*, double*, double*);
#endif
void dump_dmatrix(double*, double*, int, char*);

#endif
