#ifndef __BUILD_GRAPHS_H__
#define __BUILD_GRAPHS_H__

#include <ngraph.h>

agi::Ngraph* buildGraph();

agi::Ngraph* buildHyperGraph();

agi::Ngraph* buildGraphParts();

agi::Ngraph* buildHyperGraphParts();

agi::Ngraph* buildHyperGraphLine();

agi::Ngraph* buildRequirementsGraph();

agi::Ngraph* buildDisconnected2Graph();

agi::Ngraph* buildEmptyGraph();

agi::Ngraph* buildUnbalancedLineHG();
#endif
