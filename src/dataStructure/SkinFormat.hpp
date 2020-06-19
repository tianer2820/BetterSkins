#if !defined(SKINFORMAT_H)
#define SKINFORMAT_H

#include <string>
#include <vector>
using namespace std;

struct TextBlock
{
    int left;
    int top;
    int width;
    int height;
    string name;
};

typedef vector<TextBlock> SkinFormat;


#endif // SKINFORMAT_H
