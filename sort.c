#include <stdio.h>

int main () {
    int list[] = {10,3,7,80,48,1,94,89,4,8,366,6,8,986,3,7,88,63,577,45,356,35,45,7,0,23,89,-2,47,68,99,33,67,49,9};

    int length = sizeof(list) / sizeof(int);

    for (int i = 0; i < length; i++) {
        for (int j = i + 1; j < length; j++) {
            if (list[i] > list[j]) {
                int temp = list[i];
                list[i] = list[j];
                list[j] = temp;
            }
        }
    }

    printf ("Sorted list: ");
    for (int i = 0; i < length; i++) {
        printf ("%d, ", list[i]);
    }

    return 0;
}