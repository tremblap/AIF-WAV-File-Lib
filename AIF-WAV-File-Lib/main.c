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
    int n, SR, nbchan, samptype;
    FILE *inputfile = NULL;
    unsigned char WavHead[12];
    unsigned char WavFmt[24];
    unsigned long datachunksize = 0L;
    
    for (n=0;n<argc;n++)
    {
        //printf("argc %d is %s\r", n, argv[n]);
        //printf ("%d\r", strcmp (argv[n],"-header"));
        
        if (strcmp (argv[n],"-header") == 0)
        {
            printf("in\r");
            SR = argv[n + 1];
            nbchan = argv[n + 2];
            samptype = argv[n +3];
        }
        else
        {
            inputfile = fopen(argv[n], "r");
        }
    }
    // printf("SR = %d\rnbchan = %d\rsamptype = %d\r", SR, nbchan, samptype);
    if (inputfile == NULL)
    {
        printf("no file arg\r");
        return -1;
    }
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

    datachunksize = (unsigned long)WavHead[7]<<24 | (unsigned long)WavHead[6]<<16 | (unsigned long)WavHead[5]<<8 | (unsigned long)WavHead[4];
    
    datachunksize -= 36;
    
    printf("size of chunk b = %ld\r",datachunksize);

//    fread(WavFmt, 1, 24, inputfile);
    
    //close the file
    fclose(inputfile);
    
    return 0;
}

