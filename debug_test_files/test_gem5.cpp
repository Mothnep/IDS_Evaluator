#include <iostream>
#include <cstdio>

int main() {
    // Use printf instead of cout for better compatibility
    printf("Hello from ARM binary on gem5!\n");
    printf("Basic arithmetic test: 2 + 2 = %d\n", 2 + 2);
    printf("Float test: 3.14 * 2 = %.2f\n", 3.14 * 2);
    printf("Test completed successfully!\n");
    return 0;
}