#include "engpar_balancer.h"
#include "engpar_queue.h"
#include <PCU.h>
#include "../engpar.h"
#include <engpar_support.h>
namespace engpar {

  wgt_t getMaxWeight(agi::Ngraph* g, int dimension) {
    wgt_t w = getWeight(g,dimension);
    return PCU_Max_Double(w);
  }

  wgt_t getAvgWeight(agi::Ngraph* g, int dimension) {
    wgt_t w = getWeight(g,dimension);
    return PCU_Add_Double(w) / PCU_Comm_Peers();
  }
  
  Balancer::Balancer(agi::Ngraph* g, double f, int v, const char* n) :
    agi::Balancer(g,v,n) {
    input = new Input(g);
    input->step_factor = f;
    times[0]=0;
    times[1]=0;
  }
  Balancer::Balancer(Input* input_, int v, const char* n) :
    agi::Balancer(input_->g,v,n), input(input_) {
    times[0]=0;
    times[1]=0;
  }
  bool Balancer::runStep(double tolerance) {
    double time[2];
    time[0] = PCU_Time();
    Sides* sides = makeSides(input);
    if (verbosity)
      printf("%d: %s\n",PCU_Comm_Self(), sides->print("Sides").c_str());
    Weights* targetWeights = makeWeights(input, sides,target_dimension);
    if (verbosity)
      printf("%d: %s\n",PCU_Comm_Self(),
             targetWeights->print("Weights").c_str());
    Weights** completedWs = NULL;
    if (completed_dimensions.size()>0) {
      completedWs= new Weights*[completed_dimensions.size()];
      for (unsigned int i=0;i<completed_dimensions.size();i++) {
        completedWs[i] = makeWeights(input,sides,completed_dimensions[i]);
      }
    }
    Targets* targets = makeTargets(input,sides,targetWeights,
                                   completedWs,completed_weights);
    delete sides;
    if (completedWs) {
      for (unsigned int i=0;i<completed_dimensions.size();i++)
        delete completedWs[i];
      delete [] completedWs;
    }
    delete targetWeights;
        
    if (verbosity)
      printf("%d: %s\n",PCU_Comm_Self(), targets->print("Targets").c_str());
    Queue* pq = createIterationQueue(input->g);
    Selector* selector = makeSelector(input,pq,&completed_dimensions,
                                      &completed_weights);
    agi::Migration* plan = new agi::Migration;
    wgt_t planW = 0.0;
    for (unsigned int cavSize=2;cavSize<=12;cavSize+=2) {
      planW += selector->select(targets,plan,planW,cavSize,target_dimension);
    }
    if (completed_dimensions.size()>0) {
      //Midd* midd = selector->trim(targets,plan);
      //selector->cancel(plan,midd);
    }

    delete pq;
    delete targets;
    delete selector;
    
    time[0] = PCU_Time()-time[0];
    int numMigrate = plan->size();
    if (verbosity>=2) {
      int* counts = new int[PCU_Comm_Peers()];
      for (int i=0;i<PCU_Comm_Peers();i++)
	counts[i] = 0;
      agi::Migration::iterator itr;
      for (itr = plan->begin();itr!=plan->end();itr++)
	counts[itr->second]++;
      for (int i=0;i<PCU_Comm_Peers();i++)
	if (counts[i]>0)
	  printf("%d sending %d to %d\n",PCU_Comm_Self(),counts[i],i);
    }
    time[1] = PCU_Time();
    input->g->migrate(plan);
    time[1] = PCU_Time()-time[1];
    numMigrate = PCU_Add_Int(numMigrate);
    PCU_Max_Doubles(time,2);
    if (!PCU_Comm_Self()) {
      printf("Step took %f seconds\n",time[0]);
      printf("Migrating %d vertices took %f seconds\n",numMigrate,time[1]);
      times[0]+=time[0];
      times[1]+=time[1];
    }

    double imb = EnGPar_Get_Imbalance(getWeight(input->g,target_dimension));
    //Check for completition of criteria
    return imb>tolerance;
  }
  void Balancer::balance(double) {
    if (EnGPar_Is_Log_Open()) {
      char message[1000];
      sprintf(message,"balance() : \n");
      //Log the input parameters
      sprintf(message,"%s priorities :",message);
      for (unsigned int i=0;i<input->priorities.size();i++)
        sprintf(message,"%s %d",message,input->priorities[i]);
      sprintf(message,"%s\n",message);
      if (input->tolerances.size()>0) {
        sprintf(message,"%s tolerances :",message);
        for (unsigned int i=0;i<input->tolerances.size();i++)
          sprintf(message,"%s %f",message,input->tolerances[i]);
        sprintf(message,"%s\n",message);
      }
      sprintf(message,"%s maxIterations : %d\n",message,input->maxIterations);
      sprintf(message,"%s maxIterationsPerType : %d\n",
              message,input->maxIterationsPerType);
      sprintf(message,"%s step_factor : %f\n",message,input->step_factor);
      sprintf(message,"%s sides_edge_type : %d\n",message,input->sides_edge_type);
      sprintf(message,"%s selection_edge_type : %d\n",
              message,input->selection_edge_type);
      sprintf(message,"%s countGhosts : %d\n",message,input->countGhosts);
      
      EnGPar_Log_Function(message);
    }

    //Setup the original owners arrays before balancing
    input->g->setOriginalOwners();
    unsigned int index=0;
    target_dimension = input->priorities[index];
    double tol=1.1;
    if (input->tolerances.size()>index)
      tol = input->tolerances[index];
    if (1 == PCU_Comm_Peers()) {
      printf("EnGPar ran in serial, nothing to do exiting...\n");
      return;
    }
    int step = 0;
    int inner_steps=0;
    double time = PCU_Time();
    double targetTime=PCU_Time();
    while (step++<input->maxIterations) {
      if (!runStep(tol)||inner_steps++>=input->maxIterationsPerType) {
        completed_dimensions.push_back(target_dimension);
        double maxW = getMaxWeight(input->g,target_dimension);
        double tgtMaxW = getAvgWeight(input->g,target_dimension)*tol;
        maxW = ( maxW < tgtMaxW ) ? tgtMaxW : maxW;
        completed_weights.push_back(maxW);
        
	targetTime = PCU_Time()-targetTime;
	targetTime = PCU_Max_Double(targetTime);
	if (!PCU_Comm_Self()) {
	  printf("Completed criteria type %d in %f seconds\n",target_dimension,
		 targetTime);
	}
	targetTime=PCU_Time();
        
        index++;
	if (index==input->priorities.size())
	  break;
        inner_steps=0;
	target_dimension=input->priorities[index];
        if (input->tolerances.size()>index)
          tol = input->tolerances[index];
      }      
    }
    time = PCU_Time()-time;
    time = PCU_Max_Double(time);
    if (!PCU_Comm_Self()) {
      if (step==input->maxIterations)
	printf("EnGPar ran to completion in %d iterations in %f seconds\n",
	       input->maxIterations, time);
      else
	printf("EnGPar converged in %d iterations in %f seconds\n",step,
	       time);
      if (verbosity)
	printf("Migration took %f%% of the total time\n",times[1]/time*100);
    }
    if (EnGPar_Is_Log_Open())
      EnGPar_End_Function();
  }

  agi::Balancer* makeBalancer(Input* in,int v_) {
    if (EnGPar_Is_Log_Open()) {
      char message[25];
      sprintf(message,"makeBalancer\n");
      EnGPar_Log_Function(message);
      EnGPar_End_Function();
    }
    return new Balancer(in,v_,"balancer");
  }
}

