#include "ngraph.h"
#include <PCU.h>
#include <unordered_set>
#include <vector>
namespace agi {
  void phi() {
    if (!PCU_Comm_Self())
      printf("hi!\n");
  }
  typedef std::unordered_set<GraphVertex*> VertexVector;
  //TODO: Make a vector by using a "tag" on the edges to detect added or not
  typedef std::unordered_set<GraphEdge*> EdgeVector;
  void Ngraph::updateGhostOwners(Migration* plan) {
    PCU_Comm_Begin();
    Migration::iterator itr;
    for (itr = plan->begin();itr!=plan->end();itr++) {
      GraphVertex* v = itr->first;
      part_t toSend = itr->second;
      GraphIterator* gitr = adjacent(v);
      GraphVertex* other;
      while ((other=iterate(gitr))) {
	part_t own = owner(other);
	if (own!=PCU_Comm_Self()) {
	  gid_t gid = globalID(v);
	  PCU_COMM_PACK(own,gid);
	  PCU_COMM_PACK(own,toSend);
	}
      }
    }
    PCU_Comm_Send();
    while (PCU_Comm_Receive()) {
      gid_t gid;
      part_t own;
      PCU_COMM_UNPACK(gid);
      PCU_COMM_UNPACK(own);
      lid_t lid = vtx_mapping[gid];
      assert(lid>=num_local_verts);
      owners[lid-num_local_verts]=own;
    }
  }
  /*
    Finds the vertices and edges that need to be communicated
    Also stores the local vertices and edges that will be owned
      on this part after migration
  */
  void getAffected(Ngraph* g, Migration* plan, VertexVector& verts,
		   EdgeVector* edges) {
    verts.reserve(plan->size());
    Migration::iterator itr;
    for (itr = plan->begin();itr!=plan->end();itr++) {
      GraphVertex* v = itr->first;
      part_t toSend = itr->second;
      if (toSend!=PCU_Comm_Self()) {
	verts.insert(v);
      }
    }

    //For each edge type
    for (etype t = 0;t<g->numEdgeTypes();t++) {
      //edges[t].reserve(verts.size());
      VertexVector::iterator itr;
      //loop through the vertices being sent
      for (itr = verts.begin();itr!=verts.end();itr++) {
	GraphIterator* gitr =g->adjacent(*itr);
	GraphEdge* e;
	GraphEdge* old=NULL;
	GraphVertex* v;
	while ((v = g->iterate(gitr))) {
	  e=g->edge(gitr);
	  if (old==NULL||e!=old) 
	    edges[t].insert(e);
	}
      }      
    } 
  }
  
  void addVertices(Ngraph* g, std::vector<gid_t>& ownedVerts,
		   VertexVector& verts) {
    VertexIterator* vitr = g->begin();
    GraphVertex* v;
    while ((v = g->iterate(vitr))) {
      if (verts.find(v)==verts.end()) {
	ownedVerts.push_back(g->globalID(v));
      }
    }
  }
  void addEdges(Ngraph* g, Migration* plan, std::vector<gid_t>& ownedEdges,
		std::vector<lid_t>& degrees,std::vector<gid_t>& pins,
		std::unordered_map<gid_t,part_t>& ghost_owners) {
    VertexIterator* itr = g->begin();
    GraphVertex* v;
    while ((v = g->iterate(itr))) {
      if (plan->find(v)!=plan->end())
	continue;
      GraphVertex* other;
      GraphIterator* gitr = g->adjacent(v);
      GraphEdge* e,*old = NULL;
      while ((other = g->iterate(gitr))) {
	e=g->edge(gitr);
	if (old==NULL||e!=old) {
	  if (!PCU_Comm_Self()) {
	    printf("Kept Edge: %lu %lu\n",g->localID(e),g->globalID(e));
	    printf("Edge is %lu %lu\n",g->globalID(g->u(e)),
		   g->globalID(g->v(e)));
	  }
	  if (g->isHyper()) {
	    ownedEdges.push_back(g->globalID(e));
	    degrees.push_back(g->degree(e));
	  }
	  else {
	    ownedEdges.push_back(g->localID(e));
	    pins.push_back(g->globalID(v));
	    degrees.push_back(2);
	  }
	}
	gid_t other_gid = g->globalID(other);
	pins.push_back(other_gid);
	part_t owner = g->owner(other);
	if (plan->find(other)!=plan->end())
	  owner = plan->find(other)->second;
	if (owner!=PCU_Comm_Self()) 
	  ghost_owners[other_gid] = owner;
      }
      old=e;
    }
  }
  
  void getSenders(Ngraph* g, VertexVector& verts, EdgeVector* edges,
		  VertexVector& vSenders,EdgeVector* eSenders) {
    /*    vSenders.reserve(verts.size());
    for (int i=0;i<verts.size();i++) {
      if (
    }
    */
  }
  void Ngraph::sendVertex(GraphVertex* vtx, part_t toSend) {
    gid_t gid = globalID(vtx);
    PCU_COMM_PACK(toSend,gid);
  }

  void Ngraph::recvVertex(std::vector<gid_t>& recv) {
    gid_t gid;
    PCU_COMM_UNPACK(gid);
    recv.push_back(gid);
  }
  
  void Ngraph::migrate(Migration* plan) {
    updateGhostOwners(plan);
    VertexVector affectedVerts;
    EdgeVector* affectedEdges = new EdgeVector[num_types];
    std::vector<gid_t> ownedVerts;
    ownedVerts.reserve(num_local_verts);
    std::vector<gid_t> ownedEdges;
    ownedEdges.reserve(num_local_edges[0]);
    std::vector<lid_t> degrees;
    std::vector<gid_t> pins;
    std::unordered_map<gid_t,part_t> ghost_owners;
    
    getAffected(this,plan,affectedVerts,affectedEdges);
    addVertices(this,ownedVerts,affectedVerts);
    addEdges(this,plan,ownedEdges,degrees,pins,ghost_owners);

    Migration::iterator itr;
    PCU_Comm_Begin();
    //Send vertices
    for (itr = plan->begin();itr!=plan->end();itr++) {
      sendVertex(itr->first,itr->second);
    }
    PCU_Comm_Send();
    //Recieve vertices
    while (PCU_Comm_Receive()) {
      recvVertex(ownedVerts);
    }
    
    for (etype t = 0;t < num_types;t++) {
      PCU_Comm_Begin();
      EdgeVector::iterator eitr;

      for (eitr = affectedEdges[t].begin();eitr!=affectedEdges[t].end();
	   eitr++) {
	GraphEdge* e = *eitr;
	gid_t* pin;
	part_t* pin_owners;
	lid_t deg=0;
	gid_t id;
	std::unordered_set<part_t> residence;
	if (isHyperGraph) {
	  id = globalID(e);
	  pin = new gid_t[degree(e)];
	  pin_owners = new part_t[degree(e)];
	  
	  agi::PinIterator* pitr = this->pins(e);
	  agi::GraphVertex* vtx;
	  while ((vtx = iterate(pitr))) {
	    part_t o = owner(vtx);
	    if (plan->find(vtx)!=plan->end())
	      o= plan->find(vtx)->second;
	    pin_owners[deg]=o;
	    pin[deg++] = globalID(vtx);
	    residence.insert(o);
	  }
	}
	else {
	  id = localID(e);
	  pin = new gid_t[2];
	  pin_owners = new part_t[2];
	  GraphVertex* source = u(e);
	  pin_owners[deg] = owner(source);
	  pin[deg++] = globalID(source);
	  GraphVertex* dest = v(e);
	  part_t o = owner(dest);
	  if (plan->find(dest)!=plan->end())
	    o= plan->find(dest)->second;
	  pin_owners[deg] = o;
	  pin[deg++] = globalID(dest);
	  Migration::iterator mitr = plan->find(source);
	  pin_owners[0] = mitr->second;
	  assert(mitr!=plan->end());
	  residence.insert(mitr->second);
	}
	std::unordered_set<part_t>::iterator sitr;
	for (sitr=residence.begin();sitr!=residence.end();sitr++) {
	  PCU_COMM_PACK(*sitr,id);
	  PCU_COMM_PACK(*sitr,deg);
	  PCU_Comm_Pack(*sitr,pin,deg*sizeof(gid_t));
	  PCU_Comm_Pack(*sitr,pin_owners,deg*sizeof(part_t));
	}
	delete [] pin;
	delete [] pin_owners;
      }
  
      PCU_Comm_Send();
      while (PCU_Comm_Receive()) {
	gid_t id;
	lid_t deg;
	PCU_COMM_UNPACK(id);
	PCU_COMM_UNPACK(deg);
	gid_t* pin = new gid_t[deg];
	part_t* pin_owners = new part_t[deg];
	PCU_Comm_Unpack(pin,deg*sizeof(gid_t));
	PCU_Comm_Unpack(pin_owners,deg*sizeof(part_t));
	if (isHyperGraph) {
	  if (edge_mapping[t].find(id)!=edge_mapping[t].end())
	    continue;
	}
	edge_mapping[t][id]=0;
	ownedEdges.push_back(id);
	degrees.push_back(deg);
	for (lid_t i=0;i<deg;i++) {
	  pins.push_back(pin[i]);
	  if (pin_owners[i]!=PCU_Comm_Self())
	    ghost_owners[pin[i]]=pin_owners[i];
	}
	delete [] pin;
	delete [] pin_owners;
      }
    }
    printf("Vertices:");
    for (unsigned int i=0;i<ownedVerts.size();++i)
      printf(" %lu",ownedVerts[i]);
    printf("\n");
    lid_t deg=0;
    for (unsigned int i=0;i<ownedEdges.size();++i) {
      printf("Edge: %lu Degree: %lu\n",ownedEdges[i],degrees[i]);
      printf("Pins:");
      for (lid_t j=0;j<degrees[i];j++)
	printf(" %lu",pins[deg+j]);
      printf("\n");
      deg+=degrees[i];
    }
    printf("Ghost Owners:");
    std::unordered_map<gid_t,part_t>::iterator uitr;
    for (uitr=ghost_owners.begin();uitr!=ghost_owners.end();uitr++) {
      printf(" %lu->%d",uitr->first,uitr->second);
    }
    printf("\n");

    constructGraph(isHyperGraph,ownedVerts,ownedEdges,degrees,pins,ghost_owners);
    delete [] affectedEdges;

    return;
  }


}
