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
    unsigned int nbchan, sampdepth;
    FILE *inputfile = NULL;
    unsigned char FileHead[12], WavFmt[48], WavTemp[8], *aChunk;
    unsigned long datachunksize = 0L, n;
    double SR;
    
//    n = 0;
//    if (strcmp (argv[0],"-header") == 0)
//        {
//            printf("manual header munging\r");
//            SR = atof(argv[1]);
//            nbchan = atoi(argv[2]);
//            sampdepth = atoi(argv[3]);
//            n = 4;
//        }

    inputfile = fopen(argv[1], "r");
    
    if (inputfile == NULL)
    {
        printf("no file arg\r");
        return -1;
    }
    
    
    // checks the file is a legit AIFF or WAV by importing the header
    fread(FileHead, 1, 12, inputfile);

    //selects aiff or aifc
    if (strncmp((char *)FileHead, "FORM", 4) == 0)
    {
        if (strncmp((char *)FileHead+8, "AIFC", 4) == 0)
        {
            //check for the different headers and size
            while(fread(WavTemp, 1, 8, inputfile))
            {
                n = (unsigned long)WavTemp[4]<<24 | (unsigned long)WavTemp[5]<<16 | (unsigned long)WavTemp[6]<<8 | (unsigned long)WavTemp[7];
                //the version header
                if (strncmp((char *)WavTemp, "FVER", 4) == 0)
                {
                    aChunk = malloc(n);
                    fread(aChunk, 1, n, inputfile);
                    n = (unsigned long)aChunk[0]<<24 | (unsigned long)aChunk[1]<<16 | (unsigned long)aChunk[2]<<8 | (unsigned long)aChunk[3];
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
                    aChunk = malloc(n);
                    fread(aChunk, 1, n, inputfile);
                    
                    nbchan = (unsigned int)aChunk[0]<<8 | (unsigned int)aChunk[1];
                    datachunksize = ((unsigned long)aChunk[2]<<24 | (unsigned long)aChunk[3]<<16 | (unsigned long)aChunk[4]<<8 | (unsigned long)aChunk[5]) * nbchan;
                    sampdepth =(unsigned int)aChunk[6]<<8 | (unsigned int)aChunk[7];
                    //type of compression accepted (none or float32)
                    if (strncmp((char *)aChunk+18, "NONE", 4) == 0)
                         {
                             printf("INT\r");
                         }
                    else if (strncmp((char *)aChunk+18, "FL32", 4) == 0)
                         {
                             printf("FLOAT\r");
                         }
                    else
                    {
                        printf("neither PCM-int or FL32 AIFC\r");
                        fclose(inputfile);
                        free(aChunk);
                        return 0;
                    }
                    
                    // converts the sampling rate 80bit to double
                    //    printf("size of SR = %ld\r", sizeof(&SR));
                    SR = _af_convert_from_ieee_extended((unsigned char *)aChunk+8);
                    
                    free(aChunk);
                }
                else if (strncmp((char *)WavTemp, "SSND", 4) == 0)
                {
                    //stuff importing sounds
                }
                else
                {
                    //skip ahead in the file for the size of the chunk
                }
            }
            fclose(inputfile);
        }
        else if (strncmp((char *)FileHead+8, "AIFF", 4) == 0)
        {
            printf("this is an AIFF\r");
            fclose(inputfile);
            return 0;
        }
        else
        {
            printf("this is an unknown AIFx\r");
            fclose(inputfile);
            return 0;
        };
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
        fread(WavFmt, 1, 24, inputfile);
        
        //check proper header
        if (strncmp((char *)WavFmt, "fmt ", 4))
        {
            printf("NoFMT\r");
            fclose(inputfile);
            return -3;
        };
        
        n = (unsigned int)WavFmt[7]<<24 | (unsigned int)WavFmt[6]<<16 | (unsigned int)WavFmt[5]<<8 | (unsigned int)WavFmt[4];
        
        //    printf("%d\r",n);
        
        // import the rest of the format chunk
        if (n > 40)
        {
            printf("problem with extension size\r");
            fclose(inputfile);
            return -3;
        }
        else if (n != 16)
        {
            fread(WavFmt+24, 1, n-16, inputfile);
        }
        
        //check if PCM or float
        if (WavFmt[9]== 0)
        {
            // IF PCM
            if (WavFmt[8] == 1)
            {
                //safety check the datachunk
                fread(WavTemp, 1, 4, inputfile);
                if (strncmp((char *)WavTemp, "data", 4))
                {
                    printf("Not data\r");
                    fclose(inputfile);
                    return -4;
                }
                
                //transfer useful data from the chunk
                nbchan = (unsigned int)WavFmt[11]<<8 | (unsigned int)WavFmt[10];
                SR = (double)((unsigned long)WavFmt[15]<<24 | (unsigned long)WavFmt[14]<<16 | (unsigned long)WavFmt[13]<<8 | (unsigned long)WavFmt[12]);
                sampdepth =(unsigned int)WavFmt[23]<<8 | (unsigned int)WavFmt[22];
                fread(WavTemp, 1, 4, inputfile);
                datachunksize = (unsigned long)WavTemp[3]<<24 | (unsigned long)WavTemp[2]<<16 | (unsigned long)WavTemp[1]<<8 | (unsigned long)WavTemp[0];
                
            }
            // IF FLOAT
            else if (WavFmt[8] == 3)
            {
                //check next chunk and dismisses
                fread(WavTemp, 1, 4, inputfile);
                
                //check for fact chunk
                if (strncmp((char *)WavTemp, "fact", 4) == 0)
                {
                    printf("fact chunk dismissed\r");
                    fread(WavTemp, 1, 4, inputfile);
                    n = (unsigned long)WavTemp[3]<<24 | (unsigned long)WavTemp[2]<<16 | (unsigned long)WavTemp[1]<<8 | (unsigned long)WavTemp[0];
                    for (i = (long)n;i>=0;n-=4)
                        fread(WavTemp, 1, 4, inputfile);
                }
                
                // check for data chunk
                if (strncmp((char *)WavTemp, "data", 4))
                {
                    printf("Not data\r");
                    fclose(inputfile);
                    return -5;
                }
                
                //transfer useful data from the chunk
                nbchan = (unsigned int)WavFmt[11]<<8 | (unsigned int)WavFmt[10];
                SR =(unsigned long)WavFmt[15]<<24 | (unsigned long)WavFmt[14]<<16 | (unsigned long)WavFmt[13]<<8 | (unsigned long)WavFmt[12];
                sampdepth =(unsigned int)WavFmt[23]<<8 | (unsigned int)WavFmt[22];
                fread(WavTemp, 1, 4, inputfile);
                datachunksize = (unsigned long)WavTemp[3]<<24 | (unsigned long)WavTemp[2]<<16 | (unsigned long)WavTemp[1]<<8 | (unsigned long)WavTemp[0];
                
            }
            else
            {
                printf("NoINTofFLOAT\r");
                fclose(inputfile);
                return -3;
            }
        }
        else
        {
            printf("NoPCM\r");
            fclose(inputfile);
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
    printf("size of data chunk = %ld bytes of audio\r",datachunksize);

    
    //close the file
    fclose(inputfile);
    
    return 0;
}
