//DAVID YANG 23873609
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <limits.h>
#include <math.h>


//Set global variable to store data
int SUM[18];
int ARITHMETIC_AVERAGE[18];
int ROOT_SUM_SQUARES[18];

int getstringlen(char* str){
    int i=0;
    while(str[i]!=NULL){
        i++;
    }return i;
}
//prepare argument structure that will be passed to pthread_creat
struct parameter {
    int* arg1;//store array
    int arg2;//store thread number
};

//method to find the min number in an array
int findMin(int* ary,int num){
    int temp=INT_MAX;
    for(int i=0;i<num; i++){
        if(ary[i]<temp){
            temp=ary[i];
        }
    }return temp;
}

//first method of worker thread to find sum
void* dosum(void* param){
    struct parameter *par=(struct parameter *)param;//cast argument back to structure parameter
    for(int i=par->arg2*1000; i<(par->arg2+1)*1000; i++){
        SUM[par->arg2]+=par->arg1[i];//add-up all numbers
    }
    printf("worker thread terminating.\n");//for debug, make sure one worker thread reaches its end.
    pthread_exit(0);
}

//second one, to find ROOT_SUM_SQUARES
void* dorrs(void* param){
    struct parameter *par=(struct parameter *)param;//cast back to structure parameter
    for(int i=par->arg2*1000; i<(par->arg2+1)*1000; i++){
        ROOT_SUM_SQUARES[par->arg2]+=par->arg1[i]*par->arg1[i];//first I add all square number to array
    }
    ROOT_SUM_SQUARES[par->arg2]=sqrt(ROOT_SUM_SQUARES[par->arg2]);//then use spry() find the result
    printf("worker thread terminating.\n");
    pthread_exit(0);
}

//third one. to find average of numbers
void* doavg(void* param){
    struct parameter *par=(struct parameter *)param;//cast to structure parameter
    for(int i=par->arg2*1000; i<(par->arg2+1)*1000; i++){
        ARITHMETIC_AVERAGE[par->arg2]+=par->arg1[i];//add the number
    }
    ARITHMETIC_AVERAGE[par->arg2]=ARITHMETIC_AVERAGE[par->arg2]/1000;//find average.
    printf("worker thread terminating.\n");
    pthread_exit(0);
}

//child thread method
void* dothing (void* input){
    //initialize 3 threads.
    pthread_t ptid[3];
    pthread_attr_t pattr[3];
    //one thread for SUM
    pthread_attr_init(&pattr[0]);
    pthread_create(&ptid[0], &pattr[0],dosum,input);
    //thread for Average
    pthread_attr_init(&pattr[1]);
    pthread_create(&ptid[1], &pattr[1],doavg,input);
    //thread for Root sum square
    pthread_attr_init(&pattr[2]);
    pthread_create(&ptid[2], &pattr[2],dorrs,input);
    //join 3 threads
    for(int i=0; i<3;i++){
        pthread_join(ptid[i],NULL);
    }
    pthread_exit(0);//terminating
}

//main function
int main(int argc, char* argv[])
{
    //initialize global variable.
    for(int k=0; k<18; k++){
        SUM[k]=0;
        ROOT_SUM_SQUARES[k]=0;
        ARITHMETIC_AVERAGE[k]=0;
    }
    //read argv[];
    char* input;
    char* output;
    input=argv[1];
    output=argv[2];
    //open file.
    FILE *in = fopen(input,"r");
    if(in == NULL){//check if file opened.
        perror("Unable to open file!");
        exit(1);
    }
    int* ary = (int*)malloc(18000*sizeof(int));//dynamically allocate an int array
    char* line[18];//used to store line read, length is setted to be greater than any line in file
    //initialize value will be used in loop;
    int tnum=0;//thread number
    int count=0;//location in array
    //initialize tid and attr for thread
    pthread_t tid[18];
    pthread_attr_t attr[18];
    //read one line per time, until file is empty.
    while(fgets(line,18,in)!=NULL){
        ary[count]=atoi(line);//change char* to int
        count++;
    }
    fclose(in);//close input file
    //initialize arguments that will be passed to thread function
    struct parameter par[18];
    for(int i=0;i<18;i++){
        par[i].arg1=ary;//first is array
        par[i].arg2=i;//second is thread number
    }
    //create 18 threads.
    while(tnum<18){
        pthread_attr_init(&attr[tnum]);
        pthread_create(&tid[tnum], &attr[tnum],dothing,(void*)&par[tnum]);//cast structure to void* and pass it to method
        tnum++;
    }
    //free the memory of 18000 numbers
    free(ary);
    //join 18 threads.
    for(int i=0;i<18;i++){
        pthread_join(tid[i],NULL);
    }

    //output part.
    int out=open(argv[2],O_WRONLY,0);//open file
    for(int j=0;j<18;j++){//build and output strings to file
        char l[128];
        snprintf(l,128,"Worker Child Pthread Number = %d: \t Arithmetic Average = %d \t Sum = %d \t  Root of the Sum of Squares = %d \n",j,ARITHMETIC_AVERAGE[j],SUM[j],ROOT_SUM_SQUARES[j]);
        write(out,l,getstringlen(l));
    }
    //initialize char* to store output strings
    char minSum[60], minAvg[60], minRss[60];
    //int to store min results
    int ms=findMin(SUM,18);
    int ma=findMin(ARITHMETIC_AVERAGE,18);
    int mr=findMin(ROOT_SUM_SQUARES,18);
    //build 3 strings
    snprintf(minSum,128,"Min of sum = %d \n",ms);
    snprintf(minAvg,128,"Min of average = %d \n",ma);
    snprintf(minRss,128,"Min of the root of the sum of the squares  = %d \n",mr);
    //output 3 strings
    write(out,minSum,getstringlen(minSum));
    write(out,minAvg,getstringlen(minAvg));
    write(out,minRss,getstringlen(minRss));
    close(out);//close output file
    return 0;
 }
