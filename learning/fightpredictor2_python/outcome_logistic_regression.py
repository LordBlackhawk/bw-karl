#Outcome


#Task: Learning to predict the outcome of fight.
#Situation: two armies fight each other 
#Obstructions: terrain= flat , units= same type , upgrades= no upgrades, number of units= variable, ernergy of units= variable.

#Method: logistic regression, binary classification

#Feature vector= (x_1, ... ,x_30,x_31,...,x_60)    with x_i := energy of the i-th unit in army 0   if i<=30  , energy of the (i-30)-th unit in army 1    if i>30
#Outcome: 0= army 0 wins  , 1= army 1 wins      






#Logistic regression


#Loading packages

import csv
import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.cross_validation import train_test_split


#Reading the data into a matrix

reader= csv.reader(open("../data/sametype.csv","rt"),delimiter=',')
Data= np.matrix(list(reader)).astype('int') 


#Number of feature vectors

length= Data.shape[0]

#Splitting Data into Feature matrix and outcome



X= Data[:,0 :60 ]
Y= Data[:, 60:61 ]

#Cross_validation

X_train, X_test, Y_train, Y_test = train_test_split(X, Y, test_size=0.35, random_state=0)


#Trainig the Logistic Regression

predictor = LogisticRegression()    
predictor.fit(X_train, Y_train)

#Printing the score of performance

print("The performance of your trained predictor with " ,length ," samples is: ",predictor.score(X_test, Y_test))



