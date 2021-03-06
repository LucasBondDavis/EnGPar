#ifndef ENGPAR_SELECTOR_H
#define ENGPAR_SELECTOR_H

#include "../engpar.h"
#include "engpar_sides.h"
#include "engpar_targets.h"
#include "engpar_queue.h"
#include <unordered_set>
namespace engpar {

  typedef std::vector<agi::GraphVertex*> Cavity;
  typedef std::unordered_set<part_t> Peers;
  typedef std::unordered_map<part_t,wgt_t> Sending;
  typedef double* Ws;
  typedef std::map<int, Ws> Midd;
  struct Migr;
  struct CompareMigr;
  typedef std::set<Migr,CompareMigr> MigrComm;

  
  class Selector {
  public:
    Selector(DiffusiveInput* in_, Queue* queue,
             std::vector<int>* cd, std::vector<double>* cw) :
      in(in_), g(in_->g),
      q(queue),
      completed_dimensions(cd), completed_weights(cw) {}

    wgt_t select(Targets* targets,agi::Migration* plan,
                         wgt_t planW, unsigned int cavSize,int);
    void selectDisconnected(agi::Migration* plan, int target_dimension);
    Midd* trim(Targets* targets, agi::Migration* plan);
    void cancel(agi::Migration*& plan,Midd* capacity);

  protected:
      
    typedef std::unordered_set<agi::GraphEdge*> EdgeSet;
    typedef std::map<int,EdgeSet> PeerEdgeSet;
    //Trim functions
    void insertInteriorEdges(agi::GraphVertex*,agi::part_t, EdgeSet&,int);
    void calculatePlanWeights(agi::Migration* plan, std::unordered_map<int,double>& vtx_weight,
                              PeerEdgeSet* peerEdges, std::unordered_set<part_t>& neighbors);
    void sendPlanWeight(std::unordered_map<int,double>& vtx_weight, PeerEdgeSet* peerEdges,
                        std::unordered_set<part_t>& neighbors);
    void receiveIncomingWeight(MigrComm& incoming);
    bool determineAvailability(Ws& avail);
    void acceptWeight(MigrComm& incoming, bool& isAvail, Ws& avail, Midd& accept);
    void sendAcceptedWeights(Midd& accept);
    void gatherCapacities(Midd* capacity);



    void tempInsertInteriorEdges(agi::GraphVertex*,agi::part_t,
                                           EdgeSet&,int,const EdgeSet&);
    double weight(const EdgeSet&);
    void combineSets(EdgeSet&,const EdgeSet&);

    DiffusiveInput* in;
    agi::Ngraph* g;
    Sending sending;
    Queue* q;
    std::vector<int>* completed_dimensions;
    std::vector<double>* completed_weights;
  };

  Selector* makeSelector(DiffusiveInput* in,Queue* q,
                         std::vector<int>* cd,
                         std::vector<double>* cw );
}

#endif
