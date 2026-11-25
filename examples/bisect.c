// bisect.c   gcc -O2 bisect.c -o bisect_c
#include <stdio.h>
#include <time.h>
int main(void){
    double arg=7.0, eps=1e-9, mid=0.0;
    long n=0, outer=1000000;
    struct timespec a,b; clock_gettime(CLOCK_MONOTONIC,&a);
    for(long j=0;j<outer;j++){
        double f=1000.0, xl=0.0, xr=3.0;
        while(f>eps||f<-eps){
            mid=(xl+xr)/2.0; f=mid*mid-arg;
            if(f>0.0) xr=mid; else xl=mid;
            n++;
        }
    }
    clock_gettime(CLOCK_MONOTONIC,&b);
    double dt=(b.tv_sec-a.tv_sec)+(b.tv_nsec-a.tv_nsec)*1e-9;
    printf("C code: sqrt(7)=%.15f eps=%.15f iter=%ld time=%.3fs  time/iter=%.1f ns/iter\n",mid,eps,n,dt,dt/n*1e9);
}
