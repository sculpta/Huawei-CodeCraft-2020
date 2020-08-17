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

namespace Color {
    inline void reset() { fprintf(stderr, "\033[0m"); }

    inline void red() { fprintf(stderr, "\033[1;31m"); }

    inline void green() { fprintf(stderr, "\033[1;32m"); }

    inline void yellow() { fprintf(stderr, "\033[1;33m"); }

    inline void blue() { fprintf(stderr, "\033[1;34m"); }

    inline void magenta() { fprintf(stderr, "\033[1;35m"); }

    inline void cyan() { fprintf(stderr, "\033[1;36m"); }

    inline void orange() { fprintf(stderr, "\033[38;5;214m"); }

    inline void newline() { fprintf(stderr, "\n"); }
}


struct ThreadResult {
    char **res;
    int *resByteNum;
    int *resNum;
};


const int THREAD_COUNT = 4;
const int CHAR_MAX_SIZE = 10;
const int RECORD_MAX = 2000000;
const int DOUBLE_RECORD_MAX = RECORD_MAX << 1;

int node_sum, input[DOUBLE_RECORD_MAX], nodes[DOUBLE_RECORD_MAX], input_weight[RECORD_MAX];
vector<vector<int>> graph, graphIn;
char (*nodeStr)[CHAR_MAX_SIZE];
int *nodeStrLen;
ThreadResult ans[THREAD_COUNT];

#ifdef TEST
char res13[30 * 200000];
char res14[40 * 200000];
char res15[50 * 300000];
char res16[60 * 400000];
char res17[70 * 500000];
int resByteNum1[5];
int resNum1[5];
char *ans1[5] = {res13, res14, res15, res16, res17};

char res23[30 * 200000];
char res24[40 * 200000];
char res25[50 * 300000];
char res26[60 * 400000];
char res27[70 * 500000];
int resByteNum2[5];
int resNum2[5];
char *ans2[5] = {res23, res24, res25, res26, res27};

char res33[30 * 200000];
char res34[40 * 200000];
char res35[50 * 300000];
char res36[60 * 400000];
char res37[70 * 500000];
int resByteNum3[5];
int resNum3[5];
char *ans3[5] = {res33, res34, res35, res36, res37};

char res43[30 * 200000];
char res44[40 * 200000];
char res45[50 * 300000];
char res46[60 * 400000];
char res47[70 * 500000];
int resByteNum4[5];
int resNum4[5];
char *ans4[5] = {res43, res44, res45, res46, res47};
#else
char res13[30 * 3000000];
char res14[40 * 3000000];
char res15[50 * 4000000];
char res16[60 * 5000000];
char res17[70 * 6000000];
int resByteNum1[5];
int resNum1[5];
char *ans1[5] = {res13, res14, res15, res16, res17};

char res23[30 * 3000000];
char res24[40 * 3000000];
char res25[50 * 4000000];
char res26[60 * 5000000];
char res27[70 * 6000000];
int resByteNum2[5];
int resNum2[5];
char *ans2[5] = {res23, res24, res25, res26, res27};

char res33[30 * 3000000];
char res34[40 * 3000000];
char res35[50 * 4000000];
char res36[60 * 5000000];
char res37[70 * 6000000];
int resByteNum3[5];
int resNum3[5];
char *ans3[5] = {res33, res34, res35, res36, res37};

char res43[30 * 3000000];
char res44[40 * 3000000];
char res45[50 * 4000000];
char res46[60 * 5000000];
char res47[70 * 6000000];
int resByteNum4[5];
int resNum4[5];
char *ans4[5] = {res43, res44, res45, res46, res47};
#endif


int toCharArray(int num, char *arr) {
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


void quickSort(vector<int> &vec, int left, int right) {
    if (left < right) {
        int l = left;
        int r = right;
        int key = vec[l], weight = vec[l + 1];

        while (l < r) {
            while (l < r && key <= vec[r])
                r -= 2;
            vec[l] = vec[r];
            vec[l + 1] = vec[r + 1];
            while (l < r && key >= vec[l])
                l += 2;
            vec[r] = vec[l];
            vec[r + 1] = vec[l + 1];
        }
        vec[l] = key;
        vec[l + 1] = weight;

        quickSort(vec, left, l - 2);
        quickSort(vec, r + 2, right);
    }
}


void createGraph(const char *buffer) {
    int input_sum = 0, record_sum = 0;
    int data[3] = {0, 0, 0};
    int c, flag = 0;

    for (int i = 0; buffer[i];) {
        c = buffer[i] - '0';
        if (buffer[i] == '\n') {
            if (data[2] > 0) {
                input[input_sum++] = data[0];
                input[input_sum++] = data[1];
                input_weight[record_sum++] = data[2];
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

    memcpy(nodes, input, input_sum * sizeof(int));
    sort(nodes, nodes + input_sum);
    node_sum = unique(nodes, nodes + input_sum) - nodes;

#ifdef TEST
    Color::green();
    fprintf(stderr, "node_sum: %d\n", node_sum);
    fprintf(stderr, "record_sum: %d\n", record_sum);
    Color::reset();
#endif

    unordered_map<int, int> idMapping;
    nodeStr = new char[node_sum][CHAR_MAX_SIZE];
    nodeStrLen = new int[node_sum];
    fill(nodeStrLen, nodeStrLen + node_sum, 0);
    for (int i = 0; i < node_sum; ++i) {
        int node = nodes[i];
        idMapping[node] = i;
        nodeStrLen[i] = toCharArray(node, nodeStr[i]);
    }

    graph = vector<vector<int>>(node_sum), graphIn = vector<vector<int>>(node_sum);

    for (int i = 0, j = 0; i < input_sum; i += 2, ++j) {
        int u = idMapping[input[i]];
        int v = idMapping[input[i + 1]];
        graph[u].push_back(v);
        graph[u].push_back(input_weight[j]);
        graphIn[v].push_back(u);
        graphIn[v].push_back(input_weight[j]);
    }

    for (int i = 0; i < node_sum; ++i) {
        quickSort(graph[i], 0, graph[i].size() - 2);
        quickSort(graphIn[i], 0, graphIn[i].size() - 2);
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
    Color::red();
    chrono::duration<double, std::milli> duration = chrono::system_clock::now() - start;
    fprintf(stderr, "create graph use: %.3fms\n", duration.count());
    Color::reset();
#endif

    munmap(buffer, file_size);
}


inline bool check(int x, int y) {
    return x <= 5ll * y && y <= 3ll * x;
}


void dfs_1(bool *visit, int *isValid, int head, int cur, int w, int depth) {
    visit[cur] = true;

    switch (depth) {
        case 1:
            for (int i = 0; i < (int) graph[cur].size(); i += 2) {
                int v = graph[cur][i], w1 = graph[cur][i + 1];
                if (v < head || visit[v]) continue;
                dfs_1(visit, isValid, head, v, w1, depth + 1);
            }
            break;
        case 2:
            for (int i = 0; i < (int) graph[cur].size(); i += 2) {
                int v = graph[cur][i], w1 = graph[cur][i + 1];
                if (v < head || visit[v] || !check(w, w1)) continue;
                isValid[cur] = head;
                dfs_1(visit, isValid, head, v, w1, depth + 1);
            }
            break;
        case 3:
            for (int i = 0; i < (int) graph[cur].size(); i += 2) {
                int v = graph[cur][i], w1 = graph[cur][i + 1];
                if (v < head || visit[v] || !check(w, w1)) continue;
                isValid[cur] = head;
                isValid[v] = head;
            }
            break;
    }

    visit[cur] = false;
}


void dfs_2(int loc, bool *visit, int *isValid, int *isTail,
           vector<unordered_map<int, vector<int>>> &pathDetail, int head, int cur, int w, int depth) {
    visit[cur] = true;

    switch (depth) {
        case 1:
            for (int i = 0; i < (int) graphIn[cur].size(); i += 2) {
                int u = graphIn[cur][i], w1 = graphIn[cur][i + 1];
                if (u < head || visit[u]) continue;
                dfs_2(loc, visit, isValid, isTail, pathDetail, head, u, w1, depth + 1);
            }
            break;
        case 2:
            for (int i = 0; i < (int) graphIn[cur].size(); i += 2) {
                int u = graphIn[cur][i], w1 = graphIn[cur][i + 1];
                if (u < head || visit[u] || !check(w1, w)) continue;
                isValid[cur] = head;
                isTail[u] = head;
                pathDetail[loc][u].push_back(cur);
                pathDetail[loc][u].push_back(w);
                pathDetail[loc][u].push_back(w1);
                dfs_2(loc, visit, isValid, isTail, pathDetail, head, u, w1, depth + 1);
            }
            break;
        case 3:
            for (int i = 0; i < (int) graphIn[cur].size(); i += 2) {
                int u = graphIn[cur][i], w1 = graphIn[cur][i + 1];
                if (u < head || visit[u] || !check(w1, w)) continue;
                isValid[cur] = head;
                isValid[u] = head;
            }
            break;
    }

    visit[cur] = false;
}


void dfs(int loc, bool *visit, int *isValid, int *isTail, vector<unordered_map<int, vector<int>>> &pathDetail,
         int head, int cur, int w0, int w, int depth, int *path, ThreadResult *tr) {
    visit[cur] = true;
    path[depth - 1] = cur;

    for (int i = 0; i < (int) graph[cur].size(); i += 2) {
        int v = graph[cur][i], w3 = graph[cur][i + 1];
        if ((depth > 1 && !check(w, w3)) || isValid[v] != head || visit[v]) continue;
        if (depth == 1) w0 = w3;

        if (isTail[v] == head) {
            for (int j = 0; j < (int) pathDetail[loc][v].size(); j += 3) {
                int u1 = pathDetail[loc][v][j], w1 = pathDetail[loc][v][j + 1], w2 = pathDetail[loc][v][j + 2];
                if (visit[u1]) continue;
                if (!check(w3, w2) || !check(w1, w0)) continue;

                path[depth] = v;
                path[depth + 1] = u1;
                for (int k = 0; k < depth + 2; ++k) {
                    memcpy((*tr).res[depth - 1] + (*tr).resByteNum[depth - 1], nodeStr[path[k]], nodeStrLen[path[k]]);
                    (*tr).resByteNum[depth - 1] += nodeStrLen[path[k]];
                    (*tr).res[depth - 1][(*tr).resByteNum[depth - 1]] = (k != depth + 1) ? ',' : '\n';
                    ++(*tr).resByteNum[depth - 1];
                }
                ++(*tr).resNum[depth - 1];
            }
        }
        if (depth < 5)
            dfs(loc, visit, isValid, isTail, pathDetail, head, v, w0, w3, depth + 1, path, tr);
    }

    visit[cur] = false;
}


void threadFunc(int tid, int start, int end, ThreadResult *tr) {
#ifdef TEST
    auto start_time = chrono::system_clock::now();
#endif

    bool *visit = new bool[node_sum];
    fill(visit, visit + node_sum, false);

    int *isValid = new int[node_sum];
    fill(isValid, isValid + node_sum, -1);
    int *isTail = new int[node_sum];
    fill(isTail, isTail + node_sum, -1);

    vector<unordered_map<int, vector<int>>> pathDetail =
            vector<unordered_map<int, vector<int>>>(end - start, unordered_map<int, vector<int>>());;

    int path[7];

    for (int i = start; i < end; ++i) {
        if (!graph[i].empty() && !graphIn[i].empty()) {
            dfs_1(visit, isValid, i, i, -1, 1);
            dfs_2(i - start, visit, isValid, isTail, pathDetail, i, i, -1, 1);
            dfs(i - start, visit, isValid, isTail, pathDetail, i, i, -1, -1, 1, path, tr);
        }
    }

#ifdef TEST
    Color::cyan();
    chrono::duration<double, milli> duration = chrono::system_clock::now() - start_time;
    fprintf(stderr, "thread %d use %.3fms\n", tid + 1, duration.count());
    Color::reset();
#endif
}


void work() {
#ifdef TEST
    auto start = chrono::system_clock::now();
#endif

    ans[0].res = ans1;
    ans[0].resByteNum = resByteNum1;
    ans[0].resNum = resNum1;

    ans[1].res = ans2;
    ans[1].resByteNum = resByteNum2;
    ans[1].resNum = resNum2;

    ans[2].res = ans3;
    ans[2].resByteNum = resByteNum3;
    ans[2].resNum = resNum3;

    ans[3].res = ans4;
    ans[3].resByteNum = resByteNum4;
    ans[3].resNum = resNum4;

    vector<thread> td(THREAD_COUNT);
    int arg1 = 0.055 * node_sum, arg2 = 0.13 * node_sum, arg3 = 0.25 * node_sum;
    int args[] = {0, arg1, arg2, arg3, node_sum - 2};

    for (int i = 0; i < THREAD_COUNT; ++i) {
        td[i] = thread(&threadFunc, i, args[i], args[i + 1], &ans[i]);
    }
    for (auto &t : td) {
        t.join();
    }

#ifdef TEST
    Color::red();
    chrono::duration<double, milli> duration = chrono::system_clock::now() - start;
    printf("dfs use:  %.3fms\n", duration.count());
    Color::reset();
#endif
}


void save(const string &file_name) {
#ifdef TEST
    int bytes = 0;
    auto start = chrono::system_clock::now();
#endif

    int circle_sum = 0;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        for (int j = 0; j < 5; ++j) {
            circle_sum += ans[i].resNum[j];
        }
    }

    FILE *fp = fopen(file_name.c_str(), "w");
    char sumStr[9];
    int length = toCharArray(circle_sum, sumStr);
    sumStr[length++] = '\n';
    fwrite(sumStr, length, 1, fp);

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < THREAD_COUNT; ++j) {
#ifdef TEST
            bytes += ans[j].resByteNum[i];
#endif

            fwrite(ans[j].res[i], ans[j].resByteNum[i], 1, fp);
        }
    }
    fclose(fp);

#ifdef TEST
    Color::red();
    chrono::duration<double, std::milli> duration = chrono::system_clock::now() - start;
    printf("write file use: %.3fms\n", duration.count());
    Color::green();
    printf("circle_sum: %d\n", circle_sum);
    printf("bytes: %d\n", bytes + length);
    Color::reset();
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
