//This file is for type definitions used throughout the graph interface

#ifndef _AGI_H__
#define _AGI_H__

#include <stdint.h>
#include <apfVector.h>
#include <set>
//ID types
//TODO: discuss what these should be?
namespace agi {
  
  typedef uint64_t gid_t;
  typedef uint64_t lid_t;
  typedef double wgt_t;
  typedef int32_t part_t;
  typedef double coord_t[3];

  typedef std::set<part_t> Peers;
  
  class GraphVertex;
  typedef std::unordered_map<gid_t,part_t> PartitionMap;
  typedef std::unordered_map<GraphVertex*,part_t> Migration;
  typedef std::unordered_map<lid_t,part_t> VertexPartitionMap;
  typedef std::unordered_map<lid_t,part_t> EdgePartitionMap;

//Definitions for edge types
//static size of edge types
#define MAX_TYPES 7
typedef int etype;
#define NO_TYPE -2
#define VTX_TYPE -1
//predefined edge type for split vtx
#define SPLIT_TYPE MAX_TYPES-1 
}
#endif
