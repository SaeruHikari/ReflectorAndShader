struct TemplateD;

template <typename T>
struct Templatee
{
    void dpd();
    T t;
};

int main()
{
    Templatee<int> ti;
    ti.t = 1;
    ti.dpd();
    return 0;
}