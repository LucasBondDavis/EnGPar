#include <apfGraph.h>
#include <apfMesh2.h>
#include <cassert>
#include <PCU.h>
#include <gmi_mesh.h>
#include <apfMDS.h>
#include <binGraph.h>
#include <apf.h>
#include <set>
void testAdjacent(agi::Ngraph* g,agi::etype t=0);

int main(int argc, char* argv[]) {
  MPI_Init(&argc,&argv);
  PCU_Comm_Init();

  //Load in PUMI mesh
  gmi_register_mesh();
  apf::Mesh2* m = apf::loadMdsMesh(argv[1],argv[2]);

  //Construct Ngraph with edges over mesh faces
  agi::Ngraph* g = agi::createAPFGraph(m,3,2);

  //run the adjacency test
  testAdjacent(g);
  
  //Destroy Ngraph
  agi::destroyGraph(g);

  //Construct Ngraph with edges over mesh vertices
  g = agi::createAPFGraph(m,3,1);

  //run the adjacency test
  testAdjacent(g);

  //Destroy Ngraph
  agi::destroyGraph(g);

  //Destroy Mesh
  m->destroyNative();
  apf::destroyMesh(m);

  //Construct a ngraph from traditional graph
  g = agi::createBinGraph(argv[3]);

  //run the adjacency test
  testAdjacent(g);

  //Destroy the graph
  agi::destroyGraph(g);
  
  PCU_Barrier();
  if (!PCU_Comm_Self())
    printf("\nAll tests passed\n");

  PCU_Comm_Free();
  MPI_Finalize();
}

bool phi(char* word) {
  if (!PCU_Comm_Self())
    printf("hi %s\n", word);
  return true;
}
void testAdjacent(agi::Ngraph* g,agi::etype t) {
  if (!PCU_Comm_Self())
    printf("Beginning Traversal\n");
  agi::VertexIterator* vitr = g->begin();
  agi::GraphVertex* vtx = NULL;
  int num_pins =0;
  int num_edges=0;
  int tot_degrees=0;
  while ((vtx = g->iterate(vitr))) {
    agi::GraphIterator* gitr = g->adjacent(vtx,t);
    agi::GraphVertex* other = NULL;
    agi::GraphEdge* edge = NULL;
    while ((other = g->iterate(gitr))) {
      assert(other);
      edge = g->edge(gitr);
      assert(edge);
      if (g->isEqual(vtx,other))
	num_pins++;
      num_edges++;      
      //tot_degrees+=(g->degree(edge));
    }
    g->destroy(gitr);
  }
  if (g->isHyper()) {
    if (PCU_Comm_Peers()==1)
      assert(num_pins==g->numLocalPins(t));
    //assert(num_edges==tot_degrees);
  }
  else {
    assert(num_edges==g->numLocalEdges(t));
  }
}
