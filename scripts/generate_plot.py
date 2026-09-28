import csv

import matplotlib.pyplot as plt

x = []

seq_obj = []
seq_bit = []
seq_sort_arr = []
seq_shuff_arr = []

shuff_obj = []
shuff_bit = []
shuff_sort_arr = []
shuff_shuff_arr = []

with open("../results/bisetBench.csv", "r") as csvfile:
    lines = csv.reader(csvfile, delimiter=",")

    i = 1
    for row in lines:
        x.append(i)
        baseline = float(row[1])
        seq_obj.append(baseline)
        seq_bit.append(float(row[2]))
        seq_sort_arr.append(float(row[3]))
        seq_shuff_arr.append(float(row[4]))

        baseline = float(row[5])
        shuff_obj.append(baseline)
        shuff_bit.append(float(row[6]))
        shuff_sort_arr.append(float(row[7]))
        shuff_shuff_arr.append(float(row[8]))

        i += 1

plt.plot(x, seq_obj, label="Object Flag")
plt.plot(x, seq_bit, label="Bitset")
plt.plot(x, seq_sort_arr, label="Sorted Array")
plt.plot(x, seq_shuff_arr, label="Shuffled Array")

plt.xlabel("Each X's flag is enabled")
plt.ylabel("Percentage of baseline")
plt.title("Sequential Data")
plt.legend()
plt.show()

plt.plot(x, shuff_obj, label="Object Flag")
plt.plot(x, shuff_bit, label="Bitset")
plt.plot(x, shuff_sort_arr, label="Sorted Array")
plt.plot(x, shuff_shuff_arr, label="Shuffled Array")

plt.xlabel("Each X's flag is enabled")
plt.ylabel("Percentage of baseline")
plt.title("Shuffled Data")
plt.legend()
plt.show()
