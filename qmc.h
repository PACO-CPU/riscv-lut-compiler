#ifndef RISCV_LUT_COMPILER_QMC_H
#define RISCV_LUT_COMPILER_QMC_H

#include "lut.h"

struct B_number{
  unsigned number;
  unsigned dashes;
  bool used;
};


//----------------------------------------------------------
void qmc_pla_gen( int* current_interconnect, segment_t* subseg,
                  char* and_plane_conf, char* or_plane_conf,
                  int arch_selectorBits, int arch_segmentBits); //gen PLA conf
unsigned count_1s(unsigned number); //count the number of 1s in a number
void print_binary(unsigned number);//print the number in binary
void create_table();            //create original table sorted by the number of 1s
void print_table();             //print the table
B_number init_B_number(unsigned n,int d, bool u);//initialize a B_number
void create_p_group();          //create mid process table
void print_p_group();           //print it
void print_p_binary(unsigned n, unsigned d);//print the mid table (with -'s)
void create_final_group();              //create final table
void print_final_group();               //print final table with -'s and unused terms
bool is_printed(B_number n);            //dont print terms that were already printed
bool is_written(B_number n);
void write_interconnect(unsigned n, unsigned d, int* current_interconnect,
                        char* and_plane_conf, char* or_plane_conf,
                        int arch_selectorBits, int arch_segmentBits);
void write_pla_config( int* current_interconnect, //write pla config
                       char* and_plane_conf, char* or_plane_conf,
                       int arch_selectorBits, int arch_segmentBits);
void init( int* current_interconnect,  //start the table making and printing
                       char* and_plane_conf, char* or_plane_conf,
                       int arch_selectorBits, int arch_segmentBits);
void getinput( segment_t* segment);     //get input from segment
unsigned count_bits(unsigned n);        //min bits to represent a number
//----------------------------------------------------------

#endif
