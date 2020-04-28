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
const int NODE_MAX = 50000;
const int INDEGREE_MAX = 50;
const int OUTDEGREE_MAX = 50;


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
int graph[NODE_MAX][OUTDEGREE_MAX + 1];
int graphIn[NODE_MAX][INDEGREE_MAX + 1];
int nodes[NODE_MAX];
string idsComma[NODE_MAX];
string idsLF[NODE_MAX];
vector<unordered_map<int, vector<int>>> p1;
vector<unordered_map<int, vector<int>>> p2;
vector<Path> res;


void createGraph(const char *buffer) {
    int input_num = 0;
    int input[DOUBLE_RECORD_MAX], temp[DOUBLE_RECORD_MAX];
    int data[3] = {0, 0, 0};
    int i = 0, flag = 0;

    while (buffer[i]) {
        int c = buffer[i] - '0';
        if (buffer[i] == '\n') {
            int u = data[0], v = data[1];
            if (u < NODE_MAX && v < NODE_MAX) {
                input[input_num] = u;
                temp[input_num++] = u;
                input[input_num] = v;
                temp[input_num++] = v;
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

    sort(temp, temp + input_num);
    nodes[0] = temp[0];
    for (i = 1; i < input_num; ++i) {
        if (temp[i - 1] != temp[i])
            nodes[node_sum++] = temp[i];
    }

    for (i = 0; i < node_sum; ++i) {
        int node = nodes[i];
        graph[node][0] = 0;
        graphIn[node][0] = 0;
    }
    for (i = 0; i < input_num; i += 2) {
        int u = input[i];
        int v = input[i + 1];
        int outDegree = ++graph[u][0];
        graph[u][outDegree] = v;
        int inDegree = ++graphIn[v][0];
        graphIn[v][inDegree] = u;
    }

    for (i = 0; i < node_sum; ++i) {
        int node = nodes[i];
        sort(graph[node] + 1, graph[node] + (graph[node][0] + 1));
        idsComma[node] = to_string(node) + ',';
        idsLF[node] = to_string(node) + '\n';
    }
}


void readData(const string &file_name) {
    int fd = open(file_name.c_str(), O_RDONLY);
    long file_size = lseek(fd, 0, SEEK_END);
    char *buffer = (char *) mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    auto start = std::chrono::system_clock::now();
    createGraph(buffer);
    std::chrono::duration<double, std::milli> duration = std::chrono::system_clock::now() - start;
    std::cout << "createGraph used: " << duration.count() << "ms" << std::endl;

    munmap(buffer, file_size);
}


void dfs(bool *visit, int head, int cur, int depth, vector<int> &path) {
    visit[cur] = true;
    path.push_back(cur);

    int *it = lower_bound(graph[cur] + 1, graph[cur] + (graph[cur][0] + 1), head);
    if (it != graph[cur] + (graph[cur][0] + 1) && *it == head && depth >= 3 && depth <= 5) {
        res.emplace_back(Path(depth, path));
    }

    if (depth < 5) {
        for (; it != graph[cur] + (graph[cur][0] + 1); ++it) {
            if (!visit[*it]) {
                dfs(visit, head, *it, depth + 1, path);
            }
        }
    } else if (depth == 5) {
        auto it = p1[head].find(cur);
        if (it != p1[head].end()) {
            auto paths = (*it).second;
            int size = paths.size();
            for (int i = 0; i < size; ++i) {
                int u1 = paths[i];
                if (!visit[u1]) {
                    auto temp = path;
                    temp.push_back(u1);
                    res.emplace_back(Path(6, temp));
                }
            }
        }

        auto it1 = p2[head].find(cur);
        if (it1 != p2[head].end()) {
            auto paths = (*it1).second;
            int size = paths.size();
            for (int i = 0; i < size;) {
                int u2 = paths[i++];
                int u1 = paths[i++];
                if (!visit[u2] && !visit[u1]) {
                    auto temp = path;
                    temp.push_back(u2);
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
    p1 = vector<unordered_map<int, vector<int>>>(NODE_MAX, unordered_map<int, vector<int>>());
    p2 = vector<unordered_map<int, vector<int>>>(NODE_MAX, unordered_map<int, vector<int>>());

    auto start_time = std::chrono::system_clock::now();

    bool visit[NODE_MAX];
    fill(visit, visit + NODE_MAX, false);
    vector<int> path;

    for (int i = 0; i < node_sum - 2; ++i) {
        int head = nodes[i];
        if (graph[head][0] > 0) {
            for (int j = 1; j <= graphIn[head][0]; ++j) {
                int u1 = graphIn[head][j];
                if (u1 > head) {

                    for (int k = 1; k <= graphIn[u1][0]; ++k) {
                        int u2 = graphIn[u1][k];
                        if (u2 > head && u2 != u1) {
                            p1[head][u2].push_back(u1);

                            for (int m = 1; m <= graphIn[u2][0]; ++m) {
                                int u3 = graphIn[u2][m];
                                if (u3 > head && u3 != u1 && u3 != u2) {
                                    p2[head][u3].push_back(u2);
                                    p2[head][u3].push_back(u1);
                                }
                            }
                        }
                    }
                }
            }
            dfs(visit, head, head, 1, path);
        }
    }

    sort(res.begin(), res.end());

    std::chrono::duration<double, std::milli> duration = std::chrono::system_clock::now() - start_time;
    std::cout << "search used: " << duration.count() << "ms" << std::endl;
}


void save(const string &file_name) {
    auto start_save = std::chrono::system_clock::now();

    FILE *fd = fopen(file_name.c_str(), "wb");
    char buf[1024];

    int circles_num = res.size();
    int index = sprintf(buf, "%d\n", circles_num);
    buf[index] = '\0';
    fwrite(buf, index, sizeof(char), fd);

    for (auto item : res) {
        auto path = item.path;
        int size = path.size();
        for (int i = 0; i < size - 1; ++i) {
            auto nodeComma = idsComma[path[i]];
            fwrite(nodeComma.c_str(), nodeComma.size(), sizeof(char), fd);
        }
        auto nodeLF = idsLF[path[size - 1]];
        fwrite(nodeLF.c_str(), nodeLF.size(), sizeof(char), fd);
    }

    std::chrono::duration<double, std::milli> duration_save = std::chrono::system_clock::now() - start_save;
    std::cout << "fwrite used: " << duration_save.count() << "ms" << std::endl;
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
