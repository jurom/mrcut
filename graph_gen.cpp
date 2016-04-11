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

using namespace std;


class Vertex {
    public:
        int type;
        set<pair<Vertex*, double> > incoming;
        set<pair<Vertex*, double> > outgoing;
        Vertex(int t) : type(t) {}
        Vertex(){}
        string to_string() const {
            if (this->incoming.size() == 0) return "";
            string ret = std::to_string(type) + " " + std::to_string(this->incoming.size()) + "\n";
            for (auto it : this->incoming) {
                ret += std::to_string(it.first->type) + " " + std::to_string(it.second) + "\n";
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

double rand_df_coef(int max_factor, double prob_of_small) {
    double norm = rand01();
    double on_interval = norm + (((double)1)/max_factor)*(1 - norm);
    bool should_invert = rand01() > prob_of_small;
    return (should_invert ? 1/on_interval : on_interval);
}

string gen_rand_entwined(int height, int width, int sdata, int max_factor, double prob_of_small) {
    string ret = std::to_string((height + 1)*2);
    ret += " " + std::to_string(sdata) + "\n";
    for (int i = 0; i <= height; i++) {
        for (int current_vert = i*width+1; (current_vert <= (i+1)*width) && (current_vert <= height*width+1); current_vert++) {
            ret += std::to_string(current_vert) + " " + (i > 0 ? std::to_string(width) : "1") + "\n";
            for (int prev_vert = i*width; (prev_vert >= 0) && (prev_vert >= (i-1)*width+1); prev_vert--) {
                double df_coef = rand_df_coef(max_factor, prob_of_small);
                ret += std::to_string(prev_vert) + " " + std::to_string(df_coef) + "\n";
            }
        }
    }
    return ret;
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
void add_edge(Vertex* from, Vertex* to, int max_factor, double prob_of_small) {
    double df_coef = rand_df_coef(max_factor, prob_of_small);
    from->outgoing.insert(make_pair(to, df_coef));
    to->incoming.insert(make_pair(from, df_coef));
}

RegGraph gen_rand_dag(int num_vert, int sdata, double edge_prob, int max_factor, double prob_of_small) {
    map<int, Vertex*> type_to_vert;
    for (int i = 1; i <= num_vert; i++) {
        Vertex* v1 = get_vert_of_type(i, type_to_vert);
        if (v1->incoming.size() == 0) {
            Vertex* root = get_vert_of_type(0, type_to_vert);
            add_edge(root, v1, max_factor, prob_of_small);
        }
        for (int j = i+1; j <= num_vert; j++) {
            if (rand01() < edge_prob) {
                Vertex* v2 = get_vert_of_type(j, type_to_vert);
                add_edge(v1, v2, max_factor, prob_of_small);
            }
        }
    }
    set<Vertex*> vertices;
    Vertex* sink = get_vert_of_type(num_vert+1, type_to_vert);
    for (auto it : type_to_vert) {
        Vertex* v = it.second;
        if ((v->outgoing.size() == 0) && (v != sink)) {
            add_edge(v, sink, max_factor, prob_of_small);
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
        int height, width, sdata, max_factor;
        double prob_of_small;
        cin >> height >> width >> sdata >> max_factor >> prob_of_small;
        cout << gen_rand_entwined(height, width, sdata, max_factor, prob_of_small);
    } else if(type == "dag") {
        int num_vertices, sdata, max_factor;
        double edge_prob, prob_of_small;
        cin >> num_vertices >> sdata >> edge_prob >> max_factor >> prob_of_small;
        cout << gen_rand_dag(num_vertices, sdata, edge_prob, max_factor, prob_of_small).to_string();
    } else {
        cout << "Unknown type: " << type << endl;
    }
    
    return 0;
}

