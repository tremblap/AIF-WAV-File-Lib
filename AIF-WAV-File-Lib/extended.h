// defines Apple's routines for the compiler

// not needed but in the legally uneditable extended.c
void _af_convert_to_ieee_extended (double num, unsigned char *bytes);

// this is what we need
double _af_convert_from_ieee_extended (const unsigned char *bytes);
