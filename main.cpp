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
    public:
        double source_data;
        double **df_coef;
        map<long long, Vertex*> vertices;
        Graph(int sdata, double **coef, map<long long, Vertex*> vert)
            : source_data(sdata), df_coef(coef) {
            this->vertices = vert;
        }
        string to_string() const {
            string ret = "Graph(";
            for (auto it = this->vertices.begin(); it != this->vertices.end(); ) {
                ret += it->second->to_string();
                ret += (++it == this->vertices.end() ? "" : ", ");
            }
            ret += ")";
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
//    
//    // Rollback changes to graph
//    g.vertices[to_remove.id] = (to_remove);
//    for (auto it : to_remove.outgoing) {
//        it.second.incoming[to_remove.id] = to_remove;
//    }
//    for (auto it : to_remove.incoming) {
//        it.second.outgoing[to_remove.id] = to_remove;
//    }

    return sorted;
}

Vertex* copy_subtree(Vertex* root) {
    Vertex* new_root = new Vertex(root->type, root->id);

    for (auto it : root->outgoing) {
        Vertex* child = it.second;
        Vertex* new_child = copy_subtree(child);
        new_root->outgoing[new_child->id] = new_child;
        new_child->incoming[new_root->id] = new_root;
    }

    return new_root;
}

int main(int argc, char** argv) {
    
    // Number of vertices
    int n;
    cin >> n;
    
    map<int, Vertex*> type_to_vertex;

    // Dataflow coefficient for vertices
    double **df_coef = new double*[n];
    for (int i = 0; i < n; i++) df_coef[i] = new double[n];
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
            df_coef[v1][v2] = coef;
            Vertex* vert2 = get_vert_of_type(v2, type_to_vertex);
            vert->incoming[vert2->id] = vert2;
            vert2->outgoing[vert->id] = vert;
        }
    }
    
    // First vertex has all outgoing edges, last all incoming
    map<long long, Vertex*> vertices;
    for (auto elem : type_to_vertex) {
        vertices[elem.second->id] = elem.second;
//        cout << (*elem.second) << endl;
//        cout << "  Out: ";
//        for (auto it : elem.second->outgoing) cout << (*it.second) << ", ";
//        cout << endl << "  In: ";
//        for (auto it : elem.second->incoming) cout << (*it.second) << ", ";
//        cout << endl;
    }
    
    Graph graph(1, df_coef, vertices);
    cout << graph << endl;
    Graph cpy = graph.copy();
    cout << "copied" << endl;
    cout << cpy << endl;
    vector<Vertex*> sorted = rev_top_sort(cpy);
    rev_top_sort(graph);
    
    cout << graph << endl;
    for (Vertex* v : sorted) cout << (*v) << ", ";
    cout << endl;

    return 0;
}
