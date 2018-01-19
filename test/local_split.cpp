#include <engpar_support.h>
#include <engpar.h>
#include <engpar_input.h>
#include <binGraph.h>
#include <engpar_split.h>
#include <PCU.h>
#include <sys/types.h>
#include <unistd.h>
void switchToOriginals(int split_factor, bool& isOriginal, MPI_Comm& newComm);

int main(int argc, char* argv[]) {
  MPI_Init(&argc,&argv);
  EnGPar_Initialize();

  if (argc != 4) {
    if ( !PCU_Comm_Self() ) {
      printf("Usage: %s <graph_file> <split_factor> <out_file>\n", argv[0]);
    }
    EnGPar_Finalize();
    assert(false);
  }
  
  //Application code:
  bool isOriginal = false;
  MPI_Comm newComm;
  int split_factor = atoi(argv[2]);
  switchToOriginals(split_factor, isOriginal,newComm);

  //Calls to EnGPar:
  agi::Ngraph* g = agi::createEmptyGraph();

  //Create the input (this sets up the internal communicators,
  //                    so this must be done before graph construction.)
  double tolerance = 1.05;
  agi::etype t = 0;
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  agi::part_t* others = new agi::part_t[split_factor];
  for (int i=0;i<split_factor;i++) {
    others[i] = my_rank+i;
  }
  engpar::Input* input = engpar::createSplitInput(g,newComm,MPI_COMM_WORLD, isOriginal,
                                                  split_factor,tolerance,t,others);
  
  if (isOriginal) {
    //Only the original parts will construct the graph
    g->loadFromFile(argv[1]);
    engpar::evaluatePartition(g);
  }
  
  engpar::split(input,engpar::LOCAL_PARMETIS);
  assert(PCU_Get_Comm()==MPI_COMM_WORLD);
  engpar::evaluatePartition(g);

  //Application continues:
  MPI_Comm_free(&newComm);

  g->saveToFile(argv[3]);

  agi::destroyGraph(g);
  EnGPar_Finalize();
  MPI_Finalize();
  return 0;
}


void switchToOriginals(int split_factor, bool& isOriginal, MPI_Comm& newComm) {
  int self = PCU_Comm_Self();
  int group;
  int groupRank;
  isOriginal = self%split_factor==0;

  if (isOriginal) {
    group=0;
    groupRank=self/split_factor;
  }
  else {
    group = 1;
    groupRank = 0;
  }
  MPI_Comm_split(MPI_COMM_WORLD,group,groupRank,&newComm);
}
