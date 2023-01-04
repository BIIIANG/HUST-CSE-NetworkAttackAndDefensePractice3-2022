#include <stdio.h>
#include <unistd.h>
#include <time.h>
 
int main()
{
    while(1){
        sleep(5);
        printf("original: %ld\n", time(0));
    }
    return 0;
}