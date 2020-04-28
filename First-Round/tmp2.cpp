#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <vector>
#include <unordered_map>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

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


int nodeSum;
vector<vector<int>> graph;
vector<int> nodes;
unordered_map<int, int> idMapping;
vector<bool> visit;
vector<Path> res;

const int DATA_LEN = 1024*1024*200;
char* wout = new char[DATA_LEN];

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

    nodeSum = nodes.size();
    for (i = 0; i < nodeSum; ++i) {
        idMapping[nodes[i]] = i;
    }

    graph = vector<vector<int>>(nodeSum);
    int size = input.size();
    for (i = 0; i < size; i += 2) {
        int u = idMapping[input[i]];
        int v = idMapping[input[i + 1]];
        graph[u].push_back(v);
    }
}

char* Int2String(int num, char* writestr) {
    int i = 0;
    do {
        writestr[i++] = num%10+48;
        num /= 10;
    }while(num);
    writestr[i] = '\0';
    int j = 0;
    for(;j<i/2;j++) {
        writestr[j] = writestr[j] + writestr[i-1-j];
        writestr[i-1-j] = writestr[j] - writestr[i-1-j];
        writestr[j] = writestr[j] - writestr[i-1-j];
    }
    //  cout << &writestr[0] << endl;
    return writestr;
}

void readData(const string &file_name) {
    int fd = open(file_name.c_str(), O_RDONLY);
    long file_size = lseek(fd, 0, SEEK_END);
    char *buffer = (char *) mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    createGraph(buffer);

    munmap(buffer, file_size);
}


void dfs(int head, int cur, int depth, vector<int> &path) {
    visit[cur] = true;
    path.push_back(nodes[cur]);

    for (int v : graph[cur]) {
        if (v == head && depth >= 3 && depth <= 7) {
            res.emplace_back(Path(depth, path));
        }
        if (depth < 7 && !visit[v] && v > head) {
            dfs(head, v, depth + 1, path);
        }
    }

    visit[cur] = false;
    path.pop_back();
}


void work(const string &file_name) {
    //  memset(wout, '\0', DATA_LEN);
    visit = vector<bool>(nodeSum, false);
    vector<int> path;
    int count = 0;
    for (int i = 0; i < nodeSum; ++i) {
        if (!graph[i].empty())
            dfs(i, i, 1, path);
    }
    sort(res.begin(), res.end());

    //  ofstream out(file_name);
    //  out << res.size() << endl;
    //  char* writestr = new char[10];
    char writestr[10] = {0};
    int istr = 0;
    Int2String(res.size(), writestr);
    for (count = 0;writestr[count]!='\0';count++) {
        wout[count] = writestr[count];
    }
    wout[count++] = '\n';
    for (auto item : res) {
        auto path = item.path;
        int size = path.size();
        //  out << path[0];
        Int2String(path[0], writestr);
        for (istr=0;writestr[istr]!='\0';istr++) {
            // cout << count << endl;
            wout[count++] = writestr[istr];
        }
        for (int i = 1; i < size; ++i) {
            //  out << "," << path[i];
            wout[count++] = ',';
            Int2String(path[i], writestr);
            for (istr=0;writestr[istr]!='\0';istr++) {
                wout[count++] = writestr[istr];
            }
        }
        //    out << '\n';
        wout[count++] = '\n';

    }
    wout[count++]='\0';
    //cout << wout <<endl;
    int wfd = open(file_name.c_str(), O_RDWR | O_CREAT );
    if (wfd<0)
    {
        printf("open error!\n");
        return;
    }
    lseek(wfd, count-2, SEEK_SET);
    write(wfd, "", 1);
    void* p = mmap(NULL, count-1, PROT_WRITE, MAP_SHARED, wfd, 0);
    if (MAP_FAILED==p)
    {
        perror("mmap");
        return;
    }
    close(wfd);
    wfd = -1;
    //madvise(p, DATA_LEN, MADV_SEQUENTIAL|MADV_WILLNEED); //在写入的时候，不用madvise反而更快
    memcpy(p, wout, count-1);
    if (-1==munmap(p, count-1))
    {
        perror("munmap");
    }
    p = NULL;
}


int main(int argc, char *argv[]) {
    string file = argc > 1 ? argv[1] : "test";
    string file_name = "datasets/" + file + "/test_data.txt";
    string save_file_name = "result.txt";

    readData(file_name);
    work(save_file_name);

    exit(0);
}
