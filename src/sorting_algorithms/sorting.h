#ifndef __SORTING_H__
#define __SORTING_H__


#define NBEXPERIMENTS    10
extern long long unsigned int experiments [NBEXPERIMENTS] ;


/* utility functions */
void init_array_sequence (uint64_t *T, uint64_t size);
void init_array_random (uint64_t *T, uint64_t size);
void print_array (uint64_t *T, uint64_t size);
int is_sorted_sequence (uint64_t *T, uint64_t size);
int is_sorted (uint64_t *T, uint64_t size);
int are_vector_equals (uint64_t *T1, uint64_t *T2, uint64_t size);


/* return the average time in cycles over the values stored in
 * experiments vector */
uint64_t average_time();








#endif /* __SORTING_H__ */
