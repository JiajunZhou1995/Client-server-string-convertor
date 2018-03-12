#include "procSignature.h"
#include <string>
using namespace std;

// needed to overload in order to assign this as the key of the map
bool operator < (const procSignature &f, const procSignature &g)
{
    if(f.name == g.name)
    {
        int i = 0;
        int f_argType = f.argTypes[i];
        int g_argType = g.argTypes[i];

        while(f_argType!= 0 && g_argType != 0)
        {
            int f_arg_size = f_argType & ((1 << 16) - 1);
            int g_arg_size = g_argType & ((1 << 16) - 1);

            if (((f_argType & 0xFFFF0000) != (g_argType & 0xFFFF0000)) || (f_arg_size == 0 && g_arg_size != 0) || (f_arg_size != 0 && g_arg_size == 0))
            {
                return f_argType < g_argType;
            }
            i++;
            f_argType = f.argTypes[i];
            g_argType = g.argTypes[i];
        }

        return f_argType != 0;
    }
    else
    {
        return f.name < g.name;
    }
}
