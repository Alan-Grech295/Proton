cbuffer CBuf : register(b0)
{
    int clearCol;
    int3 padding;
};

int main() : SV_TARGET
{
    return clearCol;
}