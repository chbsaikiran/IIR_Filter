#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define NUM_SECTIONS 2  // Number of biquad sections
#define BLOCK_SIZE 100  //fs*frame_size_in_secs = 1000hz*0.1sec
#define Q31_SHIFT 31
#define Q31_MAX 0x7FFFFFFF  // Maximum positive value for Q31
#define Q31_MIN 0x80000000  // Minimum negative value for Q31
#define Q63_MAX 0x7FFFFFFFFFFFFFFF  // Maximum positive value for Q63
#define Q63_MIN 0x8000000000000000  // Minimum negative value for Q63

// Structure to hold coefficients and state for each biquad section
typedef struct {
    int b0, b1, b2;   // Numerator coefficients
    int a1, a2;       // Denominator coefficients
    int state[4];     // State variables
} Biquad;

int64_t fixed_point_multiply(int32_t a, int32_t b) {
    // Perform the multiplication, resulting in a 64-bit value
    int64_t temp = (int64_t)a * (int64_t)b;
    return temp;
}
// Function to perform 64-bit fixed-point addition with saturation
int64_t fixed_point_add(int64_t a, int64_t b) {
    int64_t result;

    // Perform the addition
    result = a + b;

    // Check for overflow and saturate if necessary
    if (a > 0 && b > 0 && result < 0) {
        result = Q63_MAX;  // Positive overflow
    }
    else if (a < 0 && b < 0 && result > 0) {
        result = Q63_MIN;  // Negative overflow
    }

    return result;
}

void biquad_filter_block(Biquad *biquad, int32_t *input, int32_t *output, int32_t block_size,int32_t qInt) {
    for (int32_t n = 0; n < block_size; n++) {
        int64_t t1, t2, t3, t4, t5;
        int64_t yn;
        t1 = (fixed_point_multiply(biquad->b0, input[n])) >> (qInt); //Q1.31 * Q1.31 = Q62 ; // Q2.30 * Q1.31 = Q61
        t2 = (fixed_point_multiply(biquad->b1, biquad->state[0])) >> (qInt);
        t3 = (fixed_point_multiply(biquad->b2, biquad->state[1])) >> (qInt);
        t4 = (fixed_point_multiply(biquad->a1, biquad->state[2])); //Q1.31 * Q1.31 = Q62 ; //Q1.31 * Q4.28 = Q59
        t5 = (fixed_point_multiply(biquad->a2, biquad->state[3]));
        yn = fixed_point_add(t1, t2);
        yn = fixed_point_add(yn, t3);
        yn = fixed_point_add(yn, t4);
        yn = fixed_point_add(yn, t5);
        biquad->state[1] = biquad->state[0];
        biquad->state[0] = input[n];
        biquad->state[3] = biquad->state[2];
        yn = yn >> 31; //BIQUAD1 Q1.31 or BIQUAD2 Q4.28 
        biquad->state[2] = ((int32_t)yn);

        if (yn > Q31_MAX) {
            biquad->state[2] = Q31_MAX;
        }
        else if (yn < ((int64_t)0xffffffff80000000)) {
            biquad->state[2] = Q31_MIN;
        }
        output[n] = biquad->state[2];
    }
}

void process_block(int32_t *input, int32_t *output, int32_t block_size, Biquad *biquads, int32_t num_sections) {
    int32_t temp[BLOCK_SIZE] = { 0 };
    int32_t *in_ptr = input;
    int32_t *out_ptr = temp;

    //for (int32_t i = 0; i < num_sections; i++) {
        biquad_filter_block(&biquads[0], in_ptr, out_ptr, block_size,0);
        in_ptr = out_ptr;
        out_ptr = (out_ptr == temp) ? output : temp;  // Alternate between temp and output
        biquad_filter_block(&biquads[1], in_ptr, out_ptr, block_size, 2);
        in_ptr = out_ptr;
        out_ptr = (out_ptr == temp) ? output : temp;  // Alternate between temp and output
    //}

    // If the number of sections is even, the final output will be in the temp buffer
    if (num_sections % 2 != 0) {
        for (int32_t n = 0; n < block_size; n++) {
            output[n] = temp[n];
        }
    }
}
//max(abs(signal))
//Out[2]: 2.280642635293526

//0.018563 * (2 * *31)
//Out[3] : 39863738.957824

//0.037126 * (2 * *31)
//Out[4] : 79727477.915648

//0.018563 * (2 * *31)
//Out[5] : 39863738.957824

//0.672741 * (2 * *31)
//Out[6] : 1444700296.839168

//0.144535 * (2 * *31)
//Out[7] : 310386549.06368

//2 * *30
//Out[8] : 1073741824

//2 * (2 * *30)
//Out[9] : 2147483648

//0.897658 * (2 * *31)
//Out[10] : 1927705876.496384

//0.527187 * (2 * *31)
//Out[11] : 1132125461.938176

//np.sum(np.abs(sos[0]))-1
//Out[18]: 0.8915281535322372

//np.sum(np.abs(sos[1]))-1
//Out[19] : 5.424844844668242

int main() {
    Biquad biquads[NUM_SECTIONS] = {
        // Example coefficients for each biquad section
        //{0.018563f,0.037126f,0.018563f, -0.672741f,0.144535f, {0, 0,0,0}},
        //{1.0f,2.0f,1.0f, -0.897658f,0.527187f, {0, 0,0,0}}
        {39863739,79727478,39863739,1444700297,-310386549,{0, 0,0,0}}, //b Q1.31, a Q1.31, out Q1.31
        {1073741824,2147483647,1073741824,1927705876,-1132125462, {0, 0,0,0}} //b Q2.30, a Q1.31, out Q4.28
    };
    int32_t i;

    float input[1000] = { 0 };  // Input block of samples
    int32_t input_fxd[1000];
    int32_t output[1000]; // Output block of samples
    float output_flt[1000];
    FILE *fin = fopen("..\\signal.bin","rb");
    FILE *fout = fopen("..\\filtered_msvc_fxd.bin","wb");

    // Fill input with some example data
    fread(input,1000,sizeof(float), fin);
    for (i = 0; i < 1000; i++)
    {
        input_fxd[i] = (int32_t)(input[i] * pow(2.0f,31)); //Q1.31
    }

    // Process the input block
    for(i = 0 ; i < 1000; i+=BLOCK_SIZE)
    {
        process_block(&input_fxd[i], &output[i], BLOCK_SIZE, biquads, NUM_SECTIONS);
    }

    for (i = 0; i < 1000; i++)
    {
        output_flt[i] = (float)(((float)output[i]) / ((float)(((int32_t)1) << 28))); //Q4.28
    }

    // Print the output samples
    fwrite(output_flt,1000,sizeof(float), fout);
    fclose(fin);
    fclose(fout);

    return 0;
}
