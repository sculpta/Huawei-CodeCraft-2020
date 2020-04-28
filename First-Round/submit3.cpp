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
vector<vector<int>> graph;
vector<int> nodes;
unordered_map<int, int> idMapping;
vector<Path> res[THREAD_COUNT];


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


void saveRes(int tid, int depth, vector<int> &path) {
    res[tid].emplace_back(Path(depth, path));
}


void threadFunc(int start) {
    vector<bool> visit(node_sum, false);
    vector<int> path;
    for (int a = start; a < node_sum - 2; a += THREAD_COUNT) {
        visit[a] = true;
        path.push_back(nodes[a]);

        for (int b : graph[a]) {
            if (!visit[b] && b > a) {
                visit[b] = true;
                path.push_back(nodes[b]);

                for (int c : graph[b]) {
                    if (!visit[c] && c > a) {
                        visit[c] = true;
                        path.push_back(nodes[c]);

                        for (int d : graph[c]) {
                            if (d == a) {
                                saveRes(start, 3, path);
                                continue;
                            }
                            if (!visit[d] && d > a) {
                                visit[d] = true;
                                path.push_back(nodes[d]);

                                for (int e : graph[d]) {
                                    if (e == a) {
                                        saveRes(start, 4, path);
                                        continue;
                                    }
                                    if (!visit[e] && e > a) {
                                        visit[e] = true;
                                        path.push_back(nodes[e]);

                                        for (int f : graph[e]) {
                                            if (f == a) {
                                                saveRes(start, 5, path);
                                                continue;
                                            }
                                            if (!visit[f] && f > a) {
                                                visit[f] = true;
                                                path.push_back(nodes[f]);

                                                for (int g : graph[f]) {
                                                    if (g == a) {
                                                        saveRes(start, 6, path);
                                                        continue;
                                                    }
                                                    if (!visit[g] && g > a) {
                                                        visit[g] = true;
                                                        path.push_back(nodes[g]);

                                                        for (int h : graph[g]) {
                                                            if (h == a) {
                                                                saveRes(start, 7, path);
                                                                continue;
                                                            }
                                                        }

                                                        visit[g] = false;
                                                        path.pop_back();
                                                    }
                                                }

                                                visit[f] = false;
                                                path.pop_back();
                                            }
                                        }

                                        visit[e] = false;
                                        path.pop_back();
                                    }
                                }

                                visit[d] = false;
                                path.pop_back();
                            }
                        }

                        visit[c] = false;
                        path.pop_back();
                    }
                }

                visit[b] = false;
                path.pop_back();
            }
        }

        visit[a] = false;
        path.pop_back();
    }
}


void work(const string &file_name) {
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

    cout << "circles: " << res[0].size() << endl;

    ofstream out(file_name);
    out << res[0].size() << endl;
    for (auto item : res[0]) {
        auto path = item.path;
        int size = path.size();
        out << path[0];
        for (i = 1; i < size; i++)
            out << "," << path[i];
        out << '\n';
    }
}


int main(int argc, char *argv[]) {
    string file = argc > 1 ? argv[1] : "test";
    string file_name = "datasets/" + file + "/test_data.txt";
    string save_file_name = "result.txt";

    readData(file_name);
    cout << "nodes: " << node_sum << endl;
    work(save_file_name);

    exit(0);
}
