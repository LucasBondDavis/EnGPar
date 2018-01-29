subroutine switchToOriginals(splitFactor, inSmall, newComm)
  use engpar
  use iso_c_binding
  implicit none
  include 'mpif.h'

  integer :: splitFactor, verbosity
  logical(C_BOOL) :: inSmall
  integer(kind=4) :: newComm
  integer :: ierr, self, group, groupRank
  integer :: mpiErr, strLen
  character(len=1024) :: errStr
  
  call mpi_comm_rank(MPI_COMM_WORLD, self, ierr)
  
  inSmall = (modulo(self,splitFactor) == 0)

  if (inSmall) then
    group=0
    groupRank=self/splitFactor
  else
    group = 1
    groupRank = 0
  end if
  call mpi_comm_split(MPI_COMM_WORLD, group, groupRank, newComm, ierr)
  if (ierr .ne. MPI_SUCCESS) then
    call mpi_error_string(ierr, errStr, strLen, mpiErr)
    write (*,*) "ERROR mpi comm split failed with error ierr", errStr, "!... exiting"
    stop
  end if
end subroutine

module helperFns
  contains
  function cleanString(str)
    use iso_c_binding
    implicit none
    character(len=256) :: cleanString
    character(len=256) :: str
    cleanString = trim(str)//c_null_char
  end function
  subroutine getArgs(self, inGraph, outGraph)
    implicit none
    integer :: self, numArgs, strlen
    character(len=256) :: inGraph, outGraph
    numArgs = command_argument_count()
    if ( numArgs .ne. 2 ) then
      if (self==0) write(*,*) "Usage: splitAndBalanceFtn </path/to/input/graph> </path/to/output/graph>"
      stop
    end if

    call get_command_argument(1,inGraph)
    call get_command_argument(2,outGraph)

    if (self==0) then
      write(*,*) "input graph: ", trim(inGraph)
      write(*,*) "output graph: ", trim(outGraph)
    end if
  end subroutine
end module

program main
  use engpar
  use helperFns
  use iso_c_binding
  implicit none
  include 'mpif.h'

  integer :: ierr, self
  integer(ENGPAR_EDGE_T) :: edgeType
  type(c_ptr) :: graph, splitInput
  logical(C_BOOL) :: inSmall
  real(C_DOUBLE) :: tol, stepfactor
  integer :: splitFactor, verbosity, newComm
  character(len=256) :: inGraphFileName, outGraphFileName, splitMethod

  call mpi_init(ierr)
  call mpi_comm_rank(MPI_COMM_WORLD, self, ierr)
  call cengpar_initialize()

  call getArgs(self, inGraphFileName, outGraphFileName)

  splitFactor = 4
  inSmall = .false.
  call switchToOriginals(splitFactor,inSmall,newComm)

  ! Switch the internal communicator (this changes PCU so use PCU_Comm_... with caution)
  call cengpar_setftncommunicator(newComm)

  graph = cengpar_createEmptyGraph()
 
  if (inSmall) then
    ! Only the original parts will construct the graph
    call cengpar_loadFromFile(graph,cleanString(inGraphFileName))
    call cengpar_evaluatePartition(graph);
  end if

  ! Create the split input
  edgeType = 0
  tol = 1.05
  splitInput = cengpar_createSplitInput(graph,newComm,MPI_COMM_WORLD,inSmall,splitFactor,tol,edgeType)

  ! Perform split 
  splitMethod = c_char_"GLOBAL_PARMETIS"//c_null_char
  call cengpar_split(splitInput,splitMethod);
  call cengpar_checkValidity(graph);

  if (self==0) write(*,*) 'After Split'
  call cengpar_evaluatePartition(graph)

  ! Create the input for diffusive load balancing (vtx>element)
  tol = 1.05
  stepFactor = 0.1
  verbosity = 1
  call cengpar_balanceVertices(graph, tol, stepFactor, verbosity);
  call cengpar_checkValidity(graph);

  if (self==0) write(*,*) 'After Balancing'
  call cengpar_evaluatePartition(graph)
  call cengpar_saveToFile(graph,cleanString(outGraphFileName))
  call MPI_Comm_free(newComm, ierr)

  call cengpar_destroyGraph(graph);
  call cengpar_finalize()
  call mpi_finalize(ierr)
  stop
end

