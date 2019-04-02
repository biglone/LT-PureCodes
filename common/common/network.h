#ifndef _NET_WORK_H_
#define _NET_WORK_H_

static bool IsBigEndian()  
{  
	const int n = 1;  
	if(*(char *)&n)  
	{  
		return false;  
	}  
	return true;  
} 

#define swap64(val) (((val) >> 56) |\
	(((val) & 0x00ff000000000000ll) >> 40) |\
	(((val) & 0x0000ff0000000000ll) >> 24) |\
	(((val) & 0x000000ff00000000ll) >> 8)   |\
	(((val) & 0x00000000ff000000ll) << 8)   |\
	(((val) & 0x0000000000ff0000ll) << 24) |\
	(((val) & 0x000000000000ff00ll) << 40) |\
	(((val) << 56)))

#define hton64(val) IsBigEndian() ? val : swap64(val)
#define ntoh64(val) hton64(val)

#endif
