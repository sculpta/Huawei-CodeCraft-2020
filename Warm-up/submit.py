# coding=utf-8

"""
@author: shenke
@project: HuaweiCompetition
@file: PyCharm.py
@date: 2020/3/11
@description: 
"""

import sys
import time
import numpy as np
import multiprocessing as mp


def job(line):
    return np.array(line.strip().split(',')).astype(float)


class LR:

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
        self.weight = []
        self.biase = []

    def load_data(self, file_name, label_existed_flag, max_read_num=-1):
        """
        读取txt文件
        """
        if (max_read_num > 0):
            with open(file_name) as f:
                with mp.Pool() as p:
                    lines = []
                    line = f.readline()
                    while line:
                        if max_read_num <= 0:
                            break
                        else:
                            max_read_num -= 1
                        lines.append(line)
                        line = f.readline()
                    res = np.array(p.map(job, lines))
        else:
            with open(file_name) as f:
                with mp.Pool() as p:
                    lines = f.readlines()
                    res = np.array(p.map(job, lines))

        return (res[:, 0:-1], res[:, -1]) if label_existed_flag else (res, [])

    def load_train_data(self):
        """
        加载训练数据
        """
        self.train_data, self.train_label = self.load_data(self.train_file, True, 1500)
        # 特征缩放（标准化）
        self.var = np.var(self.train_data, axis=0)
        self.mean = np.mean(self.train_data, axis=0)
        self.train_data = (self.train_data - self.mean) / self.var

        self.train_data = self.train_data.T
        self.train_label = np.reshape(self.train_label, (1, len(self.train_label)))

    def load_test_data(self):
        """
        加载测试数据
        """
        self.test_data, _ = self.load_data(self.test_file, False)

    def save_result(self):
        """
        保存预测结果
        """
        f = open(self.predict_result_file, 'w')
        for item in self.predict:
            f.write(str(int(item)) + "\n")
        f.close()

    def print_info(self):
        """
        打印结果
        """
        print(self.train_file)
        print(self.predict_file)
        print(self.predict_result_file)
        print(self.train_data)
        print(self.train_label)
        print(self.test_data)
        print(self.predict)

    def init_params(self, in_size, out_size):
        """
        初始化权重、偏置
        """
        weight = np.random.normal(loc=1, scale=0.5, size=(in_size, out_size)) * 0.01
        biase = np.random.normal(loc=0, scale=0.1, size=(in_size, 1)) * 0.01
        return weight, biase

    def init_layer(self, neuron_num, param_num):
        """
        初始化隐藏层
        """
        w1, b1 = self.init_params(neuron_num, param_num)
        w2, b2 = self.init_params(1, neuron_num)
        w = {'w1': w1, 'w2': w2}
        b = {'b1': b1, 'b2': b2}
        return w, b

    def sigmoid(self, x):
        """
        sigmoid激活函数
        """
        return 1.0 / (1 + np.exp(-x))

    def loss(self, pred, label):
        """
        损失函数
        """
        return -1 / pred.shape[1] * (label * np.log(pred) + (1 - label) * np.log(1 - pred)).sum()

    def accuracy(self, pred, label):
        """
        计算预测准确率
        """
        return np.sum(pred == label) / pred.shape[1] * 100

    def compute(self, input, weight, biase, activate_function=None):
        """
        计算神经元输出结果
        """
        res = np.dot(weight, input) + biase
        return activate_function(res) if activate_function else res

    def compute_output(self, w, b, data, label=None, calculate_cost_flag=True):
        """
        计算输出层结果
        """
        w1 = w['w1']
        b1 = b['b1']
        w2 = w['w2']
        b2 = b['b2']

        a1 = self.compute(data, w1, b1, np.tanh)
        a2 = self.compute(a1, w2, b2, self.sigmoid)

        if calculate_cost_flag:
            cost = self.loss(a2, label)
            cache = {'a1': a1, 'a2': a2}
            return cache, cost
        else:
            return a2

    def gradient(self, cache, w, b, data, label, alpha):
        """
        计算梯度
        """
        w1 = w['w1']
        w2 = w['w2']
        a1 = cache['a1']
        a2 = cache['a2']
        m = a2.shape[1]

        dz2 = a2 - label
        dw2 = 1 / m * np.dot(dz2, a1.T) + alpha / m * w2
        db2 = 1 / m * np.sum(dz2, keepdims=True)

        dz1 = np.dot(w2.T, dz2) * (1 - np.power(a1, 2))
        dw1 = 1 / m * np.dot(dz1, data.T) + alpha / m * w1
        db1 = 1 / m * np.sum(dz1, keepdims=True)

        grads = {"dw2": dw2,
                 "db2": db2,
                 "dw1": dw1,
                 'db1': db1}
        return grads

    def optimizer(self, cache, w, b, data, label, learning_rate, alpha):
        """
        优化器
        """
        grads = self.gradient(cache, w, b, data, label, alpha)

        w['w1'] -= learning_rate * grads["dw1"]
        b['b1'] -= learning_rate * grads["db1"]
        w['w2'] -= learning_rate * grads["dw2"]
        b['b2'] -= learning_rate * grads["db2"]
        return w, b

    def train(self):
        """
        训练
        """
        self.load_train_data()
        param_num, record_num = self.train_data.shape
        neuron_num = 2
        iterations = 5
        learning_rate = 0.7
        alpha = 100
        batch_size = 500
        batch_sum = int(record_num / batch_size)

        w, b = self.init_layer(neuron_num, param_num)
        data_batch = np.hsplit(self.train_data, batch_sum)
        label_batch = np.hsplit(self.train_label, batch_sum)

        for i in range(iterations):
            for j in range(batch_sum):
                data = data_batch[j]
                label = label_batch[j]
                cache, cost = self.compute_output(w, b, data, label, True)
                w, b = self.optimizer(cache, w, b, data, label, learning_rate, alpha)

            print('iteration: %i, cost: %f' % (i, cost))
            pred = (cache['a2'] + 0.5).astype(np.int)
            print('train accuracy: %.2f' % self.accuracy(pred, label))

        self.weight, self.biase = w, b

    def test(self):
        """
        测试并保存预测结果
        """
        self.load_test_data()
        pred = self.compute_output(self.weight, self.biase, ((self.test_data - self.mean) / self.var).T,
                                   calculate_cost_flag=False)
        pred = pred.T
        self.predict = (pred + 0.5).astype(int)
        self.save_result()


def print_help_and_exit():
    print("usage:python3 main.py train_data.txt test_data.txt predict.txt [debug]")
    sys.exit(-1)


def parse_args():
    debug = False
    if len(sys.argv) == 2:
        if sys.argv[1] == 'debug':
            print("test mode")
            debug = True
        else:
            print_help_and_exit()
    return debug


if __name__ == "__main__":
    start = time.process_time()

    debug = parse_args()
    train_file = "/data/train_data.txt"
    test_file = "/data/test_data.txt"
    predict_file = "/projects/student/result.txt"

    module = LR(train_file, test_file, predict_file)
    module.train()
    module.test()

    if debug:
        answer_file = "/projects/student/answer.txt"
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
