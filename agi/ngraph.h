#ifndef NGRAPH_H__
#define NGRAPH_H__
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>
#include "Edge.h"
#include "agi.h"
#include "EdgeIterator.h"

namespace agi {

class GraphVertex;
class GraphEdge;
class VertexIterator;
class PinIterator;
class Ngraph {
  //TODO: change to hyperedge approach
  //TODO: Make distributed version
  //TODO: Make ghost layer?

protected:
  
  //Global number of vertices and edges
  gid_t num_global_verts;
  gid_t num_global_edges[MAX_TYPES];
  gid_t num_global_pins[MAX_TYPES];
  
  // number of vertices and edges
  lid_t num_local_verts;
  lid_t num_ghost_verts;
  int num_types;
  lid_t num_local_edges[MAX_TYPES];
  lid_t num_local_pins[MAX_TYPES];
  
  // vertex weights
  // size = num_local_verts
  wgt_t* local_weights;

  //edge weights
  // size=num_edges
  Edge* es[MAX_TYPES];
  
  //TODO: do we want ghost weights?
  //wgt_t* ghost_weights

  //TODO: add coordinate container
  //coord_t* local_coords;
  
  lid_t* degree_list[MAX_TYPES];
  lid_t* edge_list[MAX_TYPES];
  lid_t* pin_degree_list[MAX_TYPES];
  lid_t* pin_list[MAX_TYPES];

  //TODO: discuss using C++11 to get unordered map
  typedef std::map<gid_t,lid_t> map_t;
  map_t vtx_mapping;
  map_t edge_mapping[MAX_TYPES];
  gid_t* local_unmap;
  gid_t* ghost_unmap;
  part_t* owners;
  
public:
  Ngraph();
  virtual ~Ngraph();
  
  //Part Information
  lid_t numVtxs() const {return num_local_verts;}
  int numEdgeTypes() const {return num_types;}
  lid_t numEdges(etype i=0) const {return num_local_edges[i];}
  lid_t numPins(etype i=0) const {return num_local_pins[i];}
  
  //Vertex Operations
  double weight(GraphVertex*) const;
  int owner(GraphVertex*) const {return 0;}
  const std::vector<double>& coordinates(GraphVertex*) const {};

  //Edge Operations
  double weight(GraphEdge*) const;
  
  //Adjacency Operations
  lid_t degree(GraphVertex*,etype) const;
  EdgeIterator* edges(GraphVertex*,etype) const;
  void destroy(EdgeIterator*) const;
  lid_t degree(GraphEdge*) const;
  PinIterator* pins(GraphEdge*) const;
  
  //Iterator Traversal
  VertexIterator* begin() const;
  //Iterate through vertices
  GraphVertex* iterate(VertexIterator*&) const;
  //Iterate through Edges
  GraphEdge* iterate(EdgeIterator*) const;
  //Iterate through Pins
  GraphVertex* iterate(PinIterator*&) const;

  //Utility
  bool isEqual(GraphVertex*,GraphVertex*) const;
  virtual void migrate(std::map<GraphVertex*,int>&) = 0;
  
 protected:
  etype addEdgeType() {assert(num_types!=MAX_TYPES);return num_types++;}

  void makeEdgeArray(etype t,int count);
  void setEdge(lid_t,gid_t,wgt_t,etype);
  
  /*
  void create_csr(int num_verts, int num_edges, int* srcs,
                  int* dsts, int* wgts);
  */
};

} //namespace

#endif
