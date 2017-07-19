#include "cengpar.h"
#include "engpar_support.h"
#include "ngraph.h"

void cengpar_initialize() {
  EnGPar_Initialize();
}

void cengpar_finalize() {
  EnGPar_Finalize();
}

void cengpar_setftncommunicator(MPI_Fint fcomm) {
  MPI_Comm comm = MPI_Comm_f2c(fcomm);
  PCU_Switch_Comm(comm);
}

ngraph cengpar_createEmptyGraph() {
  agi::Ngraph* ng = agi::createEmptyGraph();
  fprintf(stderr, "ng %p\n", ng);
  return (ngraph)ng;
}

void cengpar_constructVerts(ngraph g, bool isHg,
    agi::gid_t* verts, agi::wgt_t* weights, int nverts) {
  agi::Ngraph* ng = (agi::Ngraph*)g;
  fprintf(stderr, "ng %p\n", ng);
  fprintf(stderr, "isHg %d\n", isHg);
  fprintf(stderr, "nverts %d\n", nverts);
  std::vector<agi::gid_t> v(verts, verts + nverts);
  std::vector<agi::wgt_t> w(weights, weights + nverts);
  ng->constructVerts(isHg,v,w);
}
