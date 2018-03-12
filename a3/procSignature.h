#ifndef __PROCSIGNATURE_H_
#define __PROCSIGNATURE_H_

#include <string>
using namespace std;
struct procSignature {
    string name;
    int* argTypes;
};

bool operator < (const procSignature &ps1, const procSignature &ps2);

#endif
