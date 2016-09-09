#ifndef RISCV_LUT_COMPULER_QMC2_H
#define RISCV_LUT_COMPULER_QMC2_H
#include <alpha/alpha.h>

class QMC {
  public:
    struct implicant_t {
      uint64_t impl; // bitmask of output bits implied
      int ones;
      int color;
      char term[0];

      int numImplBits() {
        int r=0;
        for(int i=0;i<64;i++) if ((impl>>i)&1) r++;
        return r;
      }

      static int OnesCmpFunc(implicant_t *const&a, implicant_t *const&b) {
        return a->ones-b->ones;
      }
    };
  protected:
    size_t n_inputs;
    alp::array_t<implicant_t*> _minterms;
    alp::array_t<implicant_t*> _implicants;

    implicant_t *_dup(implicant_t *m) {
      implicant_t *r=(implicant_t*)malloc(sizeof(implicant_t)+n_inputs);
      memcpy(r,m,sizeof(implicant_t)+n_inputs);
      return r;
    }

    bool _combine(implicant_t *a, implicant_t *b, implicant_t *&res, uint64_t &tag) {
      if ((a->ones!=b->ones+1) && (a->ones+1!=b->ones)) return NULL;
      if ((a->impl&b->impl)==0) return false;
      ssize_t idiff=-1;
      for(size_t i=0;i<n_inputs;i++) {
        if (a->term[i]==b->term[i]) continue;
        if (a->term[i]==2) return false;
        if (b->term[i]==2) return false;
        if (idiff!=-1) return false;
        idiff=i;
      }

      if (idiff==-1) return false;

      res=(implicant_t*)malloc(sizeof(implicant_t)+n_inputs);
      memcpy(res,a,sizeof(implicant_t)+n_inputs);
      if (a->term[idiff]==1) res->ones--;
      res->impl&=b->impl;
      tag=res->impl;
      res->term[idiff]=2;
      res->color=0;

      for(size_t i=0;i<_implicants.len;i++) {
        if (
          ((_implicants[i]->impl&res->impl)==res->impl) && 
          (_covers(_implicants[i],res))) { 
          free((void*) res);
          res=NULL;
          return true;
        }

      }
      return true;
    }

    bool _covers(implicant_t *a, implicant_t *b) {
      for(size_t i=0;i<n_inputs;i++) {
        if ((a->term[i]!=b->term[i])&&(a->term[i]!=2)) return false;
      }
      return true;
    }

    void _del(size_t idx) {
      free((void*)_implicants[idx]);
      _implicants.remove(idx);
    }

  public:
    QMC(size_t n_inputs) : n_inputs(n_inputs) {
    
    }
    ~QMC() {
      for(size_t i=0;i<_minterms.len;i++) free((void*)_minterms[i]);
      for(size_t i=0;i<_implicants.len;i++) free((void*)_implicants[i]);
    }

    const alp::array_t<implicant_t*> &implicants() const { return _implicants; }

    void clearImplicants() {
      for(size_t i=0;i<_implicants.len;i++) free((void*)_implicants[i]);
    }

    void add_term(uint64_t term, uint64_t res) {
      if (res==0) return;
      implicant_t *m=(implicant_t*)malloc(sizeof(implicant_t)+n_inputs);
      m->impl=res;
      m->ones=0;
      m->color=0;

      for(size_t i=0;i<n_inputs;i++) {
        if ((m->term[i]=(term>>i)&1)==1)
          m->ones++;
      }
      _minterms.insert(m);
    }

    void minimize() {
      clearImplicants();
      for(size_t i=0;i<_minterms.len;i++) _implicants.insert(_dup(_minterms[i]));

      bool done=false;
      implicant_t *new_impl;
      uint64_t new_tag;

      int i_round=0;

      // find prime implicants
      while(!done) {
        i_round++;
        _implicants.sort(implicant_t::OnesCmpFunc);
        done=true;
        ssize_t i1=_implicants.len;
        for(ssize_t i=0;i<i1;i++) {
          for(
            ssize_t j=i+1;
            (
              (j<i1) &&
              (_implicants[j]->ones<=_implicants[i]->ones+1));
            j++) {
            if (_combine(_implicants[i],_implicants[j],new_impl,new_tag)) {
              if (new_impl!=NULL) { _implicants.insert(new_impl); done=false; }
              if (_implicants[j]->impl==new_tag) { _implicants[j]->color=1; done=false; }
              if (_implicants[i]->impl==new_tag) { _implicants[i]->color=1; done=false; }
            }
          }
        }
        for(ssize_t i=0;i<(ssize_t)_implicants.len;i++) {
          if (_implicants[i]->color==1) { _del(i--); i1--; }
        }
        
      }

      size_t n_terms=0;
      for(size_t i=0;i<_minterms.len;i++) {
        n_terms+=_minterms[i]->numImplBits(); 
      }

      
      char *tbl=(char*)malloc((n_terms+1)*(_implicants.len+1));
      memset(tbl,0,(n_terms+1)*(_implicants.len+1));
      char *impl_used=tbl+n_terms*_implicants.len;
      char *term_covered=impl_used+_implicants.len;
      size_t terms_covered=0;

      /*
          minterm1 minterm2 minterm3
          o1 o2    o1 o2 o3 o1 o4
       p1  
       p2
       p3

      */
      
      {
        char *p=tbl;
        for(size_t i=0;i<_implicants.len;i++) {
          for(size_t j=0;j<_minterms.len;j++) {
            char covered=_covers(_implicants[i],_minterms[j])?1:0;
            for(size_t k=0;k<64;k++) {
              if (((_minterms[j]->impl>>k)&1)==0) continue;
              *p++=covered&(_implicants[i]->impl>>k);
            }
          }
        }
      }

      // check all necessary prime implicants
      for(size_t i=0;i<n_terms;i++) {
        ssize_t idx=-1;
        for(size_t j=0;j<_implicants.len;j++) {
          if (tbl[j*n_terms+i]==0) continue;
          if (idx!=-1) { 
            idx=-1;
            break;
          }
          idx=j;
        }
        if (idx!=-1) {
          impl_used[idx]=1;
          for(size_t j=0;j<n_terms;j++) 
            if (term_covered[j]==0) {
              term_covered[j]=1;
              terms_covered++;
            }
        }
      }

      // greedily choose implicants covering the most terms
      while(terms_covered<_minterms.len) {
        size_t best_pick=0;
        size_t best_quality=0;
        for(size_t i=0;i<_implicants.len;i++) {
          if (impl_used[i]!=0) continue;
          size_t quality=0;
          for(size_t j=0;j<n_terms;j++) {
            if ((tbl[i*n_terms+j]==1)&&(term_covered[j]==0)) quality++;
          }
          if (quality>best_quality) {
            best_quality=quality;
            best_pick=i;
          }
        }
        for(size_t j=0;j<n_terms;j++) {
          if ((tbl[best_pick*n_terms+j]==1)&&(term_covered[j]==0)) {
            term_covered[j]=1;
            terms_covered++;
          }
        }
        impl_used[best_pick]=1;
      }

      for(ssize_t j=(ssize_t)_implicants.len-1;j>-1;j--)
        if (impl_used[j]!=1) _del(j);
      
      free((void*)tbl);
      

    }

    void print() {
      for(size_t i=0;i<_implicants.len;i++) {
        for(size_t j=0;j<n_inputs;j++) switch(_implicants[i]->term[j]) {
          case 0: printf("0"); break;
          case 1: printf("1"); break;
          case 2: printf("-"); break;
          default: printf("?"); break;
        }
        printf(" (%i, %i) ",_implicants[i]->ones,_implicants[i]->color);
        for(size_t j=0;j<12;j++)
          if (_implicants[i]->impl&(1<<j)) printf("1"); else printf("0");
        printf("\n");
      }
    }


};

#endif
