// Compute container offsets
//
// Copyright:
//   Matthew Brewer (mbrewer@smalladventures.net)
//   2009-09-22

#include <stddef.h>

#ifndef OFFSET_H
#define OFFSET_H

#define OFFSET(type, name) ((ptrdiff_t)&(((type*)(0))->name))
#define GET_CONTAINER(var, con_type, name) ((con_type*)(((char*)(var))-OFFSET(con_type, name)))

#endif
