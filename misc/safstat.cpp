#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <htslib/bgzf.h>

#include <cstring>
#include "safstat.h"
#include "realSFS_args.h"
//#include "realSFS.h"
//lazy binomial
int choose(int n,int m){
  if(n==2&&m==2)
    return 1;
  else if(n==3&&m==2)
    return 3;
  else{
    fprintf(stderr,"\t-> Never here\n");
    exit(0);
  }
  return -1;
}


void calcCoef(int sfs1,int sfs2,double **aMat,double **bMat){
  fprintf(stderr,"\t-> [%s] sfs1:%d sfs2:%d dimspace:%d \n",__FUNCTION__,sfs1,sfs2,(sfs1+1)*(sfs2+1));
  *aMat = new double[(sfs1+1)*(sfs2+1)];
  *bMat = new double[(sfs1+1)*(sfs2+1)];
  int at=0;
  for(int a1=0;a1<=sfs1;a1++)
    for(int a2=0;a2<=sfs2;a2++){
      double p1 = 1.0 * a1/(1.0*sfs1);
      double p2 = 1.0 * a2/(1.0*sfs2);
      double q1 = 1 - p1;
      double q2 = 1 - p2;
      double alpha1 = 1 - (p1*p1 + q1*q1);
      double alpha2 = 1 - (p2*p2 + q2*q2);
      
      double al =  0.5 * ( pow(p1-p2,2.0) + pow(q1-q2,2)) - (sfs1+sfs2) *  (sfs1*alpha1 + sfs2*alpha2) / (4*sfs1*sfs2*(sfs1+sfs2-1));
      double bal = 0.5 * ( pow(p1-p2,2) + pow(q1-q2,2)) + (4*sfs1*sfs2-sfs1-sfs2)*(sfs1*alpha1 + sfs2*alpha2) / (4*sfs1*sfs2*(sfs1+sfs2-1));
      (*aMat)[at] = al;
      (*bMat)[at] = bal;
      //      fprintf(stderr,"p1:%f p2:%f q1:%f q2:%f alhpa1:%f alpha:%f al:%f bal:%f\n",p1,p2,q1,q2,alpha1,alpha2,al,bal);
      at++;
    }
}

void block_coef(Matrix<float > *gl1,Matrix<float> *gl2,double *prior,double *a1,double *b1,std::vector<double> &ares,std::vector<double> &bres){
  assert(prior!=NULL);
  double tre[3]={0,0,0};//a/b,sum(a),sum(0)
  for(int s=0;s<gl1->x;s++){
    int inc =0 ;
    double tmp[(gl1->y+1)*(gl2->y+1)];
    
    for(int i=0;i<gl1->y;i++)
      for(int j=0;j<gl2->y;j++){
	tmp[inc] = prior[inc]* gl1->mat[s][i] *gl2->mat[s][j];
	inc++;
      }
    //    exit(0);
    double as=0;
    double bs=0;
    void normalize(double *,int);
    normalize(tmp,inc);
    for(int i=0;i<inc;i++){
      as += a1[i]*tmp[i];
      bs += b1[i]*tmp[i];
    }
    tre[0] += as/bs;
    tre[1] += as;
    tre[2] += bs;
    ares.push_back(as);
    bres.push_back(bs);
      
    //    fprintf(stdout,"%f %f\n",as,bs);
  }
  //  fprintf(stderr,"bres.size:%lu\n",bres.size());
  // fprintf(stderr,"u:%f w:%f\n",tre[0]/(1.0*gl1->x),tre[1]/tre[2]);
}

#include "fstreader.h"
int fst_print(int argc,char **argv){

  char *bname = *argv;
  fprintf(stderr,"\t-> Assuming idxname:%s\n",bname);
  perfst *pf = perfst_init(bname);
  writefst_header(stderr,pf);  
  args *pars = getArgs(--argc,++argv);  
  int *ppos = NULL;
  fprintf(stderr,"choose:%d \n",choose(pf->names.size(),2));
  double **ares = new double*[choose(pf->names.size(),2)];
  double **bres = new double*[choose(pf->names.size(),2)];
  for(myFstMap::iterator it=pf->mm.begin();it!=pf->mm.end();++it){
    if(pars->chooseChr!=NULL){
      it = pf->mm.find(pars->chooseChr);
      if(it==pf->mm.end()){
	fprintf(stderr,"Problem finding chr: %s\n",pars->chooseChr);
	break;
      }
    }
    if(it->second.nSites==0)
      continue;
    bgzf_seek(pf->fp,it->second.off,SEEK_SET);
    ppos = new int[it->second.nSites];
    
    bgzf_read(pf->fp,ppos,sizeof(int)*it->second.nSites);
    for(int i=0;i<choose(pf->names.size(),2);i++){
      ares[i] = new double[it->second.nSites];
      bres[i] = new double[it->second.nSites];
      bgzf_read(pf->fp,ares[i],sizeof(double)*it->second.nSites);
      bgzf_read(pf->fp,bres[i],sizeof(double)*it->second.nSites);
    }
    


    int first=0;
    if(pars->start!=-1)
      while(ppos[first]<pars->start) 
	first++;
    
    int last=it->second.nSites;

    if(pars->stop!=-1&&pars->stop<=ppos[last-1]){
      last=first;
      while(ppos[last]<pars->stop) 
	last++;
    }

    fprintf(stderr,"pars->stop:%d ppos:%d first:%d last:%d\n",pars->stop,ppos[last-1],first,last);

    for(int s=first;s<last;s++){
      fprintf(stdout,"%s\t%d",it->first,ppos[s]+1);
      for(int i=0;i<choose(pf->names.size(),2);i++)
	fprintf(stdout,"\t%f\t%f",ares[i][s],bres[i][s]);
      fprintf(stdout,"\n");
    }
    for(int i=0;i<choose(pf->names.size(),2);i++){
      delete [] ares[i];
      delete [] bres[i];
    }
    
    delete [] ppos;
    
    if(pars->chooseChr!=NULL)
      break;
  }
  delete [] ares;
  delete [] bres;
  destroy_args(pars);
  perfst_destroy(pf);
  return 0;
}

int fst_stat(int argc,char **argv){
  
  char *bname = *argv;
  fprintf(stderr,"\t-> Assuming idxname:%s\n",bname);
  perfst *pf = perfst_init(bname);
  args *pars = getArgs(--argc,++argv);  
  int *ppos = NULL;
  int chs = choose(pf->names.size(),2);
  // fprintf(stderr,"choose:%d \n",chs);
  double **ares = new double*[chs];
  double **bres = new double*[chs];
  double unweight[chs];
  double wa[chs];
  double wb[chs];
  size_t nObs =0;
  for(int i=0;i<chs;i++)
    unweight[i] = wa[i] = wb[i] =0.0;
  for(myFstMap::iterator it=pf->mm.begin();it!=pf->mm.end();++it){
    if(pars->chooseChr!=NULL){
      it = pf->mm.find(pars->chooseChr);
      if(it==pf->mm.end()){
	fprintf(stderr,"Problem finding chr: %s\n",pars->chooseChr);
	break;
      }
    }
    if(it->second.nSites==0)
      continue;
    bgzf_seek(pf->fp,it->second.off,SEEK_SET);
    ppos = new int[it->second.nSites];
    
    bgzf_read(pf->fp,ppos,sizeof(int)*it->second.nSites);
    for(int i=0;i<choose(pf->names.size(),2);i++){
      ares[i] = new double[it->second.nSites];
      bres[i] = new double[it->second.nSites];
      bgzf_read(pf->fp,ares[i],sizeof(double)*it->second.nSites);
      bgzf_read(pf->fp,bres[i],sizeof(double)*it->second.nSites);
    }
    


    int first=0;
    if(pars->start!=-1)
      while(ppos[first]<pars->start) 
	first++;
    
    int last=it->second.nSites;

    if(pars->stop!=-1&&pars->stop<=ppos[last-1]){
      last=first;
      while(ppos[last]<pars->stop) 
	last++;
    }

    //  fprintf(stderr,"pars->stop:%d ppos:%d first:%d last:%d\n",pars->stop,ppos[last-1],first,last);

    for(int s=first;s<last;s++){
#if 0
      fprintf(stdout,"%s\t%d",it->first,ppos[s]+1);
      for(int i=0;i<choose(pf->names.size(),2);i++)
	fprintf(stdout,"\t%f\t%f",ares[i][s],bres[i][s]);
      fprintf(stdout,"\n");
#endif
      for(int i=0;i<choose(pf->names.size(),2);i++){
	unweight[i] += ares[i][s]/bres[i][s];
	wa[i] += ares[i][s];
	wb[i] += bres[i][s];
      }
      nObs++;
    }
    for(int i=0;i<choose(pf->names.size(),2);i++){
      delete [] ares[i];
      delete [] bres[i];
    }
    
    delete [] ppos;
    
    if(pars->chooseChr!=NULL)
      break;
  }
  for(int i=0;i<chs;i++){
    fprintf(stdout,"\t-> FST.Unweight:%f Fst.Weight:%f\n",unweight[i]/(1.0*nObs),wa[i]/wb[i]);
  }
  delete [] ares;
  delete [] bres;
  destroy_args(pars);
  perfst_destroy(pf);
  return 0;
}
