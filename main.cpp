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
        map<long long, Vertex*> incoming;
        map<long long, Vertex*> outgoing;
        Vertex(int t, long long _id) : type(t), id(_id) {}
        Vertex(int t) : type(t) {
            this->id = get_next_id();
        }
        Vertex(){}
        string to_string() const {
            return "[V" + std::to_string(this->type) + ", I" + std::to_string(this->id) + "]";
        }
};

bool operator<(const Vertex& left, const Vertex& other) {
    return left.id < other.id;
}
bool operator>(const Vertex& left, const Vertex& other) {
    return left.id > other.id;
}
bool operator==(const Vertex& left, const Vertex& other) {
    return left.id == other.id;
}
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
                for (pair<long long, Vertex*> in_v : vertex->incoming) {
                    flow += compute_flow(in_v.second, flows) * this->df_coef[make_pair(in_v.second->type, vertex->type)];
                }
            }
            flows[vertex] = flow;
            return flow;
        }
    public:
        double source_data;
        map<pair<int, int>, double> df_coef;
        map<long long, Vertex*> vertices;
        Graph(double sdata, map<pair<int, int>, double> coef, map<long long, Vertex*> vert)
            : source_data(sdata), df_coef(coef) {
            this->vertices = vert;
        }
        string to_string() const {
            string ret = "Graph:\n";
            for (auto it : this->vertices) {
                ret += "  " + it.second->to_string() + ":";
                ret += "\n    Out: ";
                for (auto out: it.second->outgoing) {
                    ret += out.second->to_string() + ", ";
                }
                ret += "\n    In:  ";
                for (auto in: it.second->incoming) {
                    ret += in.second->to_string() + ", ";
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
            map<long long, Vertex*> new_vertices;
            // Create a copy of each vertex
            for (pair<long long, Vertex*> it : this->vertices) {
                Vertex* new_v = new Vertex(it.second->type, it.first);
                new_vertices[it.first] = new_v;
            }
            // Assign incoming and outgoing edges
            for (pair<long long, Vertex*> it : this->vertices) {
                Vertex* v = it.second;
                Vertex* new_v = new_vertices[v->id];
                for (pair<long long, Vertex*> itv : v->incoming) {
                    new_v->incoming[itv.first] = new_vertices[itv.first];
                }
                for (pair<long long, Vertex*> itv : v->outgoing) {
                    new_v->outgoing[itv.first] = new_vertices[itv.first];
                }
            }
            
            Graph result(this->source_data, this->df_coef, new_vertices);
            return result;
        }
        map<Vertex*, double> getFlows() {
            map<Vertex*, double> flows;
            for (pair<long long, Vertex*> it : this->vertices) {
                compute_flow(it.second, flows);
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
    }return type_to_vertex[type];
}

vector<Vertex*> rev_top_sort(Graph g) {
    vector<Vertex*> sorted;    
    if (g.vertices.size() == 0) {
        return sorted;
    }
    Vertex* to_remove = NULL;
    for (pair<long long, Vertex*> it : g.vertices) {
        if (it.second->incoming.size() == 0) {
            to_remove = it.second;
            break;
        }
    }
    if (to_remove == NULL) {
        cout << "There are no vertices without incoming edges ! " << endl;
    }
    for (auto it : to_remove->outgoing) {
        it.second->incoming.erase(to_remove->id);
    }
    for (auto it : to_remove->incoming) {
        it.second->outgoing.erase(to_remove->id);
    }
    g.vertices.erase(to_remove->id);
    sorted = rev_top_sort(g);
    sorted.push_back(to_remove);
    
    // Rollback changes to graph
    g.vertices[to_remove->id] = to_remove;
    for (auto it : to_remove->outgoing) {
        it.second->incoming[to_remove->id] = to_remove;
    }
    for (auto it : to_remove->incoming) {
        it.second->outgoing[to_remove->id] = to_remove;
    }

    return sorted;
}

Vertex* copy_subtree(Graph &g, Vertex* root) {
    Vertex* new_root = new Vertex(root->type);
    
    for (auto it : root->outgoing) {
        Vertex* child = it.second;
        Vertex* new_child = copy_subtree(g, child);
        new_root->outgoing[new_child->id] = new_child;
        new_child->incoming[new_root->id] = new_root;
    }
    g.vertices[new_root->id] = new_root;
    return new_root;
}

Graph normalize(Graph g) {
    Graph graph = g.copy();
    vector <Vertex*> sorted = rev_top_sort(graph);
    for (Vertex* vertex : sorted) {
        if (vertex->incoming.size() > 1) {
            for (map<long long, Vertex*>::iterator it = vertex->incoming.begin(); ++it != vertex->incoming.end(); ) {
                Vertex* parent = it->second;
                Vertex* new_subtree = copy_subtree(graph, vertex);
                new_subtree->incoming[parent->id] = parent;
                parent->outgoing.erase(vertex->id);
                parent->outgoing[new_subtree->id] = new_subtree;
            }
        }
    }
    return graph;
}

pair<double, vector<Vertex*>> _cut_normalized_subtree(Vertex* root, map<Vertex*, double> &flows) {
    vector<Vertex *> sub_vert;
    if (root->outgoing.size() == 0) return make_pair(INFINITY, sub_vert);
    double cut_subtrees = 0;
    double root_flow = flows[root];
    for (auto it : root->outgoing) {
        Vertex* child = it.second;
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
    for (auto it : g.vertices) {
        if (it.second->type == 0) {
            map<Vertex*, double> flows = g.getFlows();
            return _cut_normalized_subtree(it.second, flows);
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
            vert->incoming[vert2->id] = vert2;
            vert2->outgoing[vert->id] = vert;
        }
    }
    
    // First vertex has all outgoing edges, last all incoming
    map<long long, Vertex*> vertices;
    for (auto elem : type_to_vertex) {
        vertices[elem.second->id] = elem.second;
    }
    
    Graph graph(sdata, df_coef, vertices);
    cout << graph << endl;

    Graph normalized = normalize(graph);
    
    map<Vertex*, double> flows = normalized.getFlows();
    
    cout << "Flows: " << endl;
    for (pair<Vertex*, double> flow : flows) {
        cout << (*flow.first) << ": " << flow.second << endl;
    }
    
    pair<double, vector<Vertex*> > cut = cut_normalized(normalized);
    
    cout << "Min cut weight: " << cut.first << endl;
    cout << "Vertices in cut: ";
    for (Vertex* v : cut.second) {
        cout << (*v) << ", ";
    }
    cout << endl;
    
    return 0;
}
