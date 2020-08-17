//  结构体中存储权重，head无法判定比例

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


struct Weight {
    int node;
    int w;

    Weight(int node, int w) : node(node), w(w) {}

    bool operator<(const Weight &item) {
        return node < item.node;
    }
};


int node_sum, circle_sum;
vector<vector<Weight>> graph, graphIn;
char *nodeStr;
int *nodeStrLen, *isValid, *isTail;
vector<unordered_map<int, vector<int>>> pathDetail;

#ifdef TEST
char res3[30 * 5000000];
char res4[40 * 5000000];
char res5[50 * 8000000];
char res6[60 * 10000000];
char res7[70 * 15000000];
#else
char res3[30 * 8000000];
char res4[40 * 8000000];
char res5[50 * 10000000];
char res6[60 * 15000000];
char res7[70 * 18000000];
#endif

char *res[5] = {res3, res4, res5, res6, res7};
int resByteNum[5];

#ifdef TEST
int resNum[5];
#endif


int toString(int num, char *arr) {
    if (num == 0) {
        arr[0] = '0';
        return 1;
    }

    int i = 0, j = 0, length = 0;
    char temp[10];

    while (num) {
        temp[i++] = num % 10 + '0';
        num /= 10;
    }
    length = i--;
    while (i >= 0) {
        arr[j++] = temp[i--];
    }
    return length;
}


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

#ifdef TEST
    cout << "node_sum: " << node_sum << endl;
    cout << "input_sum: " << input_sum << endl;
#endif

    unordered_map<int, int> idMapping;
    nodeStr = new char[10 * node_sum];
    nodeStrLen = new int[node_sum];
    fill(nodeStrLen, nodeStrLen + node_sum, 0);
    for (int i = 0; i < node_sum; ++i) {
        int node = temp[i];
        idMapping[node] = i;
        nodeStrLen[i] = toString(node, nodeStr + (i * 10));
    }

    graph = vector<vector<Weight>>(node_sum), graphIn = vector<vector<int>>(node_sum);

    for (int i = 0; i < input_sum; ++i) {
        int u = idMapping[input[i << 1]];
        int v = idMapping[input[i << 1 | 1]];
        graph[u].emplace_back(Weight(v, input_weight[i]));
        graphIn[v].emplace_back(Weight(u, input_weight[i]));
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
    return (x == -1 || y == -1) ? true : ((x > 0 && y > 0) ? (x <= 5ll * y && y <= 3ll * x) : false);
}


void dfs_1(bool *visit, int head, int father, int cur, int w, int depth) {
    visit[cur] = true;

    switch (depth) {
        case 1:
            for (auto item : graph[cur]) {
                int v = item.node;
                if (visit[v]) continue;
                dfs_1(visit, head, cur, v, item.w, depth + 1);
            }
            break;
        case 2:
            for (auto item : graph[cur]) {
                int v = item.node, w1 = item.w;
                if (visit[v] || !check(w, w1)) continue;
                dfs_1(visit, head, cur, v, w1, depth + 1);
            }
            break;
        case 3:
            for (auto item : graph[cur]) {
                int v = item.node;
                if (visit[v] || !check(w, item.w)) continue;
                isValid[father] = head;
                isValid[cur] = head;
                isValid[v] = head;
            }
            break;
    }

    visit[cur] = false;
}


void dfs_2(bool *visit, int head, int cur, int w, int depth) {
    visit[cur] = true;

    for (auto item : graph[cur]) {
        int u = item.node, w1 = item.w;
        if (u < head || visit[u] || !check(w1, w)) continue;
        isValid[u] = head;
        if (depth == 2) {
            isTail[u] = head;
            pathDetail[head][u].push_back(cur);
        }
        if (depth >= 3) continue;
        dfs_2(visit, head, u, w1, depth + 1);
    }

    visit[cur] = false;
}


void dfs(bool *visit, int head, int cur, int w, int depth, int *path) {
    visit[cur] = true;
    path[depth - 1] = cur;

    for (auto item : graph[cur]) {
        int v = item.node, w1 = item.w;
        if (isValid[v] != head || visit[v] || !check(w, w1)) continue;
        if (isTail[v] == head) {
            for (int u1 : pathDetail[head][v]) {
                if (visit[u1]) continue;
                int w2 = (depth == 1) ? w1 : weight[(ull) head << 32 | path[1]];
                if (!check(weight[(ull) u1 << 32 | head], w2)) continue;
                if (depth == 5 && !check(w, w1)) continue;

                path[depth] = v;
                path[depth + 1] = u1;
                for (int j = 0; j < depth + 2; ++j) {
                    memcpy(res[depth - 1] + resByteNum[depth - 1], nodeStr + (path[j] * 10), nodeStrLen[path[j]]);
                    resByteNum[depth - 1] += nodeStrLen[path[j]];
                    res[depth - 1][resByteNum[depth - 1]] = (j != depth + 1) ? ',' : '\n';
                    ++resByteNum[depth - 1];
                }
                ++circle_sum;
#ifdef TEST
                ++resNum[depth - 1];
#endif
            }
        }
        if (depth < 4 || (depth == 4 && check(w, w1)))
            dfs(visit, head, v, w1, depth + 1, path);
    }

    visit[cur] = false;
}


void work() {
#ifdef TEST
    auto start = chrono::system_clock::now();
#endif

    isValid = new int[node_sum], isTail = new int[node_sum];
    fill(isValid, isValid + node_sum, -1);
    fill(isTail, isTail + node_sum, -1);
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

//    delete[]nodeStr;
//    delete[]nodeStrLen;
//    delete[]isValid;
//    delete[]isTail;

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
    for (int i = 0; i < 5; ++i) {
#ifdef TEST
        cout << (i + 3) << " circle: " << resNum[i] << ", bytes: " << resByteNum[i] << endl;
        bytes += resByteNum[i];
#endif
        fwrite(res[i], resByteNum[i], 1, fp);
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
#ifdef TEST
    string file = argc > 1 ? argv[1] : "test";
    string file_name = "datasets/" + file + "/test_data.txt";
    string save_file_name = "result.txt";
#else
    string file_name = "/data/test_data.txt";
    string save_file_name = "/projects/student/result.txt";
#endif

    readData(file_name);
    work();
    save(save_file_name);

    exit(0);
}