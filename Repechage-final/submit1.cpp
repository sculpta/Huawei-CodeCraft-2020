#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <vector>
#include <unordered_map>


//#define TEST

using namespace std;


struct ThreadResult {
    char **res;
    int (*circleByteNum)[6];
    int *resByteNum;
    int *resNum;
};


const int THREAD_COUNT = 4;
const int CHAR_MAX_SIZE = 10;
const int RECORD_MAX = 2000000;
const int DOUBLE_RECORD_MAX = RECORD_MAX << 1;

int node_sum, circle_sum;
int input[DOUBLE_RECORD_MAX], nodes[DOUBLE_RECORD_MAX];
double input_weight[RECORD_MAX];
vector<vector<int>> graph, graphIn;
char (*nodeStr)[CHAR_MAX_SIZE];
int *nodeStrLen;
ThreadResult threadRes[THREAD_COUNT];

#ifdef TEST
char res13[30 * 2000000];
char res14[40 * 2000000];
char res15[50 * 3000000];
char res16[60 * 4000000];
char res17[70 * 5000000];
char res18[80 * 5000000];
int resByteNum1[6];
int resNum1[6];
char *res1[6] = {res13, res14, res15, res16, res17, res18};

char res23[30 * 2000000];
char res24[40 * 2000000];
char res25[50 * 3000000];
char res26[60 * 4000000];
char res27[70 * 5000000];
char res28[80 * 5000000];
int resByteNum2[6];
int resNum2[6];
char *res2[6] = {res23, res24, res25, res26, res27, res28};

char res33[30 * 2000000];
char res34[40 * 2000000];
char res35[50 * 3000000];
char res36[60 * 4000000];
char res37[70 * 5000000];
char res38[80 * 5000000];
int resByteNum3[6];
int resNum3[6];
char *res3[6] = {res33, res34, res35, res36, res37, res38};

char res43[30 * 2000000];
char res44[40 * 2000000];
char res45[50 * 3000000];
char res46[60 * 4000000];
char res47[70 * 5000000];
char res48[80 * 5000000];
int resByteNum4[6];
int resNum4[6];
char *res4[6] = {res43, res44, res45, res46, res47, res48};

char ans3[30 * 5000000];
char ans4[40 * 5000000];
char ans5[50 * 8000000];
char ans6[60 * 10000000];
char ans7[70 * 15000000];
char ans8[80 * 15000000];
char *ans[6] = {ans3, ans4, ans5, ans6, ans7, ans8};
int ansByteNum[6];
#else
char res13[30 * 3000000];
char res14[40 * 3000000];
char res15[50 * 4000000];
char res16[60 * 5000000];
char res17[70 * 6000000];
char res18[80 * 6000000];
int resByteNum1[6];
int resNum1[6];
char *res1[6] = {res13, res14, res15, res16, res17, res18};

char res23[30 * 3000000];
char res24[40 * 3000000];
char res25[50 * 4000000];
char res26[60 * 5000000];
char res27[70 * 6000000];
char res28[80 * 6000000];
int resByteNum2[6];
int resNum2[6];
char *res2[6] = {res23, res24, res25, res26, res27, res28};

char res33[30 * 3000000];
char res34[40 * 3000000];
char res35[50 * 4000000];
char res36[60 * 5000000];
char res37[70 * 6000000];
char res38[80 * 6000000];
int resByteNum3[6];
int resNum3[6];
char *res3[6] = {res33, res34, res35, res36, res37, res38};

char res43[30 * 3000000];
char res44[40 * 3000000];
char res45[50 * 4000000];
char res46[60 * 5000000];
char res47[70 * 6000000];
char res48[80 * 6000000];
int resByteNum4[6];
int resNum4[6];
char *res4[6] = {res43, res44, res45, res46, res47, res48};

char ans3[30 * 8000000];
char ans4[40 * 8000000];
char ans5[50 * 10000000];
char ans6[60 * 15000000];
char ans7[70 * 18000000];
char ans8[80 * 18000000];
char *ans[6] = {ans3, ans4, ans5, ans6, ans7, ans8};
int ansByteNum[6];
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


void createGraph(const string &file_name) {
    FILE *fp = fopen(file_name.c_str(), "r");
    int input_sum = 0, record_sum = 0;
    int u, v;
    double c;

    while (fscanf(fp, "%d,%d,%lf", &u, &v, &c) != EOF) {
        input[input_sum++] = u;
        input[input_sum++] = v;
        input_weight[record_sum++] = c;
    }

    memcpy(nodes, input, input_sum * sizeof(int));
    sort(nodes, nodes + input_sum);
    node_sum = unique(nodes, nodes + input_sum) - nodes;

#ifdef TEST
    printf("node_sum: %d\n", node_sum);
    printf("record_sum: %d\n", record_sum);
#endif

    unordered_map<int, int> idMapping;
    nodeStr = new char[node_sum][CHAR_MAX_SIZE];
    nodeStrLen = new int[node_sum];
    memset(nodeStrLen, 0, node_sum * sizeof(int));
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
        graph[u].push_back(j);
        graphIn[v].push_back(u);
        graphIn[v].push_back(j);
    }

    for (int i = 0; i < node_sum; ++i) {
        quickSort(graph[i], 0, graph[i].size() - 2);
        quickSort(graphIn[i], 0, graphIn[i].size() - 2);
    }
}


inline bool check(double x, double y) {
    return x <= double(5) * y && y <= double(3) * x;
}


void dfs_1(bool *visit, int *isValid, int head, int cur, int w, int depth) {
    visit[cur] = true;

    switch (depth) {
        case 1:
            for (int i = 0; i < (int) graph[cur].size(); i += 2) {
                int v = graph[cur][i];
                if (v < head || visit[v]) continue;
                dfs_1(visit, isValid, head, v, graph[cur][i + 1], depth + 1);
            }
            break;
        case 2:
            for (int i = 0; i < (int) graph[cur].size(); i += 2) {
                int v = graph[cur][i];
                double w1 = input_weight[graph[cur][i + 1]], w2 = input_weight[w];
                if (v < head || visit[v] || !check(w2, w1)) continue;
                isValid[cur] = head;
                dfs_1(visit, isValid, head, v, graph[cur][i + 1], depth + 1);
            }
            break;
        case 3:
            for (int i = 0; i < (int) graph[cur].size(); i += 2) {
                int v = graph[cur][i];
                double w1 = input_weight[graph[cur][i + 1]], w2 = input_weight[w];
                if (v < head || visit[v] || !check(w2, w1)) continue;
                isValid[cur] = head;
                dfs_1(visit, isValid, head, v, graph[cur][i + 1], depth + 1);
            }
            break;
        case 4:
            for (int i = 0; i < (int) graph[cur].size(); i += 2) {
                int v = graph[cur][i];
                double w1 = input_weight[graph[cur][i + 1]], w2 = input_weight[w];
                if (v < head || visit[v] || !check(w2, w1)) continue;
                isValid[cur] = head;
                isValid[v] = head;
            }
            break;

    }

    visit[cur] = false;
}


void
dfs_2(int loc, bool *visit, int *isValid, int *isTail, vector<unordered_map<int, vector<int>>> &pathDetail, int head,
      int cur, int w, int depth) {

    visit[cur] = true;

    switch (depth) {
        case 1:
            for (int i = 0; i < (int) graphIn[cur].size(); i += 2) {
                int u = graphIn[cur][i];
                if (u < head || visit[u]) continue;
                dfs_2(loc, visit, isValid, isTail, pathDetail, head, u, graphIn[cur][i + 1], depth + 1);
            }
            break;
        case 2:
            for (int i = 0; i < (int) graphIn[cur].size(); i += 2) {
                int u = graphIn[cur][i];
                double w1 = input_weight[graphIn[cur][i + 1]], w2 = input_weight[w];
                if (u < head || visit[u] || !check(w1, w2)) continue;
                isValid[cur] = head;
                isTail[u] = head;
                pathDetail[loc][u].push_back(cur);
                pathDetail[loc][u].push_back(w);
                pathDetail[loc][u].push_back(graphIn[cur][i + 1]);
                dfs_2(loc, visit, isValid, isTail, pathDetail, head, u, graphIn[cur][i + 1], depth + 1);
            }
            break;
        case 3:
            for (int i = 0; i < (int) graphIn[cur].size(); i += 2) {
                int u = graphIn[cur][i];
                double w1 = input_weight[graphIn[cur][i + 1]], w2 = input_weight[w];
                if (u < head || visit[u] || !check(w1, w2)) continue;
                isValid[cur] = head;
                dfs_2(loc, visit, isValid, isTail, pathDetail, head, u, graphIn[cur][i + 1], depth + 1);
            }
            break;
        case 4:
            for (int i = 0; i < (int) graphIn[cur].size(); i += 2) {
                int u = graphIn[cur][i];
                double w1 = input_weight[graphIn[cur][i + 1]], w2 = input_weight[w];
                if (u < head || visit[u] || !check(w1, w2)) continue;
                isValid[cur] = head;
                isValid[u] = head;
            }
            break;
    }

    visit[cur] = false;
}


void dfs(int loc, bool *visit, int *isValid, int *isTail, vector<unordered_map<int, vector<int>>> &pathDetail, int head,
         int cur, int w0, int w, int depth, int *path, ThreadResult *tr) {
    visit[cur] = true;
    path[depth - 1] = cur;

    for (int i = 0; i < (int) graph[cur].size(); i += 2) {
        int v = graph[cur][i];
        double w4 = input_weight[w], w3 = input_weight[graph[cur][i + 1]];
        if ((depth > 1 && !check(w4, w3)) || isValid[v] != head || visit[v]) continue;
        if (depth == 1) w0 = graph[cur][i + 1];

        if (isTail[v] == head) {
            for (int j = 0; j < (int) pathDetail[loc][v].size(); j += 3) {
                int u1 = pathDetail[loc][v][j];
                double w1 = input_weight[pathDetail[loc][v][j + 1]];
                double w2 = input_weight[pathDetail[loc][v][j + 2]];
                double w5 = input_weight[w0];
                if (visit[u1]) continue;
                if (!check(w3, w2) || !check(w1, w5)) continue;

                path[depth] = v;
                path[depth + 1] = u1;
                for (int k = 0; k < depth + 2; ++k) {
                    memcpy((*tr).res[depth - 1] + (*tr).resByteNum[depth - 1], nodeStr[path[k]], nodeStrLen[path[k]]);
                    (*tr).resByteNum[depth - 1] += nodeStrLen[path[k]];
                    (*tr).circleByteNum[head][depth - 1] += nodeStrLen[path[k]];
                    (*tr).res[depth - 1][(*tr).resByteNum[depth - 1]] = (k != depth + 1) ? ',' : '\n';
                    ++(*tr).resByteNum[depth - 1];
                    ++(*tr).circleByteNum[head][depth - 1];
                }
                ++(*tr).resNum[depth - 1];
            }
        }
        if (depth < 6)
            dfs(loc, visit, isValid, isTail, pathDetail, head, v, w0, graph[cur][i + 1], depth + 1, path, tr);
    }

    visit[cur] = false;
}


void threadFunc(int tid, ThreadResult *tr) {
#ifdef TEST
    auto start_time = chrono::system_clock::now();
#endif

    bool *visit = new bool[node_sum];
    memset(visit, 0, node_sum * sizeof(bool));

    int *isValid = new int[node_sum];
    memset(isValid, -1, node_sum * sizeof(int));
    int *isTail = new int[node_sum];
    memset(isTail, -1, node_sum * sizeof(int));

    vector<unordered_map<int, vector<int>>> pathDetail =
            vector<unordered_map<int, vector<int>>>(node_sum / THREAD_COUNT + 1, unordered_map<int, vector<int>>());;

    int path[8];

    for (int i = tid; i < node_sum - 2; i += THREAD_COUNT) {
        if (!graph[i].empty() && !graphIn[i].empty()) {
            dfs_1(visit, isValid, i, i, -1, 1);
            dfs_2(i / THREAD_COUNT, visit, isValid, isTail, pathDetail, i, i, -1, 1);
            dfs(i / THREAD_COUNT, visit, isValid, isTail, pathDetail, i, i, -1, -1, 1, path, tr);
        }
    }

#ifdef TEST
    chrono::duration<double, milli> duration = chrono::system_clock::now() - start_time;
    printf("thread %d use %.3fms\n", tid + 1, duration.count());
#endif
}


void work() {
#ifdef TEST
    auto start = chrono::system_clock::now();
#endif

    threadRes[0].res = res1;
    int (*circleByteNum1)[6] = new int[node_sum][6];
    fill(circleByteNum1[0], circleByteNum1[0] + node_sum * 6, 0);
    threadRes[0].circleByteNum = circleByteNum1;
    threadRes[0].resByteNum = resByteNum1;
    threadRes[0].resNum = resNum1;

    threadRes[1].res = res2;
    int (*circleByteNum2)[6] = new int[node_sum][6];
    fill(circleByteNum2[0], circleByteNum2[0] + node_sum * 6, 0);
    threadRes[1].circleByteNum = circleByteNum2;
    threadRes[1].resByteNum = resByteNum2;
    threadRes[1].resNum = resNum2;

    threadRes[2].res = res3;
    int (*circleByteNum3)[6] = new int[node_sum][6];
    fill(circleByteNum3[0], circleByteNum3[0] + node_sum * 6, 0);
    threadRes[2].circleByteNum = circleByteNum3;
    threadRes[2].resByteNum = resByteNum3;
    threadRes[2].resNum = resNum3;

    threadRes[3].res = res4;
    int (*circleByteNum4)[6] = new int[node_sum][6];
    fill(circleByteNum4[0], circleByteNum4[0] + node_sum * 6, 0);
    threadRes[3].circleByteNum = circleByteNum4;
    threadRes[3].resByteNum = resByteNum4;
    threadRes[3].resNum = resNum4;

    vector<thread> td(THREAD_COUNT);
    for (int i = 0; i < THREAD_COUNT; ++i) {
        td[i] = thread(&threadFunc, i, &threadRes[i]);
    }
    for (auto &t : td) {
        t.join();
    }

#ifdef TEST
    chrono::duration<double, milli> duration = chrono::system_clock::now() - start;
    printf("dfs use:  %.3fms\n", duration.count());
#endif

#ifdef TEST
    start = chrono::system_clock::now();
#endif

    for (int i = 0; i < THREAD_COUNT; ++i) {
        for (int j = 0; j < 6; ++j) {
            circle_sum += threadRes[i].resNum[j];
        }
    }

    int loc1 = 0, loc2[THREAD_COUNT];
    for (int i = 0; i < 6; ++i) {
        loc1 = 0;
        fill(loc2, loc2 + THREAD_COUNT, 0);

        for (int j = 0; j < node_sum; ++j) {
            int resByte = threadRes[j % THREAD_COUNT].circleByteNum[j][i];
            memcpy(ans[i] + loc1, threadRes[j % THREAD_COUNT].res[i] + loc2[j % THREAD_COUNT], resByte);
            loc1 += resByte;
            loc2[j % THREAD_COUNT] += resByte;
        }
        ansByteNum[i] = loc1;
    }

#ifdef TEST
    duration = chrono::system_clock::now() - start;
    printf("merge answer use:  %.3fms\n", duration.count());
#endif
}


void save(const string &file_name) {
#ifdef TEST
    int bytes = 0;
    auto start = chrono::system_clock::now();
#endif

    FILE *fp = fopen(file_name.c_str(), "w");
    char sumStr[9];
    int length = toCharArray(circle_sum, sumStr);
    sumStr[length++] = '\n';
    fwrite(sumStr, length, 1, fp);

    for (int i = 0; i < 6; ++i) {
#ifdef TEST
        bytes += ansByteNum[i];
        printf("circle %d bytes: %d\n", i + 3, ansByteNum[i]);
#endif

        fwrite(ans[i], ansByteNum[i], 1, fp);
    }
    fclose(fp);

#ifdef TEST
    chrono::duration<double, std::milli> duration = chrono::system_clock::now() - start;
    printf("write file use: %.3fms\n", duration.count());
    printf("circle_sum: %d\n", circle_sum);
    printf("bytes: %d\n", bytes + length);
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

    createGraph(file_name);
    work();
    save(save_file_name);

    exit(0);
}
