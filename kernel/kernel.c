#include "ts7200.h"
#include "sched.h"

#define FOREVER for( ; ; )


void initialize( ) {


}



void kernel( ) {
  
  

  initialize( );  // includes starting the first user task
  FOREVER {
    request = getNextRequest( );
    handle( request );
  }


}

int main(int argc, char const *argv[])
{
	kernel();
	return 0;
}