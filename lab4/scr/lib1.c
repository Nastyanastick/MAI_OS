#include <math.h>

float sin_integral(float a, float b, float e) {
    float integral = 0.0;
    float x = a;
    while (x < b) {
        float next_x = x + e;
        if (next_x > b) next_x = b;
        float mid = (x + next_x) / 2;
        integral += sin(mid) * e;
        x = next_x;
    }
    return integral;
}

float cos_derivative(float a, float dx) {
    return (cos(a + dx) - cos(a)) / dx;
}