program main
  use engpar
  use iso_c_binding
  implicit none
  include 'mpif.h'
#include "../agi/agi_types.h"

  integer, parameter :: nverts = 2, nedges = 1, npins = 2, nghosts = 0
  integer :: ierr, self
  integer(AGI_GID_FT) :: verts(nverts), edges(nedges), pins(npins)
  integer(AGI_LID_FT) :: degs(nedges)
  real(AGI_WGT_FT) :: weights(nverts)
  integer(AGI_GID_FT) :: ghostverts(nghosts)
  integer(AGI_PART_FT) :: ghostowners(nghosts), parts(nverts)
  type(c_ptr) :: graph
  logical(C_BOOL) :: isHg = .false.
  real(C_DOUBLE) :: tol, stepfactor
  integer :: verbosity
  call mpi_init(ierr)
  call mpi_comm_rank(MPI_COMM_WORLD, self, ierr)
  call cengpar_initialize()
  call cengpar_setftncommunicator(MPI_COMM_WORLD)
  graph = cengpar_createEmptyGraph()
  verts = (/ 0, 1 /)
  weights = (/ 1.0, 1.0 /)
  edges = (/ 0 /)
  degs = (/ 2 /)
  pins = (/ 0, 1 /)
  call cengpar_constructVerts(graph, isHg, verts, weights, nverts)
  call cengpar_constructEdges(graph, edges, degs, pins, nedges, npins)
  call cengpar_constructGhosts(graph, ghostverts, ghostowners, nghosts)
  call cengpar_checkValidity(graph);
  tol = 1.05
  stepfactor = 0.1
  verbosity = 1
  call cengpar_balanceVertices(graph, tol, stepfactor, verbosity);
  call cengpar_getPartition(graph, verts, parts, nverts)
  call cengpar_destroyGraph(graph);
  call cengpar_finalize()
  call mpi_finalize(ierr)
  stop
end
