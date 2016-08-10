/** \file riscv-lut-compiler.cpp
  * \brief Main routine for the LUT compiler tool.
  */
#include "weights.h"
#include "lut.h"
#include "segment.h"
#include "error.h"
#include "keyvalue.h"
#include "options.h"
#include "strategies.h"
#include <alpha/alpha.h>

// forward declarations for better overview
static int run_weights_test(options_t &options);
static int run_lut_compilation(options_t &options);
int main(int argn, char **argv);

/** Main toolflow for running a weights test
  */
static int run_weights_test(options_t &options) {
  WeightsTable *tbl=new WeightsTable();
  tbl->grab();
  FILE *fOutput;

  try {
    tbl->parseWeightsFile(options.fnInput.ptr);
  } catch(FileIOException &e) {
    fprintf(
      stderr,"\x1b[31;1mError loading weights file: %s\x1b[30;0m\n",
      e.what());
    return 1;
  } catch(SyntaxError &e) {
    fprintf(
      stderr,"\x1b[31;1mError parsing weights file %s: %s\x1b[30;0m\n",
      options.fnInput.ptr,e.what());
    return 1;
  }
  
  options.computeOutputName();
  fOutput=fopen(options.outputName.ptr,"wb");
  if (!fOutput) {
    fprintf(
      stderr,"\x1b[31;1mError opening output file: %s\x1b[30;0m\n",
      FileIOException(options.outputName).what());
    return 1;
  }

  double min=tbl->first();
  double max=tbl->last();
  double step= 
    (((max-min)>options.maxWeightSteps) || !tbl->isAllIntegers())
    ? (max-min)/(double)options.maxWeightSteps
    : 1;
  seg_data_t v;
  

  if (tbl->isAllIntegers()) {
    for(double p=min;p<=max;p+=step) {
      int64_t pi=(int64_t)p;
      tbl->evaluate(pi,v);
      switch(v.kind) {
        case seg_data_t::Double: 
          fprintf(fOutput,"%li\t%f\n",pi,v.data_f); 
          break;
        case seg_data_t::Integer: 
          fprintf(fOutput,"%li\t%li\n",pi,v.data_i); 
          break;
      }
    }
  } else {
    for(double p=min;p<=max;p+=step) {
      tbl->evaluate(p,v);
      switch(v.kind) {
        case seg_data_t::Double: 
          fprintf(fOutput,"%f\t%f\n",p,v.data_f); 
          break;
        case seg_data_t::Integer: 
          fprintf(fOutput,"%f\t%li\n",p,v.data_i); 
          break;
      }
    }

  }

  fclose(fOutput);
  tbl->drop();;

  return 0;

}

/** Main toolflow for running LUT compilation
  */
static int run_lut_compilation(options_t &options) {
  LookupTable *lut=new LookupTable(options.arch);
  WeightsTable *weights=NULL;
  bool forgo_approximation=false;

  // handle input files (intermediate / input)
  try {
    if (options.fInputIntermediate) {
      lut->parseIntermediateFile(options.fnInput.ptr);
    } else {
      lut->parseInputFile(options.fnInput.ptr);
    }
  } catch(FileIOException &e) {
    fprintf(
      stderr,"\x1b[31;1mError loading lut file: %s\x1b[30;0m\n",
      e.what());
    return 1;
  } catch(SyntaxError &e) {
    fprintf(
      stderr,"\x1b[31;1mError parsing lut file %s: %s\x1b[30;0m\n",
      options.fnInput.ptr,e.what());
    return 1;
  } catch(RuntimeError &e) {
    fprintf(
      stderr,"\x1b[31;1mError evaluating lut file %s: %s\x1b[30;0m\n",
      options.fnInput.ptr,e.what());
    return 1;
    
  }
  
  // load a weights table if specified
  // first try to locate it with the VFS in our options_t and then
  // load it 
  if (lut->fn_weights().len>0) {
    try {
      alp::string fn=options.vfsWeights.locate(lut->fn_weights());
      if (fn.len<1) {
        throw RuntimeError("Unable to locate weights file");
      }
      weights=new WeightsTable();
      weights->grab();
      weights->parseWeightsFile(fn.ptr);
    } catch(FileIOException &e) {
      fprintf(
        stderr,"\x1b[31;1mError loading weights file: %s\x1b[30;0m\n",
        e.what());
      return 1;
    } catch(SyntaxError &e) {
      fprintf(
        stderr,"\x1b[31;1mError parsing weights file %s: %s\x1b[30;0m\n",
        options.fnInput.ptr,e.what());
      return 1;
    } catch(RuntimeError &e) {
      fprintf(
        stderr,"\x1b[31;1mError loading weights file %s: %s\x1b[30;0m\n",
        options.fnInput.ptr,e.what());
      return 1;
    }
  }
  
  // run segmentation and approximation if we read an input file (not 
  // intermediate)
  if (!options.fInputIntermediate) {
    try {
      
      lut->computeSegmentSpace();
      lut->computePrincipalSegments();

      // principal segmentation exhibits the maximum resolution possible.
      // Thus, if it does not use too many segments, we do not want to use
      // any segmentation strategy.

      if (lut->segments().len<=(1uL<<options.arch.segmentBits)) {
        alp::logf(
          "INFO: "
          "principal segmentation is perfect. Forgoing segmentation phase\n",
          alp::LOGT_INFO);
      } else {
        // only perform segmentation strategies if they are actually needed,
        // i.e. we have more 

        // primary segmentation
        lut->clearSegments();

        if (lut->strategy1()!=segment_strategy::INVALID) {
          segment_strategy::get(lut->strategy1())->execute(lut,weights,options);
        } else if (lut->explicit_segments().data().len>0) {
          const alp::array_t<Bounds::interval_t> &intervals=
            lut->explicit_segments().data();
          for(size_t i=0;i<intervals.len;i++)
            lut->addSegment(intervals[i].start,intervals[i].end,true);
          // fixme: think about whether specifying 'true' here makes sense.
        } else {
          throw RuntimeError(
            "No segmentation method (strategy / explicit segments) specified");
        }

        // secondary segmentation (if desired)
        if (lut->strategy2()!=segment_strategy::INVALID) {
          segment_strategy::get(lut->strategy2())->execute(lut,weights,options);
        }
      }
      
      // approximation (if not handled before as part of segmentation)
      if (forgo_approximation) {
      } else if (lut->approximation_strategy()!=approx_strategy::INVALID) {
        approx_strategy::get(lut->approximation_strategy())->execute(
          lut,weights,options);
      } else if (!options.fOutputIntermediate) {
        throw RuntimeError(
          "No approximation strategy specified and not outputting intermediate "
          "representation");
      }
    } catch(RuntimeError &e) {
      fprintf(
        stderr,"\x1b[31;1mError compiling lut file %s: %s\x1b[30;0m\n",
        options.fnInput.ptr,e.what());
      return 1;
    }
    if (options.fGenerateGnuplot) {
      try {
        options.computeOutputName();
        FILE *f=fopen(
          alp::string::Format("%s.dat",options.outputName.ptr).ptr,"w");
        seg_data_t x_raw,y_raw,weight_raw;
        double weight=1;

        fprintf(f,"target\n");
        for(size_t i_segment=0;i_segment<lut->segments().len;i_segment++) {
          const segment_t &seg=lut->segments()[i_segment];
          for(
            uint64_t x=0;
            x<(((uint64_t)seg.width)<<lut->segment_interpolation_bits());
            x++) {
            lut->hardwareToInputSpace(i_segment,x,x_raw);
            lut->evaluate(i_segment,x,y_raw);
            if (weights!=NULL) {
              weights->evaluate(x_raw,weight_raw);
              weight=(double)weight_raw;
            }

            fprintf(f,"%g\t%g\t%g\n",(double)x_raw,(double)y_raw,weight);
          }
        }
        fprintf(f,"\n\n");

        fprintf(f,"segments\n");
        for(size_t i_segment=0;i_segment<lut->segments().len;i_segment++) {
          const segment_t &seg=lut->segments()[i_segment];
          
          lut->hardwareToInputSpace(
            i_segment,0,x_raw);
          lut->hardwareToInputSpace(
            i_segment,
            ((uint64_t)seg.width)<<lut->segment_interpolation_bits(),y_raw);
            fprintf(
              f,"%g\t%g\t%g\t%g\n",
              (double)x_raw,(double)y_raw,(double)seg.y0,(double)seg.y1);
        }
        fprintf(f,"\n\n");

        fclose(f);

        f=fopen(
          alp::string::Format("%s.gnuplot",options.outputName.ptr).ptr,"w");
        
        fprintf(f,
          "set terminal pdf\n"
          "set output '%s.lut.pdf'\n"
          "\n"
          "set title \"visualization of LUT '%s'\"\n"
          "set grid\n"
          "set xlabel \"input word\"\n"
          "set ylabel \"output word\"\n"
          "plot \\\n"
          "  '%s.lut.dat' i 0 "
            "u 1:2 with points title \"target function\", \\\n"
          "  '%s.lut.dat' i 1 "
            "u 1:3:($2-$1):($4-$3) with vectors title \"segments\"\n",
          options.outputBase.ptr,
          options.outputBase.ptr,
          options.outputBase.ptr,
          options.outputBase.ptr);
        fclose(f);

      } catch(RuntimeError &e) {
        fprintf(
          stderr,"\x1b[31;1mError generating gnuplot files for %s: "
          "%s\x1b[30;0m\n",
          options.fnInput.ptr,e.what());
        return 1;
      }

    }
  }
  try {
    options.computeOutputName();
    if (options.fOutputIntermediate) {
      lut->saveIntermediateFile(options.outputName.ptr);
    } else if (options.fOutputDump) {
      lut->translate();
      if (options.fOutputC) {
        lut->saveOutputFile(options.outputName.ptr);
      } else if (options.fOutputDump) {
        lut->saveOutputDumpFile(options.outputName.ptr);
      } else {
        TempDir tmpdir;
        alp::string fn_c=tmpdir.path()+"lut.c";
        int code;
        lut->saveOutputFile(fn_c.ptr);

        if ((code=system(alp::string::Format(
          "%s \"%s\" -o \"%s\"",
          options.cmdCompileTargetO.ptr,
          fn_c.ptr,options.outputName.ptr).ptr))!=0) 
          throw RuntimeError("unable to compile LUT to ELF");
      }
    }
  } catch(RuntimeError &e) {
    fprintf(
      stderr,"\x1b[31;1mError outputting LUT %s: %s\x1b[30;0m\n",
      options.fnInput.ptr,e.what());
    return 1;
  }
  return 0;
}

/** Main entry point.
  *
  * Performs command-line argument parsing and calls the appropriate 
  * toolflow (see above)
  */
int main(int argn, char **argv) {
  options_t options;

  try {
    int res;
    switch((res=options.parseCommandLine(argn-1,(const char**)(argv+1)))) {
      case 2: 
        exit(0);

      default:
        break;
    }
  } catch(CommandLineError &e) {
    options.print(stderr);
    fprintf(stderr,"\x1b[31;1mERROR: %s\x1b[30;0m\n",e.what());
    exit(1);
  }

  if (options.fInputWeights) {
    return run_weights_test(options);
  } else {
    return run_lut_compilation(options);
  }

  return 0;
}
