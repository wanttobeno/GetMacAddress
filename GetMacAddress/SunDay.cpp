#include "SunDay.h"


unsigned char* SUNDAY(unsigned char *lpBase, unsigned char *lpCmp,long len,long maxSize)
{
	size_t temp[256]; 
	size_t *shift = temp; 
	size_t i, patt_size = len, text_size = maxSize; 

	for( i=0; i < 256; i++ ) 
		*(shift+i) = patt_size+1; 

	for( i=0; i < patt_size; i++ ) 
		*(shift+unsigned char(*(lpCmp+i))) = patt_size-i; 
	size_t limit = text_size-patt_size+1; 
	for( i=0; i < limit; i += shift[ lpBase[i+patt_size] ] ) 
	{ 
		if( lpBase[i] == *lpCmp ) 
		{ 
			unsigned char *match_text = lpBase+i+1; 
			size_t match_size = 1; 
			do 
			{
				if( match_size == patt_size )
				{
					return lpBase+i;
				}
			} 
			while( (*match_text++) == lpCmp[match_size++] ); 
		} 
	} 
	return NULL;
}
