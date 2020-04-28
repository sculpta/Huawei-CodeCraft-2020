# coding=utf-8

"""
@author: shenke
@project: Huawei-CodeCraft-2020
@file: temp.py
@date: 2020/3/26
@description: 
"""

import sys
import time
import numpy as np
import multiprocessing as mp
import matplotlib.pyplot as plt


def job(line):
    return np.array(line.strip().split(',')).astype(float)


class Bayes:

    def __init__(self, train_file, test_file, predict_file):
        """
        初始化
        """
        self.train_file = train_file
        self.test_file = test_file
        self.predict_result_file = predict_file

        self.train_data = []
        self.train_label = []
        self.test_data = []
        self.predict = []

    def load_data(self, file_name, label_existed_flag):
        """
        读取txt文件
        """
        with open(file_name) as f:
            with mp.Pool() as p:
                lines = f.readlines()
                res = np.array(p.map(job, lines))
        return (res[:, 0:-1], res[:, -1]) if label_existed_flag else (res, [])

    def load_train_data(self):
        """
        加载训练数据
        """
        self.train_data, self.train_label = self.load_data(self.train_file, True)
        # 特征缩放（标准化）
        self.var = np.var(self.train_data, axis=0)
        self.mean = np.mean(self.train_data, axis=0)
        self.train_data = (self.train_data - self.mean) / self.var

    def load_test_data(self):
        """
        加载测试数据
        """
        self.test_data, _ = self.load_data(self.test_file, False)
        self.test_data = (self.test_data - self.mean) / self.var

    def save_result(self):
        """
        保存预测结果
        """
        f = open(self.predict_result_file, 'w')
        for item in self.predict:
            f.write(str(int(item)) + "\n")
        f.close()

    def normal(self, mean_value, std_value, value):
        return 1 / np.sqrt(2 * np.pi * std_value) * np.exp(-1 * np.power(value - mean_value, 2) / 2 * std_value)

    def compute(self, type, data):
        array = self.train_data[np.where(self.train_label == type)]
        means = np.mean(array, axis=0)
        stds = np.std(array, axis=0)
        res = 1.0
        for i in range(len(data)):
            res *= self.normal(means[i], stds[i], data[i])
        print(res)
        return res

    def test(self):
        """
        测试并保存预测结果
        """
        for data in self.test_data:
            res0 = self.compute(0, data)
            res1 = self.compute(1, data)
            if res1 > res0:
                self.predict.append(1)
            else:
                self.predict.append(0)
        self.save_result()


if __name__ == "__main__":
    start = time.process_time()

    train_file = "./datasets/train_data.txt"
    test_file = "./datasets/test_data.txt"
    answer_file = "./datasets/answer.txt"
    predict_file = "result.txt"

    module = Bayes(train_file, test_file, predict_file)
    module.load_train_data()
    module.load_test_data()
    module.test()

    f_a = open(answer_file, 'r')
    f_p = open(predict_file, 'r')
    a = []
    p = []
    lines = f_a.readlines()
    for line in lines:
        a.append(int(float(line.strip())))
    f_a.close()

    lines = f_p.readlines()
    for line in lines:
        p.append(int(float(line.strip())))
    f_p.close()

    print("answer lines:%d" % (len(a)))
    print("predict lines:%d" % (len(p)))

    errline = 0
    for i in range(len(a)):
        if a[i] != p[i]:
            errline += 1

    accuracy = (len(a) - errline) / len(a)
    print("accuracy:%f" % (accuracy))

    end = time.process_time()
    print("总运行时间: %s s" % (str(end - start)))
