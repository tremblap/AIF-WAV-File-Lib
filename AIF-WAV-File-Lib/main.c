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

int main(int argc, const char * argv[])
{
    long i;
    unsigned int nbchan, sampdepth, frames = 0, n;
    unsigned char FileHead[12], WavTemp[8], *aChunk, AIFCflag;
    double SR;
    FILE *inputfile = NULL;
    
    inputfile = fopen(argv[1], "r");
    
    if (inputfile == NULL)
    {
        printf("no file arg\r");
        return -1;
    }
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
            fclose(inputfile);
            return 0;
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
                    fclose(inputfile);
                    free(aChunk);
                    return 0;
                }
                free(aChunk);
            }
            // the common header
            else if (strncmp((char *)WavTemp, "COMM", 4) == 0)
            {
//                printf("in COMM\r");
                aChunk = malloc(n);
                fread(aChunk, 1, n, inputfile);
                
                nbchan = (unsigned int)aChunk[0]<<8 | (unsigned int)aChunk[1];
                sampdepth =(unsigned int)aChunk[6]<<8 | (unsigned int)aChunk[7];
                frames = (unsigned int)aChunk[2]<<24 | (unsigned int)aChunk[3]<<16 | (unsigned int)aChunk[4]<<8 | (unsigned int)aChunk[5];
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
                        fclose(inputfile);
                        free(aChunk);
                        return 0;
                    }
                }
                // converts the sampling rate 80bit to double
                //    printf("size of SR = %ld\r", sizeof(&SR));
                // tried with http://blogs.perl.org/users/rurban/2012/09/reading-binary-floating-point-numbers-numbers-part2.html but was not working. Found the old apple conversion routine
                SR = _af_convert_from_ieee_extended((unsigned char *)aChunk+8);
                
                free(aChunk);
            }
            else if (strncmp((char *)WavTemp, "SSND", 4) == 0)
            {
  //              printf("in SSND\r");
                aChunk = malloc(n);
                fread(aChunk, 1, n, inputfile);
                //do something with these samples
                free(aChunk);
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
            fclose(inputfile);
            return -5;
        }

        fclose(inputfile);
    }
    // OR checks the file is a legit WAV and process
    else if (strncmp((char *)FileHead, "RIFF", 4) == 0)
    {
        if (strncmp((char *)FileHead+8, "WAVE", 4))
        {
            printf("NoWAVE\r");
            fclose(inputfile);
            return -2;
        };
        
        // check the different parameters
        // imports the first chunk + the header of the 2nd
        fread(WavTemp, 1, 8, inputfile);
        
        //check proper header
        if (strncmp((char *)WavTemp, "fmt ", 4))
        {
            printf("NoFMT\r");
            fclose(inputfile);
            return -3;
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
            nbchan = (unsigned int)aChunk[3]<<8 | (unsigned int)aChunk[2];
            SR = (double)((unsigned int)aChunk[7]<<24 | (unsigned int)aChunk[6]<<16 | (unsigned int)aChunk[5]<<8 | (unsigned int)aChunk[4]);
            sampdepth =(unsigned int)aChunk[15]<<8 | (unsigned int)aChunk[14];
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
                fclose(inputfile);
                return -3;
            }
            
            while(fread(WavTemp, 1, 8, inputfile))
            {
                n = (unsigned int)WavTemp[7]<<24 | (unsigned int)WavTemp[6]<<16 | (unsigned int)WavTemp[5]<<8 | (unsigned int)WavTemp[4];
                //looking for the data header
                if (strncmp((char *)WavTemp, "data", 4) == 0)
                {
//                    printf("in data\r");
                    frames = ((unsigned int)WavTemp[7]<<24 | (unsigned int)WavTemp[6]<<16 | (unsigned int)WavTemp[5]<<8 | (unsigned int)WavTemp[4]) * 8 / sampdepth / nbchan;

                    //transfer the data
                    aChunk = malloc(n);
                    // do something with the sounds here
                    free(aChunk);
                }
                else
                {
//                    printf("in %s\r",WavTemp);
                    fseek(inputfile, n, SEEK_CUR);
                }
                
            }
            fclose(inputfile);
            
            if (!frames)
            {
                printf("no data\r");
                fclose(inputfile);
                return -5;
            }
            
        }
        else
        {
            printf("NoPCM\r");
            fclose(inputfile);
            free(aChunk);
            return -3;
        }
    }
    // OR discarts
    else
    {
        printf("Not a supported filetype\r");
        fclose(inputfile);
        return -2;
    };
    
    
    
    printf("SR = %lf samps/sec\rnbchan = %d\rsampdepth = %d bit per sample\r", SR, nbchan, sampdepth);
    printf("nb of frames = %ld\r",frames);
    
    
    //close the file
    fclose(inputfile);
    
    return 0;
}
