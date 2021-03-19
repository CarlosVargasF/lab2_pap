#include <stdio.h>
#include <omp.h>
#include <stdint.h>
#include <string.h>

#include <x86intrin.h>
#include <stdbool.h>

#include "sorting.h"

/* 
   odd-even sort -- sequential, parallel -- 
*/

/* This is basically the same structure of Bubble sort, the main difference is 
the step=2 in the For loops. In the 1st stage, we swap (if necessary) every 
element in an odd position with the next one. In the 2nd one, we dp the same with
every element in even pos.
*/

/*This implementation allows to do that in one For loop using startIndex to
interleave between odds and evens.
*/
void sequential_oddeven_sort (uint64_t *T, const uint64_t size)
{
    /* TODO: sequential implementation of odd-even sort */
    uint64_t startIndex, i, temp, sorted;

    
    startIndex = 0;
    do
    {   
        sorted = 1;
        for(i = startIndex; i < size-1; i+=2)
        {
            if (T[i] > T[i+1])
            {
                temp = T[i+1];
                T[i+1] = T[i];
                T[i] = temp;

                sorted = 0;
            }
        }
        startIndex = 1 - startIndex;
    } while (sorted == 0);
    return ;
}


void parallel_oddeven_sort (uint64_t *T, const uint64_t size)
{

    /* TODO: parallel implementation of odd-even sort */ 
    uint64_t startIndex, i, temp, sorted, chunk_sz;
    
    chunk_sz = size / omp_get_max_threads();
    // startIndex = 0;
    do
    {
        sorted = 1;
	/*We split the array in chunks and try to swap odds-possitioned elems inside each chunk with threads*/    
        #pragma omp parallel for private(temp)
        for(i = 0; i < size-1; i+=2)
        {
            if (T[i] > T[i+1])
            {
                temp = T[i+1];
                T[i+1] = T[i];
                T[i] = temp;

                sorted = 0;
            }
        }
	/*When all prevouis threads have finished, try to swap even-pos elems inside same chunk partitions*/
        #pragma omp parallel for private(temp)
        for(i = 1; i < size-1; i+=2)
        {
            if (T[i] > T[i+1])
            {
                temp = T[i+1];
                T[i+1] = T[i];
                T[i] = temp;

                sorted = 0;
            }
        }
        // startIndex = 1 - startIndex;
    } while (sorted == 0);
    return ;
}


int main (int argc, char **argv)
{
    uint64_t start, end;
    uint64_t av ;
    unsigned int exp ;

    /* the program takes one parameter N which is the size of the array to
       be sorted. The array will have size 2^N */
    if (argc != 2)
    {
        fprintf (stderr, "odd-even.run N \n") ;
        exit (-1) ;
    }

    uint64_t N = 1 << (atoi(argv[1])) ;
    /* the array to be sorted */
    uint64_t *X = (uint64_t *) malloc (N * sizeof(uint64_t)) ;

    printf("================================================\n");
    printf(" Max number of threads: %d \n", omp_get_max_threads());    
    printf(" --> Sorting an array of size %lu (2^%u)\n", N, atoi(argv[1]));
#ifdef RINIT
    printf("--> The array is initialized randomly\n");
#endif
    

    for (exp = 0 ; exp < NBEXPERIMENTS; exp++){
#ifdef RINIT
        init_array_random (X, N);
#else
        init_array_sequence (X, N);
#endif
        
      
        start = _rdtsc () ;
        
        sequential_oddeven_sort (X, N) ;
     
        end = _rdtsc () ;
        experiments [exp] = end - start ;

        /* verifying that X is properly sorted */
#ifdef RINIT
        if (! is_sorted (X, N))
        {
            fprintf(stderr, "ERROR: the sequential sorting of the array failed\n") ;
            print_array (X, N) ;
            exit (-1) ;
	}
#else
        if (! is_sorted_sequence (X, N))
        {
            fprintf(stderr, "ERROR: the sequential sorting of the array failed\n") ;
            print_array (X, N) ;
            exit (-1) ;
	}
#endif
    }

    av = average_time() ;  

    double serial_cycles = (double)av/1000000;
    printf ("\n odd-even serial \t\t\t %.2lf Mcycles\n", (double)av/1000000) ;

  
    for (exp = 0 ; exp < NBEXPERIMENTS; exp++)
    {
#ifdef RINIT
        init_array_random (X, N);
#else
        init_array_sequence (X, N);
#endif
        
        start = _rdtsc () ;

        parallel_oddeven_sort (X, N) ;
     
        end = _rdtsc () ;
        experiments [exp] = end - start ;

        /* verifying that X is properly sorted */
#ifdef RINIT
        if (! is_sorted (X, N))
        {
            fprintf(stderr, "ERROR: the parallel sorting of the array failed\n") ;
            exit (-1) ;
	}
#else
        if (! is_sorted_sequence (X, N))
        {
            fprintf(stderr, "ERROR: the parallel sorting of the array failed\n") ;
            print_array (X, N) ;
            exit (-1) ;
	}
#endif
                
        
    }
    
    av = average_time() ;  

    double parallel_cycles = (double)av/1000000;
    printf ("\n odd-even parallel \t\t %.2lf Mcycles\n\n", (double)av/1000000) ;

    FILE *f = fopen("speedups_odd.txt", "a+w");

    printf(" Speedup: \t\t%f\n", serial_cycles/parallel_cycles);
    fprintf(f, "%f\n", serial_cycles/parallel_cycles);
  
    /* print_array (X, N) ; */

    /* before terminating, we run one extra test of the algorithm */
    uint64_t *Y = (uint64_t *) malloc (N * sizeof(uint64_t)) ;
    uint64_t *Z = (uint64_t *) malloc (N * sizeof(uint64_t)) ;

#ifdef RINIT
    init_array_random (Y, N);
#else
    init_array_sequence (Y, N);
#endif

    memcpy(Z, Y, N * sizeof(uint64_t));

    sequential_oddeven_sort (Y, N) ;
    parallel_oddeven_sort (Z, N) ;

    if (! are_vector_equals (Y, Z, N)) {
        fprintf(stderr, "ERROR: sorting with the sequential and the parallel algorithm does not give the same result\n") ;
        exit (-1) ;
    }


    free(X);
    free(Y);
    free(Z);
    
    printf("================================================\n\n");
}
