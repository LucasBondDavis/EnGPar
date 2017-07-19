module engpar
  use :: iso_c_binding
#include "agi_types.h"
  public
  interface
  subroutine cengpar_initialize() bind(C, NAME='cengpar_initialize')
    use :: iso_c_binding
  end subroutine
  subroutine cengpar_finalize() bind(C, NAME='cengpar_finalize')
    use :: iso_c_binding
  end subroutine
  subroutine cengpar_setftncommunicator(comm) &
             bind(C, NAME='cengpar_setftncommunicator')
    use :: iso_c_binding
    integer(c_int), value :: comm
  end subroutine
  function cengpar_createEmptyGraph() &
             bind(C, NAME='cengpar_createEmptyGraph')
    use :: iso_c_binding
    type(c_ptr) cengpar_createEmptyGraph
  end function
  subroutine cengpar_constructVerts(graph,isHg,verts,weights,nverts) &
             bind(C, NAME='cengpar_constructVerts')
    use :: iso_c_binding
    type(c_ptr), value :: graph
    logical(c_bool), intent(in), value :: isHg
    integer(AGI_GID_FT), intent(in), dimension(nverts) :: verts
    real(AGI_WGT_FT), intent(in), dimension(nverts) :: weights
    integer(C_INT), intent(in), value :: nverts
  end subroutine
  end interface
end module

