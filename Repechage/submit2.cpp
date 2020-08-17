#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <vector>
#include <unordered_map>


typedef unsigned long long ull;

using namespace std;


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


int node_sum;
vector<vector<int>> graph, graphIn;
unordered_map<ull, int> weight;
unordered_map<int, int> idMapping;
vector<string> idComma, idLF;
vector<unordered_map<int, vector<int>>> pathDetail;
vector<Path> res;


void createGraph(const char *buffer) {
    vector<int> input, input_weight;
    int data[3] = {0, 0, 0};
    int flag = 0;

    for (int i = 0; buffer[i];) {
        int c = buffer[i] - '0';
        if (buffer[i] == '\n') {
            input.push_back(data[0]);
            input.push_back(data[1]);
            input_weight.push_back(data[2]);
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

    int input_sum = input_weight.size();
    vector<int> temp = input;
    sort(temp.begin(), temp.end());
    temp.erase(unique(temp.begin(), temp.end()), temp.end());
    node_sum = temp.size();

    idComma = vector<string>(node_sum), idLF = vector<string>(node_sum);
    for (int i = 0; i < node_sum; ++i) {
        int node = temp[i];
        idMapping[node] = i;
        idComma[i] = to_string(node) + ',';
        idLF[i] = to_string(node) + '\n';
    }

    graph = vector<vector<int>>(node_sum), graphIn = vector<vector<int>>(node_sum);

    for (int i = 0; i < input_sum; ++i) {
        int u = idMapping[input[i << 1]];
        int v = idMapping[input[i << 1 | 1]];
        graph[u].push_back(v);
        graphIn[v].push_back(u);
        weight[(ull) u << 32 | v] = input_weight[i];
    }

    for (int i = 0; i < node_sum; ++i) {
        sort(graph[i].begin(), graph[i].end());
        sort(graphIn[i].begin(), graphIn[i].end());
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


inline bool check(int x, int y) {
    if (x == 0 || y == 0) return false;
    return x <= 5ll * y && y <= 3ll * x;
}


void createPathDetail() {
    pathDetail = vector<unordered_map<int, vector<int>>>(node_sum, unordered_map<int, vector<int>>());

    for (int i = 0; i < node_sum; ++i) {
        for (int j = 0; j < (int) graphIn[i].size(); ++j) {
            int u1 = graphIn[i][j], w1 = weight[(ull) u1 << 32 | i];
            if (u1 > i) {
                for (int k = 0; k < (int) graphIn[u1].size(); ++k) {
                    int u2 = graphIn[u1][k], w2 = weight[(ull) u2 << 32 | u1];
                    if (u2 > i && u2 != u1 && check(w2, w1)) {
                        pathDetail[i][u2].push_back(u1);
                    }
                }
            }
        }
    }
}


void dfs(bool *visit, int head, int cur, int w, int depth, vector<int> &path) {
    visit[cur] = true;
    path.push_back(cur);

    auto it = lower_bound(graph[cur].begin(), graph[cur].end(), head);
    if (it != graph[cur].end() && *it == head && depth >= 3 && depth < 7 && check(w, weight[(ull) cur << 32 | *it]) &&
        check(weight[(ull) cur << 32 | *it], weight[(ull) head << 32 | path[1]])) {
        res.emplace_back(Path(depth, path));
    }

    if (depth == 1) {
        for (; it != graph[cur].end(); ++it) {
            if (!visit[*it]) {
                dfs(visit, head, *it, weight[(ull) cur << 32 | *it], depth + 1, path);
            }
        }
    } else if (depth < 6) {
        for (; it != graph[cur].end(); ++it) {
            if (!visit[*it] && check(w, weight[(ull) cur << 32 | *it])) {
                dfs(visit, head, *it, weight[(ull) cur << 32 | *it], depth + 1, path);
            }
        }
    } else if (depth == 6) {
        auto it = pathDetail[head].find(cur);
        if (it != pathDetail[head].end()) {
            auto detail = (*it).second;

            for (int i = 0; i < (int) detail.size(); ++i) {
                int u1 = detail[i];
                if (!visit[u1] && check(w, weight[(ull) cur << 32 | u1]) &&
                    check(weight[(ull) u1 << 32 | head], weight[(ull) head << 32 | path[1]])) {
                    auto temp = path;
                    temp.push_back(u1);
                    res.emplace_back(Path(7, temp));
                }
            }
        }
    }

    visit[cur] = false;
    path.pop_back();
}


void work() {
    createPathDetail();

    bool visit[node_sum];
    fill(visit, visit + node_sum, false);
    vector<int> path;

    for (int i = 0; i < node_sum - 2; ++i) {
        if (!graph[i].empty() && !graphIn[i].empty()) {
            dfs(visit, i, i, -1, 1, path);
        }
    }

    sort(res.begin(), res.end());
}


void save(const string &file_name) {
    FILE *fd = fopen(file_name.c_str(), "wb");
    char buf[1024];

    int circle_sum = res.size();
    int index = sprintf(buf, "%d\n", circle_sum);
    buf[index] = '\0';
    fwrite(buf, index, sizeof(char), fd);

    for (auto item : res) {
        auto path = item.path;
        int size = path.size();
        for (int i = 0; i < size - 1; ++i) {
            auto nodeComma = idComma[path[i]];
            fwrite(nodeComma.c_str(), nodeComma.size(), sizeof(char), fd);
        }
        auto nodeLF = idLF[path[size - 1]];
        fwrite(nodeLF.c_str(), nodeLF.size(), sizeof(char), fd);
    }

}


int main(int argc, char *argv[]) {
    string file = argc > 1 ? argv[1] : "test";
    string file_name = "datasets/" + file + "/test_data.txt";
    string save_file_name = "result.txt";

    readData(file_name);
    work();
    save(save_file_name);

    exit(0);
}
