int main()
{
    size_t n = 10;
    int l = 0;
    int f = 0;
    for (volatile size_t i = 0; i < n; i++) {
        if (i % 2 == 0)
            l++;
        if (i % 3 == 0)
            f++;
    }

    return (l > f) ? l : f;
}

// #include <stdio.h>
// #include <windows.h>

// #pragma warning(disable : 4996)

// // a=1, b=4, c=2 -> x+y+z=3
// // a=1, b=5, c=0 -> x+y+z!=3 (nu respecta b<5)
// // a=0, b=3, c=1 -> respecta b<5 si !a si c!=0 dar x are o valoare care face
// // suma finala sa fie 1+2-2

// int main(int argc, char *argv[])
// {
//     int a, b, c;
//     scanf("%d", &a);
//     scanf("%d", &b);
//     scanf("%d", &c);

//     int x = 0, y = 0, z = 0;
//     if (!a)
//         x = -2;
//     else {
//         c = c + 10;
//         b = b - 2;
//         if (c)
//             a = (b * 3) % (c / 2);
//     }
//     if (b < 5) {
//         if (!a && c)
//             y = 1;
//         z = 2;
//     }
//     if (x + y + z != 3)
//         printf("x + y + z = %d !=3\n", x + y + z);
//     else
//         printf("x + y + z == 3\n");

//     return 0;
// }
