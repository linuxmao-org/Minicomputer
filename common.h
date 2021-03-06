#ifndef COMMON_H_
#define COMMON_H_

// customizable options

// debug output
//#define _DEBUG

// prefetch support with intrinsics
//#define _PREFETCH

// vectorize calculations
//#define _VECTOR

// not customizable options ----------------------------------
// amount of parameters per patch
#define _PARACOUNT 139

// how many additional settings per sound in multi to store
#define _MULTISETTINGS 6

// the version number as string
#define _VERSION "1.41"

#endif
