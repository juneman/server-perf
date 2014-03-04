#ifndef __IOLOG_H
#define __IOLOG_H

/* debug support */
#ifdef NM_DEBUG
#define D(format, ...)					\
	fprintf(stderr, "%s [%d] " format "\n",		\
	__FUNCTION__, __LINE__, ##__VA_ARGS__)

#else
#define D(format, ...)	do {} while(0)
#endif







#endif // end of __IOLOG_H
