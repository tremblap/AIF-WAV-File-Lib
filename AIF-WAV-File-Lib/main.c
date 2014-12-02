//
//  main.c
//  AIF-WAV-File-Lib
//
//  Created by Pierre Alexandre Tremblay on 29/11/14.
//  Copyright (c) 2014 Pierre Alexandre Tremblay. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "extended.h"

long audiofile_header_extractor(FILE *inputfile, float *SR, unsigned int *nbchan, unsigned int *depth, unsigned char *isfloat, unsigned int *frames);

int main(int argc, const char * argv[])
{
    unsigned int nbchan, depth, frames;
    float SR;
    long index;
    unsigned char isfloat;
    
    FILE *inputfile = NULL;
    
    inputfile = fopen(argv[1], "r");
    
    if (inputfile == NULL)
    {
        printf("no file arg\r");
        return -1;
    }
    
    index = audiofile_header_extractor(inputfile,&SR,&nbchan,&depth,&isfloat,&frames);
    
    //deal with negative indices as error
    
    printf("SR = %lf samps/sec\rnbchan = %d\rdepth = %d byte per sample\r", SR, nbchan, depth);
    printf("nb of frames = %u\r",frames);
    
    // read the file in a buffer
    
    //close the file
    fclose(inputfile);
    
    return 0;
}

long audiofile_header_extractor(FILE *inputfile, float *SR, unsigned int *nbchan, unsigned int *depth, unsigned char *isfloat, unsigned int *frames)
{
    unsigned char FileHead[12], WavTemp[8], *aChunk, AIFCflag;
    unsigned int n;
    
    //sort ENDIAN - AIF compression type to sowt (none is big-endian, sowt is little endian
    //for WAV - if riff its little, if RIFX it is big
    
    
    // checks the file is a legit AIFF or WAV by importing the header
    fread(FileHead, 1, 12, inputfile);
    
    //selects aiff or aifc
    if (strncmp((char *)FileHead, "FORM", 4) == 0)
    {
        //check if 'only' an AIFF
        if (strncmp((char *)FileHead+8, "AIFF", 4) == 0)
            AIFCflag = 0;
        else if (strncmp((char *)FileHead+8, "AIFC", 4) == 0)
            AIFCflag = 1;
        else
        {
            printf("this is an unknown AIFx\r");
            return -1;
        }
        printf("AIFCflag = %d\r",AIFCflag);
        
        //check for the different headers and size
        while(fread(WavTemp, 1, 8, inputfile))
        {
            n = (unsigned int)WavTemp[4]<<24 | (unsigned int)WavTemp[5]<<16 | (unsigned int)WavTemp[6]<<8 | (unsigned int)WavTemp[7];
            //the version header
            if (strncmp((char *)WavTemp, "FVER", 4) == 0)
            {
                //                printf("in FVER\r");
                aChunk = malloc(n);
                fread(aChunk, 1, n, inputfile);
                n = (unsigned int)aChunk[0]<<24 | (unsigned int)aChunk[1]<<16 | (unsigned int)aChunk[2]<<8 | (unsigned int)aChunk[3];
                if (n != 0xA2805140)
                {
                    printf("wrong version of AIFC\r");
                    free(aChunk);
                    return -1;
                }
                free(aChunk);
            }
            // the common header
            else if (strncmp((char *)WavTemp, "COMM", 4) == 0)
            {
                //                printf("in COMM\r");
                aChunk = malloc(n);
                fread(aChunk, 1, n, inputfile);
                
                *nbchan = (unsigned int)aChunk[0]<<8 | (unsigned int)aChunk[1];
                *depth =(unsigned int)aChunk[6]<<8 | (unsigned int)aChunk[7];
                *frames = (unsigned int)aChunk[2]<<24 | (unsigned int)aChunk[3]<<16 | (unsigned int)aChunk[4]<<8 | (unsigned int)aChunk[5];
                //type of compression accepted (none or float32)
                // if aifc it can be float
                if (AIFCflag)
                {
                    if (strncmp((char *)aChunk+18, "NONE", 4) == 0)
                    {
                        //                       printf("INT\r");
                    }
                    else if (strncmp((char *)aChunk+18, "FL32", 4) == 0)
                    {
                        //                       printf("FLOAT\r");
                    }
                    else
                    {
                        printf("neither PCM-int or FL32 AIFC\r");
                        free(aChunk);
                        return -1;
                    }
                }
                // converts the sampling rate 80bit to double
                //    printf("size of SR = %ld\r", sizeof(&SR));
                // tried with http://blogs.perl.org/users/rurban/2012/09/reading-binary-floating-point-numbers-numbers-part2.html but was not working. Found the old apple conversion routine
                *SR = _af_convert_from_ieee_extended((unsigned char *)aChunk+8);
                
                free(aChunk);
            }
            else if (strncmp((char *)WavTemp, "SSND", 4) == 0)
            {
                //              printf("in SSND\r");
                aChunk = malloc(n);
                fread(aChunk, 1, n, inputfile);
                //do something with these samples
                free(aChunk);
                return 1;
            }
            else
                // jumps the chunk
            {
                //               printf("in %s\r",WavTemp);
                fseek(inputfile, n, SEEK_CUR);
            }
        }
        if (!frames)
        {
            printf("no data\r");
            return -1;
        }
    }
    // OR checks the file is a legit WAV and process
    else if (strncmp((char *)FileHead, "RIFF", 4) == 0)
    {
        if (strncmp((char *)FileHead+8, "WAVE", 4))
        {
            printf("NoWAVE\r");
            return -1;
        };
        
        // check the different parameters
        // imports the first chunk + the header of the 2nd
        fread(WavTemp, 1, 8, inputfile);
        
        //check proper header
        if (strncmp((char *)WavTemp, "fmt ", 4))
        {
            printf("NoFMT\r");
            return -1;
        };
        
        n = (unsigned int)WavTemp[7]<<24 | (unsigned int)WavTemp[6]<<16 | (unsigned int)WavTemp[5]<<8 | (unsigned int)WavTemp[4];
        
        printf("WaveFile\r");
        
        // import the rest of the format chunk
        aChunk= malloc(n);
        fread(aChunk, 1, n, inputfile);
        
        //check if PCM or float
        if (aChunk[1]== 0)
        {
            //safety check the datachunk
            //transfer useful data from the chunk
            *nbchan = (unsigned int)aChunk[3]<<8 | (unsigned int)aChunk[2];
            *SR = (double)((unsigned int)aChunk[7]<<24 | (unsigned int)aChunk[6]<<16 | (unsigned int)aChunk[5]<<8 | (unsigned int)aChunk[4]);
            *depth =(unsigned int)aChunk[15]<<8 | (unsigned int)aChunk[14];
            free(aChunk);
            
            // IF PCM or FLOAT
            if (aChunk[0] == 1)
            {
                printf("int\r");
            }
            // IF FLOAT
            else if (aChunk[0] == 3)
            {
                printf("float\r");
            }
            else
            {
                printf("NoINTofFLOAT\r");
                return -1;
            }
            
            while(fread(WavTemp, 1, 8, inputfile))
            {
                n = (unsigned int)WavTemp[7]<<24 | (unsigned int)WavTemp[6]<<16 | (unsigned int)WavTemp[5]<<8 | (unsigned int)WavTemp[4];
                //looking for the data header
                if (strncmp((char *)WavTemp, "data", 4) == 0)
                {
                    //                    printf("in data\r");
                    *frames = ((unsigned int)WavTemp[7]<<24 | (unsigned int)WavTemp[6]<<16 | (unsigned int)WavTemp[5]<<8 | (unsigned int)WavTemp[4]) * 8 / *depth / *nbchan;
                    
                    //transfer the data
                    aChunk = malloc(n);
                    // do something with the sounds here
                    free(aChunk);
                    return 1;
                }
                else
                {
                    //                    printf("in %s\r",WavTemp);
                    fseek(inputfile, n, SEEK_CUR);
                }
                
            }
            
            if (!frames)
            {
                printf("no data\r");
                return -1;
            }
            
        }
        else
        {
            printf("NoPCM\r");
            free(aChunk);
            return -1;
        }
    }
    // OR discarts
    else
    {
        printf("Not a supported filetype\r");
        return -1;
    };
    return -1;
}