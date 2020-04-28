#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <vector>
#include <unordered_map>

#define THREAD_COUNT 4

using namespace std;


struct Path {
    int length;
    vector<int> path;

    Path(int length, const vector<int> &path) : length(length), path(path) {}

    bool operator<(const Path &rhs) const {
        if (length != rhs.length) return length < rhs.length;
        for (int i = 0; i < length; ++i) {
            if (path[i] != rhs.path[i])
                return path[i] < rhs.path[i];
        }
        return false;
    }
};


int node_sum;
int circle_sum;
vector<vector<int>> graph;
vector<int> nodes;
unordered_map<int, int> idMapping;
vector<Path> res[5];


void createGraph(const char *buffer) {
    vector<int> input;
    int data[3] = {0, 0, 0};
    int i = 0, flag = 0;

    while (buffer[i]) {
        int c = buffer[i] - '0';
        if (buffer[i] == '\n') {
            input.push_back(data[0]);
            input.push_back(data[1]);
            data[0] = data[1] = data[2] = flag = 0;
            ++i;
        } else if (c >= 0) {
            while (c >= 0) {
                data[flag] = data[flag] * 10 + c;
                c = buffer[++i] - '0';
            }
            ++flag;
        } else ++i;
    }

    nodes = input;
    sort(nodes.begin(), nodes.end());
    nodes.erase(unique(nodes.begin(), nodes.end()), nodes.end());

    node_sum = nodes.size();
    for (i = 0; i < node_sum; ++i) {
        idMapping[nodes[i]] = i;
    }

    graph = vector<vector<int>>(node_sum);
    int size = input.size();
    for (i = 0; i < size; i += 2) {
        int u = idMapping[input[i]];
        int v = idMapping[input[i + 1]];
        graph[u].push_back(v);
    }
}


void readData(const string &file_name) {
    int fd = open(file_name.c_str(), O_RDONLY);
    long file_size = lseek(fd, 0, SEEK_END);
    char *buffer = (char *) mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    createGraph(buffer);

    munmap(buffer, file_size);
}


void dfs(vector<bool> &visit, int head, int cur, int depth, vector<int> &path) {
    visit[cur] = true;
    path.push_back(nodes[cur]);

    for (int v : graph[cur]) {
        if (v == head && depth >= 3 && depth <= 7) {
            res[depth - 3].emplace_back(Path(depth, path));
            ++circle_sum;
        }
        if (depth < 7 && !visit[v] && v > head) {
            dfs(visit, head, v, depth + 1, path);
        }
    }

    visit[cur] = false;
    path.pop_back();
}


void threadFunc(int start, int end) {
    vector<bool> visit = vector<bool>(node_sum, false);
    vector<int> path;
    for (int i = start; i < end; ++i) {
        if (!graph[i].empty())
            dfs(visit, i, i, 1, path);
    }
}


void work(const string &file_name) {
    int i;
    int batch_size = node_sum / THREAD_COUNT;
    vector<thread> td(THREAD_COUNT);
    for (i = 0; i < THREAD_COUNT - 1; i++) {
        td[i] = thread(&threadFunc, i * batch_size, (i + 1) * batch_size);
    }
    td[THREAD_COUNT - 1] = thread(&threadFunc, i * batch_size, node_sum);
    for (auto &t : td) {
        t.join();
    }

    ofstream out(file_name);
    out << circle_sum << endl;
    for (auto item : res) {
        sort(item.begin(), item.end());
        for (auto circle : item) {
            auto path = circle.path;
            int size = path.size();
            out << path[0];
            for (i = 1; i < size; ++i)
                out << "," << path[i];
            out << '\n';
        }
    }
}


int main() {
    string file_name = "/data/test_data.txt";
    string save_file_name = "/projects/student/result.txt";

    readData(file_name);
    work(save_file_name);

    exit(0);
}
