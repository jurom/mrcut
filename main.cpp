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
        virtual string to_string() const =0;
};

class PD : public Vertex {
    public:
        PD(int t, long long _id) : Vertex(t, _id) {}
        PD(int t) : Vertex(t) {}
        PD() {}
        string to_string() const {
            return "[PD " + std::to_string(this->type) + ", I" + std::to_string(this->id) + "]";
        }
};

class GBK : public Vertex {
    public:
        GBK(int t, long long _id) : Vertex(t, _id) {}
        GBK(int t) : Vertex(t) {}
        string to_string() const {
            return "[GBK " + std::to_string(this->type) + "-" + std::to_string(this->id) + "]";
        }
};

class Input : public Vertex {
    public:
        Input(int t) : Vertex(t) {}
        string to_string() const {
            return "[Input " + std::to_string(this->type) + "]";
        }
};

class Output : public Vertex {
    public:
        Output(int t) : Vertex(t) {}
        string to_string() const {
            return "[Output " + std::to_string(this->type) + "]";
        }
};

bool is_GBK(Vertex* vert) {
    GBK* gbk = dynamic_cast<GBK*>(vert);
    return gbk != 0;
}

bool is_PD(Vertex* vert) {
    PD* pd = dynamic_cast<PD*>(vert);
    return pd != 0;
}

bool is_Input(Vertex* vert) {
    Input* input = dynamic_cast<Input*>(vert);
    return input != 0;
}

bool is_Output(Vertex* vert) {
    Output* output = dynamic_cast<Output*>(vert);
    return output != 0;
}

Vertex* copy_vertex(Vertex* v) {
    if (is_PD(v)) return new PD(v->type);
    if (is_GBK(v)) return new GBK(v->type);
    if (is_Input(v)) return new Input(v->type);
    if (is_Output(v)) return new Output(v->type);
}

ostream& operator<<(ostream &strm, const Vertex* v) {
    strm << v->to_string();
    return strm;
}

class Graph {
    private:
        double compute_flow(Vertex* vertex, map<Vertex*, double> &flows) {
            if (flows.find(vertex) != flows.end()) return flows[vertex];
            double flow = 0;
            if (is_Input(vertex)) {
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
                Vertex* new_v = copy_vertex(v);
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
    
Vertex* get_vert_of_type(string vtype, int type, map<pair<string, int>, Vertex*> &type_to_vertex) {
    if (type_to_vertex.find(make_pair(vtype, type)) == type_to_vertex.end()) {
        Vertex* vert = NULL;
        if (vtype == "G") vert = new GBK(type);
        if (vtype == "P") vert = new PD(type);
        if (vtype == "I") vert = new Input(type);
        if (vtype == "O") vert = new Output(type);
        cout << "Is input " << is_Input(vert) << endl;
        type_to_vertex[make_pair(vtype, type)] = vert;
    }
    return type_to_vertex[make_pair(vtype, type)];
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

/* 
 * Copy subtree rooted in [root]
 */
Vertex* copy_subtree(Graph &g, Vertex* root) {
    Vertex* new_root = copy_vertex(root);
    
    set<Vertex*> out_to_add;
    for (Vertex* child : root->outgoing) {
        Vertex* new_child = copy_subtree(g, child);
        out_to_add.insert(new_child);
        new_child->incoming.insert(new_root);
    }
    new_root->outgoing.insert(out_to_add.begin(), out_to_add.end());
    g.vertices.insert(new_root);
    return new_root;
}

Graph normalize(Graph g) {
    Graph graph = g.copy();
    vector <Vertex*> sorted = rev_top_sort(graph);
    cout << "Starting split for every PD" << endl;
    clock_t time = clock();
    for (Vertex* vertex : sorted) {
        // Split only PD vertices
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
    map<pair<string, int>, vector<Vertex*>> vert_by_type;
    for (Vertex* vertex : graph.vertices) {
        if (is_GBK(vertex)) vert_by_type[make_pair("G", vertex->type)].push_back(vertex);
        if (is_Output(vertex)) vert_by_type[make_pair("O", vertex->type)].push_back(vertex);
    }
    for (auto it : vert_by_type) {
        cout << it.first.first << " " << it.first.second << ": " << it.second.size() << endl;
        Vertex* vertex = it.second[0];
        for (int i = 1; i < it.second.size(); i++) {
            Vertex* copy = it.second[i];
            for (Vertex* parent : copy->incoming) {
                parent->outgoing.erase(copy);
                parent->outgoing.insert(vertex);
                vertex->incoming.insert(parent);
            }
            for (Vertex* child : copy->outgoing) {
                child->incoming.erase(copy);
                child->incoming.insert(vertex);
                vertex->outgoing.insert(child);
            }
            graph.vertices.erase(copy);
        }
    }
    
    cout << "Normalize done, took " << (clock() - time)/(CLOCKS_PER_SEC/1000) << "ms" << endl;
    return graph;
}
pair<double, vector<Vertex*> > _cut_from_gbk_under_vert(Vertex* root, map<Vertex*, double> &flows);

pair<double, vector<Vertex*> > _cut_vert_from_gbk(Vertex* root, map<Vertex*, double> &flows) {
    vector<Vertex *> sub_vert;
    if (is_GBK(root)) return make_pair(INFINITY, sub_vert);
    if (is_Output(root)) return make_pair(0, sub_vert);
    return _cut_from_gbk_under_vert(root, flows);
}

pair<double, vector<Vertex*> > _cut_from_gbk_under_vert(Vertex* root, map<Vertex*, double> &flows) {
    vector<Vertex *> sub_vert;
    double cut_subtrees = 0;
    double root_flow = flows[root];
    for (Vertex* child : root->outgoing) {
        pair<double, vector<Vertex*> > sub_cut = _cut_vert_from_gbk(child, flows);
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
    vector<Vertex*> cut_vertices;
    double cut_cost = 0;
    map<Vertex*, double> flows = g.getFlows();
    for (Vertex* vertex : g.vertices) {
        if (is_GBK(vertex)) {
            
            pair<double, vector<Vertex*> > cut = _cut_from_gbk_under_vert(vertex, flows);
            cut_cost += cut.first;
            cut_vertices.insert(cut_vertices.end(), cut.second.begin(), cut.second.end());
        }
    }
    return make_pair(cut_cost, cut_vertices);
}

int main(int argc, char** argv) {
    
    // Number of vertices
    int n;
    double sdata;
    cin >> n >> sdata;
    
    map<pair<string, int>, Vertex*> type_to_vertex;

    // Dataflow coefficient for vertices
    map<pair<int, int>, double> df_coef;
    for (int i = 0; i < n; i++) {
        // Vertex index & number of incoming edges
        int v1, ve;
        string vtype;
        cin >> vtype >> v1 >> ve;
        Vertex* vert = get_vert_of_type(vtype, v1, type_to_vertex);
        for (int j = 0; j < ve; j++) {
            int v2;
            double coef;
            // Vertex index of incoming edge & dataflow coefficient
            string vtype2;
            cin >> vtype2 >> v2 >> coef;
            df_coef[make_pair(v2, v1)] = coef;
            Vertex* vert2 = get_vert_of_type(vtype2, v2, type_to_vertex);
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
//    cout << "Flows: " << endl;
//    for (auto it : normalized.getFlows()) {
//        cout << it.first << ": " << it.second << endl;
//    }
    cout << "Min cut weight: " << cut.first << endl;
    cout << "Vertices in cut: ";
    for (Vertex* v : cut.second) {
        cout << v << ", ";
    }
    cout << "Took: " << (double)((clock() - time)/(CLOCKS_PER_SEC/1000)) << "ms" << endl;
    cout << endl;
    
    return 0;
}
