using DataFrames
using LIBSVM
using Gadfly

df = readtable("../data/sametype.csv", header = false, separator = ',')
# Count the number of active units and the ratio of units.
df[:numownunits] = zeros(size(df)[1])
df[:numopponentunits] = zeros(size(df)[1])
df[:unitratio] = zeros(size(df)[1])
df[:unitratiodisc] = zeros(size(df)[1])
# Discretization period.
t_ratio = 0.2
for row = eachrow(df)
	row[:numownunits] = sum(map(x -> bool(x[2]), row[1:30]))
	row[:numopponentunits] = sum(map(x -> bool(x[2]), row[31:60]))
	row[:unitratio] = row[:numownunits]/(row[:numownunits] + row[:numopponentunits])
	# Discretize the ratios
	row[:unitratio] == 1.0 ? row[:unitratiodisc] = 1/0.25 : row[:unitratiodisc] = ifloor(row[:unitratio]/t_ratio) + 1
end

# TODO Choose the learning values at random.
# TODO Standard quality metrics for binary classifiers.
# TODO Include the ratio as feature in learning.
# TODO Could a mixed model be created based on:
#	- total health,
#	- individual healths,
#	- damage,
#	- damage frequency,
#	- combination of damage,
#	- geography,
#	- distribution?
#	How to account for weakening of one army by losing troops quickly?

# To each ratio its own accuracy.
classifier_quality = Dict()
classifier_quality["accuracy"] = zeros(int(1/t_ratio), 1)
for r = 1:int(1/t_ratio)
	labels = df[df[:unitratiodisc] .== r, :x61]
	instances = array(df[df[:unitratiodisc] .== r, 1:60])'
	model = svmtrain(labels[1:2:end], instances[:, 1:2:end])
	(predicted_labels, decision_values) = svmpredict(model, instances[:, 2:2:end])
	classifier_quality["accuracy"][r] = mean((predicted_labels .== labels[2:2:end]))
	@printf "Accuracy: %.2f%%\n" classifier_quality["accuracy"][r]
end

# Plot the classification quality for varios ratios of units.
x_vals = map(r -> (r - 1)*t_ratio + 0.5*t_ratio, 1:int(1/t_ratio))
plot(x=x_vals , y=classifier_quality["accuracy"], Scale.y_continuous(minvalue=0.0, maxvalue=1.0), Scale.x_continuous(minvalue=0.0, maxvalue=1.0), Guide.xticks(ticks=x_vals), Guide.yticks(ticks=[0.0:0.1:1.0]), Guide.xlabel("Discretized Unit Ratios"), Guide.ylabel("Accuracy"), Guide.title("SVM - Fighpredictor for Same-type Units"))

