#ifndef	STDARG_H_
#define	STDARG_H_

/*---------------------------------INCLUDES--------------------------------*/
/*---------------------------------CONSTANTS-------------------------------*/

#ifndef VA_LIST_
#define VA_LIST_
typedef char *va_list;
#endif

#define va_start(list, start) ((void)((list) = (sizeof(start)<4 ? \
	(char *)((int *)&(start)+1) : (char *)(&(start)+1))))
#define va_end(list) ((void)0)
#define va_arg(list, mode) *(mode *)(&(list = (char*)(((int)list + 7)&~3U))[-4])

#endif
