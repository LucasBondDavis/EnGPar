#include "ngraph.h"
#include <stdio.h>
#include <PCU.h>
#include <cstring>
#include <sys/stat.h>
namespace agi {

  /* File Format 
   * <isHyperGraph>
   * <number of vertices>
   * <vertex gid> <vertex weight> ...
   * <number of edges>
   * <edge gid> <edge weight> <edge degree> <pins to vertices ...> ...
   * <ghost gid> <owner> ...
   */

  void writeHeader(FILE* f, Ngraph* g) {
    bool isH = g->isHyper();
    fwrite(&isH,sizeof(bool),1,f);
  }
  void writeVertices(FILE* f, Ngraph* g) {
    lid_t numVtxs = g->numLocalVtxs();
    fwrite(&numVtxs,sizeof(lid_t),1,f);
    agi::VertexIterator* itr = g->begin();
    agi::GraphVertex* vtx;
    while ((vtx = g->iterate(itr))) {
      gid_t gid =  g->globalID(vtx);
      wgt_t w = g->weight(vtx);
      fwrite(&gid,sizeof(gid_t),1,f);
      fwrite(&w,sizeof(wgt_t),1,f);
    }
  }
  void writeEdges(FILE* f,Ngraph* g) {
    gid_t numEdges = g->numLocalEdges();
    fwrite(&numEdges,sizeof(gid_t),1,f);
    agi::EdgeIterator* eitr = g->begin(0);
    agi::GraphEdge* edge;
    std::unordered_map<gid_t,part_t> owns;
    while ((edge = g->iterate(eitr))) {
      gid_t gid = g->globalID(edge);
      wgt_t w = g->weight(edge);
      lid_t deg = g->degree(edge);
      gid_t* ps = new gid_t[deg];
      agi::PinIterator* pitr = g->pins(edge);
      for (lid_t i = 0;i<deg;i++) {
        agi::GraphVertex* v = g->iterate(pitr);
        ps[i] = g->globalID(v);
        if (g->owner(v)!=PCU_Comm_Self())
          owns[ps[i]] = g->owner(v);
      }
      g->destroy(pitr);
      fwrite(&gid,sizeof(gid_t),1,f);
      fwrite(&w,sizeof(wgt_t),1,f);
      fwrite(&deg,sizeof(lid_t),1,f);
      fwrite(ps,sizeof(gid_t),deg,f);
      delete [] ps;
    }
    g->destroy(eitr);

    lid_t numOwners = owns.size();
    fwrite(&numOwners,sizeof(lid_t),1,f);
    std::unordered_map<gid_t,part_t>::iterator itr;
    for (itr=owns.begin();itr!=owns.end();itr++) {
      fwrite(&itr->first,sizeof(gid_t),1,f);
      fwrite(&itr->second,sizeof(part_t),1,f);
    }
  }

  //Recursive mkdir function modified from here:
  //http://nion.modprobe.de/blog/archives/357-Recursive-directory-creation.html
  void mkdir_r(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;
    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
      tmp[len - 1] = 0;
    for(p = tmp + 1; *p; p++)
      if(*p == '/') {
        *p = 0;
        mkdir(tmp, S_IRWXU);
        *p = '/';
      }
  }
  void Ngraph::saveToFile(char* prefix) {
    char filename[256];
    sprintf(filename,"%s_%d.bgd",prefix,PCU_Comm_Self());
    mkdir_r(filename);
    FILE* file = fopen(filename,"wb");
    if (!file) {
      printf("Could not open file for saving: %s#.bgd\n",prefix);
      throw 1;
    }
    writeHeader(file,this); 
    writeVertices(file,this); 
    writeEdges(file,this); 
    fclose(file);
  }

  bool readHeader(FILE* f) {
    bool isHG;
    size_t s = fread(&isHG,sizeof(bool),1,f);
    assert(s==1);
    return isHG;
  }
  void readVertices(FILE* f, std::vector<gid_t>& verts,
                    std::vector<wgt_t>& weights) {
    lid_t nv;
    size_t s = fread(&nv,sizeof(lid_t),1,f);
    assert(s == 1);
    for (lid_t i=0;i<nv;i++) {
      gid_t gid;
      wgt_t w;
      s = fread(&gid,sizeof(gid_t),1,f);
      s = fread(&w,sizeof(wgt_t),1,f);
      verts.push_back(gid);
      weights.push_back(w);
    }
  }
  void readEdges(FILE* f,std::vector<gid_t>& edges, std::vector<wgt_t>& eWeights,
                 std::vector<lid_t>& degs, std::vector<gid_t>& pins2v) {
    gid_t ne;
    size_t s = fread(&ne,sizeof(gid_t),1,f);
    assert( s ==1);
    for (gid_t i=0;i<ne;i++) {
      gid_t gid;
      wgt_t w;
      lid_t deg=0;
      
      s = fread(&gid,sizeof(gid_t),1,f);
      s = fread(&w,sizeof(wgt_t),1,f);
      edges.push_back(gid);
      eWeights.push_back(w);
      s = fread(&deg,sizeof(lid_t),1,f);
      degs.push_back(deg);
      gid_t pin;
      for (lid_t j=0;j<deg;j++) {
        s = fread(&pin,sizeof(gid_t),1,f);
        pins2v.push_back(pin);
      }
    }
    
  }
  void readGhosts(FILE* f, std::unordered_map<gid_t,part_t>& owns) {
    lid_t num_gs;
    size_t s = fread(&num_gs,sizeof(lid_t),1,f);
    assert(s == 1);
    gid_t v;
    part_t o;
    for (lid_t i =0;i<num_gs;i++) {
      s = fread(&v,sizeof(gid_t),1,f);
      s = fread(&o,sizeof(part_t),1,f);
      owns[v] = o;
    }
  }

  void Ngraph::loadFromFile(char* prefix) {
    char filename[256];
    sprintf(filename,"%s_%d.bgd",prefix,PCU_Comm_Self());
    FILE* file = fopen(filename,"rb");
    if (!file) {
      printf("Could not open file for loading: %s#.bgd",prefix);
      throw 1;
    }
    bool isHG = readHeader(file);
    if (isHG) {}
    std::vector<gid_t> verts;
    std::vector<wgt_t> weights;
    readVertices(file,verts,weights);
    
    std::vector<gid_t> es;
    std::vector<wgt_t> eWeights;
    std::vector<lid_t> degs;
    std::vector<gid_t> pins2v;
    std::unordered_map<gid_t,part_t> owns;
    readEdges(file,es,eWeights,degs,pins2v);
    readGhosts(file,owns);
    constructGraph(isHG,verts,weights,es,degs,pins2v,owns);
    setEdgeWeights(eWeights,0);
    
    fclose(file);
  }
}
