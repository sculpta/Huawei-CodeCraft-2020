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


struct Path {
    int u2, u1, w3, w1;

    Path(int u2, int u1, int w3, int w1) : u2(u2), u1(u1), w3(w3), w1(w1) {}

    bool operator<(const Path &item) {
        return u2 != item.u2 ? u2 < item.u2 : u1 < item.u1;
    }
};


const int CHAR_MAX_SIZE = 10;
const int DETAIL_SOURCE_MAX = 20000;
const int DETAIL_MAX = 100;


int node_sum, circle_sum;
vector<vector<int>> graph, graphIn;
char (*nodeStr)[CHAR_MAX_SIZE];
int *nodeStrLen, *isValid, *isTail, *source;
Path (*pathDetail)[DETAIL_MAX];

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


void quickGraphSort(vector<int> &vec, int left, int right) {
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

        quickGraphSort(vec, left, l - 2);
        quickGraphSort(vec, r + 2, right);
    }
}


void createGraph(const char *buffer) {
    vector<int> input, input_weight;
    int data[3] = {0, 0, 0};
    int c, flag = 0;

    for (int i = 0; buffer[i];) {
        c = buffer[i] - '0';
        if (buffer[i] == '\n') {
            if (data[2] > 0) {
                input.push_back(data[0]);
                input.push_back(data[1]);
                input_weight.push_back(data[2]);
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
    nodeStr = new char[node_sum][CHAR_MAX_SIZE];
    nodeStrLen = new int[node_sum];
    fill(nodeStrLen, nodeStrLen + node_sum, 0);
    for (int i = 0; i < node_sum; ++i) {
        int node = temp[i];
        idMapping[node] = i;
        nodeStrLen[i] = toCharArray(node, nodeStr[i]);
    }

    graph = vector<vector<int>>(node_sum), graphIn = vector<vector<int>>(node_sum);

    for (int i = 0; i < input_sum; ++i) {
        int u = idMapping[input[i << 1]];
        int v = idMapping[input[i << 1 | 1]];
        graph[u].push_back(v);
        graph[u].push_back(input_weight[i]);
        graphIn[v].push_back(u);
        graphIn[v].push_back(input_weight[i]);
    }

    for (int i = 0; i < node_sum; ++i) {
        quickGraphSort(graph[i], 0, graph[i].size() - 2);
        quickGraphSort(graphIn[i], 0, graphIn[i].size() - 2);
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
    return x <= 5ll * y && y <= 3ll * x;
}


void dfs_1(bool *visit, int head, int cur, int w, int depth) {
    visit[cur] = true;

    switch (depth) {
        case 1:
            for (int i = 0; i < (int) graph[cur].size(); i += 2) {
                int v = graph[cur][i], w1 = graph[cur][i + 1];
                if (v < head || visit[v]) continue;
                dfs_1(visit, head, v, w1, depth + 1);
            }
            break;
        case 2:
            for (int i = 0; i < (int) graph[cur].size(); i += 2) {
                int v = graph[cur][i], w1 = graph[cur][i + 1];
                if (v < head || visit[v] || !check(w, w1)) continue;
                isValid[cur] = head;
                dfs_1(visit, head, v, w1, depth + 1);
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


void dfs_2(bool *visit, int head, int father, int cur, int w1, int w, int depth) {
    visit[cur] = true;

    switch (depth) {
        case 1:
            for (int i = 0; i < (int) graphIn[cur].size(); i += 2) {
                int u = graphIn[cur][i], w4 = graphIn[cur][i + 1];
                if (u < head || visit[u]) continue;
                dfs_2(visit, head, cur, u, w4, w4, depth + 1);
            }
            break;
        case 2:
            for (int i = 0; i < (int) graphIn[cur].size(); i += 2) {
                int u = graphIn[cur][i], w4 = graphIn[cur][i + 1];
                if (u < head || visit[u] || !check(w1, w)) continue;
                isValid[cur] = head;
                dfs_2(visit, head, cur, u, w1, w4, depth + 1);
            }
            break;
        case 3:
            for (int i = 0; i < (int) graphIn[cur].size(); i += 2) {
                int u = graphIn[cur][i], w4 = graphIn[cur][i + 1];
                if (u < head) continue;
                if ((u == head || !visit[u]) && check(w4, w)) {
                    isValid[cur] = head;
                    isValid[u] = head;
                    isTail[u] = head;
                    pathDetail[head][u].emplace_back(Path(cur, father, w4, w1));
                }
            }
            break;
    }

    visit[cur] = false;
}


void dfs(bool *visit, int head, int cur, int w0, int w, int depth, int *path) {
    visit[cur] = true;
    path[depth - 1] = cur;

    for (int i = 0; i < (int) graph[cur].size(); i += 2) {
        int v = graph[cur][i], w4 = graph[cur][i + 1];
        if ((depth > 1 && !check(w, w4)) || isValid[v] != head || visit[v]) continue;
        if (depth == 1) w0 = w4;

        if (isTail[v] == head) {
            for (int j = 0; j < (int) pathDetail[head][v].size(); ++j) {
                auto detail = pathDetail[head][v][j];
                int u2 = detail.u2, u1 = detail.u1, w3 = detail.w3, w1 = detail.w1;
                if (visit[u1] || visit[u2]) continue;
                if (!check(w4, w3) || !check(w1, w0)) continue;
                if (depth == 4 && !check(w, w4)) continue;

                path[depth] = v;
                path[depth + 1] = u2;
                path[depth + 2] = u1;
                for (int j = 0; j < depth + 3; ++j) {
                    memcpy(res[depth] + resByteNum[depth], nodeStr[path[j]], nodeStrLen[path[j]]);
                    resByteNum[depth] += nodeStrLen[path[j]];
                    res[depth][resByteNum[depth]] = (j != depth + 2) ? ',' : '\n';
                    ++resByteNum[depth];
                }
                ++circle_sum;
#ifdef TEST
                ++resNum[depth];
#endif
            }
        }
        if (depth < 4)
            dfs(visit, head, v, w0, w4, depth + 1, path);
    }

    visit[cur] = false;
}


void work() {
#ifdef TEST
    auto start = chrono::system_clock::now();
#endif

    isValid = new int[node_sum];
    fill(isValid, isValid + node_sum, -1);
    isTail = new int[node_sum];
    fill(isTail, isTail + node_sum, -1);
    pathDetail = vector<unordered_map<int, vector<Path>>>(node_sum, unordered_map<int, vector<Path>>());

    bool visit[node_sum];
    fill(visit, visit + node_sum, false);
    int path[7];

    for (int i = 0; i < node_sum - 2; ++i) {
        if (!graph[i].empty() && !graphIn[i].empty()) {
            dfs_1(visit, i, i, -1, 1);
            dfs_2(visit, i, -1, i, -1, -1, 1);

//            for (auto map : pathDetail[i]) {
//                sort(map.second.begin(), map.second.end());
//            }

            if (isTail[i] == i) {
                for (int j = 0; j < (int) pathDetail[i][i].size(); ++j) {
                    auto detail = pathDetail[i][i][j];
                    int u2 = detail.u2, u1 = detail.u1, w3 = detail.w3, w1 = detail.w1;
                    if (!check(w1, w3)) continue;

                    path[0] = i;
                    path[1] = u2;
                    path[2] = u1;
                    for (int j = 0; j < 3; ++j) {
                        memcpy(res[0] + resByteNum[0], nodeStr[path[j]], nodeStrLen[path[j]]);
                        resByteNum[0] += nodeStrLen[path[j]];
                        res[0][resByteNum[0]] = (j != 2) ? ',' : '\n';
                        ++resByteNum[0];
                    }
                    ++circle_sum;
#ifdef TEST
                    ++resNum[0];
#endif
                }
            }

            dfs(visit, i, i, -1, -1, 1, path);
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
    char sumStr[9];
    int length = toCharArray(circle_sum, sumStr);
    sumStr[length++] = '\n';
    fwrite(sumStr, length, 1, fp);

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
    cout << "bytes: " << bytes + length << endl;
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
