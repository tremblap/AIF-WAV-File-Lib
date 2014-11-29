//
//  main.c
//  AIF-WAV-File-Lib
//
//  Created by Pierre Alexandre Tremblay on 29/11/14.
//  Copyright (c) 2014 Pierre Alexandre Tremblay. All rights reserved.
//

#include <stdio.h>
#include <string.h>

int main(int argc, const char * argv[])
{
    unsigned int n, nbchan, sampdepth;
    FILE *inputfile = NULL;
    unsigned char WavHead[12], WavFmt[48], WavTemp[4];
    unsigned long datachunksize = 0L, SR;
    
    for (n=0;n<argc;n++)
    {
        //printf("argc %d is %s\r", n, argv[n]);
        //printf ("%d\r", strcmp (argv[n],"-header"));
        
        if (strcmp (argv[n],"-header") == 0)
        {
            printf("in\r");
            SR = argv[n + 1];
            nbchan = argv[n + 2];
            sampdepth = argv[n +3];
        }
        else
        {
            inputfile = fopen(argv[n], "r");
        }
    }
    
    if (inputfile == NULL)
    {
        printf("no file arg\r");
        return -1;
    }

    
    // checks the file is a legit WAV
    fread(WavHead, 1, 12, inputfile);

    if (strncmp(WavHead, "FORM", 4) == 0)
    {
        if (strncmp(WavHead+8, "AIFF", 4))
        {
            printf("this is an AIFF\r");
            fclose(inputfile);
            return 0;
        }
        else if (strncmp(WavHead+8, "AIFC", 4))
        {
            printf("this is an AIFC\r");
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
    else if (strncmp(WavHead, "RIFF", 4))
    {
        printf("NoRIFF\r");
        fclose(inputfile);
        return -2;
    };
 
    if (strncmp(WavHead+8, "WAVE", 4))
    {
        printf("NoWAVE\r");
        fclose(inputfile);
        return -2;
    };

    // check the different parameters
    // imports the first chunk + the header of the 2nd
    fread(WavFmt, 1, 24, inputfile);
    
    //check proper header
    if (strncmp(WavFmt, "fmt ", 4))
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
            if (strncmp(WavTemp, "data", 4))
            {
                printf("Not data\r");
                fclose(inputfile);
                return -4;
            }
            
            //transfer useful data from the chunk
            nbchan = (unsigned int)WavFmt[11]<<8 | (unsigned int)WavFmt[10];
            SR =(unsigned long)WavFmt[15]<<24 | (unsigned long)WavFmt[14]<<16 | (unsigned long)WavFmt[13]<<8 | (unsigned long)WavFmt[12];
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
            if (strncmp(WavTemp, "fact", 4) == 0)
            {
                printf("fact chunk dismissed\r");
                fread(WavTemp, 1, 4, inputfile);
                n = (unsigned long)WavTemp[3]<<24 | (unsigned long)WavTemp[2]<<16 | (unsigned long)WavTemp[1]<<8 | (unsigned long)WavTemp[0];
                for (;n>=0;n-=4)
                    fread(WavTemp, 1, 4, inputfile);
            }
            
            // check for data chunk
            if (strncmp(WavTemp, "data", 4))
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
    };
    
    printf("SR = %ld samps/sec\rnbchan = %d\rsampdepth = %d bit per sample\r", SR, nbchan, sampdepth);
    printf("size of data chunk = %ld bytes of audio\r",datachunksize);

    
    //close the file
    fclose(inputfile);
    
    return 0;
}

