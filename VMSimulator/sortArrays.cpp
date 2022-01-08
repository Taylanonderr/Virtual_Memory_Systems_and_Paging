#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <iostream> 
#include <vector>
#include <string.h>
#include <time.h>
#include <string>
#include <cmath>
#include <pthread.h>         
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h> 

pthread_mutex_t lock; 
pthread_mutex_t lockSet; 
pthread_mutex_t sequential; 

using namespace std;
int const timeSize=25;

int totalSize=1024;
int byte=1024;
int const directSize=10;
int const fileSize=14;
struct dataType{
    int nodeNumber;
};

FILE*  diskFile;

int **physicalframes;
int pageTableCounter;

struct statistics{
    char *name;
    int numberOfReads;
    int numberOfWrites;
    int numberOfPageMisses;
    int numberOfPageReplacements;
    int diskPageWrite;
    int diskPageRead; 
}statisticArray;

statistics printStatistic[5];

struct virtualAddress {
    int index;
    int referenced;
    int modified;
    int absentorNot;
    int usedCounter;
    int startAdress;
    int priorityNum;
    char type;
};
virtualAddress *virtualAddressSpace;

int pageTablePrintInt=0;
int numVirtual;
char pageReplacement[10];
int numPhysical;
int numLength=0;
int **ram;
int frameSize=0;
int indexAdressSpace=0;
int indexAdressPyhsical=0;
int start=0;
int processNumber=0;
int finishControl=1;
sem_t mutex; 
sem_t mutex2; 

void set(unsigned int index, int value, char * tName); 

int get(unsigned int index, char * tName);

void* refreshReferenced(void *a){
    unsigned int mSeconds = 20;
    int returnCode,i=0;

    while(finishControl==1){
        returnCode = usleep(mSeconds);
        for(i=0;i<numVirtual;i++){
            virtualAddressSpace[i].referenced=0;
        }
    }
    pthread_exit(NULL);
}

void writeFile(int indexParameter){
    int i=0,value=0;
    int physicalIndex=virtualAddressSpace[indexParameter].index;
    int startAdress=virtualAddressSpace[indexParameter].startAdress;
    fseek(diskFile,startAdress*sizeof(int),SEEK_SET); 
    for(i=0;i<frameSize;i++){
        value=physicalframes[physicalIndex][i];
        fwrite(&value , sizeof(int) ,1 ,  diskFile ); 
    }
}


void merge(int l, int m, int r) 
{ 
    int i, j, k; 
    int n1 = m - l + 1; 
    int n2 = r - m; 
    char threadName[10]="merge";
  
    int Left[n1], Right[n2]; 
  
    for (i = 0; i < n1; i++) 
        Left[i] = get(l + i,threadName); 
    for (j = 0; j < n2; j++) 
        Right[j] = get(m + 1 + j,threadName);
  
    i = 0;
    j = 0; 
    k = l; 
    while (i < n1 && j < n2) { 
        if (Left[i] <= Right[j]) { 
            set((k),Left[i],threadName); 
            i++; 
        } 
        else { 
            set((k),Right[j],threadName);
            j++; 
        } 
        k++; 
    } 
    while (i < n1) { 
        set((k),Left[i],threadName);    
        i++; 
        k++; 
    } 
    while (j < n2) { 
        set((k),Right[j],threadName);
        j++; 
        k++; 
    } 
} 

void mergeSortFunct(int left, int right) 
{ 
    if (left < right) { 

        int m = left + (right - left) / 2; 
  
        mergeSortFunct(left, m); 
        mergeSortFunct(m + 1, right); 
  
        merge(left, m, right); 
    } 
} 

void* mergeFunction(void *a){
    pthread_mutex_lock(&sequential);
    int i=0,j=0,valueTemp=0;
    mergeSortFunct(2*numLength,(3*numLength)-1);
    for(j=0;j<numVirtual;j++){
        if(virtualAddressSpace[j].absentorNot==1 && virtualAddressSpace[j].type=='m'){
            pthread_mutex_lock(&lock);
            int indexParam=j;
            int i=0,value=0;
            int physicalIndex=virtualAddressSpace[j].index;
            int startAdress=virtualAddressSpace[j].startAdress;
            printStatistic[3].diskPageWrite++;
            fseek(diskFile,startAdress*sizeof(int),SEEK_SET); 
            for(i=0;i<frameSize;i++){
                value=physicalframes[physicalIndex][i];
                fwrite(&value , sizeof(int) , 1 , diskFile ); 
            }
            virtualAddressSpace[j].absentorNot=-2;
            pthread_mutex_unlock(&lock);
        }
    }
    pthread_mutex_unlock(&sequential);

}


void* bubleSort(void *a){
    pthread_mutex_lock(&sequential);

    int i,j, flag = 1;    
    int temp;             
    int firstNum=0;
    int secondNum=0,valueTemp=0; 
    char threadName[10]="bubble";
    int start=0;
    for (start = 0; start < numLength-1; start++)
     {
        firstNum=get(start,threadName);

        for (j = 0; j < numLength-start-1; j++)
        {
            firstNum=get(j,threadName);
            secondNum=get((j+1),threadName);           

            if (secondNum < firstNum)      
            {
                set((j+1),firstNum,threadName);
                set((j),secondNum,threadName); 
            }
        }
        firstNum=get(j,threadName);
    }
    for(j=0;j<numVirtual;j++){
        if(virtualAddressSpace[j].absentorNot==1 && virtualAddressSpace[j].type=='b'){
            pthread_mutex_lock(&lock);
            int indexParam=j;
            int i=0,value=0;
            int physicalIndex=virtualAddressSpace[j].index;
            int startAdress=virtualAddressSpace[j].startAdress;
            fseek(diskFile,(startAdress)*sizeof(int),SEEK_SET); 
            printStatistic[2].diskPageWrite++;
            for(i=0;i<frameSize;i++){
                value=physicalframes[physicalIndex][i];
                fwrite(&value , sizeof(int) , 1 , diskFile );
            }
             virtualAddressSpace[j].absentorNot=-2;
            pthread_mutex_unlock(&lock);           
        }
    }

    pthread_mutex_unlock(&sequential);
    pthread_exit(NULL);
}


int partition (int low, int high)  
{  
    char threadName[10]="quick";
    int pivot = get(high,threadName);  
    int i = (low - 1);  
    int firstNum=0,secondNum=0;

    for (int j = low; j <= high - 1; j++)  
    {  
        firstNum=get(j,threadName);
        if (firstNum <= pivot)  
        {  
            i++; 
            secondNum=get(i,threadName);
            set((i),firstNum,threadName);
            set((j),secondNum,threadName); 
        }  
    }
    firstNum=get(i + 1,threadName);
    secondNum=get(high,threadName);
    set((high),firstNum,threadName);
    set((i+1),secondNum,threadName);
    return (i + 1);  
} 

void quickSort(int low, int high){
    int stack[high - low + 1]; 
    int top = -1; 
    stack[++top] = low; 
    stack[++top] = high; 
      while (top >= 0) { 
        high = stack[top--]; 
        low = stack[top--]; 
  
        int p = partition(low,high); 

        if (p - 1 > low) { 
            stack[++top] = low; 
            stack[++top] = p - 1; 
        } 
  
        if (p + 1 < high) { 
            stack[++top] = p + 1; 
            stack[++top] = high; 
        } 
    }
}

void* quickSortFunction(void*a){
    pthread_mutex_lock(&sequential);

    int i=0,j=0,valueTemp=0;
    quickSort(numLength,(2*numLength)-1);
    for(j=0;j<numVirtual;j++){
        if(virtualAddressSpace[j].absentorNot==1 && virtualAddressSpace[j].type=='q'){
            pthread_mutex_lock(&lock);
            int indexParam=j;
            int i=0,value=0;
            int physicalIndex=virtualAddressSpace[j].index;
            int startAdress=virtualAddressSpace[j].startAdress;
            printStatistic[1].diskPageWrite++;
            fseek(diskFile,startAdress*sizeof(int),SEEK_SET); 
            for(i=0;i<frameSize;i++){
                value=physicalframes[physicalIndex][i];
                fwrite(&value , sizeof(int) , 1 , diskFile ); 

            }
            virtualAddressSpace[j].absentorNot=-2;
            pthread_mutex_unlock(&lock);            
        }
    }
    pthread_mutex_unlock(&sequential); 
    pthread_exit(NULL);
}


void* indexSort(void* arg){
    char threadName[10]="index";
    int index[numVirtual*frameSize/4];
    int firstNum=0,secondNum=0,t=0,i=0,j=0;
    for(i=3*numLength;i<=(3*numLength)+(numVirtual*frameSize/4);i++)
    {
      index[t]=i;
      t++;
    }
    for(i=0;i<=5;i++)
    {
      for(j=i+1;j<=6;j++)
      {
       int temp;
       firstNum=get(index[i],threadName);
       secondNum=get(index[j],threadName);
       if(firstNum >secondNum)
       {
        temp = index[i];
        index[i] = index[j];
        index[j] = temp;
       }
      }
    }    
    int indexParam=j;
    int value=0;

    fseek(diskFile,3*numLength*sizeof(int),SEEK_SET); 
    
    for(j=0;j<=numVirtual/4;j++)
    { 
        for(i=0;i<frameSize;i++){
            firstNum=get(index[i],threadName);
            set(i,firstNum,threadName);

            fwrite(&firstNum ,  sizeof(int) ,1 , diskFile ); 
            printStatistic[4].diskPageWrite++;
        }
    }
    virtualAddressSpace[j].absentorNot=-2;

    fseek(diskFile,3*numLength*sizeof(int),SEEK_SET); 

    for(i=0;i<=numVirtual;i++)
    { 
        if(virtualAddressSpace[j].absentorNot==1 && virtualAddressSpace[j].type=='q'){

            for(j=0;j<frameSize;j++){
                fread(&value,sizeof(int),1, diskFile);    
            }
            firstNum=get(index[i],threadName);
           
        }
    }
}

void* fillRandomNumber(void* frameSizeInput){
    pthread_mutex_lock(&sequential);
    srand(1000);

    int i=0,j=0,index=0;
    char type[10]="fill";
    for (i = 0; i < numVirtual; i++) 
    {
        for (j = 0; j < frameSize; j++) 
        {
            set(index, (int) rand(),type );
            index++;
        }
    }
    sem_post(&mutex);
    pthread_mutex_unlock(&sequential);

    pthread_exit(NULL);

}

int main(int argc, char **argv) {
    int frameSizeInput=0,numPhysicalInput=0,numVirtualInput=0,i=0,j=0,valueTemp=0;
    string allocPolicy;
    char diskFileName[50];
    if(atoi(argv[1])<=0) {
        fprintf(stderr, "Error occured. Framesize must be bigger than 0.\n" );
        exit(EXIT_FAILURE);
    }
    else if(atoi(argv[2])<=0) {
        fprintf(stderr, "Error occured. Number of physical frame must be bigger than 0.\n" );
        exit(EXIT_FAILURE);
    }
    else if(atoi(argv[3])<=0) {
        fprintf(stderr, "Error occured. Number of virtual frame must be bigger than 0.\n" );
        exit(EXIT_FAILURE);
    }
    if(strcmp("FIFO",argv[4])!=0 && strcmp("SC",argv[4])!=0 && strcmp("LRU",argv[4])!=0 && strcmp("NRU",argv[4])!=0 && strcmp("WSClock",argv[4])!=0 ){
        fprintf(stderr, "Error occured. Page Replacement should be FIFO or SC or LRU or NRU or WSClock.\n" );
        exit(EXIT_FAILURE);
    }
    if(strcmp("global",argv[5])!=0 && strcmp("local",argv[5])!=0  ){
        fprintf(stderr, "Error occured. Alloc Policiy should be local or global.\n" );
        exit(EXIT_FAILURE);
    }

    if(argc !=8) {
        cout<<"Usage: Plese inputs like : sortArrays frameSize numPhysical numVirtual pageReplacement allocPolicy pageTablePrintInt diskFileName.dat\n"<<endl;
        exit(0);
    }
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
       if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    for(i=0;i<5;i++){
        printStatistic[i].numberOfReads=0;
        printStatistic[i].numberOfWrites=0;
        printStatistic[i].numberOfPageMisses=0;
        printStatistic[i].numberOfPageReplacements=0;
        printStatistic[i].diskPageWrite=0;
        printStatistic[i].diskPageRead=0;                    
    }

    sem_init(&mutex, 0, 0);
    sem_init(& mutex2, 0, 1);
    frameSizeInput=atoi(argv[1]);;
    numPhysicalInput=atoi(argv[2]);
    numVirtualInput=atoi(argv[3]);
    strcpy(pageReplacement,argv[4]);
    allocPolicy=argv[5];
    pageTablePrintInt=atoi(argv[6]);
    strcpy(diskFileName,argv[7]);
    frameSize=pow(2,frameSizeInput);
    numPhysical=pow(2,numPhysicalInput);
    numVirtual=pow(2,numVirtualInput);
    numLength=numVirtual*frameSize/4;
    virtualAddressSpace = new virtualAddress[numVirtual];
    for(i = 0; i < numVirtual; ++i){
        virtualAddressSpace[i].absentorNot = 0;
        virtualAddressSpace[i].modified = 0;
        virtualAddressSpace[i].referenced = 0;
        virtualAddressSpace[i].usedCounter = 0;
        virtualAddressSpace[i].type='x';
    }
    diskFile = fopen(diskFileName,"r+");
    pthread_t threadArr[6];
    pthread_create(&threadArr[0], NULL, fillRandomNumber, &frameSize);
    sem_wait(&mutex);
    physicalframes=new int*[numPhysical];
    for(i = 0; i < numPhysical; ++i)
        physicalframes[i] = new int[frameSize];



    pthread_create(&threadArr[1], NULL, bubleSort, NULL);
    pthread_create(&threadArr[2], NULL, quickSortFunction, NULL);
    pthread_create(&threadArr[3], NULL, mergeFunction, NULL);
    pthread_create(&threadArr[4], NULL, indexSort, NULL);

    if(strcmp(pageReplacement,"NRU")==0){
        pthread_create(&threadArr[5], NULL, refreshReferenced, NULL);
    }

    for(i=0;i<6;i++){
        if(i==5){
           finishControl=0;
        }
        if(strcmp(pageReplacement,"NRU")==0 || i!=5){
            pthread_join(threadArr[i], NULL);
        }

    }

    for(i=0;i<5;i++){
        if(i==0)
            printf("Fill istatistics\n");
        else if(i==1)
            printf("QuickSort istatistics\n");
        else if(i==2)
            printf("BubbleSort istatistics\n");
        else if(i==3)
            printf("MergeSort istatistics\n");
        else if(i==4)
            printf("IndexSort istatistics\n");
        printf("Number of reads %d \n",printStatistic[i].numberOfReads );
        printf("Number of writes %d \n",printStatistic[i].numberOfWrites );
        printf("Number of page misses %d \n", printStatistic[i].numberOfPageMisses);
        printf("Number of page replacements %d \n",printStatistic[i].numberOfPageReplacements );
        printf("Number of disk page writes %d \n", printStatistic[i].diskPageWrite);
        printf("Number of disk page reads %d \n",printStatistic[i].diskPageRead );
        printf("\n");  
              
    }

    for(i = 0; i < numPhysical; ++i) {
        delete physicalframes[i];
    }
    delete physicalframes;
    return 0;
}

    
void set(unsigned int index, int value, char * tName)
{
    pthread_mutex_lock(&lock);
    int i=0;
    pageTableCounter++;
    if(pageTableCounter!=0 && (pageTableCounter%pageTablePrintInt)==0){
        for(i=0;i<numVirtual;i++){
            printf("Page table frame %d \n",i );
            printf("Page table index %d \n",virtualAddressSpace[i].index );
            printf("Page table referenced %d \n",virtualAddressSpace[i].referenced );
            printf("Page table modified %d \n",virtualAddressSpace[i].modified );
            printf("Page table absent %d \n\n",virtualAddressSpace[i].absentorNot );
        }
        printf("\n\n");
    }
    int indexAddress=0;
    int boxIndex=index/frameSize;
    if(strcmp(tName,"fill")==0){
        printStatistic[0].numberOfWrites++;
        printStatistic[0].diskPageWrite++;
        fwrite(&value ,  sizeof(int),1  , diskFile );

    }
    else if(strcmp(tName,"quick")==0){
        virtualAddressSpace[boxIndex].modified = 1;
        virtualAddressSpace[boxIndex].referenced = 1;
        virtualAddressSpace[boxIndex].type='q';
        printStatistic[1].numberOfWrites++;
        physicalframes[virtualAddressSpace[boxIndex].index][index%frameSize]=value;
    }
    else if(strcmp(tName,"bubble")==0){
        virtualAddressSpace[boxIndex].modified = 1;
        virtualAddressSpace[boxIndex].referenced = 1;
        virtualAddressSpace[boxIndex].type='b';
        printStatistic[2].numberOfWrites++;
        physicalframes[virtualAddressSpace[boxIndex].index][index%frameSize]=value;

    }
    else if(strcmp(tName,"merge")==0){
        virtualAddressSpace[boxIndex].type=='m';
        virtualAddressSpace[boxIndex].modified = 1;
        virtualAddressSpace[boxIndex].referenced = 1;
        printStatistic[3].numberOfWrites++;
        physicalframes[virtualAddressSpace[boxIndex].index][index%frameSize]=value;

    }
    else if(strcmp(tName,"index")==0){
        virtualAddressSpace[boxIndex].type=='i';
        virtualAddressSpace[boxIndex].modified = 1;
        virtualAddressSpace[boxIndex].referenced = 1;
        printStatistic[4].numberOfWrites++;
        physicalframes[virtualAddressSpace[boxIndex].index][index%frameSize]=value;
    }
    else if(strcmp(tName,"check")==0){
        
    }
    pthread_mutex_unlock(&lock);

}
void scMethod(int boxIndex){
    int i=0,SCindexValue=0,SCindex=-1,flag=0;
    SCindexValue=numVirtual*frameSize;
    for(i=0;i<numVirtual;i++){
        if(virtualAddressSpace[i].absentorNot==1){
            if(SCindexValue>virtualAddressSpace[i].priorityNum){
                if(virtualAddressSpace[i].referenced==1){
                    virtualAddressSpace[i].referenced=0;
                }
                else{
                    SCindexValue=virtualAddressSpace[i].priorityNum;
                    SCindex=i;
                    if(flag==1)
                    break;    
                }

            }
        }
        if(i==numVirtual-1 && SCindex==-1){
            flag=1;
            i=0;
            for(i=0;i<numVirtual;i++){
                if(virtualAddressSpace[i].absentorNot==1){
                    SCindexValue=virtualAddressSpace[i].priorityNum;
                    SCindex=i;
                    break;    
                  
                } 
            }
            break;
        }
    }
    writeFile(SCindex);
    virtualAddressSpace[SCindex].absentorNot=-2;
    virtualAddressSpace[boxIndex].index=virtualAddressSpace[SCindex].index;
}

void wsClockMethod(int boxIndex){

}

void fifoMethod(int boxIndex){
    int i=0,FIFOindex=0,FIFOindexValue=0;
    FIFOindexValue=numVirtual*frameSize;
    for(i=0;i<numVirtual;i++){
        if(virtualAddressSpace[i].absentorNot==1){
            if(FIFOindexValue>virtualAddressSpace[i].priorityNum){
                FIFOindexValue=virtualAddressSpace[i].priorityNum;
                FIFOindex=i;
            }
        }
    }
    writeFile(FIFOindex);
    virtualAddressSpace[FIFOindex].absentorNot=-2;
    virtualAddressSpace[boxIndex].index=virtualAddressSpace[FIFOindex].index;
}

void lruMethod(int boxIndex){
    int LRUindex=0,LRUindexValue=0,i=0;
    LRUindexValue=numVirtual*frameSize;
    for(i=0;i<numVirtual;i++){
        if(virtualAddressSpace[i].absentorNot==1 && LRUindexValue>virtualAddressSpace[i].usedCounter){
            LRUindexValue=virtualAddressSpace[i].usedCounter;
            LRUindex=i;
        }
    }
    writeFile(LRUindex);
    virtualAddressSpace[LRUindex].absentorNot=-2;
    virtualAddressSpace[boxIndex].index=virtualAddressSpace[LRUindex].index;
}

void nruMethod(int boxIndex){
    int i=0,control=0,NRUindex=-1;
    for(i=0;i<numVirtual;i++){
        if(control==0){
            if(virtualAddressSpace[i].absentorNot==1 && virtualAddressSpace[i].modified==0 && virtualAddressSpace[i].referenced==0){
                NRUindex=i;
                break;
            }
            if(i==numVirtual-1){
                i=0;
                control=1;
            }   
        }
        else if(control==1){
            if(virtualAddressSpace[i].absentorNot==1 && virtualAddressSpace[i].modified==1 && virtualAddressSpace[i].referenced==0){
                NRUindex=i;
                break;
            }
            if(i==numVirtual-1){
                i=0;
                control=2;
            }   
        }
        else if(control==2){
            if(virtualAddressSpace[i].absentorNot==1 && virtualAddressSpace[i].modified==0 && virtualAddressSpace[i].referenced==1){
                NRUindex=i;
                break;
            }
            if(i==numVirtual-1){
                i=0;
                control=3;
            }   
        }
        else if(control==3){
            if(virtualAddressSpace[i].absentorNot==1 && virtualAddressSpace[i].modified==1 && virtualAddressSpace[i].referenced==1){
                NRUindex=i;
                break;
            }
            if(i==numVirtual-1){
                i=0;
                break;
            } 
        }
    }
    writeFile(NRUindex);
    virtualAddressSpace[NRUindex].absentorNot=-2;
    virtualAddressSpace[boxIndex].index=virtualAddressSpace[NRUindex].index;
}

int get(unsigned int index, char * tName)
{
    pthread_mutex_lock(&lock);
    int indexAddress=0,value=0,i=0,j=0;
    int boxIndex=0;
    int valueTemp=0,control=0;
    pageTableCounter++;
    if(pageTableCounter!=0 && (pageTableCounter%pageTablePrintInt)==0){
        for(i=0;i<numVirtual;i++){
            printf("Page table frame %d \n",i );
            printf("Page table index %d \n",virtualAddressSpace[i].index );
            printf("Page table referenced %d \n",virtualAddressSpace[i].referenced );
            printf("Page table modified %d \n",virtualAddressSpace[i].modified );
            printf("Page table absent %d \n\n",virtualAddressSpace[i].absentorNot );
        }
        printf("\n\n");
    }
    if(strcmp(tName,"fill")==0){       
        fread(&value, sizeof(int),1, diskFile);
    }
    else if(strcmp(tName,"quick")==0){
        printStatistic[1].numberOfReads++;
         boxIndex=index/frameSize;
         if(virtualAddressSpace[boxIndex].absentorNot==1){
            virtualAddressSpace[boxIndex].modified = 1;
            virtualAddressSpace[boxIndex].referenced = 1;
            if(strcmp(pageReplacement,"LRU")==0){
               for(i=0;i<numVirtual;i++){
                    if(virtualAddressSpace[i].absentorNot==1 ){
                        virtualAddressSpace[i].usedCounter--;                        
                    }
                }
                virtualAddressSpace[boxIndex].usedCounter=0;
            }
            virtualAddressSpace[boxIndex].type='q';
            pthread_mutex_unlock(&lock);
            return physicalframes[virtualAddressSpace[boxIndex].index][index%frameSize];
        }
        else{
            printStatistic[1].numberOfPageMisses++; 
            if(indexAdressPyhsical>=numPhysical){
                if(strcmp(pageReplacement,"LRU")==0){
                    lruMethod(boxIndex);
                    printStatistic[1].numberOfPageReplacements++;
                    printStatistic[1].diskPageWrite++;
                }
                else if(strcmp(pageReplacement,"NRU")==0){
                    nruMethod(boxIndex);
                    printStatistic[1].numberOfPageReplacements++;
                    printStatistic[1].diskPageWrite++;
                }
                else if(strcmp(pageReplacement,"FIFO")==0){
                    fifoMethod(boxIndex);
                    printStatistic[1].numberOfPageReplacements++;
                    printStatistic[1].diskPageWrite++;
                }
                else if(strcmp(pageReplacement,"SC")==0){       
                    scMethod(boxIndex);
                    printStatistic[1].numberOfPageReplacements++;
                    printStatistic[1].diskPageWrite++;
                }
                else if(strcmp(pageReplacement,"WSClock")==0){
                    wsClockMethod(boxIndex);
                    printStatistic[1].numberOfPageReplacements++;
                    printStatistic[1].diskPageWrite++;
                }            
            }
            else{
                virtualAddressSpace[boxIndex].index=indexAdressSpace;           
                indexAdressSpace++;
            }
            fseek(diskFile,index*sizeof(int),SEEK_SET);
            printStatistic[1].diskPageRead++;
            for(i=0;i<frameSize;i++){
                fread(&valueTemp,sizeof(int),1, diskFile);   
                physicalframes[virtualAddressSpace[boxIndex].index][i]=valueTemp;
            }
            indexAdressPyhsical++;
            virtualAddressSpace[boxIndex].absentorNot=1;
            virtualAddressSpace[boxIndex].startAdress=index;
            virtualAddressSpace[boxIndex].priorityNum=processNumber;
            virtualAddressSpace[boxIndex].type='q';
            processNumber++;
            pthread_mutex_unlock(&lock);
            return physicalframes[virtualAddressSpace[boxIndex].index][0];
        }
    }
    else if(strcmp(tName,"bubble")==0){
        printStatistic[2].numberOfReads++;
        boxIndex=index/frameSize;
         if(virtualAddressSpace[boxIndex].absentorNot==1){
            virtualAddressSpace[boxIndex].modified = 1;
            virtualAddressSpace[boxIndex].referenced = 1;
            if(strcmp(pageReplacement,"LRU")==0){
               for(i=0;i<numVirtual;i++){
                    if(virtualAddressSpace[i].absentorNot==1 ){
                        virtualAddressSpace[i].usedCounter--;                        
                    }
                }
                virtualAddressSpace[boxIndex].usedCounter=0;
            }
            virtualAddressSpace[boxIndex].type='b';

            pthread_mutex_unlock(&lock);
            return physicalframes[virtualAddressSpace[boxIndex].index][index%frameSize];
        }
        else{ 
            printStatistic[2].numberOfPageMisses++;
            if(indexAdressPyhsical>=numPhysical){
                if(strcmp(pageReplacement,"LRU")==0){
                    lruMethod(boxIndex);
                    printStatistic[2].numberOfPageReplacements++;
                    printStatistic[2].diskPageWrite++;
                }
                else if(strcmp(pageReplacement,"NRU")==0){
                    nruMethod(boxIndex);
                    printStatistic[2].numberOfPageReplacements++;
                    printStatistic[2].diskPageWrite++;                    
                }
                else if(strcmp(pageReplacement,"FIFO")==0){
                    fifoMethod(boxIndex);
                    printStatistic[2].numberOfPageReplacements++;
                    printStatistic[2].diskPageWrite++;                    
                }
                else if(strcmp(pageReplacement,"SC")==0){       
                    scMethod(boxIndex);
                    printStatistic[2].numberOfPageReplacements++;
                    printStatistic[2].diskPageWrite++;                    
                }
                else if(strcmp(pageReplacement,"WSClock")==0){
                    
                }            
            }
            else{
                virtualAddressSpace[boxIndex].index=indexAdressSpace;           
                indexAdressSpace++;
            }
            printStatistic[2].diskPageRead++;
            fseek(diskFile,index*sizeof(int),SEEK_SET);
            for(i=0;i<frameSize;i++){
                fread(&valueTemp,sizeof(int),1, diskFile);   
                physicalframes[virtualAddressSpace[boxIndex].index][i]=valueTemp;
            }
            indexAdressPyhsical++;
            virtualAddressSpace[boxIndex].absentorNot=1;
            virtualAddressSpace[boxIndex].startAdress=index;
            virtualAddressSpace[boxIndex].priorityNum=processNumber;
            virtualAddressSpace[boxIndex].type='b';

            processNumber++;
            pthread_mutex_unlock(&lock);
            return physicalframes[virtualAddressSpace[boxIndex].index][0];
        }
    }
    else if(strcmp(tName,"merge")==0){
        printStatistic[3].numberOfReads++;
        boxIndex=index/frameSize;
         if(virtualAddressSpace[boxIndex].absentorNot==1){
            virtualAddressSpace[boxIndex].modified = 1;
            virtualAddressSpace[boxIndex].referenced = 1;
            if(strcmp(pageReplacement,"LRU")==0){
               for(i=0;i<numVirtual;i++){
                    if(virtualAddressSpace[i].absentorNot==1 ){
                        virtualAddressSpace[i].usedCounter--;                        
                    }
                }
                virtualAddressSpace[boxIndex].usedCounter=0;
            }
            pthread_mutex_unlock(&lock);
            return physicalframes[virtualAddressSpace[boxIndex].index][index%frameSize];
        }
        else{ 
            printStatistic[3].numberOfPageMisses++;
            printStatistic[3].diskPageRead;
            fseek(diskFile,index*sizeof(int),SEEK_SET);
            if(indexAdressPyhsical>=numPhysical){
                if(strcmp(pageReplacement,"LRU")==0){
                    lruMethod(boxIndex);
                    printStatistic[3].numberOfPageReplacements++;
                    printStatistic[3].diskPageWrite++;                   
                }
                else if(strcmp(pageReplacement,"NRU")==0){
                    nruMethod(boxIndex);
                    printStatistic[3].numberOfPageReplacements++;
                    printStatistic[3].diskPageWrite++;                    
                }
                else if(strcmp(pageReplacement,"FIFO")==0){
                    fifoMethod(boxIndex);
                    printStatistic[3].numberOfPageReplacements++;
                    printStatistic[3].diskPageWrite++;                    
                }
                else if(strcmp(pageReplacement,"SC")==0){       
                    scMethod(boxIndex);
                    printStatistic[3].numberOfPageReplacements++;
                    printStatistic[3].diskPageWrite++;                    
                }
                else if(strcmp(pageReplacement,"WSClock")==0){
                    
                }            
            }
            else{
                virtualAddressSpace[boxIndex].index=indexAdressSpace;           
                indexAdressSpace++;
            }
            fseek(diskFile,index*sizeof(int),SEEK_SET);
            for(i=0;i<frameSize;i++){
                fread(&valueTemp,sizeof(int),1, diskFile);   
                physicalframes[virtualAddressSpace[boxIndex].index][i]=valueTemp;
            }
            indexAdressPyhsical++;
            virtualAddressSpace[boxIndex].absentorNot=1;
            virtualAddressSpace[boxIndex].startAdress=index;
            virtualAddressSpace[boxIndex].priorityNum=processNumber;
            virtualAddressSpace[boxIndex].type='m';
            processNumber++;
            pthread_mutex_unlock(&lock);
            return physicalframes[virtualAddressSpace[boxIndex].index][0];
        }
    }
    else if(strcmp(tName,"index")==0){
        printStatistic[4].numberOfReads++;
        boxIndex=index/frameSize;
         if(virtualAddressSpace[boxIndex].absentorNot==1){
            virtualAddressSpace[boxIndex].modified = 1;
            virtualAddressSpace[boxIndex].referenced = 1;
            if(strcmp(pageReplacement,"LRU")==0){
               for(i=0;i<numVirtual;i++){
                    if(virtualAddressSpace[i].absentorNot==1 ){
                        virtualAddressSpace[i].usedCounter--;                        
                    }
                }
                virtualAddressSpace[boxIndex].usedCounter=0;
            }
            pthread_mutex_unlock(&lock);
            return physicalframes[virtualAddressSpace[boxIndex].index][index%frameSize];
        }
        else{ 
            printStatistic[4].numberOfPageMisses++;
            printStatistic[4].diskPageRead;
            fseek(diskFile,index*sizeof(int),SEEK_SET);
            if(indexAdressPyhsical>=numPhysical){
                if(strcmp(pageReplacement,"LRU")==0){
                    lruMethod(boxIndex);
                    printStatistic[4].numberOfPageReplacements++;
                    printStatistic[4].diskPageWrite++;                    
                }
                else if(strcmp(pageReplacement,"NRU")==0){
                    nruMethod(boxIndex);
                    printStatistic[4].numberOfPageReplacements++;
                    printStatistic[4].diskPageWrite++;                    
                }
                else if(strcmp(pageReplacement,"FIFO")==0){
                    fifoMethod(boxIndex);
                    printStatistic[4].numberOfPageReplacements++;
                    printStatistic[4].diskPageWrite++;                    
                }
                else if(strcmp(pageReplacement,"SC")==0){       
                    scMethod(boxIndex);
                    printStatistic[4].numberOfPageReplacements++;
                    printStatistic[4].diskPageWrite++;                    
                }
                else if(strcmp(pageReplacement,"WSClock")==0){
                    
                }            
            }
            else{
                virtualAddressSpace[boxIndex].index=indexAdressSpace;           
                indexAdressSpace++;
            }
            fseek(diskFile,index*sizeof(int),SEEK_SET);
            for(i=0;i<frameSize;i++){
                fread(&valueTemp,sizeof(int),1, diskFile);   
                physicalframes[virtualAddressSpace[boxIndex].index][i]=valueTemp;
            }
            indexAdressPyhsical++;
            virtualAddressSpace[boxIndex].absentorNot=1;
            virtualAddressSpace[boxIndex].startAdress=index;
            virtualAddressSpace[boxIndex].priorityNum=processNumber;
            virtualAddressSpace[boxIndex].type='i';
            processNumber++;
            pthread_mutex_unlock(&lock);
            return physicalframes[virtualAddressSpace[boxIndex].index][0];    
        }
    }
    else if(strcmp(tName,"check")==0){
        
    }
    pthread_mutex_unlock(&lock);

    return 0;
}