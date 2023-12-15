// copyright 2023 Aowei Zhao aowei8@bu.edu
// copyright 2023 Haolin Ye 

#include <algorithm>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Province_data{
    string state;
    string county;
    double latitude;
    double longitude;
};

struct Node{
    Province_data point;
    Node* left;
    Node* right;
};

struct CompareDistance{
    bool operator()(const pair<double, Province_data>& p1, const pair<double, Province_data>& p2) {
        return p1.first < p2.first;
    }
};

Node* newNode(Province_data point){
    Node* temp = new Node;
    temp->point = point;
    temp->left = temp->right = nullptr;
    return temp;
};


vector<Province_data> load_data(const string& filename){
    ifstream file(filename);
    vector<Province_data> Data_list;
    string buffer;
    while (getline(file, buffer)){
        Province_data data;
        istringstream iss(buffer);
        vector<string> word_list;
        string token;
        while (iss >> token){
            word_list.push_back(token);
        }
        unsigned int len = word_list.size();
        data.state = word_list[0];
        data.latitude = stod(word_list[len-2]);
        data.longitude = stod(word_list[len-1]);
        for (int i=1; i<len-2; i++){
            if (!data.county.empty()){
                data.county += " ";
            }
            data.county += word_list[i];
        }
        Data_list.push_back(data);
    }
    return Data_list;
};

double distance_calculator(Province_data d1, Province_data d2){
    const double R = 6371; 

    double x = (d2.longitude - d1.longitude) * cos((d1.latitude + d2.latitude) / 2);
    double y = d2.latitude - d1.latitude;
    double d = sqrt(x*x + y*y) * R;
    return d; 
};

bool comparePoints(const Province_data& a, const Province_data& b, int level){
    if (level % 2 == 0) 
        return a.latitude < b.latitude;
    return a.longitude < b.longitude;
}

Node* buildKdTree(vector<Province_data>& points, int start, int end, int level){
    if (start > end)
        return nullptr;

    int mid = (start + end) / 2;
    sort(points.begin() + start, points.begin() + end + 1,
              [level](const Province_data& a, const Province_data& b) {
                  return comparePoints(a, b, level);
              });

    Node* root = newNode(points[mid]);
    root->left = buildKdTree(points, start, mid - 1, level + 1);
    root->right = buildKdTree(points, mid + 1, end, level + 1);
    return root;
};

//This is the function to find k nearest with KD-Tree
vector<Province_data> kNearestNeighbors(Node* root, const Province_data& query, int k){
    vector<Province_data> k_nearest_neighbors;
    if (root == nullptr)
        return k_nearest_neighbors;

    priority_queue<pair<double, Province_data>, vector<pair<double, Province_data>>, CompareDistance> minHeap;

    function<void(Node*)> search = [&](Node* current){
        if (current == nullptr)
            return;

        double distance = distance_calculator(current->point, query);
        minHeap.push(make_pair(distance, current->point));
        if (minHeap.size() > k)
            minHeap.pop();
        bool isLeft = (query.latitude < current->point.latitude);
        if ((isLeft && current->left) || (!isLeft && current->right)) {
            if (isLeft)
                search(current->left);
            else
                search(current->right);
        }
        double currentAxisDistance = (isLeft) ? abs(query.latitude - current->point.latitude) : abs(query.longitude - current->point.longitude);
        if (minHeap.size() < k || currentAxisDistance < minHeap.top().first) {
            if (isLeft)
                search(current->right);
            else
                search(current->left);
        }
    };

    search(root);
    while (!minHeap.empty()) {
        k_nearest_neighbors.push_back(minHeap.top().second);
        minHeap.pop();
    }
    reverse(k_nearest_neighbors.begin(), k_nearest_neighbors.end());
    return k_nearest_neighbors;
};

//This is the function to find k nearest with linear scanning
vector<Province_data> liner_nearest_neighbor(const vector<Province_data> &Data, Province_data target, int k){
    auto cmp = [target](const Province_data& a, const Province_data& b) {
        auto distanceToA = distance_calculator(target, a);
        auto distanceToB = distance_calculator(target, b);
        return distanceToA < distanceToB;
    };
    priority_queue<Province_data, vector<Province_data>, decltype(cmp)> min_heap(cmp);
    for (auto data : Data){
        if (min_heap.size() < k){
            min_heap.push(data);
        }
        else{
            double dist = distance_calculator(target, data);
            if (dist < distance_calculator(target, min_heap.top())){
                min_heap.pop();
                min_heap.push(data);
            }
        }
    }
    vector<Province_data> k_nearest_neighbors;
    while(!min_heap.empty()){
        k_nearest_neighbors.push_back(min_heap.top());
        min_heap.pop();
    }
    reverse(k_nearest_neighbors.begin(), k_nearest_neighbors.end());
    return k_nearest_neighbors;
};




int main(){
    string filename = "./data.txt";
    vector<Province_data> points = load_data(filename);
    std::cout << "K=1" << std::endl;
    for (int m=0; m<3; m++){
        double a, b;
        cout << "Please enter the latitude and longitude: ";
        cin >> a >> b;
        Province_data queryPoint = {"x","y",a, b};
        int k = 1; // Number of nearest neighbors to find
        clock_t s, e;

        auto c_start = std::chrono::high_resolution_clock::now();
        Node* root = buildKdTree(points, 0, points.size() - 1, 0);
        auto c_end = std::chrono::high_resolution_clock::now();

        auto start = std::chrono::high_resolution_clock::now();
        vector<Province_data> nearestPoints = kNearestNeighbors(root, queryPoint, k);
        auto end = std::chrono::high_resolution_clock::now();

        // Calculate the elapsed time
        std::chrono::duration<double> c_duration = c_end - c_start;
        std::chrono::duration<double> duration = end - start;

        // Print the elapsed time in seconds with high precision
        std::cout << "K-D Tree: " << std::endl;
        std::cout << "Construction time: " << c_duration.count()/10 << " seconds" << std::endl;
        std::cout << "Elapsed time: " << duration.count()/10 << " seconds" << std::endl;
        int i =0;
        for (const auto& point : nearestPoints) {
            cout << "(" << point.state << ", " << point.county << ")" << "  ";
            i++;
//            if (i == 5){
//                cout << endl;
//            }
        }
        cout << endl;



        auto sta = std::chrono::high_resolution_clock::now();
        vector<Province_data> nearestP2 = liner_nearest_neighbor(points, queryPoint, k);
        auto en = std::chrono::high_resolution_clock::now();

        // Calculate the elapsed time
        std::chrono::duration<double> duration2 = en - sta;
        std::cout << "Brute Force: " << std::endl;

        // Print the elapsed time in seconds with high precision
        std::cout << "Elapsed time: " << duration2.count() << " seconds" << std::endl;

        i = 0;
        for (const auto& point : nearestP2) {
            cout << "(" << point.state << ", " << point.county << ")" << "  ";
            i++;
//            if (i == 5){
//                cout << endl;
//            }
        }
        cout << endl;
    }
    return 0;
}