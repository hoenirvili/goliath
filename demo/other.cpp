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