#include <stdio.h>
#include <omp.h>
#include <stdint.h>
#include <string.h>

#include <x86intrin.h>

#include "sorting.h"


int compare_function(const void *x, const void *y)
{
  /* Compare function for qsort() */

  return (*(uint64_t*)x - *(uint64_t*)y);
}


/* 
   Merge two sorted chunks of array T!
   The two chunks are of size size
   First chunck starts at T[0], second chunck starts at T[size]
*/
void merge (uint64_t *T, const uint64_t size)
{
  uint64_t *X = (uint64_t *) malloc (2 * size * sizeof(uint64_t)) ;
  
  uint64_t i = 0 ;
  uint64_t j = size ;
  uint64_t k = 0 ;
  
  while ((i < size) && (j < 2*size))
  {
    if (T[i] < T [j])
	  {
      X [k] = T [i] ;
      i = i + 1 ;
	  }
    else
    {
      X [k] = T [j] ;
      j = j + 1 ;
    }
    k = k + 1 ;
  }

  if (i < size)
  {
    for (; i < size; i++, k++)
    {
      X [k] = T [i] ;
    }
  }
  else
  {
    for (; j < 2*size; j++, k++)
    {
      X [k] = T [j] ;
    }
  }
  
  memcpy (T, X, 2*size*sizeof(uint64_t)) ;
  free (X) ;
  
  return ;
}

// ------------------------------------------------

/* 
   quick sort -- sequential, parallel -- 
*/

void sequential_quicksort(uint64_t *T, const uint64_t size)
{
  // Just calls the built-in qsort function
  qsort(T, size, sizeof(uint64_t), compare_function);
  return;
}

void parallel_quicksort(uint64_t *T, const uint64_t size)
{
  uint64_t chunk_size, trim_size;
  uint64_t i;

  chunk_size = size/omp_get_max_threads();

  #pragma omp parallel for schedule(static)
    for (i = 0; i < size; i+=chunk_size)
    {
      sequential_quicksort(T+i, chunk_size);
    }
  // printf("After seq sorting:\n");
  // print_array(T, size);

  // The outer loop keeps on doubling the size of chunk (`trim_size`),
  // which is taken for merging
  for (trim_size = chunk_size; trim_size <= size/2; trim_size+=trim_size)
  {
    // Merge 2 consecutive chunks for whole array
    #pragma omp parallel for schedule(static), shared(T, i, trim_size)
      for (i = 0; i < size; i+=2*trim_size)
      {
        merge(T+i, trim_size);
      }
    
  }
  
  // printf("After merging:\n");
  // print_array(T, size);

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
      fprintf (stderr, "quicksort.run N \n") ;
      exit (-1) ;
  }

  uint64_t N = 1 << (atoi(argv[1])) ;
  /* the array to be sorted */
  uint64_t *X = (uint64_t *) malloc (N * sizeof(uint64_t)) ;

  printf(" --> Sorting an array of size %lu (2^%u)\n", N, atoi(argv[1]));
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
      
      sequential_quicksort (X, N) ;
      
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
  printf ("\n Quicksort serial \t%.2lf Mcycles\n\n", serial_cycles) ;


  for (exp = 0 ; exp < NBEXPERIMENTS; exp++)
  {
      #ifdef RINIT
              init_array_random (X, N);
      #else
              init_array_sequence (X, N);
      #endif
      
      start = _rdtsc () ;
      
      parallel_quicksort (X, N) ;
        
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
          print_array (X, N);
          exit (-1) ;
      }
      #endif
              
  }
  
  av = average_time() ;  
  double parallel_cycles = (double)av/1000000;
  printf (" Quicksort parallel \t%.2lf Mcycles\n\n", (double)av/1000000) ;
  
  FILE *f = fopen("speedups.txt", "a+w");

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

  sequential_quicksort (Y, N) ;
  parallel_quicksort (Z, N) ;

  if (! are_vector_equals (Y, Z, N)) {
      fprintf(stderr, "ERROR: sorting with the sequential and the parallel algorithm does not give the same result\n") ;
      exit (-1) ;
  }


  free(X);
  free(Y);
  free(Z);

  printf("================================================\n\n");
    
}
