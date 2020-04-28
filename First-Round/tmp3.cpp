#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <vector>


const int THREAD_COUNT = 4;
const int RECORD_MAX = 280000;
const int DOUBLE_RECORD_MAX = RECORD_MAX << 1;
const int NODE_MAX = 200000;
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
int pathDetail[THREAD_COUNT][NODE_MAX][700];
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
        sort(graphIn[node] + 1, graphIn[node] + (graphIn[node][0] + 1));
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


void dfs(int tid, bool *visit, int head, int cur, int depth, vector<int> &path) {
    visit[cur] = true;
    path.push_back(cur);

    auto it = lower_bound(graph[cur] + 1, graph[cur] + (graph[cur][0] + 1), head);
    if (it != graph[cur] + (graph[cur][0] + 1) && *it == head && depth >= 3 && depth <= 6) {
        res[tid].emplace_back(Path(depth, path));
    }

    if (depth < 6) {
        for (; it != graph[cur] + (graph[cur][0] + 1); ++it) {
            if (!visit[*it]) {
                dfs(tid, visit, head, *it, depth + 1, path);
            }
        }
    } else if (depth == 6) {
//        for (int i = 1; i <= pathDetail[tid][cur][0] << 1; i += 2) {
//            int u2 = pathDetail[tid][cur][i], u1 = pathDetail[tid][cur][i + 1];
//            if (!visit[u2] && !visit[u1]) {
//                auto temp = path;
//                temp.push_back(u2);
//                temp.push_back(u1);
//                res[tid].emplace_back(Path(7, temp));
//            }
//        }
        for (int j = 1; j <= pathDetail[tid][cur][601]; ++j) {
            int u1 = pathDetail[tid][cur][601 + j];
            if (!visit[u1]) {
                auto temp = path;
                temp.push_back(u1);
                res[tid].emplace_back(Path(7, temp));
            }
        }
    }

    visit[cur] = false;
    path.pop_back();
}


void threadFunc(int start) {
    bool visit[node_sum];
    fill(visit, visit + node_sum, false);
    vector<int> path;

    for (int i = start; i < node_sum - 2; i += THREAD_COUNT) {
        int head = nodes[i];
        if (graph[head][0] > 0) {
            for (int m = 0; m < node_sum; ++m) {
                pathDetail[start][nodes[m]][0] = 0;
                pathDetail[start][nodes[m]][601] = 0;
            }

            for (int j = 1; j <= graphIn[head][0]; ++j) {
                int u1 = graphIn[head][j];
                if (u1 > head) {

                    for (int k = 1; k <= graphIn[u1][0]; ++k) {
                        int u2 = graphIn[u1][k];
                        if (u2 > head && u2 != u1) {
                            int sum1 = ++pathDetail[start][u2][601];
                            pathDetail[start][u2][601 + sum1] = u1;

//                            for (int l = 1; l <= graphIn[u2][0]; ++l) {
//                                int u3 = graphIn[u2][l];
//                                if (u3 > head && u3 != u1 && u3 != u2) {
//                                    int sum2 = ++pathDetail[start][u3][0];
//                                    pathDetail[start][u3][2 * sum2 - 1] = u2;
//                                    pathDetail[start][u3][2 * sum2] = u1;
//                                }
//                            }
                        }
                    }
                }
            }

            dfs(start, visit, head, head, 1, path);
        }
    }
}


void work(const string &file_name) {
    vector<thread> td(THREAD_COUNT);
    for (int i = 0; i < THREAD_COUNT; ++i) {
        td[i] = thread(&threadFunc, i);
    }
    for (auto &t : td) {
        t.join();
    }

    for (int i = 1; i < THREAD_COUNT; ++i) {
        res[0].insert(res[0].end(), res[i].begin(), res[i].end());
    }

    sort(res[0].begin(), res[0].end());

    ofstream out(file_name);
    out << res[0].size() << '\n';
    for (auto item : res[0]) {
        auto path = item.path;
        out << path[0];
        for (int i = 1; i < item.length; ++i)
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