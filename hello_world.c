// #include <stdio.h>

// int main(int argc, char const *argv[]) {
//     int a;
//     a = 0;
//     for(int i = 0; i < 10; i++)
//     	a += i;
        
//     printf("Value of a: %d\n", a);

//     return 0;
// }

// #include <stdio.h>
// #include <stdint.h>

int main(void) {
    // int a;
    // a = 0;

    int volatile *NetworkInterface = (int volatile *) 0xa0000000;

    *NetworkInterface = 100;

    // for(int i = 0; i < 10; i++)
    // 	a += i;
        
    // printf("Value of a: %d\n", a);

    return 0;
}
