#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "stdint.h"
#include "string.h"

void __attribute__((noinline)) __parsec_roi_begin() { printf("APP: enter roi\n"); };
void __attribute__((noinline)) __parsec_roi_end() { printf("APP: exit roi\n"); };

typedef struct params{
    uint32_t N_chain;
    uint32_t N_parallel;
    uint64_t array_size;
    uint16_t offset;
} Params;

Params parse_params(int argc, char** argv)
{
    if (argc < 5) {
        fprintf(stderr, "Parameters: N_chain N_parallel array_size offset\n");
        abort();
    }
    Params params;
    params.N_chain = atoi(argv[1]);
    params.N_parallel = atoi(argv[2]);
    params.array_size = atoi(argv[3]);
    params.offset = atoi(argv[4]);
    return params;
}

void calculate_sum(float* sum, float* array, uint64_t size, uint8_t offset)
{
    for (uint64_t i = 0; i < size; i += offset) {
        *sum += array[i];
    }

}

float** init_arrays(unsigned int N_arrays, uint64_t array_size)
{
    float* data = malloc(N_arrays * array_size * sizeof(float));
    assert(data != NULL);
    memset(data, 1, N_arrays * array_size);
//    for (uint64_t i = 0; i < N_arrays * array_size; ++i) {
//        data[i] = i;
//    }

    float** pointers = malloc(N_arrays * sizeof(float*));
    assert(pointers != NULL);

    for (uint32_t i = 0; i < N_arrays; ++i) {
        pointers[i] = &data[i * array_size];
    }
    return pointers;
}


int main(int argc, char** argv)
{
    Params params = parse_params(argc, argv);

    float** chain_arrays = init_arrays(params.N_chain, params.array_size);
    float** parallel_arrays = init_arrays(params.N_parallel, params.array_size);
    assert(chain_arrays != NULL);
    assert(parallel_arrays != NULL);

    __parsec_roi_begin();

    float chain_sum = 0;
    float parallel_sum = 0;

    for (int i = 0; i < params.N_chain; ++i) {
        #pragma omp task label(chain_task) in([params.array_size](chain_arrays[i])) out(chain_sum)
        calculate_sum(&chain_sum, chain_arrays[i], params.array_size, params.offset);
    }

    for (int i = 0; i < params.N_parallel; ++i) {
        #pragma omp task label(parallel_task) in([params.array_size](parallel_arrays[i])) concurrent(parallel_sum)
        calculate_sum(&parallel_sum, parallel_arrays[i], params.array_size, params.offset);
    }

    #pragma omp taskwait

    __parsec_roi_end();

    fprintf(stdout, "%2.3f\n", chain_sum + parallel_sum);
    return 0;
}
