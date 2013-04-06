#include <userspace.h>

// -------------------------------------------------------------------
// Math Lib
// This file contains math supporting functions
// -------------------------------------------------------------------

/*
	get_abs_value.
	- Returns the absolute value of the number provided. 
*/
inline int get_abs_value( int num ){
	if ( num < 0 )
		num *= -1;

	return num; 
}