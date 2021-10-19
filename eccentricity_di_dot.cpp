#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/exterior_property.hpp>  //comprised of container and prpty_map. extn: for matrix property
#include <boost/graph/graphviz.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <libs/graph/src/read_graphviz_new.cpp>
#include <boost/graph/floyd_warshall_shortest.hpp>

using namespace std;
using namespace boost;

template <class WeightMap>
class edge_writer {
public:
  edge_writer(WeightMap w) : wm(w) {}
  template <class Edge>
  void operator()(ostream &out, const Edge& e) const {
    out << "[label=\"" << wm[e] << "\", weight=\"" << wm[e]  << "\"]";
  }
private:
  WeightMap wm;
};

template <class WeightMap>
inline edge_writer<WeightMap> 
make_edge_writer(WeightMap w) {
  return edge_writer<WeightMap>(w);
} 

string * labelGenerator(int* eccentricity,int n)
{

	string *name = new string[n];

	for(int i=0;i<n;i++)
	{
		name[i]= to_string(i) + " (e=" + to_string(eccentricity[i]) + ")";
	
	}

	return name;
} 

// graph type
typedef adjacency_list<vecS, vecS, directedS, property<vertex_name_t, string>, property<edge_name_t, string, property<edge_weight_t, double> > > Graph;


int main()
{

    string *name;
    Graph g_in;
    dynamic_properties dp;
    dp.property("node_id", get(vertex_name, g_in));
    dp.property("label",   get(edge_weight, g_in));
    dp.property("weight",  get(edge_weight, g_in));

   
    ifstream indata("input_di.gv");
    ofstream outdata("output.gv");
    if (read_graphviz(indata, g_in, dp)) 
    {
    property_map<Graph, edge_weight_t>::type weight_pmap = get(edge_weight, g_in);
    
   //matrix type and its corresponding prpty map that will contain the distances between each pair of vertices.
   typedef exterior_vertex_property<Graph, double> DistanceProperty;
   DistanceProperty::matrix_type distances(num_vertices(g_in));
   DistanceProperty::matrix_map_type distancematrix (distances,g_in);


  // find all pairs shortest paths
  bool valid = floyd_warshall_all_pairs_shortest_paths(g_in, distancematrix, weight_map(weight_pmap));

  // check if there no negative cycles
  if (!valid) {
    cerr << " Error: Negative cycle in matrix" << endl;
    return -1;
  }

 /* // print distance matrix
  cout << "Distance matrix: " << endl;
  for (int i = 0; i < num_vertices(g_in); ++i) 
  {
    for (int j = 0; j < num_vertices(g_in); ++j) 
    {
      cout << "From vertex " << i << " to " << j << " : ";
      if(distances[i][j] == numeric_limits<double>::max())
        cout << "inf" << endl;
      else
        cout << distances[i][j] << endl;
    }
    cout << endl;
  } */
  
  // print eccentricity
  
  int *eccentricity = new int[num_vertices(g_in)];
  
  int diameter=0,radius = INT_MAX;
  cout << "Eccentricity: " << endl;
  for (int i = 0; i < num_vertices(g_in); ++i) 
  {
    int max = 0;
    for (int j = 0; j < num_vertices(g_in); ++j) 
    {
       if(max < distances[i][j])
           max = distances[i][j];
    }
    eccentricity[i] = max;
    
    dp.property("e"+to_string(i),make_constant_property<Graph*>(max));
    
    cout << "Eccentricity of vertex " << i << " is " << " : " << max << endl ;
    if(diameter < max)
      diameter = max;
    if(radius > max)
       radius = max;   
  } 
  
  name=labelGenerator(eccentricity,num_vertices(g_in));
  
  cout <<  "Diameter of a graph is : " << diameter << endl;
  cout <<  "Radius of a graph is : " << radius << endl;
  
  write_graphviz(outdata,g_in,make_label_writer(name),make_edge_writer(get(edge_weight,g_in)));
 
  }
  return 0;
}
