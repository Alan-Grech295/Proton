cbuffer ObjectCBuf : register(b1)
{
    uint EntityID;
    int3 padding;
};

int main() : SV_Target1
{
    return EntityID;
}