#include "ngraph.h"
#include <PCU.h>
#include <unordered_set>
#include <vector>
#include <engpar_support.h>
#include "agiMigrationTimers.h"


namespace agi {

  void Ngraph::migrate(WeightMigration* plan, MigrationTimers*) {
    //Get initial vertex weights
    wgt_t* v_weights = new wgt_t[numLocalVtxs()];
    GraphVertex* v;
    int i=0;
    VertexIterator* vitr = begin();
    while ((v = iterate(vitr))) {
      v_weights[i++] = weight(v);
    }

    //Send the weight in the plan
    PCU_Comm_Begin();
    WeightMigration::iterator plan_itr = plan->begin();
    for (;plan_itr!=plan->end();plan_itr++) {
      GraphVertex* mine = plan_itr.u();
      GraphVertex* peer = plan_itr.v();
      gid_t gid = globalID(peer);
      wgt_t w = plan_itr.weight();
      v_weights[localID(mine)]-=w;
      PCU_COMM_PACK(owner(peer),gid);
      PCU_COMM_PACK(owner(peer),w);
    }
    PCU_Comm_Send();
    //Receive the weight in the plan
    while (PCU_Comm_Receive()) {
      gid_t gid;
      wgt_t w;
      PCU_COMM_UNPACK(gid);
      PCU_COMM_UNPACK(w);
      agi::GraphVertex* mine = findGID(gid);
      v_weights[localID(mine)]+=w;
    }

    setWeights(v_weights);
    delete [] v_weights;
    delete plan;
  }

}
