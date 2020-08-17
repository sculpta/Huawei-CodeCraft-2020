#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <vector>
#include <unordered_map>


#define TEST

using namespace std;

typedef unsigned long long ull;

const int RECORD_MAX = 2000000;
const int DOUBLE_RECORD_MAX = RECORD_MAX << 1;


int node_sum, circle_sum;
vector<vector<int>> graph, graphIn;
unordered_map<ull, int> weight;
vector<int> nodes, isValid, isTail;
vector<unordered_map<int, vector<int>>> pathDetail;

#ifdef TEST
char res3[30 * 500000];
char res4[40 * 500000];
char res5[50 * 1000000];
char res6[60 * 2000000];
char res7[70 * 3000000];
#else
char res3[30 * 3000000];
char res4[40 * 3000000];
char res5[50 * 4000000];
char res6[60 * 7000000];
char res7[70 * 10000000];
#endif

char *res[] = {res3, res4, res5, res6, res7};
int resByteNum[5] = {0};


void createGraph(const char *buffer) {
    int input_sum = 0;
    vector<int> input(DOUBLE_RECORD_MAX), input_weight(RECORD_MAX);
    int data[3] = {0, 0, 0};
    int flag = 0;

    for (int i = 0; buffer[i];) {
        int c = buffer[i] - '0';
        if (buffer[i] == '\n') {
            input[input_sum << 1] = data[0];
            input[input_sum << 1 | 1] = data[1];
            input_weight[input_sum++] = data[2];
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

    vector<int> temp = input;
    sort(temp.begin(), temp.begin() + (input_sum << 1));
    temp.erase(unique(temp.begin(), temp.begin() + (input_sum << 1)), temp.end());
    node_sum = temp.size();
    nodes = temp;

#ifdef TEST
    cout << "node_sum: " << node_sum << endl;
    cout << "input_sum: " << input_sum << endl;
#endif

    unordered_map<int, int> idMapping;
    for (int i = 0; i < node_sum; ++i) {
        int node = temp[i];
        idMapping[node] = i;
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

#ifdef TEST
    auto start = chrono::system_clock::now();
#endif

    createGraph(buffer);

#ifdef TEST
    chrono::duration<double, std::milli> duration = chrono::system_clock::now() - start;
    cout << "create graph use: " << duration.count() << "ms" << endl;
#endif

    munmap(buffer, file_size);
}


inline bool check(int x, int y) {
    if (x == -1 || y == -1) return true;
    return (x > 0 && y > 0) ? (x <= 5ll * y && y <= 3ll * x) : false;
}


void dfs_1(bool *visit, int head, int cur, int w, int depth) {
    visit[cur] = true;

    auto it = lower_bound(graph[cur].begin(), graph[cur].end(), head);
    for (; it != graph[cur].end(); ++it) {
        int w1 = weight[(ull) cur << 32 | *it];
        if (visit[*it] || !check(w, w1)) continue;
        isValid[*it] = head;
        if (depth >= 3) continue;
        dfs_1(visit, head, *it, w1, depth + 1);
    }
    visit[cur] = false;
}

void dfs_2(bool *visit, int head, int cur, int w, int depth) {
    visit[cur] = true;

    auto it = lower_bound(graphIn[cur].begin(), graphIn[cur].end(), head);
    for (; it != graphIn[cur].end(); ++it) {
        int w1 = weight[(ull) *it << 32 | cur];
        if (visit[*it] || !check(w1, w)) continue;
        isValid[*it] = head;
        if (depth == 2) {
            isTail[*it] = head;
            pathDetail[head][*it].push_back(cur);
        }
        if (depth >= 3) continue;
        dfs_2(visit, head, *it, w1, depth + 1);
    }
    visit[cur] = false;
}


void dfs(bool *visit, int head, int cur, int w, int depth, int *path) {
    visit[cur] = true;
    path[depth - 1] = cur;

    auto it = lower_bound(graph[cur].begin(), graph[cur].end(), head);
    for (; it != graph[cur].end(); ++it) {
        int w1 = weight[(ull) cur << 32 | *it];
        if (isValid[*it] != head || visit[*it] || !check(w, w1)) continue;
        if (isTail[*it] == head) {
            for (int u1 : pathDetail[head][*it]) {
                if (visit[u1]) continue;
                if (!check(w1, weight[(ull) *it << 32 | u1])) continue;
                int w2 = (depth == 1) ? w1 : weight[(ull) head << 32 | path[1]];
                if (!check(weight[(ull) u1 << 32 | head], w2)) continue;
                if (depth == 5 && !check(w, w1)) continue;

                path[depth] = *it;
                path[depth + 1] = u1;
                for (int j = 0; j < depth + 1; ++j) {
                    resByteNum[depth - 1] += sprintf(res[depth - 1] + resByteNum[depth - 1], "%d", nodes[path[j]]);
                    res[depth - 1][resByteNum[depth - 1]++] = ',';
                }
                resByteNum[depth - 1] += sprintf(res[depth - 1] + resByteNum[depth - 1], "%d", nodes[path[depth + 1]]);
                res[depth - 1][resByteNum[depth - 1]++] = '\n';
                ++circle_sum;
            }
        }
        if (depth < 4 || (depth == 4 && check(w, w1)))
            dfs(visit, head, *it, w1, depth + 1, path);
    }

    visit[cur] = false;
}


void work() {
#ifdef TEST
    auto start = chrono::system_clock::now();
#endif

    isValid = vector<int>(node_sum, -1), isTail = vector<int>(node_sum, -1);
    pathDetail = vector<unordered_map<int, vector<int>>>(node_sum, unordered_map<int, vector<int>>());

    bool visit[node_sum];
    fill(visit, visit + node_sum, false);
    int path[16];

    for (int i = 0; i < node_sum - 2; ++i) {
        if (!graph[i].empty() && !graphIn[i].empty()) {
            dfs_1(visit, i, i, -1, 1);
            dfs_2(visit, i, i, -1, 1);
            dfs(visit, i, i, -1, 1, path);
        }
    }

#ifdef TEST
    chrono::duration<double, milli> duration = chrono::system_clock::now() - start;
    cout << "dfs use: " << duration.count() << "ms" << endl;
#endif
}


void save(const string &file_name) {
#ifdef TEST
    int bytes = 0;
    auto start = chrono::system_clock::now();
#endif

    FILE *fp = fopen(file_name.c_str(), "w");
    const char *t = (to_string(circle_sum) + "\n").c_str();
    fwrite(t, strlen(t), 1, fp);
    for (int i = 3; i <= 7; ++i) {
#ifdef TEST
        bytes += resByteNum[i - 3];
#endif
        fwrite(res[i - 3], resByteNum[i - 3], 1, fp);
    }
    fclose(fp);

#ifdef TEST
    chrono::duration<double, std::milli> duration = chrono::system_clock::now() - start;
    cout << "write file use: " << duration.count() << "ms" << endl;
    cout << "circle_sum: " << circle_sum << endl;
    cout << "bytes: " << bytes + strlen(t) << endl;
#endif
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
