#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <vector>
#include <unordered_map>

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


int nodeSum;
vector<vector<int>> graph;
vector<int> nodes;
unordered_map<int, int> idMapping;
vector<int> midNode;
vector<unordered_map<int, vector<int>>> midPath;
vector<bool> visit;
vector<Path> res;


void calPaths() {
    midPath = vector<unordered_map<int, vector<int>>>(nodeSum, unordered_map<int, vector<int>>());
    for (int i = 0; i < nodeSum; ++i) {
        for (int k : graph[i]) {
            for (int j : graph[k]) {
                if (j != i)
                    midPath[i][j].push_back(k);
            }
        }
        for (auto rec : midPath[i]) {
            if (rec.second.size() > 1) {
                sort(rec.second.begin(), rec.second.end());
            }
        }
    }
}


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

    nodeSum = nodes.size();
    for (i = 0; i < nodeSum; ++i) {
        idMapping[nodes[i]] = i;
    }

    graph = vector<vector<int>>(nodeSum);
    vector<int> inDegree(nodeSum, 0);
    int size = input.size();
    for (i = 0; i < size; i += 2) {
        int u = idMapping[input[i]];
        int v = idMapping[input[i + 1]];
        graph[u].push_back(v);
        ++inDegree[v];
    }

    queue<int> que;
    for (int i = 0; i < nodeSum; ++i) {
        if (0 == inDegree[i])
            que.push(i);
    }
    while (!que.empty()) {
        int u = que.front();
        que.pop();
        for (int v : graph[u]) {
            if (0 == --inDegree[v])
                que.push(v);
        }
    }
    for (int i = 0; i < nodeSum; ++i) {
        if (inDegree[i] == 0)
            graph[i].clear();
        else
            sort(graph[i].begin(), graph[i].end());
    }

    calPaths();
}


void readData(const string &file_name) {
    int fd = open(file_name.c_str(), O_RDONLY);
    int file_size = lseek(fd, 0, SEEK_END);
    char *buffer = (char *) mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    createGraph(buffer);

    munmap(buffer, file_size);
}


void dfs(int head, int cur, int depth, vector<int> &path) {
    visit[cur] = true;
    path.push_back(nodes[cur]);

    vector<int> vs = graph[cur];
    auto it = lower_bound(vs.begin(), vs.end(), head);
    if (it != vs.end() && *it == head && depth >= 3 && depth < 7) {
        res.emplace_back(Path(depth, path));
    }
    if (depth < 6) {
        for (; it != vs.end(); ++it) {
            if (!visit[*it]) {
                dfs(head, *it, depth + 1, path);
            }
        }
    } else if (midNode[cur] > -1 && depth == 7) {
        vector<int> ks = midPath[cur][head];
        int sz = ks.size();
        for (int i = midNode[cur]; i < sz; ++i) {
            int k = ks[i];
            if (visit[k]) continue;
            auto tmp = path;
            tmp.push_back(k);
            res.emplace_back(Path(depth + 1, tmp));
        }
    }

    visit[cur] = false;
    path.pop_back();
}


void work(const string &file_name) {
    midNode = vector<int>(nodeSum, -1);
    visit = vector<bool>(nodeSum, false);
    vector<int> path;

    for (int i = 0; i < nodeSum; ++i) {
        if (!graph[i].empty()) {
            fill(midNode.begin() + i + 1, midNode.end(), -1);
            for (int j = i + 1; j < nodeSum; ++j) {
                auto it = midPath[j].find(i);
                if (it != midPath[j].end()) {
                    vector<int> ks = it->second;
                    int size = ks.size();
                    for (int k = 0; k < size; k++) {
                        if (ks[k] > i) {
                            midNode[j] = k;
                            break;
                        }
                    }
                }
            }
            dfs(i, i, 1, path);
        }
    }
    sort(res.begin(), res.end());

    ofstream out(file_name);
    out << res.size() << endl;
    for (auto item : res) {
        auto path = item.path;
        int size = path.size();
        out << path[0];
        for (int i = 1; i < size; ++i)
            out << "," << path[i];
        out << '\n';
    }
}


int main(int argc, char *argv[]) {
    string file = argc > 1 ? argv[1] : "test";
    string file_name = "datasets/" + file + "/test_data.txt";
    string save_file_name = "result.txt";

    readData(file_name);
    work(save_file_name);

    exit(0);
}

