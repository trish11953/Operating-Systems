#include "simulator.h"
#include <string.h>
//#include <time.h>
#include <stdbool.h>
#include <math.h>
#include "fileIO.h"
#include "pagetable.h"
#include "tLB.h"
#include <stdint.h>
#include <stdlib.h>

int misses = 0, hits = 0, diskint = 0, pagefaults = 0;
int clock;
int totalaccess = 0;

int DRAM_SIZE = 0;
int PTLEVEL1SIZE = 0;
int PTLEVEL2SIZE = 0;
int PTLEVEL3SIZE = 0;
int OFFSETSIZE = 0;
uint32_t level1mask = 1;
uint32_t level2mask = 1;
uint32_t level3mask = 1;
uint32_t maskoffset = 1;

int mmu(const char* va, int N1, int N2, int N3)
{  
   DRAM_SIZE = pow(2,22);

   PTLEVEL1SIZE = pow(2,N1);
   PTLEVEL2SIZE = pow(2,N2);
   PTLEVEL3SIZE = pow(2,N3);

   OFFSETSIZE = pow(2,3);

   level1mask = PTLEVEL1SIZE - 1;
   level2mask = PTLEVEL2SIZE - 1;
   level3mask = PTLEVEL3SIZE - 1;

   maskoffset = OFFSETSIZE - 1;
   memoryaccess(PTLEVEL1SIZE);

   //lrulevel1 = (struct lru_struct *) malloc (PTLEVEL1SIZE * sizeof(struct lru_struct));
   //lru_init(lrulevel1, PTLEVEL1SIZE);   
   PTlevel1 = (struct pagetable *) malloc (PTLEVEL1SIZE * sizeof(struct pagetable));   

   //tlb = (struct tlb_struct *) malloc (TLB_SIZE * sizeof(struct tlb_struct));
   
   //printf("test: %s \n", va);
   int vpn = (int)strtol(va, NULL, 0);
   //printf("VA in int: %x \n", vpn);
   int flag = tlb_access(vpn);
   return flag;
}

void init()
{
    current_time = 0;
    nextQuanta = current_time + quantum;
    readyProcess = gll_init();
    runningProcess= gll_init();
    blockedProcess = gll_init();
    interruptList = gll_init();
    
    processList = gll_init();
    traceptr = openTrace(traceFileName);

    sysParam = readSysParam(traceptr);

    //read traces from trace file and put them in the processList
    struct PCB* temp = readNextTrace(traceptr);
    if(temp == NULL)
    {
        printf("No data in file. Exit.\n");
        exit(1);
    }

    while(temp != NULL)
    {
        gll_pushBack(processList, temp);
        temp = readNextTrace(traceptr);
    }

    //transfer ready processes from processList to readyProcess list
    temp = gll_first(processList);
    
    while((temp!= NULL) && ( temp->start_time <= current_time))
    {
        struct NextMem* tempAddr;
        temp->memoryFile = openTrace(temp->memoryFilename);
        temp->numOfIns = readNumIns(temp->memoryFile);
        tempAddr = readNextMem(temp->memoryFile);
        while(tempAddr!= NULL)
        {
            gll_pushBack(temp->memReq, tempAddr);
            tempAddr = readNextMem(temp->memoryFile);
        }
        gll_pushBack(readyProcess, temp);
        gll_pop(processList);

        temp = gll_first(processList);
    }
    //TODO: Initialize what you need
    //printf("bits: %d" , sysParam->N1_in_bits);
}

/*
void print()
{
 printf("TLB = %d", tlb[0]); 
}
*/

void finishAll()
{
    if((gll_first(readyProcess)!= NULL) || (gll_first(runningProcess)!= NULL) || (gll_first(blockedProcess)!= NULL) || (gll_first(processList)!= NULL))
    {
        printf("Something is still pending\n");
    }
    gll_destroy(readyProcess);
    gll_destroy(runningProcess);
    gll_destroy(blockedProcess);
    gll_destroy(processList);
    //printf("TLB = %d", tlb[0]);
    //TODO: Anything else you want to destroy
   
    closeTrace(traceptr);
}

void statsinit()
{
    // statsList = gll_init();
    resultStats.perProcessStats = gll_init();
    resultStats.executionOrder = gll_init();
    resultStats.start_time = current_time;
}

void statsUpdate()
{
    resultStats.OSModetime = OSTime;
    resultStats.userModeTime  = userTime;   
    resultStats.numberOfContextSwitch = numberContextSwitch;
    resultStats.end_time = current_time;
}

//returns 1 on success, 0 if trace ends, -1 if page fault
int readPage(struct PCB* p, uint64_t stopTime)
{   
    struct NextMem* addr = gll_first(p->memReq);
    uint64_t timeAvailable = stopTime - current_time;
    if(addr == NULL)
    {
        return 0;
    }
    if(debug == 1)
    {
        printf("Request::%s::%s::\n", addr->type, addr->address);
    }
    if(strcmp(addr->type, "NONMEM") == 0)
    {
        uint64_t timeNeeded = (p->fracLeft > 0)? p->fracLeft: sysParam->non_mem_inst_length;
    
        if(timeAvailable < timeNeeded)
        {
            current_time += timeAvailable;
            userTime += timeAvailable;
            p->user_time += timeAvailable;
            p->fracLeft = timeNeeded - timeAvailable;
        }
        else{
            gll_pop(p->memReq);
            current_time += timeNeeded; 
            userTime += timeNeeded;
            p->user_time += timeNeeded;
            p->fracLeft = 0;
        }

        if(gll_first(p->memReq) == NULL)
        {
            return 0;
        }
        return 1;
    }
    else
    {
          // TODO: for MEM traces
          //uint64_t timeNeeded = (p->fracLeft > 0)? p->fracLeft: sysParam->non_mem_inst_length;                 
          int flag =  mmu(addr->address, sysParam->N1_in_bits, sysParam->N2_in_bits, sysParam->N3_in_bits);
	  char* add = addr->address;	
          int vpn = (int)strtol(add, NULL, 0);
          current_time += sysParam->TLB_latency;
	  p->OS_time += sysParam->TLB_latency;
          totalaccess++;
	  if(flag == 0)
	  {     current_time = 0;
		
		//printf("Miss count = %d\n", misses);
                diskint++;
		current_time += sysParam->Swap_interrupt_handling_time;
		p->OS_time += sysParam->Swap_interrupt_handling_time;
		struct PCB* temp;
		int flag2 = accessinglevel1(sysParam->N1_in_bits, sysParam->N2_in_bits, sysParam->N3_in_bits, vpn, level1mask, level2mask, level3mask, maskoffset);
		if(flag2 == 0)
		{       
 			misses++;
                        p->OS_time += sysParam->Page_fault_trap_handling_time;
			current_time += sysParam->Page_fault_trap_handling_time;
			temp = gll_first(runningProcess);

                        if(diskint == 0)
     				gll_push(interruptList, temp);
			if(diskint > 0)
                                gll_pushBack(interruptList, temp);
			pagefaults++;
			 
		}
		else
		{       hits++;
		}	
           }
	   else
	   {       
		hits++;
		return 1;
      	   }
	   p->missCount += misses;
           //printf("total access %d \n", totalaccess); 
           resultStats.totalTLBmiss = misses;
           resultStats.numberOfDiskInt = diskint;
	   resultStats.totalPgFaults = pagefaults;
           return 0;
    }
}

void schedulingRR(int pauseCause)
{
    //move first readyProcess to running
    gll_push(runningProcess, gll_first(readyProcess));
    gll_pop(readyProcess);

    if(gll_first(runningProcess) != NULL)
    {
        current_time = current_time + contextSwitchTime;
        OSTime += contextSwitchTime;
        numberContextSwitch++;
        struct PCB* temp = gll_first(runningProcess);
        gll_pushBack(resultStats.executionOrder, temp->name);
    }
}

/*runs a process. returns 0 if page fault, 1 if quanta finishes, -1 if traceFile ends, 2 if no running process, 4 if disk Interrupt*/
int processSimulator()
{
    uint64_t stopTime = nextQuanta;
    int stopCondition = 1;
    if(gll_first(runningProcess)!=NULL)
    {
        //TODO
        //if(TODO: if there is a pending disk operation in the future)
        //{
            //TODO: stopTime = occurance of the first disk interrupt
        //    stopCondition = 4;
        //}
        while(current_time < stopTime)
        {
            
            int read = readPage(gll_first(runningProcess), stopTime);
            if(debug == 1){
                printf("Read: %d\n", read);
                printf("Current Time %" PRIu64 ", Next Quanta Time %" PRIu64 " %" PRIu64 "\n",current_time, nextQuanta, stopTime);
            }
            if(read == 0)
            {
                return -1;
                break;
            }
            else if(read == -1) //page fault
            {
                if(gll_first(runningProcess) != NULL)
                {
                    gll_pushBack(blockedProcess, gll_first(runningProcess));
                    gll_pop(runningProcess);

                    return 0;
                }
                
            }
        }
        if(debug == 1)
        {
            printf("Stop condition found\n");
            printf("Current Time %" PRIu64 ", Next Quanta Time %" PRIu64 "\n",current_time, nextQuanta);
        }
        return stopCondition;
    }
    if(debug == 1)
    {
        printf("No running process found\n");
    }
    return 2;
}

void cleanUpProcess(struct PCB* p)
{
    //struct PCB* temp = gll_first(runningProcess);
    struct PCB* temp = p;
    //TODO: Adjust the amount of available memory as this process is finishing
    
    struct Stats* s = malloc(sizeof(stats));
    s->processName = temp->name;
    s->hitCount = temp->hitCount;
    s->missCount = temp->missCount;
    s->user_time = temp->user_time;
    s->OS_time = temp->OS_time;

    s->duration = current_time - temp->start_time;
    gll_pushBack(resultStats.perProcessStats, s);
    gll_destroy(temp->memReq);
    closeTrace(temp->memoryFile);

}

void printPCB(void* v)
{
    struct PCB* p = v;
    if(p!=NULL){
        printf("%s, %" PRIu64 "\n", p->name, p->start_time);
    }
}

void printStats(void* v)
{
    struct Stats* s = v;
    if(s!=NULL){
        double hitRatio = s->hitCount / (1.0* s->hitCount + 1.0 * s->missCount);
        printf("\n\nProcess: %s: \nHit Ratio = %lf \tProcess completion time = %" PRIu64 
                "\tuser time = %" PRIu64 "\tOS time = %" PRIu64 "\n", s->processName, hitRatio, s->duration, s->user_time, s->OS_time) ;
    }
}

void printExecOrder(void* v)
{
    char* c = v;
    if(c!=NULL){
        printf("%s\n", c) ;
    }
}



void diskToMemory()
{
    // TODO: Move requests from disk to memory
    // TODO: move appropriate blocked process to ready process
    gll_pop(interruptList);
    gll_pushBack(readyProcess, gll_first(blockedProcess));
    gll_pop(blockedProcess);
    
//    resultStats.numberOfContextSwitch += 1;
    if(debug == 1)
    {   
        printf("Done diskToMemory\n");
    }
}

void simulate()
{
    init();
    statsinit();

    //get the first ready process to running state
    struct PCB* temp = gll_first(readyProcess);
    gll_pushBack(runningProcess, temp);
    gll_pop(readyProcess);

    struct PCB* temp2 = gll_first(runningProcess);
    gll_pushBack(resultStats.executionOrder, temp2->name);

    while(1)
    {
        int simPause = processSimulator();
        if(current_time == nextQuanta)
        {
            nextQuanta = current_time + quantum;
        }

        //transfer ready processes from processList to readyProcess list
        struct PCB* temp = gll_first(processList);
        
        while((temp!= NULL) && ( temp->start_time <= current_time))
        {
            temp->memoryFile = openTrace(temp->memoryFilename);
            temp->numOfIns = readNumIns(temp->memoryFile);

            struct NextMem* tempAddr = readNextMem(temp->memoryFile);

	        while(tempAddr!= NULL)
            {
                gll_pushBack(temp->memReq, tempAddr);
                tempAddr = readNextMem(temp->memoryFile);
            }
            gll_pushBack(readyProcess, temp);
            gll_pop(processList);

            temp = gll_first(processList);
        }

        //move elements from disk to memory
        diskToMemory();

        //This memory trace done
        if(simPause == -1)
        {
            //finish up this process
            cleanUpProcess(gll_first(runningProcess));
            gll_pop(runningProcess);
        }

        //move running process to readyProcess list
        int runningProcessNUll = 0;
        if(simPause == 1 || simPause == 4)
        {
            if(gll_first(runningProcess) != NULL)
            {
                gll_pushBack(readyProcess, gll_first(runningProcess));
                gll_pop(runningProcess);
            }
            else{
                runningProcessNUll = 1;
            }
            if(simPause == 1)
            {
                nextQuanta = current_time + quantum;
            }
        }

        schedulingRR(simPause);

        //Nothing in running or ready. need to increase time to next timestamp when a process becomes ready.
        if((gll_first(runningProcess) == NULL) && (gll_first(readyProcess) == NULL))
        {
            if(debug == 1)
            {
                printf("\nNothing in running or ready\n");
            }
            if((gll_first(blockedProcess) == NULL) && (gll_first(processList) == NULL))
            {

                    if(debug == 1)
                    {
                        printf("\nAll done\n");
                    }
                    break;
            }
            struct PCB* tempProcess = gll_first(processList);
            struct PCB* tempBlocked = gll_first(blockedProcess);

            //TODO: Set correct value of timeOfNextPendingDiskInterrupt
            uint64_t timeOfNextPendingDiskInterrupt = 0;

            if(tempBlocked == NULL)
            {
                if(debug == 1)
                {
                    printf("\nGoing to move from proess list to ready\n");
                }
                struct NextMem* tempAddr;
                tempProcess->memoryFile = openTrace(tempProcess->memoryFilename);
                tempProcess->numOfIns = readNumIns(tempProcess->memoryFile);
                tempAddr = readNextMem(tempProcess->memoryFile);
                while(tempAddr!= NULL)
                {
                    gll_pushBack(tempProcess->memReq, tempAddr);
                    tempAddr = readNextMem(tempProcess->memoryFile);
                }
                gll_pushBack(readyProcess, tempProcess);
                gll_pop(processList);
                
                while(nextQuanta < tempProcess->start_time)
                {   
                    
                    current_time = nextQuanta;
                    nextQuanta = current_time + quantum;
                }
                OSTime += (tempProcess->start_time-current_time);
                current_time = tempProcess->start_time; 
            }
            else
            {
                if(tempProcess == NULL)
                {
                    if(debug == 1)
                    {
                        printf("\nGoing to move from blocked list to ready\n");
                    }
                    OSTime += (timeOfNextPendingDiskInterrupt-current_time);
                    current_time = timeOfNextPendingDiskInterrupt;
                    while (nextQuanta < current_time)
                    {
                        nextQuanta = nextQuanta + quantum;
                    }
                    diskToMemory();
                }
                else if(tempProcess->start_time >= timeOfNextPendingDiskInterrupt)
                {
                    if(debug == 1)
                    {
                        printf("\nGoing to move from blocked list to ready\n");
                    }
                    OSTime += (timeOfNextPendingDiskInterrupt-current_time);
                    current_time = timeOfNextPendingDiskInterrupt;
                    while (nextQuanta < current_time)
                    {
                        nextQuanta = nextQuanta + quantum;
                    }
                    diskToMemory();
                }
                else{
                    struct NextMem* tempAddr;
                    if(debug == 1)
                    {
                        printf("\nGoing to move from proess list to ready\n");
                    }
                    tempProcess->memoryFile = openTrace(tempProcess->memoryFilename);
                    tempProcess->numOfIns = readNumIns(tempProcess->memoryFile);
                    tempAddr = readNextMem(tempProcess->memoryFile);
                    while(tempAddr!= NULL)
                    {
                        gll_pushBack(tempProcess->memReq, tempAddr);
                        tempAddr = readNextMem(tempProcess->memoryFile);
                    }
                    gll_pushBack(readyProcess, tempProcess);
                    gll_pop(processList);
                    
                    while(nextQuanta < tempProcess->start_time)
                    {   
                        current_time = nextQuanta;
                        nextQuanta = current_time + quantum;
                    }
                    OSTime += (tempProcess->start_time-current_time);
                    current_time = tempProcess->start_time; 
                }
            }   
        }
    }
}

int main(int argc, char** argv)
{
    if(argc == 1)
    {
        printf("No file input\n");
        exit(1);
    }
    traceFileName = argv[1];
    outputFileName = argv[2];

    simulate();
    finishAll();
    statsUpdate();

    if(writeToFile(outputFileName, resultStats) == 0)
    {
        printf("Could not write output to file\n");
    }
    printf("User time = %" PRIu64 "\nOS time = %" PRIu64 "\n", resultStats.userModeTime, resultStats.OSModetime);
    printf("Context switched = %d\n", resultStats.numberOfContextSwitch);
    printf("Start time = 0\nEnd time =%llu", current_time);
    gll_each(resultStats.perProcessStats, &printStats);

    // printf("\nExec Order:\n");
    // gll_each(resultStats.executionOrder, &printExecOrder);
    printf("\n");
}
