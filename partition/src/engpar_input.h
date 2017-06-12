#ifndef __ENGPAR_INPUT_H__
#define __ENGPAR_INPUT_H__

#include <ngraph.h>
namespace engpar {


  class Input {
  public:
    Input(agi::Ngraph*);
    ~Input();
    /** \brief The graph being balanced */
    agi::Ngraph* g;
    /** \brief The order of graph entities to be balanced 
     *
     *  -1 represents graph vertices, 0-MAX_TYPES represent edge_types
     */
    std::vector<int> priorities;
    /** \brief The imbalance tolerance for each priority
     *
     * tolerances must be given in the same order as in the priorities vector
     */
    std::vector<double> tolerances;
    /** \brief The maximum iterations for all load balancing */
    int maxIterations;
    /** \brief The maximum iterations of load balancing foreach graph entity type 
     *
     *  defaults to 100
     */
    int maxIterationsPerType;
    /** \brief The percent of difference in weight to send in each iteration 
     *
     * defaults to .1 
     */
    double step_factor;
    
    /** \brief The edge type used for determining part boundaries 
     *
     * defaults to 0
     */
    int sides_edge_type;
    /** \brief The edge type used for creating cavities for selection 
     *
     * defaults to 0
     */
    int selection_edge_type;

    /** \brief If ghosts should be accounted for in the weight of a part 
     *
     * defaults to false
     */
    bool countGhosts;
  };
    
}

#endif
