auto TestBinary() {
    // binary op
    int n = 0 + 2 - 56;

    // binary assign ops
    int m = n += 65;
    int x = n -= 65;
    int xx = n *= 65;
    int yy = n /= 65;
    int ww = n %= 65;

    int v = 5;
    int complex = v += 5;

    return m + x + xx + yy + ww + complex;
}

int main()
{
    TestBinary();
    return 0;
}