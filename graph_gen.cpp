/* 
 * File:   graph_gen.cpp
 * Author: juro
 *
 * Created on April 11, 2016, 9:19 AM
 */

#include <cstdlib>
#include <string>
#include <time.h>
#include <iostream>
#include <set>
#include <map>
#include <vector>

using namespace std;


class Vertex {
    public:
        int type;
        string vtype;
        set<pair<Vertex*, double> > incoming;
        set<pair<Vertex*, double> > outgoing;
        Vertex(int t, string vt) : type(t), vtype(vt) {} 
        Vertex(int t) : type(t) {
            this->vtype = "P";
        }
        Vertex(){}
        string to_string() const {
            string ret = vtype + " " + std::to_string(type) + " " + std::to_string(this->incoming.size()) + "\n";
            for (auto it : this->incoming) {
                ret += it.first->vtype + " " + std::to_string(it.first->type) + " " + std::to_string(it.second) + "\n";
            }
            return ret;
        }
};

class RegGraph {
    public:
        double source_data;
        set<Vertex*> vertices;
        RegGraph(int sdata, set<Vertex*> vert) : source_data(sdata), vertices(vert) {}
        string to_string() const {
            string ret = std::to_string(this->vertices.size()) + " " + std::to_string(this->source_data) + "\n";
            for (Vertex* v : this->vertices) {
                ret += v->to_string();
            }
            return ret;
        }
};

double rand01() {
    return ((double)rand())/RAND_MAX;
}

double rand_df_coef(double low, double high, double prob_of_small) {
    double norm = rand01();
    double on_interval = norm*(high - low) + low;
    bool should_invert = rand01() > prob_of_small;
    return (should_invert ? 1/on_interval : on_interval);
}

RegGraph gen_rand_entwined(int height, int width, int sdata, double low, double high, double prob_of_small) {
    Vertex* input = new Vertex(0, "I");
    Vertex* source = new Vertex(1, "G");
    vector<Vertex*> vertices;
    input->outgoing.insert(make_pair(source, 1));
    source->incoming.insert(make_pair(input, 1));
    vertices.push_back(input);
    vertices.push_back(source);
    for (int i = 0; i <= height; i++) {
        for (int current_vert = i*width+2; (current_vert < (i+1)*width+2) && (current_vert <= height*width+2); current_vert++) {
            Vertex* vertex = new Vertex(current_vert, current_vert == height*width+2 ? "G" : "P");
            vertices.push_back(vertex);
            for (int prev_vert = i*width+1; (prev_vert >= 1) && (prev_vert > (i-1)*width+1); prev_vert--) {
                double df_coef = rand_df_coef(low, high, prob_of_small);
                vertex->incoming.insert(make_pair(vertices[prev_vert], df_coef));
                vertices[prev_vert]->outgoing.insert(make_pair(vertex, df_coef));
            }
        }
    }
    Vertex* output = new Vertex(height*width+3, "O");
    output->incoming.insert(make_pair(vertices[height*width+2], 1));
    vertices[height*width+2]->outgoing.insert(make_pair(output, 1));
    vertices.push_back(output);
    set<Vertex*> vertices_set;
    for (Vertex* v : vertices) vertices_set.insert(v);
    RegGraph rg(sdata, vertices_set);
    return rg;
}

Vertex* get_vert_of_type(int type, map<int, Vertex*> &type_to_vert) {
    if (type_to_vert.find(type) == type_to_vert.end()) {
        Vertex* v = new Vertex(type);
        type_to_vert[type] = v;
        return v;
    } else {
        return type_to_vert[type];
    }
}
void add_edge(Vertex* from, Vertex* to, double low, double high, double prob_of_small) {
    double df_coef = rand_df_coef(low, high, prob_of_small);
    from->outgoing.insert(make_pair(to, df_coef));
    to->incoming.insert(make_pair(from, df_coef));
}

RegGraph gen_rand_dag(int num_vert, int num_gbks, int sdata, double edge_prob, double low, double high, double prob_of_small) {
    vector<Vertex*> type_to_vert;
    Vertex* input = new Vertex(0, "I");
    type_to_vert.push_back(input);
    int step = num_vert/num_gbks;
    for (int i = 1; i <= num_vert; i++) {
        Vertex* vert = NULL;
        if ((i + step/2)%step == 0) {
            vert = new Vertex(i, "G");
        } else {
            vert = new Vertex(i, "P");
        }
        type_to_vert.push_back(vert);
    }
    Vertex* output = new Vertex(num_vert+1, "O");
    type_to_vert.push_back(output);
    for (int i = 1; i <= num_vert; i++) {
        Vertex* v1 = type_to_vert[i];
        if (v1->incoming.size() == 0) {
            add_edge(input, v1, low, high, prob_of_small);
        }
        for (int j = i+1; j <= num_vert; j++) {
            if (rand01() < edge_prob) {
                Vertex* v2 = type_to_vert[j];
                add_edge(v1, v2, low, high, prob_of_small);
            }
        }
    }
    set<Vertex*> vertices;
    for (Vertex* v : type_to_vert) {
        if ((v->outgoing.size() == 0) && (v != output)) {
            add_edge(v, output, low, high, prob_of_small);
        }
        vertices.insert(v);
    }
    RegGraph rg(sdata, vertices);
    return rg;
}

int main(int argc, char** argv) {
    srand(time(NULL));
    
    string type;
    
    cin >> type;
    
    if (type == "entwined") {
        int height, width, sdata;
        double prob_of_small, low, high;
        cin >> height >> width >> sdata >> low >> high >> prob_of_small;
        cout << gen_rand_entwined(height, width, sdata, low, high, prob_of_small).to_string();
    } else if(type == "dag") {
        int num_vertices, sdata, num_gbk;
        double edge_prob, prob_of_small, low, high;
        cin >> num_vertices >> num_gbk >> sdata >> edge_prob >> low >> high >> prob_of_small;
        cout << gen_rand_dag(num_vertices, num_gbk, sdata, edge_prob, low, high, prob_of_small).to_string();
    } else {
        cout << "Unknown type: " << type << endl;
    }
    
    return 0;
}

