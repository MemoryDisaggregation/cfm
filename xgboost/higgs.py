import xgboost as xgb
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.metrics import roc_auc_score
# import numpy as np
import time
import os


data = pd.read_csv('/users/YuqiLi/HIGGS.csv', header=None)
X = data.iloc[:, 1:]
y = data.iloc[:, 0]

X = X.astype('float64')

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
X_train, X_val, y_train, y_val = train_test_split(X_train, y_train, test_size=0.25, random_state=42)


dtrain = xgb.DMatrix(X_train, label=y_train)
dval = xgb.DMatrix(X_val, label=y_val)
dtest = xgb.DMatrix(X_test, label=y_test)


param = {'max_depth': 9, 'eta': 0.03, 'objective': 'binary:logistic', 'nthread': 4}
num_round = 1000  


#bst = xgb.train(param, dtrain, num_round, [(dval, 'eval')], early_stopping_rounds=50)
bst = xgb.train(param, dtrain, num_round)

preds = bst.predict(dtest)

print('AUC: ', roc_auc_score(y_test, preds))
