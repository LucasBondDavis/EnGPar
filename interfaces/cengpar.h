#ifndef CENGPAR
#define CENGPAR

#include <mpi.h>
#include "agi.h"

#ifdef __cplusplus
extern "C" {
#endif

void cengpar_initialize();
void cengpar_finalize();
void cengpar_setftncommunicator(MPI_Fint fcomm);

typedef void* ngraph;
ngraph cengpar_createEmptyGraph();
void cengpar_constructVerts(ngraph g, bool isHg,
    agi::gid_t* verts, agi::wgt_t* weights, int nverts);
#ifdef __cplusplus
}
#endif

#endif
