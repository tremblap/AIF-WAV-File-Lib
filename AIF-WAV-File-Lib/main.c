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
    fread(WavFmt, 1, 24, inputfile);
    
    //close the file
    fclose(inputfile);
    
    for (n=0; n<4; n++)
        printf("%c", WavHead[n]);
    printf("\n");
    
    for (n=8; n<12; n++)
        printf("%c", WavHead[n]);
    printf("\n");
    
    
    for (n=0; n<4; n++)
        printf("%c", WavFmt[n]);
    printf("\n");
  
    
    return 0;
}

