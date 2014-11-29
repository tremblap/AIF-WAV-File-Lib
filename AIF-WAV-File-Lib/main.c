//
//  main.c
//  AIF-WAV-File-Lib
//
//  Created by Pierre Alexandre Tremblay on 29/11/14.
//  Copyright (c) 2014 Pierre Alexandre Tremblay. All rights reserved.
//

#include <stdio.h>

int main(int argc, const char * argv[])
{
    int n;
    for (n=0;n<argc;n++)
        printf("argc %d is %s\r", n, argv[n]);
    return 0;
}

