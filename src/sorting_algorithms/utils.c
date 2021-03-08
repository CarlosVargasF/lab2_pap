#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "sorting.h"

long long unsigned int experiments[NBEXPERIMENTS];

uint64_t average_time (void)
{
    unsigned int i ;
    uint64_t s = 0 ;

    for (i = 0; i < NBEXPERIMENTS; i++)
    {
        s = s + experiments [i] ;
    }

    return s / NBEXPERIMENTS ;
}


void init_array_sequence (uint64_t *T, uint64_t size)
{
    uint64_t i;

    for (i = 0 ; i < size ; i++)
    {
        T [i] = size - i ;
    }
}

void init_array_random (uint64_t *T, uint64_t size)
{
    uint64_t i;

    srand(time(NULL));

    for (i = 0 ; i < size ; i++)
    {
        T [i] = rand() % size;
    }
}



void print_array (uint64_t *T, uint64_t size)
{
    uint64_t i ;

    for (i = 0 ; i < size ; i++)
    {
        printf ("%d ", T[i]) ;
    }
    printf ("\n") ;
}

/* test if the array is sorted assuming that the elements to be sorted
 * is a sequence of consecutive values */
int is_sorted_sequence (uint64_t *T, uint64_t size)
{
    uint64_t i ;
  
    for (i = 1 ; i < size ; i++)
    {
        /* test designed specifically for our usecase */
        if (T[i-1] +1  != T [i] )
            return 0 ;
    }
    return 1 ;
}

/* test if the array is sorted */
int is_sorted (uint64_t *T, uint64_t size)
{
    uint64_t i ;
  
    for (i = 1 ; i < size ; i++)
    {
        if (T[i-1] > T [i] ){
            return 0 ;
        }
    }
    return 1 ;
}

/* test if T1 and T2 are equal */
int are_vector_equals (uint64_t *T1, uint64_t *T2, uint64_t size)
{
    uint64_t i ;
  
    for (i = 0 ; i < size ; i++)
    {
        if (T1[i] != T2 [i] ){
            return 0 ;
        }
    }
    return 1 ;
    
}
