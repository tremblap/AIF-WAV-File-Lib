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
    unsigned char WavHead[12], WavFmt[32];
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

    if (strncmp(WavHead, "RIFF", 4))
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
    fread(WavFmt, 1, 32, inputfile);
    
    //check proper header
    if (strncmp(WavFmt, "fmt ", 4))
    {
        printf("NoFMT\r");
        fclose(inputfile);
        return -3;
    };
    
    //check if PCM or float
    if (WavFmt[9]== 0)
    {
        
        if (WavFmt[8] == 1)
        {
            //safety check the datachunk
            if (strncmp(WavFmt+24, "data", 4))
            {
                printf("Nodata\r");
                fclose(inputfile);
                return -3;
            }
            //transfer useful data from the chunk
            
            nbchan = (unsigned int)WavFmt[11]<<8 | (unsigned int)WavFmt[10];
            SR =(unsigned long)WavFmt[15]<<24 | (unsigned long)WavFmt[14]<<16 | (unsigned long)WavFmt[13]<<8 | (unsigned long)WavFmt[12];
            sampdepth =(unsigned int)WavFmt[23]<<8 | (unsigned int)WavFmt[22];
            datachunksize = (unsigned long)WavFmt[31]<<24 | (unsigned long)WavFmt[30]<<16 | (unsigned long)WavFmt[29]<<8 | (unsigned long)WavFmt[28];

        }
        else if (WavFmt[8] == 3)
        {
            printf("thisisafloat\r");
            printf("extchunksize = %d",((unsigned int)WavFmt[25]<<8 | (unsigned int)WavFmt[24]));
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

