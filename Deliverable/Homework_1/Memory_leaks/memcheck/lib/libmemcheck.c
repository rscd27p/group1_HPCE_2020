//Compile with: gcc -shared -fPIC -o libmemcheck.so libmemcheck.c -ldl

#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>


// The counters below will save the number of times malloc and free are called
int malloc_count = 0;
int free_count = 0;
int count_enabled = 0;

// Constructor and destructor are defined
void __attribute__((constructor)) enable_count();
void __attribute__((destructor)) report_results();


// Constructor is used to enable the count. In this way we ignore calls before this library is fully loaded
void enable_count(){
    count_enabled = 1;
}
// Destructor is called when library is unloaded and will be useful to print the final results to user
void report_results() 
{ 
    printf("\nAnalysis finished!\n");
    printf("Memory allocations: %d\n", malloc_count);
    printf("Memory free: %d\n", free_count);
    printf("Total memory leaks found: %d\n", malloc_count - free_count);
} 



// This function will get called when invoking malloc
void *malloc(size_t size)
{
    // Call the real malloc to keep expected functionality
    void *(*libc_malloc)(size_t size) = dlsym(RTLD_NEXT, "malloc");
    // Increase the malloc count if enabled
    if(count_enabled){
      malloc_count++;
    }
    //fprintf(stderr, "malloc %d\n", malloc_count);
    return libc_malloc(size);
}


// This function will get called when invoking free
void free(void *ptr)
{
    // Call the real free to keep expected functionality
    void (*libc_free)(void *ptr) = dlsym(RTLD_NEXT, "free");
    // Increase the free count if enabled
    if(count_enabled){
      free_count++;
    }
    //fprintf(stderr, "free %d\n", free_count);
    libc_free(ptr);
}


