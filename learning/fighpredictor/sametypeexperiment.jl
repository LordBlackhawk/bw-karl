using DataFrames
using LIBSVM
using Gadfly

df = readtable("../data/sametype.csv", header = false, separator = ',')
# Count the number of active units and the ratio of units.
df[:numownunits] = zeros(size(df)[1])
df[:numopponentunits] = zeros(size(df)[1])
df[:unitratio] = zeros(size(df)[1])
for row = eachrow(df)
	row[:numownunits] = sum(map(x -> bool(x[2]), row[1:30]))
	row[:numopponentunits] = sum(map(x -> bool(x[2]), row[31:60]))
	row[:unitratio] = row[:numownunits]/(row[:numownunits] + row[:numopponentunits])
end

# TODO Choose the learning values on random.
# TODO Compute the accuracy separately for different ratios or "buckets" of ratios.
# TODO Include the ratio as feature in learning.
# TODO Could a mixed model be created based on:
#	- total health
#	- individual healths
#	- damage
#	- damage frequency
#	- combination of damage
#	- geography
#	- distribution
#	How to account for weakening of one army by losing troops quickly?
labels = df[:x61]
instances = array(df[:, 1:60])'
model = svmtrain(labels[1:2:end], instances[:, 1:2:end])
(predicted_labels, decision_values) = svmpredict(model, instances[:, 2:2:end])
@printf "Accuracy: %.2f%%\n" mean((predicted_labels .== labels[2:2:end]))*100

plot(df, x=:numownunits, y=:unitratio)


