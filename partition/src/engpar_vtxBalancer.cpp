#include "../engpar.h"
#include "engpar_bounds.h"
#include "engpar_vtxWeights.h"
#include "engpar_balancer.h"
#include "engpar_targets.h"
#include <PCU.h>
namespace {
  class VtxBalancer : public engpar::Balancer {
  public:
    VtxBalancer(agi::Ngraph* g, double f, int v)
      : Balancer(g,f,v,"Vtx") {
      
    }
    ~VtxBalancer() {}
    bool runStep(double tol) {
      engpar::Bounds* b = engpar::makeEdgeBounds(graph);
      printf("%d %s",PCU_Comm_Self(),b->print("Boundaries").c_str());
      engpar::VtxWeights* w = engpar::makeVtxWeights(graph,b);
      printf("%d %s",PCU_Comm_Self(),w->print("Weights").c_str());
      engpar::Targets* t = engpar::makeTargets(b,w,factor);
      printf("%d %s",PCU_Comm_Self(),t->print("Targets").c_str());
      return true;
    }
    
  };
}
namespace engpar {
  agi::Balancer* makeVtxBalancer(agi::Ngraph* g, double f, int v) {
    return new VtxBalancer(g,f,v);
  }
}
