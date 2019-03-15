#ifndef I18N_H_
#define I18N_H_

#ifdef MINICOMPUTER_I18N
#include <libintl.h>
#include <locale.h>
#define _(x) gettext(x)
#else
#define _(x) (x)
#endif

#endif  // I18N_H_
