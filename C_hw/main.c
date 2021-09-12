#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

int main(int argc, char* argv[]) { // argv[1]=record/replay     argv[2]=file
    if (argc != 3) {
        errx(1, "Invalid number of arguments. Try to enter 3 arguments !");
    }
    struct timeval start;
    gettimeofday(&start, NULL);
    if (strcmp(argv[1],"record") == 0) {
        int fd1 = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd1 == -1) {
            int olderr = errno;
            close(fd1);
            errno = olderr;
            err(3, "File fail to open !");
        }
        uint16_t messageType;
        ssize_t read_size_messageType;
        while ((read_size_messageType = read(0, &messageType, sizeof(messageType))) > 0) {
            struct timeval end;
            gettimeofday(&end, NULL);
            if (write(fd1, &messageType, read_size_messageType) != read_size_messageType ) {
                int olderr = errno;
                close(fd1);
                errno = olderr;
                err(4, "Record: error while writing message type in the file !");
            }
            uint32_t temp = (uint32_t)((double)((((end.tv_sec * 1000000 + end.tv_usec)-(start.tv_sec * 1000000 + start.tv_usec))/1000000))*1000);
            if( write(fd1, &temp, sizeof(temp)) == -1) {
                int olderr = errno;
                close(fd1);
                errno = olderr;
                err(5, "Record: error while writing temp in the file !");
            }
            fprintf(stderr,"[%.3f] ",(double)(((end.tv_sec * 1000000 + end.tv_usec)-(start.tv_sec * 1000000 + start.tv_usec)))/1000000);
            if (messageType == 0x0001) {
                fprintf(stderr,"<state> ");
                uint16_t slots;
                ssize_t read_size_slots;
                if ((read_size_slots = read(0, &slots, sizeof(slots))) > 0) { 
                    if (write(fd1, &slots, read_size_slots) != read_size_slots ) {
                        int olderr = errno;
                        close(fd1);
                        errno = olderr;
                        err(6, "Record: error while writing slots in the file !");
                    }
                    uint32_t temperature;
                    ssize_t read_size_temperature;
                    if ((read_size_temperature = read(0, &temperature, sizeof(temperature))) > 0) { 
                        if (write(fd1, &temperature, read_size_temperature) != read_size_temperature ) {
                            int olderr = errno;
                            close(fd1);
                            errno = olderr;
                            err(7, "Record: error while writing temperature in the file !");
                        }
                        float decimalTemperature = ((temperature/100)-273.15);
                        fprintf(stderr,"temp: %.2f°C, ", decimalTemperature);    
                    }
                    if (read_size_temperature == -1) {
                        err(8, "Record: failed to read temperature from the stdin !");
                    }
                    fprintf(stderr,"slots:  ");
                    for (int i=1; i<=16; i++) {            
                       if ( slots == 1 ) {
                           fprintf(stderr,"%d[ ] ", i); // i[ ]
                       }
                       else if (( slots % 2 ) == 0 ) {
                           fprintf(stderr,"%d[ ] ", i); // i[ ]
                           slots = slots/2;
                       }
                       else if (( slots % 2 ) == 1) {
                           fprintf(stderr,"%d[X] ", i); // i[X]
                           slots = slots/2;
                       }
                    }
                    fprintf(stderr,"\n");
                }
                else if (read_size_slots == -1) {
                    err(9, "Record: failed to read slots from the stdin !");
                }
            }
            if (messageType == 0x0002) {
                fprintf(stderr,"<slot text> ");
                uint8_t slotID;
                ssize_t read_size_slotID;
                if ((read_size_slotID = read(0, &slotID, sizeof(slotID))) > 0) { 
                    if (write(fd1, &slotID, read_size_slotID) != read_size_slotID ) {
                        int olderr = errno;
                        close(fd1);
                        errno = olderr;
                        err(10, "Record: error while writing slot ID in the file !");
                    }
                    fprintf(stderr,"slot %d: ", slotID);      
                }
                else if (read_size_slotID == -1) {
                    err(11, "Record: failed to read slot ID from the stdin !");
                }
                char text[13];
                ssize_t read_size_text;
                if ((read_size_text = read(0, &text, sizeof(text))) > 0) { 
                    if (write(fd1, &text, read_size_text) != read_size_text ) {
                        int olderr = errno;
                        close(fd1);
                        errno = olderr;
                        err(12, "Record: error while writing slot text in the file !");
                    }
                    fprintf(stderr,"%s", text);
                    fprintf(stderr,"\n");
                }
                else if (read_size_text == -1) {
                    err(13, "Record: failed to read text from the stdin!");
                }
            }
        }
        if (read_size_messageType == -1) {
            err(14, "Record: failed to read message type from the stdin !");
        }
        close(fd1);
    }
    else if (strcmp(argv[1],"replay") == 0) {
        uint32_t last_time=0;
        int fd1 = open(argv[2], O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd1 == -1) {
            int olderr = errno;
            close(fd1);
            errno = olderr;
            err(15, "File fail to open !");
        }
        uint16_t messageType=0;
        ssize_t read_size_messageType=0;
        while ((read_size_messageType = read(fd1, &messageType, sizeof(messageType))) > 0) {
            uint32_t temp;
            ssize_t read_size_temp;
            if ((read_size_temp = read(fd1, &temp, sizeof(temp))) > 0) { 
                usleep((temp-last_time)*1000);
                last_time=temp;
            }
            else if (read_size_temp == -1) {
                err(16, "Replay: failed to read time from the file !");
            }
            struct timeval end;
            gettimeofday(&end, NULL);
            if (write(1, &messageType, read_size_messageType) != read_size_messageType ) {
                int olderr = errno;
                close(fd1);
                errno = olderr;
                err(17, "Replay: error while writing message type in the stdout !");
            }
            fprintf(stderr,"[%.3f] ",(double)(((end.tv_sec * 1000000 + end.tv_usec)-(start.tv_sec * 1000000 + start.tv_usec)))/1000000);
            if (messageType == 0x0001) {
                fprintf(stderr,"<state> ");
                uint16_t slots;
                ssize_t read_size_slots;
                if ((read_size_slots = read(fd1, &slots, sizeof(slots))) > 0) { 
                    if (write(1, &slots, read_size_slots) != read_size_slots ) {
                        int olderr = errno;
                        close(fd1);
                        errno = olderr;
                        err(18, "Replay: error while writing slots in the stdout !");
                    }
                uint32_t temperature;
                ssize_t read_size_temperature;
                if ((read_size_temperature = read(fd1, &temperature, sizeof(temperature))) > 0) { 
                    if (write(1, &temperature, read_size_temperature) != read_size_temperature ) {
                        int olderr = errno;
                        close(fd1);
                        errno = olderr;
                        err(19, "Replay: error while writing temperature in the stdout !");
                    }
                    temperature = (double)((temperature/100)-273.15);
                    fprintf(stderr,"temp: %d°C, ", temperature);    
                }
                else if (read_size_temperature == -1) {
                    err(20, "Replay: failed to read temperature from the file !");
                }
                fprintf(stderr,"slots:  ");
                for (int i=1; i<=16; i++) {           
                    if ( slots == 1 ) {
                        fprintf(stderr,"%d[ ] ", i); // i[ ]
                    }
                    else if (( slots % 2 ) == 0 ) {
                        fprintf(stderr,"%d[ ] ", i); // i[ ]
                        slots = slots/2;
                    }
                    else if (( slots % 2 ) == 1) {
                        fprintf(stderr,"%d[X] ", i); // i[X]
                        slots = slots/2;
                    }
                }
                fprintf(stderr,"\n");
                }
                else if (read_size_slots == -1) {
                    err(21, "Replay: failed to read slots from the file !");
                }
            }
            if (messageType == 0x0002) {
                fprintf(stderr,"<slot text> ");
                uint8_t slotID;
                ssize_t read_size_slotID;
                if ((read_size_slotID = read(fd1, &slotID, sizeof(slotID))) > 0) { 
                    if (write(1, &slotID, read_size_slotID) != read_size_slotID ) {
                        int olderr = errno;
                        close(fd1);
                        errno = olderr;
                        err(22, "Replay: error while writing slot ID in the stdout !");
                    }
                    fprintf(stderr,"slot %d: ", slotID);      
                }
                else if (read_size_slotID == -1) {
                    err(23, "Replay: failed to read slot ID from the file !");
                }
                char text[13];
                ssize_t read_size_text;
                if ((read_size_text = read(fd1, &text, sizeof(text))) > 0) { 
                    if (write(1, &text, read_size_text) != read_size_text ) {
                        int olderr = errno;
                        close(fd1);
                        errno = olderr;
                        err(24, "Replay: error while writing slot text in the stdout !");
                    }
                    fprintf(stderr,"%s", text);
                    fprintf(stderr,"\n");
                }
                else if (read_size_text == -1) {
                    err(25, "Replay: failed to read slot text from the file !");
                }
            }
        }
        if (read_size_messageType == -1) {
            err(26, "Replay: failed to read message type from the file !");
        }
        close(fd1);
    }
     else {
         errx(2, "Second argument is not valid. Please try to enter: record or replay !");
    }
    exit(0);
}


