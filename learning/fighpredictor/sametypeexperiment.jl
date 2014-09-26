using DataFrames
using LIBSVM

df = readtable("../data/sametype.csv", header = false, separator = ',')
# Count the number of active units.
#info("$(df[(df[:, :x1] > 0)])")
# Compute the ratio of parties.
labels = df[:x61]
instances = array(df[:, 1:60])'
model = svmtrain(labels[1:2:end], instances[:, 1:2:end])
(predicted_labels, decision_values) = svmpredict(model, instances[:, 2:2:end])
@printf "Accuracy: %.2f%%\n" mean((predicted_labels .== labels[2:2:end]))*100



