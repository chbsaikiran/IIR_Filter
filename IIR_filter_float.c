#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#define NUM_SECTIONS 2  // Number of biquad sections
#define BLOCK_SIZE 100  //fs*frame_size_in_secs = 1000hz*0.1sec

// Structure to hold coefficients and state for each biquad section
typedef struct {
    float b0, b1, b2;   // Numerator coefficients
    float a1, a2;       // Denominator coefficients
    float state[4];     // State variables
} Biquad;

void biquad_filter_block(Biquad *biquad, float *input, float *output, int block_size) {
    for (int n = 0; n < block_size; n++) {
        float yn = biquad->b0 * input[n] + biquad->state[0]*biquad->b1+biquad->b2*biquad->state[1];
        biquad->state[1] = biquad->state[0];
        biquad->state[0] = input[n];

        yn -= (biquad->state[2]*biquad->a1+biquad->a2*biquad->state[3]);
        biquad->state[3] = biquad->state[2];
        biquad->state[2] = yn;

        output[n] = yn;
    }
}

void process_block(float *input, float *output, int block_size, Biquad *biquads, int num_sections) {
    float temp[BLOCK_SIZE] = { 0 };
    float *in_ptr = input;
    float *out_ptr = temp;

    for (int i = 0; i < num_sections; i++) {
        biquad_filter_block(&biquads[i], in_ptr, out_ptr, block_size);
        in_ptr = out_ptr;
        out_ptr = (out_ptr == temp) ? output : temp;  // Alternate between temp and output
    }

    // If the number of sections is even, the final output will be in the temp buffer
    if (num_sections % 2 != 0) {
        for (int n = 0; n < block_size; n++) {
            output[n] = temp[n];
        }
    }
}

int main() {
    Biquad biquads[NUM_SECTIONS] = {
        // Example coefficients for each biquad section
        {0.018563f,0.037126f,0.018563f, -0.672741f,0.144535f, {0, 0,0,0}},
        {1.0f,2.0f,1.0f, -0.897658f,0.527187f, {0, 0,0,0}}
    };
    int i;

    float input[1000] = { 0 };  // Input block of samples
    float output[1000]; // Output block of samples
    FILE *fin = fopen("..\\signal.bin","rb");
    FILE *fout = fopen("..\\filtered_msvc_float.bin","wb");

    // Fill input with some example data
    fread(input,1000,sizeof(float), fin);

    // Process the input block
    for(i = 0 ; i < 1000; i+=BLOCK_SIZE)
    {
        process_block(&input[i], &output[i], BLOCK_SIZE, biquads, NUM_SECTIONS);
    }

    // Print the output samples
    fwrite(output,1000,sizeof(float), fout);
    fclose(fin);
    fclose(fout);

    return 0;
}
