/*
This is modified from Stefan Deian's QMCS 1.0.

QMCS - Quine McCluskey Simplifier. This program uses the Quine McCluskey
method to mimize a boolean function. Inputs are decimal numbers, of which
the binary representation of that number is used to determine the prime
implicants (p_group) and then the essential prime implicants(final_gorup

Copyright (C) 2005 Deian Stefan   stefan (at) cooper (dot) edu
   
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include "qmc.h"
#include "error.h"
using namespace std;

int MIN_BITS = 1;   //minimum bits to print
vector<unsigned> input_values;  
bool show_mid = false;    //show middle process

vector<vector<B_number> > table;  //original table
vector<vector<B_number> > p_group;  //mid process table
vector<vector<B_number> > final_group;  //final table
vector<B_number> printed_numbers; //avoid printing the same final numbers 
vector<B_number> written_numbers; //avoid rewriting the same config on further
                                  //interconnects



void qmc_pla_gen( int* current_interconnect, int current_segment,
                  segment_t* subseg, char* and_plane_conf, char* or_plane_conf,
                  int arch_selectorBits, int arch_segmentBits,
                  int arch_plaInterconnects){
  /**
    * Generate configuration for the PLA for one segment.
    * current_interconnect points to the current index of
    * the interconnect line and is updated before return.
    * This is modified from Deian Stefan's QMCS 1.0, thanks!
    */

  getinput( subseg);
  if (MIN_BITS<arch_segmentBits) MIN_BITS=arch_segmentBits;
  init( current_interconnect, current_segment, and_plane_conf, or_plane_conf,
        arch_selectorBits, arch_segmentBits,arch_plaInterconnects);
}

/* counts 1s by getting the LSB (%2) and then shifting until 0 */
unsigned count_1s(unsigned number) {
  short bit =0;
  int count = 0;
  while(number>0) {
    bit = number%2;
    number>>=1;
    if(bit) {
      count++;
    }
  }
  return count;
}
/*get LSB, arrange it in array, the print array in reverse order so MSB is on
the left */
void print_binary(unsigned number) {
  unsigned bits[MIN_BITS];
  int count = 0;
  
  while(number>0||count<MIN_BITS) {
    bits[count] = number%2;
    number>>= 1;
    count++;
  }
  for(int i=count-1;i>=0;i--)
    cout<<bits[i];
}
/*creating first table: append current number to the array located in
table[number of 1s f this number]*/
void create_table() {
  short tmp;
  B_number temp_num;
  for(size_t i=0;i<input_values.size();i++) {
    tmp = count_1s(input_values[i]);
    if(tmp+1>(ssize_t)table.size())
      table.resize(tmp+1);
    
    temp_num = init_B_number(input_values[i],0,false);
    table[tmp].push_back(temp_num);
  }
}

void print_table() {
  
  cout<<endl<<"COMPUTING:"<<endl;
  for(size_t i=0;i<table.size();i++) {
    cout<<i;
    for(size_t j=0;j<table[i].size();j++) {
      cout<<"\tm"<<table[i][j].number<<"\t";
      print_binary(table[i][j].number);
      cout<<endl;
    }
    cout<<"\n-------------------------------------"<<endl;
  }
}
/* initialize a B_number variable - like a constructor */
B_number init_B_number(unsigned n,int d, bool u) {
  B_number num;
  num.number = n;
  num.dashes = d;
  num.used = u;
  return num;
}
/*like the original table, but the paring of numbers from the original table-
dashes are represented by a 1. for example original A=0011 B=1011, new number 
is -011 which is represented as C.number=A&B=0011,C.dashes=A^B=1000*/
void create_p_group() {
  short tmp;
  B_number temp_num;
  unsigned temp_number, temp_dashes;
  for(size_t i=0;i<table.size()-1;i++) {
    for(size_t j=0;j<table[i].size();j++) {
      for(size_t k=0;k<table[i+1].size();k++) {
        temp_number = table[i][j].number & table[i+1][k].number;
        temp_dashes = table[i][j].number ^ table[i+1][k].number;
        
        if(count_1s(temp_dashes)==1) {
          table[i][j].used = true;
          table[i+1][k].used = true;
          
          
          tmp = count_1s(temp_number);
          
          if(tmp+1>(ssize_t)p_group.size())
            p_group.resize(tmp+1);
          
          temp_num = init_B_number(temp_number, temp_dashes, false);
          p_group[tmp].push_back(temp_num);
        }
      }
    }
  }
}

void print_p_group() {
  cout<<endl<<"MID PROCESS COMPUTATION:"<<endl;
  
  for(size_t i=0;i<p_group.size();i++) {
    cout<<i;
    for(size_t j=0;j<p_group[i].size();j++) {
      cout<<"\t\t";
      print_p_binary(p_group[i][j].number,p_group[i][j].dashes);
      cout<<endl;
    }
    cout<<"\n-------------------------------------"<<endl;
  }
  
}
/*print a number such as -001; this allocates bits in an array dash=2 then 
prints reverse array */
void print_p_binary(unsigned n, unsigned d) {
  unsigned bits[MIN_BITS];
  int count = 0;
  
  while(n>0||count<MIN_BITS) {
    if(!(d%2))
      bits[count] = n%2;
    else
      bits[count] = 2;
    
    n >>= 1;
    d >>= 1;
    count++;
  }
  for(int i=count-1;i>=0;i--) {
    if(bits[i]!=2)
      cout<<bits[i];
    else
      cout<<"-";
  }
}
/*creates final table. works like p_group(). example; in p_group you have: 
A=-001 B=-011 -> C= -0-1 which will be represented as 
C.number=A&B=0001&0011=0001, and C.dashes=A^B^A.dashes=0001^0011^1000=1010. 
Computation is done only when A.dashes = b.dashes*/
void create_final_group() {
  short tmp;
  B_number temp_num;
  unsigned temp_number, temp_dashes;
  for(size_t i=0;i<p_group.size()-1;i++) {
    for(size_t j=0;j<p_group[i].size();j++) {
      for(size_t k=0;k<p_group[i+1].size();k++) {
        if(p_group[i][j].dashes == p_group[i+1][k].dashes) {
          temp_number = p_group[i][j].number & p_group[i+1][k].number;
          temp_dashes = p_group[i][j].number ^ p_group[i+1][k].number;
          if(count_1s(temp_dashes)==1) {
            temp_dashes^= p_group[i][j].dashes;
            
            p_group[i][j].used = true;
            p_group[i+1][k].used = true;
            
            tmp = count_1s(temp_number);
            
            if(tmp+1>(ssize_t)final_group.size())
              final_group.resize(tmp+1);
            
            temp_num = init_B_number(temp_number, temp_dashes, true);
            final_group[tmp].push_back(temp_num);
          }
        }
      }
    }
  }
}
/*print all the values from the final table, except for duplicates.
  print all the unused numbers from original table and mid process table*/
void print_final_group() {
  //cout<<endl<<"FINAL:\n-------------------------------------"<<endl;
  size_t i,j;
  for(i=0;i<final_group.size();i++) {
    for(j=0;j<final_group[i].size();j++) {
      if(!is_printed(final_group[i][j])) {
        print_p_binary(final_group[i][j].number,final_group[i][j].dashes);
        cout<<endl;
        printed_numbers.push_back(final_group[i][j]);
      }
    }
  }
  for(i=0;i<p_group.size();i++) {
    for(j=0;j<p_group[i].size();j++) {
      if(!p_group[i][j].used) {
        print_p_binary(p_group[i][j].number,p_group[i][j].dashes);
        cout<<endl;
      }
    }
  }
  for(i=0;i<table.size();i++) {
    for(j=0;j<table[i].size();j++) {
      if(!table[i][j].used) {
        print_p_binary(table[i][j].number,table[i][j].dashes);
        cout<<endl;
      }
    }
  }
  //cout<<"-------------------------------------"<<endl;
}
/*used to avoid printing duplicates that can exist in the final table*/
bool is_printed(B_number n) {
  for(size_t i=0;i<printed_numbers.size();i++)
    if(n.number==printed_numbers[i].number && n.dashes == printed_numbers[i].dashes)
      return true;
  
  return false;
}

/*used to avoid writing the same config on further interconnects*/
bool is_written(B_number n) {
  for(size_t i=0;i<written_numbers.size();i++)
    if(n.number==written_numbers[i].number && n.dashes == written_numbers[i].dashes)
      return true;
  
  return false;
}

void write_interconnect(unsigned n, unsigned d, int* current_interconnect,
                       int current_segment, char* and_plane_conf,
                       char* or_plane_conf, int arch_selectorBits,
                       int arch_segmentBits, int arch_plaInterconnects){
  int count = 0;
  unsigned bits[arch_selectorBits];

  if (*current_interconnect>=arch_plaInterconnects)
    throw HWResourceExceededError(HWResourceExceededError::PLAInterconnects);

  // transfer Minterm from n (containing 1 or 0),
  // and d (containing don't-cares) into
  // containing an field for each bit.
  while(count<arch_selectorBits) {
    if(!(d%2)) // if last bit don't-care not set:
      bits[count] = n%2; //last bit 1 or 0
    else
      bits[count] = 2; //don't-care encoded as 2

    n >>= 1; //remove last bit from binary input
    d >>= 1; //remove last bit from don't-care input
    count++;
  }
  // config and-plane for one interconnect
  // normal, then inverted selector lines
  //printf("AND plane protocol: ");
  for(int i=0;i<count;i++){
    //cout << "and-normal-index: " << *current_interconnect*2*arch_selectorBits+i;
    //cout << ", and-inverted-index: " << (*current_interconnect*2+1)*arch_selectorBits+i << endl;
    if(bits[i]==1){ // normal selector line must be connected
      //and_plane_conf[*current_interconnect*2*arch_selectorBits+i]=true;
      and_plane_conf[*current_interconnect*2*arch_selectorBits+i]=false;
      and_plane_conf[(*current_interconnect*2+1)*arch_selectorBits+i]=true;
    }else if(bits[i]==0){ // inverted selector line must be connected
      and_plane_conf[*current_interconnect*2*arch_selectorBits+i]=true;
      and_plane_conf[(*current_interconnect*2+1)*arch_selectorBits+i]=false;
      //and_plane_conf[(*current_interconnect*2+1)*arch_selectorBits+i]=false;
    }else{ // don't-care: both must not be connected
      and_plane_conf[*current_interconnect*2*arch_selectorBits+i]=false;
      and_plane_conf[(*current_interconnect*2+1)*arch_selectorBits+i]=false;
    }
  }
  // config or-plane for that same interconnect
  int tmp_cs = current_segment;
  //cout << "and plane written, current_segment: " << tmp_cs <<
  //         ", current_interconnect: " << *current_interconnect << endl;
  for(int i=0;i<arch_segmentBits;i++) {
    if((tmp_cs%2)==1){
      or_plane_conf[*current_interconnect+arch_plaInterconnects*i] = true;
    } else {
      or_plane_conf[*current_interconnect+arch_plaInterconnects*i] = false;
    }
    tmp_cs >>= 1;
  }
  (*current_interconnect)++;
}

/*write pla config (and, or planes) to and_plane_conf and or_plane_conf*/
void write_pla_config( int* current_interconnect, int current_segment,
                       char* and_plane_conf, char* or_plane_conf,
                       int arch_selectorBits, int arch_segmentBits,
                       int arch_plaInterconnects){
  //cout << "current interconnect: " << *current_interconnect << endl;
  size_t i,j;
  for(i=0;i<final_group.size();i++) {
    for(j=0;j<final_group[i].size();j++) {
      if(!is_written(final_group[i][j])) {
        write_interconnect(final_group[i][j].number,final_group[i][j].dashes,
                         current_interconnect, current_segment, and_plane_conf,
                         or_plane_conf, arch_selectorBits, arch_segmentBits,
                         arch_plaInterconnects);
        written_numbers.push_back(final_group[i][j]);
      }
    }
  }
  for(i=0;i<p_group.size();i++) {
    for(j=0;j<p_group[i].size();j++) {
      if(!p_group[i][j].used) {
        write_interconnect(p_group[i][j].number,p_group[i][j].dashes,
                         current_interconnect, current_segment, and_plane_conf,
                         or_plane_conf, arch_selectorBits, arch_segmentBits,
                         arch_plaInterconnects);
      }
    }
  }
  for(i=0;i<table.size();i++) {
    for(j=0;j<table[i].size();j++) {
      if(!table[i][j].used) {
        write_interconnect(table[i][j].number,table[i][j].dashes,
                         current_interconnect, current_segment, and_plane_conf,
                         or_plane_conf, arch_selectorBits, arch_segmentBits,
                         arch_plaInterconnects);
      }
    }
  }
}

/*initialize all table*/
void init( int* current_interconnect, int current_segment,
           char* and_plane_conf, char* or_plane_conf,
           int arch_selectorBits, int arch_segmentBits,
           int arch_plaInterconnects){
  table.clear();
  p_group.clear();
  final_group.clear();
  printed_numbers.clear();
  table.resize(1);
  p_group.resize(1);
  final_group.resize(1);
  create_table();
  //print_table();
  create_p_group();
  //if(show_mid)
  //  print_p_group();
  create_final_group();
  write_pla_config( current_interconnect, current_segment,
                    and_plane_conf, or_plane_conf,
                    arch_selectorBits, arch_segmentBits,
                    arch_plaInterconnects);
  //print_final_group();
}

void getinput( segment_t* segment) {
  int num_bits=0;
  input_values.clear();
  for( uint32_t in = segment->prefix; in < segment->prefix + segment->width; in++){
    //cout<<"in: " << in << "\n";
    input_values.push_back(in);
    num_bits = count_bits(in);  
    if(num_bits>MIN_BITS)
      MIN_BITS = num_bits;
  }
}
/*return min number of bits a number is represented by. used for best output*/
unsigned count_bits(unsigned n) {
  int count = 0;
  while(n>0) {
    n>>=1;
    count++;
  }
  return count;
}
