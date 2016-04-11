/* 
 * File:   main.cpp
 * Author: juro
 *
 * Created on April 5, 2016, 10:11 AM
 */

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <math.h>
#include <ctime>

#define min(a,b) a < b ? a : b;

using namespace std;

/*
 * 
 */

long long last_id = 0;

long long get_next_id() {
    return last_id++;
}

class Vertex {
    public:
        int type;
        long long id;
        set<Vertex*> incoming;
        set<Vertex*> outgoing;
        Vertex(int t, long long _id) : type(t), id(_id) {}
        Vertex(int t) : type(t) {
            this->id = get_next_id();
        }
        Vertex(){}
        string to_string() const {
            return "[V" + std::to_string(this->type) + ", I" + std::to_string(this->id) + "]";
        }
};

ostream& operator<<(ostream &strm, const Vertex &v) {
    strm << v.to_string();
    return strm;
}

class Graph {
    private:
        double compute_flow(Vertex* vertex, map<Vertex*, double> &flows) {
            if (flows.find(vertex) != flows.end()) return flows[vertex];
            double flow = 0;
            if (vertex->type == 0) {
                flow = this->source_data;
            } else {
                for (Vertex* in_v : vertex->incoming) {
                    flow += compute_flow(in_v, flows) * this->df_coef[make_pair(in_v->type, vertex->type)];
                }
            }
            flows[vertex] = flow;
            return flow;
        }
    public:
        double source_data;
        map<pair<int, int>, double> df_coef;
        set<Vertex*> vertices;
        Graph(double sdata, map<pair<int, int>, double> coef, set<Vertex*> vert)
            : source_data(sdata), df_coef(coef) {
            this->vertices = vert;
        }
        string to_string() const {
            string ret = "Graph:\n";
            for (Vertex* vertex : this->vertices) {
                ret += "  " + vertex->to_string() + ":";
                ret += "\n    Out: ";
                for (Vertex* out: vertex->outgoing) {
                    ret += out->to_string() + ", ";
                }
                ret += "\n    In:  ";
                for (Vertex* in: vertex->incoming) {
                    ret += in->to_string() + ", ";
                }
                ret += "\n";
            }
            ret += "Dataflow coefficients: ";
            for (auto it : this->df_coef) {
                ret += std::to_string(it.first.first) + "->" + std::to_string(it.first.second) + ": " + std::to_string(it.second) + ", ";
            }
            ret += "\n";
            return ret;
        }
        Graph copy() {
            map<Vertex*, Vertex*> copies;
            set<Vertex*> new_vertices;
            // Create a copy of each vertex
            for (Vertex* v : this->vertices) {
                Vertex* new_v = new Vertex(v->type, v->id);
                copies[v] = new_v;
                new_vertices.insert(new_v);
            }
            // Assign incoming and outgoing edges
            for (Vertex* v : this->vertices) {
                Vertex* new_v = copies[v];
                for (Vertex* in_v : v->incoming) {
                    new_v->incoming.insert(copies[in_v]);
                }
                for (Vertex* out_v : v->outgoing) {
                    new_v->outgoing.insert(copies[out_v]);
                }
            }
            
            Graph result(this->source_data, this->df_coef, new_vertices);
            return result;
        }
        map<Vertex*, double> getFlows() {
            map<Vertex*, double> flows;
            for (Vertex* vertex : this->vertices) {
                compute_flow(vertex, flows);
            }
            return flows;
        }
        
};

ostream& operator<<(ostream &strm, const Graph &g) {
    strm << g.to_string();
    return strm;
}
    
Vertex* get_vert_of_type(int type, map<int, Vertex*> &type_to_vertex) {
    if (type_to_vertex.find(type) == type_to_vertex.end()) {
        Vertex* vert = new Vertex(type);
        type_to_vertex[type] = vert;
    }
    return type_to_vertex[type];
}

vector<Vertex*> rev_top_sort(Graph g) {
    vector<Vertex*> sorted;    
    if (g.vertices.size() == 0) {
        return sorted;
    }
    Vertex* to_remove = NULL;
    for (Vertex* vertex : g.vertices) {
        if (vertex->incoming.size() == 0) {
            to_remove = vertex;
            break;
        }
    }
    if (to_remove == NULL) {
        cout << "There are no vertices without incoming edges ! " << endl;
    }
    for (Vertex* out_v : to_remove->outgoing) {
        out_v->incoming.erase(to_remove);
    }
    for (Vertex* in_v : to_remove->incoming) {
        in_v->outgoing.erase(to_remove);
    }
    g.vertices.erase(to_remove);
    sorted = rev_top_sort(g);
    sorted.push_back(to_remove);
    
    // Rollback changes to graph
    g.vertices.insert(to_remove);
    for (Vertex* out_v : to_remove->outgoing) {
        out_v->incoming.insert(to_remove);
    }
    for (Vertex* in_v : to_remove->incoming) {
        in_v->outgoing.insert(to_remove);
    }

    return sorted;
}

Vertex* copy_subtree(Graph &g, Vertex* root) {
    Vertex* new_root = new Vertex(root->type);
    
    for (Vertex* child : root->outgoing) {
        Vertex* new_child = copy_subtree(g, child);
        new_root->outgoing.insert(new_child);
        new_child->incoming.insert(new_root);
    }
    g.vertices.insert(new_root);
    return new_root;
}

Graph normalize(Graph g) {
    Graph graph = g.copy();
    vector <Vertex*> sorted = rev_top_sort(graph);
    for (Vertex* vertex : sorted) {
        if (vertex->incoming.size() > 1) {
            for (set<Vertex*>::iterator it = vertex->incoming.begin(); ++it != vertex->incoming.end(); ) {
                Vertex* parent = *it;
                Vertex* new_subtree = copy_subtree(graph, vertex);
                new_subtree->incoming.insert(parent);
                parent->outgoing.erase(vertex);
                parent->outgoing.insert(new_subtree);
            }
        }
    }
    return graph;
}

pair<double, vector<Vertex*> > _cut_normalized_subtree(Vertex* root, map<Vertex*, double> &flows) {
    vector<Vertex *> sub_vert;
    if (root->outgoing.size() == 0) return make_pair(INFINITY, sub_vert);
    double cut_subtrees = 0;
    double root_flow = flows[root];
    for (Vertex* child : root->outgoing) {
        pair<double, vector<Vertex*> > sub_cut = _cut_normalized_subtree(child, flows);
        cut_subtrees += sub_cut.first;
        if (cut_subtrees > root_flow) break;
        sub_vert.insert(sub_vert.end(), sub_cut.second.begin(), sub_cut.second.end());
    }
    if (cut_subtrees < root_flow) {
        return make_pair(cut_subtrees, sub_vert);
    } else {
        sub_vert.clear();
        sub_vert.push_back(root);
        return make_pair(root_flow, sub_vert);
    }
}

pair<double, vector<Vertex*> > cut_normalized(Graph g) {
    for (Vertex* vertex : g.vertices) {
        if (vertex->type == 0) {
            map<Vertex*, double> flows = g.getFlows();
            return _cut_normalized_subtree(vertex, flows);
        }
    }
}

int main(int argc, char** argv) {
    
    // Number of vertices
    int n;
    double sdata;
    cin >> n >> sdata;
    
    map<int, Vertex*> type_to_vertex;

    // Dataflow coefficient for vertices
    map<pair<int, int>, double> df_coef;
    for (int i = 0; i < n-1; i++) {
        // Vertex index & number of incoming edges
        int v1, ve;
        scanf("%d %d\n", &v1, &ve);
        Vertex* vert = get_vert_of_type(v1, type_to_vertex);
        for (int j = 0; j < ve; j++) {
            int v2;
            double coef;
            // Vertex index of incoming edge & dataflow coefficient
            scanf("%d %lf", &v2, &coef);
            df_coef[make_pair(v2, v1)] = coef;
            Vertex* vert2 = get_vert_of_type(v2, type_to_vertex);
            vert->incoming.insert(vert2);
            vert2->outgoing.insert(vert);
        }
    }
    
    // First vertex has all outgoing edges, last all incoming
    set<Vertex*> vertices;
    for (auto elem : type_to_vertex) {
        vertices.insert(elem.second);
    }
    
    Graph graph(sdata, df_coef, vertices);
    cout << graph << endl;

    clock_t time;
    time = clock();
    Graph normalized = normalize(graph);

    pair<double, vector<Vertex*> > cut = cut_normalized(normalized);
    
    cout << "Min cut weight: " << cut.first << endl;
    cout << "Vertices in cut: ";
    for (Vertex* v : cut.second) {
        cout << (*v) << ", ";
    }
    cout << "Took: " << (double)((clock() - time)/(CLOCKS_PER_SEC/1000)) << "ms" << endl;
    cout << endl;
    
    return 0;
}
