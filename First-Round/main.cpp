#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <vector>
#include <unordered_map>


const int RECORD_MAX = 280000;
const int DOUBLE_RECORD_MAX = RECORD_MAX << 1;
const int NODE_MAX = 50000;
const int INDEGREE_MAX = 50;
const int OUTDEGREE_MAX = 50;


using namespace std;


int node_sum = 0, circle_sum = 0;
int graph[NODE_MAX][OUTDEGREE_MAX + 1];
int graphIn[NODE_MAX][INDEGREE_MAX + 1];
int nodes[NODE_MAX];
string idComma[NODE_MAX];
string idLF[NODE_MAX];
vector<unordered_map<int, vector<int>>> p1;
int res3[3 * 1000000 + 1] = {0};
int res4[4 * 1000000 + 1] = {0};
int res5[5 * 1000000 + 1] = {0};
int res6[6 * 2000000 + 1] = {0};
int res7[7 * 3000000 + 1] = {0};
int *res[] = {res3, res4, res5, res6, res7};


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
    nodes[node_sum++] = temp[0];
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
        idComma[node] = to_string(node) + ',';
        idLF[node] = to_string(node) + '\n';
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


void insert(int depth, int *path) {
    int sum = res[depth - 3][0];
    int loc = sum * depth;
    for (int i = 0; i < depth; ++i) {
        res[depth - 3][++loc] = path[i];
    }
    ++res[depth - 3][0];
    ++circle_sum;
}


void dfs(bool *visit, int head, int cur, int depth, int *path) {
    visit[cur] = true;
    path[depth - 1] = cur;

    int *it = lower_bound(graph[cur] + 1, graph[cur] + (graph[cur][0] + 1), head);
    if (it != graph[cur] + (graph[cur][0] + 1) && *it == head && depth >= 3 && depth <= 6) {
        insert(depth, path);
    }

    if (depth < 6) {
        for (; it != graph[cur] + (graph[cur][0] + 1); ++it) {
            if (!visit[*it]) {
                dfs(visit, head, *it, depth + 1, path);
            }
        }
    } else if (depth == 6) {
        auto it = p1[head].find(cur);
        if (it != p1[head].end()) {
            auto paths = (*it).second;
            int size = paths.size();
            for (int i = 0; i < size; ++i) {
                int u1 = paths[i];
                if (!visit[u1]) {
                    path[depth] = u1;
                    insert(depth + 1, path);
                }
            }
        }
    }

    visit[cur] = false;
}


void work() {
    p1 = vector<unordered_map<int, vector<int>>>(NODE_MAX, unordered_map<int, vector<int>>());

    bool visit[NODE_MAX];
    fill(visit, visit + NODE_MAX, false);
    int path[7];

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
                        }
                    }
                }
            }
            dfs(visit, head, head, 1, path);
        }
    }
}


void save(const string &file_name) {
    FILE *fd = fopen(file_name.c_str(), "wb");
    char buf[1024];

    int index = sprintf(buf, "%d\n", circle_sum);
    buf[index] = '\0';
    fwrite(buf, index, sizeof(char), fd);

    for (int i = 3; i <= 7; ++i) {
        for (int j = 0; j < res[i - 3][0]; ++j) {
            for (int k = 0; k < i - 1; ++k) {
                int node = res[i - 3][j * i + k + 1];
                auto nodeComma = idComma[node];
                fwrite(nodeComma.c_str(), nodeComma.size(), sizeof(char), fd);
            }
            int node = res[i - 3][(j + 1) * i];
            auto nodeLF = idLF[node];
            fwrite(nodeLF.c_str(), nodeLF.size(), sizeof(char), fd);
        }
    }
    fclose(fd);
}


int main() {
    string file_name = "/data/test_data.txt";
    string save_file_name = "/projects/student/result.txt";

    readData(file_name);
    work();
    save(save_file_name);

    exit(0);
}
