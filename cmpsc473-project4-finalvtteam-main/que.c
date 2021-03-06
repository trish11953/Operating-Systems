#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"



//This initializes the FIFO structure with the given buffer and size
void fifo_init(fifo_t * f, int size){
     f->head = 0;
     f->tail = 0;
     f->size = size;
     f->avilSize = size;
     f->buf = (char*) malloc(sizeof(char)*size);
     pthread_mutex_init(&(f->Qlock), NULL);
}

//This reads nbytes bytes from the FIFO
//The number of bytes read is returned
int fifo_read(fifo_t * f, void * buf, int nbytes){
     int i;
     char * p;
     p = buf;
     for(i=0; i < nbytes; i++){
          if( f->tail != f->head ){ //see if any data is available
               *p++ = f->buf[f->tail];  //grab a byte from the buffer
               f->tail++;  //increment the tail
               if( f->tail == f->size ){  //check for wrap-around
                    f->tail = 0;
               }
          } else {
               return i; //number of bytes read
          }
     }
     pthread_mutex_lock(&(f->Qlock));
     f->avilSize+=nbytes;
     pthread_mutex_unlock(&(f->Qlock));
     return nbytes;
}

//This writes up to nbytes bytes to the FIFO
//If the head runs in to the tail, not all bytes are written
//The number of bytes written is returned
int fifo_write(fifo_t * f, const void * buf, int nbytes){
     int i;
     const char * p;
     p = buf;
     for(i=0; i < nbytes; i++){
           //first check to see if there is space in the buffer
           if( (f->head + 1 == f->tail) ||
                ( (f->head + 1 == f->size) && (f->tail == 0) )){
                 return i; //no more room
           } else {
               f->buf[f->head] = *p++;
               f->head++;  //increment the head
               if( f->head == f->size ){  //check for wrap-around
                    f->head = 0;
               }
           }
     }
     f->avilSize-=nbytes;
     return nbytes;
}
int fifo_avail_size(fifo_t * f)
{
     pthread_mutex_lock(&(f->Qlock));
     int avlsize = f->avilSize;
     pthread_mutex_unlock(&(f->Qlock));
     return avlsize;
}

int fifo_used_size(fifo_t * f)
{
     return (f->size - f->avilSize);
}

void fifo_free(fifo_t * f)
{
     pthread_mutex_destroy(&(f->Qlock));
     free(f->buf);
     free(f);
}

