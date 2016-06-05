#include "weights.h"
#include "lut.h"
#include "segment.h"
#include "error.h"
#include "keyvalue.h"
#include "options.h"
#include "strategies.h"

static int run_weights_test(options_t &options);
int main(int argn, char **argv);


static int run_weights_test(options_t &options) {
  WeightsTable *tbl=new WeightsTable;
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
  delete tbl;

  return 0;

}


static int run_lut_compilation(options_t &options) {
  LookupTable *lut=new LookupTable(options.arch);
  WeightsTable *weights=NULL;
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

  if (lut->fn_weights().len>0) {
    try {
      alp::string fn=options.vfsWeights.locate(lut->fn_weights());
      if (fn.len<1) {
        throw RuntimeError("Unable to locate weights file");
      }
      weights=new WeightsTable();
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

  if (!options.fInputIntermediate) {
    try {
      if (lut->strategy1()!=segment_strategy::INVALID) {
        segment_strategy::get(lut->strategy1())->execute(lut,weights,options);
      } else if (lut->explicit_segments().data().len>0) {
        const alp::array_t<Bounds::interval_t> &intervals=
          lut->explicit_segments().data();
        for(size_t i=0;i<intervals.len;i++)
          lut->addSegment(segment_t(intervals[i].start,intervals[i].end),true);
        // fixme: think about whether specifying 'true' here makes sense.
      } else {
        throw RuntimeError(
          "No segmentation method (strategy / explicit segments) specified");
      }

      if (lut->strategy2()!=segment_strategy::INVALID) {
        segment_strategy::get(lut->strategy2())->execute(lut,weights,options);
      }

      if (lut->approximation_strategy()!=approx_strategy::INVALID) {
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
  }
  try {
    options.computeOutputName();
    if (options.fOutputIntermediate) {
      lut->saveIntermediateFile(options.outputName.ptr);
    } else {
      lut->translate();
      if (options.fOutputC) {
        lut->saveOutputFile(options.outputName.ptr);
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
