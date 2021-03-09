#include <stdio.h>
#include <omp.h>
#include <stdint.h>
#include <string.h>

#include <x86intrin.h>

#include "sorting.h"

/* 
   bubble sort -- sequential, parallel -- 
*/


void sequential_bubble_sort (uint64_t *T, const uint64_t size)
{
    /* sequential implementation of bubble sort */ 

    uint64_t i, temp;
    uint64_t sorted;

    do
    {
        sorted = 1;
        for(i = 0; i < size - 1; i++)
        {
            if (T[i] > T[i+1])
            {
                temp = T[i+1];
                T[i+1] = T[i];
                T[i] = temp;

                sorted = 0;
            }
        }
        
    } while (sorted == 0);
    
    return ;
}

int sequential_bubble_onepass(uint64_t *T, const uint64_t size)
{
    uint64_t i, temp;
    uint64_t flag = 0;
    // uint64_t sorted;

    for(i = 0; i < size - 1; i++)
    {
        if (T[i] > T[i+1])
        {
            flag = 1;

            temp = T[i+1];
            T[i+1] = T[i];
            T[i] = temp;

            // sorted = 0;
        }
    }
    if(flag == 1){ return 0; }  //=> sorted-->0 (this chunk was modified)
    else return 1;              //=> sorted-->1 (this chunk was untouched)

}

void parallel_bubble_sort (uint64_t *T, const uint64_t size)
{
    /* parallel implementation of bubble sort */

    uint64_t temp, sorted, i;
    uint64_t ch_sz, ret_val;
    
    ch_sz = size / omp_get_max_threads();  
    do
    {
        // print_array (T, size) ;

        sorted = 1;
        #pragma omp parallel for schedule(static), private(ret_val)
        for (i=0; i<size; i+=ch_sz)
        {
            ret_val = sequential_bubble_onepass(T+i, ch_sz);
            if(ret_val==0) { sorted = 0; }

            // NOTE: We cannot simply directly assign the returned value to
            // `sorted` variable, because there will be multiple returned
            // values due to multiple threads.
            // So we filter through all the returned values and only
            // use it if it returns 0, which means the array was just modified,
            // (by one of the threads) and hence the array may not be fully sorted
            // which implies go through do-while again, in case the below for loop
            // (border checking) does not make sorted = 0.

            // TLDR: Even if one chunk is unsorted, `sorted` is assigned 0.
        }    
        #pragma omp parallel for schedule(static), private(temp)
        for (i=ch_sz; i<size; i+=ch_sz)
        {
            if (T[i] < T[i-1])
            {
                temp = T[i-1];
                T[i-1] = T[i];
                T[i] =  temp;
                sorted = 0;
            }
        }   
        
    }while (sorted == 0);

    return;
}


int main (int argc, char **argv)
{
    uint64_t start, end;
    uint64_t av ;
    unsigned int exp ;

    printf("================================================\n");
    printf(" Max number of threads: %d \n", omp_get_max_threads());

    /* the program takes one parameter N which is the size of the array to
       be sorted. The array will have size 2^N */
    if (argc != 2)
    {
        fprintf (stderr, "bubble.run N \n") ;
        exit (-1) ;
    }

    uint64_t N = 1 << (atoi(argv[1])) ;
    /* the array to be sorted */
    uint64_t *X = (uint64_t *) malloc (N * sizeof(uint64_t)) ;

    printf(" --> Sorting an array of size %lu\n",N);
    #ifdef RINIT
    printf("--> The array is initialized randomly\n");
    #endif
    

    for (exp = 0 ; exp < NBEXPERIMENTS; exp++)
    {
        #ifdef RINIT
            init_array_random (X, N);
        #else
            init_array_sequence (X, N);
        #endif
        
        start = _rdtsc () ;
        
        sequential_bubble_sort (X, N) ;
     
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
    printf ("\n bubble serial \t\t%.2lf Mcycles\n\n", serial_cycles) ;

  
    for (exp = 0 ; exp < NBEXPERIMENTS; exp++)
    {
        #ifdef RINIT
            init_array_random (X, N);
        #else
            init_array_sequence (X, N);
        #endif
        
        start = _rdtsc () ;

        parallel_bubble_sort (X, N) ;
     
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
    printf (" bubble parallel \t%.2lf Mcycles\n\n", parallel_cycles) ;
  
    printf(" Speedup: \t\t%f\n", serial_cycles/parallel_cycles);
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

    sequential_bubble_sort (Y, N) ;
    parallel_bubble_sort (Z, N) ;

    if (! are_vector_equals (Y, Z, N)) {
        fprintf(stderr, "ERROR: sorting with the sequential and the parallel algorithm does not give the same result\n") ;
        exit (-1) ;
    }


    free(X);
    free(Y);
    free(Z);

    printf("================================================\n\n");
    
}
