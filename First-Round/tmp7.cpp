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


int node_sum = 1;
int graph[NODE_MAX][OUTDEGREE_MAX + 1];
int graphIn[NODE_MAX][INDEGREE_MAX + 1];
int nodes[NODE_MAX];
vector<unordered_map<int, vector<int>>> p1;
vector<unordered_map<int, vector<int>>> p2;
vector<string> idsComma;
vector<string> idsLF;
int res3[3 * 1000000 + 1] = {0};
int res4[4 * 1000000 + 1] = {0};
int res5[5 * 1000000 + 1] = {0};
int res6[6 * 2000000 + 1]= {0};
int res7[7 * 2000000 + 1]= {0};
int res[] = {*res3, *res4, *res5, *res6, *res7};


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

    idsComma.reserve(node_sum);
    idsLF.reserve(node_sum);
    for (i = 0; i < node_sum; ++i) {
        int node = nodes[i];
        sort(graph[node] + 1, graph[node] + (graph[node][0] + 1));
        idsComma.push_back(to_string(node) + ',');
        idsLF.push_back(to_string(node) + '\n');
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


void dfs(int tid, bool *visit, int head, int cur, int depth, vector<int> &path) {
    visit[cur] = true;
    path.push_back(cur);

    int *it = lower_bound(graph[cur] + 1, graph[cur] + (graph[cur][0] + 1), head);
    if (it != graph[cur] + (graph[cur][0] + 1) && *it == head && depth >= 3 && depth <= 5) {
        res[tid].emplace_back(Path(depth, path));
    }

    if (depth < 5) {
        for (; it != graph[cur] + (graph[cur][0] + 1); ++it) {
            if (!visit[*it]) {
                dfs(tid, visit, head, *it, depth + 1, path);
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
                    res[tid].emplace_back(Path(6, temp));
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
                    res[tid].emplace_back(Path(7, temp));
                }
            }
        }
    }

    visit[cur] = false;
    path.pop_back();
}


void threadFunc(int start) {
    auto start_time = std::chrono::system_clock::now();

    bool visit[NODE_MAX];
    fill(visit, visit + NODE_MAX, false);
    vector<int> path;

    for (int i = start; i < node_sum - 2; i += 1) {
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
            dfs(start, visit, head, head, 1, path);
        }
    }

    std::chrono::duration<double, std::milli> duration = std::chrono::system_clock::now() - start_time;
    std::cout << "thread " << start << " used: " << duration.count() << "ms" << std::endl;
}


void work(const string &file_name) {
    p1 = vector<unordered_map<int, vector<int>>>(NODE_MAX, unordered_map<int, vector<int>>());
    p2 = vector<unordered_map<int, vector<int>>>(NODE_MAX, unordered_map<int, vector<int>>());

    threadFunc(0);
//    vector<thread> td(THREAD_COUNT);
//    for (int i = 0; i < THREAD_COUNT; ++i) {
//        td[i] = thread(&threadFunc, i);
//    }
//    for (auto &t : td) {
//        t.join();
//    }
//
//    for (int i = 1; i < THREAD_COUNT; ++i) {
//        res[0].insert(res[0].end(), res[i].begin(), res[i].end());
//    }

    sort(res[0].begin(), res[0].end());

    auto start_save = std::chrono::system_clock::now();

    FILE *fd = fopen(file_name.c_str(), "wb");
    char buf[1024];

    int circles_num = res[0].size();
    int index = sprintf(buf, "%d\n", circles_num);
    buf[index] = '\0';
    fwrite(buf, index, sizeof(char), fd);

    for (auto item : res[0]) {
        auto path = item.path;
        int size = path.size();
        for (int i = 0; i < size - 1; ++i) {
            auto res = idsComma[path[i]];
            fwrite(res.c_str(), res.size(), sizeof(char), fd);
        }
        auto res = idsLF[path[size - 1]];
        fwrite(res.c_str(), res.size(), sizeof(char), fd);
    }

    std::chrono::duration<double, std::milli> duration_save = std::chrono::system_clock::now() - start_save;
    std::cout << "fwrite used: " << duration_save.count() << "ms" << std::endl;
}


int main(int argc, char *argv[]) {
    string file = argc > 1 ? argv[1] : "test";
    string file_name = "datasets/" + file + "/test_data.txt";
    string save_file_name = "result.txt";

    readData(file_name);
    work(save_file_name);

    exit(0);
}
