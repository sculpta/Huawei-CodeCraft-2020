#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <vector>
#include <unordered_map>


const int THREAD_COUNT = 4;
const int RECORD_MAX = 280000;
const int DOUBLE_RECORD_MAX = RECORD_MAX << 1;
const int NODE_MAX = 250000;
const int DEGREE_MAX = 50;


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


int node_sum = 1;
int graph[NODE_MAX][DEGREE_MAX + 1];
int nodes[NODE_MAX];
unordered_map<int, int> idMapping;
vector<unordered_map<int, vector<int>>> pathDetail;
vector<Path> res[THREAD_COUNT];


void createGraph(const char *buffer) {
    int input_num = 0;
    int input[DOUBLE_RECORD_MAX], temp[DOUBLE_RECORD_MAX];
    int data[3] = {0, 0, 0};
    int i = 0, flag = 0;

    while (buffer[i]) {
        int c = buffer[i] - '0';
        if (buffer[i] == '\n') {
            input[input_num] = data[0];
            temp[input_num++] = data[0];
            input[input_num] = data[1];
            temp[input_num++] = data[1];
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

    sort(temp, temp + input_num);
    nodes[0] = temp[0];
    for (i = 1; i < input_num; ++i) {
        if (temp[i - 1] != temp[i])
            nodes[node_sum++] = temp[i];
    }

    for (i = 0; i < node_sum; ++i) {
        idMapping[nodes[i]] = i;
    }

    for (i = 0; i < node_sum; ++i) {
        graph[i][0] = 0;
    }
    for (i = 0; i < input_num; i += 2) {
        int u = idMapping[input[i]];
        int v = idMapping[input[i + 1]];
        int degree = ++graph[u][0];
        graph[u][degree] = v;
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


void createPathDetail() {
    pathDetail = vector<unordered_map<int, vector<int>>>(node_sum, unordered_map<int, vector<int>>());
    for (int i = 0; i < node_sum; ++i) {
        if (graph[i][0] > 1)
            sort(graph[i] + 1, graph[i] + graph[i][0] + 1);

        for (int k = 1; k <= graph[i][0]; ++k) {
            int nodek = graph[i][k];
            for (int j = 1; j <= graph[nodek][0]; ++j) {
                int nodej = graph[nodek][j];
                if (nodej != i)
                    pathDetail[i][nodej].push_back(nodek);
            }
        }
        for (auto path : pathDetail[i]) {
            if (path.second.size() > 1) {
                sort(path.second.begin(), path.second.end());
            }
        }
    }
}


void dfs(int tid, bool *visit, int *reach, int head, int cur, int depth, vector<int> &path) {
    visit[cur] = true;
    path.push_back(nodes[cur]);

    auto it = lower_bound(graph[cur] + 1, graph[cur] + graph[cur][0] + 1, head);
    if (it != graph[cur] + graph[cur][0] + 1 && *it == head && depth >= 3 && depth < 7) {
        res[tid].emplace_back(Path(depth, path));
    }

    if (depth < 6) {
        for (; it != graph[cur] + graph[cur][0] + 1; ++it) {
            if (!visit[*it]) {
                dfs(tid, visit, reach, head, *it, depth + 1, path);
            }
        }
    } else if (reach[cur] > -1 && depth == 6) {
        auto ks = pathDetail[cur][head];
        int size = ks.size();
        for (int i = reach[cur]; i < size; ++i) {
            int k = ks[i];
            if (visit[k]) continue;
            auto tmp = path;
            tmp.push_back(k);
            res[tid].emplace_back(Path(7, tmp));
        }
    }

    visit[cur] = false;
    path.pop_back();
}


void threadFunc(int start) {
    bool visit[node_sum];
    fill(visit, visit + node_sum, false);

    int reach[node_sum];
    vector<int> path;

    for (int i = start; i < node_sum - 2; i += THREAD_COUNT) {
        if (graph[i][0] > 0) {
            fill(reach + i + 1, reach + node_sum, -1);
            for (int j = i + 1; j < node_sum; ++j) {
                auto it = pathDetail[j].find(i);
                if (it != pathDetail[j].end()) {
                    auto ks = (*it).second;
                    int size = ks.size();
                    for (int k = 0; k < size; ++k) {
                        if (ks[k] > i) {
                            reach[j] = k;
                            break;
                        }
                    }
                }
            }

            dfs(start, visit, reach, i, i, 1, path);
        }
    }
}


void work(const string &file_name) {
    createPathDetail();

    int i;
    vector<thread> td(THREAD_COUNT);
    for (i = 0; i < THREAD_COUNT; ++i) {
        td[i] = thread(&threadFunc, i);
    }
    for (auto &t : td) {
        t.join();
    }

    for (i = 1; i < THREAD_COUNT; ++i) {
        res[0].insert(res[0].end(), res[i].begin(), res[i].end());
    }

    sort(res[0].begin(), res[0].end());

    ofstream out(file_name);
    out << res[0].size() << endl;
    for (auto item : res[0]) {
        auto path = item.path;
        out << path[0];
        for (i = 1; i < item.length; ++i)
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
