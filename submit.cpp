#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <vector>
#include <map>

using namespace std;

struct ArcNode {
    int receiver;
    int num;
    struct ArcNode *nextArc;
};

struct Path {
    int length;
    vector<int> path;

    Path(int length, const vector<int> &path) : length(length), path(path) {}

    bool operator<(const Path &rhs) const {
        if (length != rhs.length) return length < rhs.length;
        for (int i = 0; i < length; i++) {
            if (path[i] != rhs.path[i])
                return path[i] < rhs.path[i];
        }
        return false;
    }
};

map<int, ArcNode *> graph;
vector<bool> visit;
vector<Path> res;

void createGraph(const char *buffer) {
    int data[3] = {0, 0, 0};
    int i = 0, flag = 0;

    while (buffer[i]) {
        int c = buffer[i] - '0';
        if (buffer[i] == '\n') {
            ArcNode *node = new ArcNode{data[1], data[2], NULL};
            if (ArcNode *p = graph[data[0]]) {
                //  排序
                if (node->receiver < p->receiver) {
                    graph[data[0]] = node;
                    node->nextArc = p;
                } else {
                    while (p && p->nextArc && node->receiver > p->nextArc->receiver) p = p->nextArc;
                    node->nextArc = p->nextArc;
                    p->nextArc = node;
                }
            } else {
                graph[data[0]] = node;
            }
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
}

void readData(const string &file_name) {
    struct stat sb;
    int fd = open(file_name.c_str(), O_RDONLY);
    fstat(fd, &sb);
    char *buffer = (char *) mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    createGraph(buffer);

    munmap(buffer, sb.st_size);
}

void dfs(int head, int cur, int depth, vector<int> &path) {
    if (graph.end() != graph.find(cur)) {
        visit[cur] = true;
        path.push_back(cur);

        ArcNode *p = graph[cur];
        while (p) {
            if (p->receiver == head && depth >= 3 && depth <= 7) {
                res.emplace_back(depth, path);
            }
            if (depth < 7 && !visit[p->receiver] && p->receiver > head) {
                dfs(head, p->receiver, depth + 1, path);
            }
            p = p->nextArc;
        }
        visit[cur] = false;
        path.pop_back();
    }
}

void work() {
    visit = vector<bool>(graph.size(), false);
    vector<int> path;

    for (auto item : graph) {
        dfs(item.first, item.first, 1, path);
    }
    sort(res.begin(), res.end());
}

void save(const string &file_name) {
    ofstream out(file_name);
    out << res.size() << endl;
    for (auto &x:res) {
        auto path = x.path;
        int size = path.size();
        out << path[0];
        for (int i = 1; i < size; i++)
            out << "," << path[i];
        out << '\n';
    }
}

int main() {
    string file_name = "/data/test_data.txt";
    string save_file_name = "/projects/student/result.txt";

    readData(file_name);
    work();
    save(save_file_name);

    exit(0);
}
